/*
 * $XConsortium: charproc.c /main/196 1996/12/03 16:52:46 swick $
 * $XFree86: xc/programs/xterm/charproc.c,v 3.65 1998/07/04 14:48:26 robin Exp $
 */

/*

Copyright (c) 1988  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

*/
/*
 * Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.
 *
 *                         All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Digital Equipment
 * Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 *
 *
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/* charproc.c */

#ifdef HAVE_CONFIG_H
#include <xtermcfg.h>
#endif

#include "ptyx.h"
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/StringDefs.h>
#include <X11/Xmu/Atoms.h>
#include <X11/Xmu/CharSet.h>
#include <X11/Xmu/Converters.h>

#if OPT_INPUT_METHOD
#include <X11/Xaw/XawImP.h>
#endif

#include <stdio.h>
#include <setjmp.h>
#include <ctype.h>

#ifdef MINIX
#include <sys/nbio.h>

#define select(n,r,w,x,t) nbio_select(n,r,w,x,t)
#define read(f,b,s) nbio_read(f,b,s)
#define write(f,b,s) nbio_write(f,b,s)
#endif

#include "VTparse.h"
#include "data.h"
#include "error.h"
#include "menu.h"
#include "main.h"
#include "xterm.h"

#ifndef NO_ACTIVE_ICON
#include <X11/Shell.h>
#endif /* NO_ACTIVE_ICON */

/*
 * Check for both EAGAIN and EWOULDBLOCK, because some supposedly POSIX
 * systems are broken and return EWOULDBLOCK when they should return EAGAIN.
 * Note that this macro may evaluate its argument more than once.
 */
#if defined(EAGAIN) && defined(EWOULDBLOCK)
#define E_TEST(err) ((err) == EAGAIN || (err) == EWOULDBLOCK)
#else
#ifdef EAGAIN
#define E_TEST(err) ((err) == EAGAIN)
#else
#define E_TEST(err) ((err) == EWOULDBLOCK)
#endif
#endif

extern jmp_buf VTend;
extern XtermWidget term;
extern Widget toplevel;
extern char *ProgramName;

static int LoadNewFont (TScreen *screen, char *nfontname, char *bfontname, Bool doresize, int fontnum);
static int in_put (void);
static int set_character_class (char *s);
static void FromAlternate (TScreen *screen);
static void RequestResize (XtermWidget termw, int rows, int cols, int text);
static void SwitchBufs (TScreen *screen);
static void ToAlternate (TScreen *screen);
static void VTallocbuf (void);
static void WriteText (TScreen *screen, Char *str, int len);
static void ansi_modes (XtermWidget termw, void (*func)(unsigned *p, unsigned mask));
static void bitclr (unsigned *p, unsigned mask);
static void bitcpy (unsigned *p, unsigned q, unsigned mask);
static void bitset (unsigned *p, unsigned mask);
static void dpmodes (XtermWidget termw, void (*func)(unsigned *p, unsigned mask));
static void restoremodes (XtermWidget termw);
static void savemodes (XtermWidget termw);
static void set_vt_box (TScreen *screen);
static void unparseputn (unsigned int n, int fd);
static void update_font_info (TScreen *screen, Bool doresize);
static void window_ops (XtermWidget termw);

#if OPT_BLINK_CURS
static void BlinkCursor ( XtPointer closure, XtIntervalId* id);
static void StartBlinking (TScreen *screen);
static void StopBlinking (TScreen *screen);
#else
#define StartBlinking(screen) /* nothing */
#define StopBlinking(screen) /* nothing */
#endif

#define	DEFAULT		-1
#define	TEXT_BUF_SIZE	256
#define TRACKTIMESEC	4L
#define TRACKTIMEUSEC	0L
#define BELLSUPPRESSMSEC 200

#define XtNallowSendEvents	"allowSendEvents"
#define XtNalwaysHighlight	"alwaysHighlight"
#define XtNappcursorDefault	"appcursorDefault"
#define XtNappkeypadDefault	"appkeypadDefault"
#define XtNautoWrap		"autoWrap"
#define XtNawaitInput		"awaitInput"
#define XtNbackarrowKey		"backarrowKey"
#define XtNbellSuppressTime	"bellSuppressTime"
#define XtNboldColors		"boldColors"
#define XtNboldFont		"boldFont"
#define XtNc132			"c132"
#define XtNcharClass		"charClass"
#define XtNcolor0		"color0"
#define XtNcolor1		"color1"
#define XtNcolor10		"color10"
#define XtNcolor11		"color11"
#define XtNcolor12		"color12"
#define XtNcolor13		"color13"
#define XtNcolor14		"color14"
#define XtNcolor15		"color15"
#define XtNcolor2		"color2"
#define XtNcolor3		"color3"
#define XtNcolor4		"color4"
#define XtNcolor5		"color5"
#define XtNcolor6		"color6"
#define XtNcolor7		"color7"
#define XtNcolor8		"color8"
#define XtNcolor9		"color9"
#define XtNcolorAttrMode	"colorAttrMode"
#define XtNcolorBD		"colorBD"
#define XtNcolorBDMode		"colorBDMode"
#define XtNcolorBL		"colorBL"
#define XtNcolorBLMode		"colorBLMode"
#define XtNcolorMode		"colorMode"
#define XtNcolorUL		"colorUL"
#define XtNcolorULMode		"colorULMode"
#define XtNcurses		"curses"
#define XtNcursorBlinkTime	"cursorBlinkTime"
#define XtNcursorColor		"cursorColor"
#define XtNcutNewline		"cutNewline"
#define XtNcutToBeginningOfLine	"cutToBeginningOfLine"
#define XtNdecTerminalID	"decTerminalID"
#define XtNdynamicColors	"dynamicColors"
#define XtNeightBitControl	"eightBitControl"
#define XtNeightBitInput	"eightBitInput"
#define XtNeightBitOutput	"eightBitOutput"
#define XtNhighlightSelection	"highlightSelection"
#define XtNhpLowerleftBugCompat	"hpLowerleftBugCompat"
#define XtNinternalBorder	"internalBorder"
#define XtNjumpScroll		"jumpScroll"
#define XtNloginShell		"loginShell"
#define XtNmarginBell		"marginBell"
#define XtNmultiClickTime	"multiClickTime"
#define XtNmultiScroll		"multiScroll"
#define XtNnMarginBell		"nMarginBell"
#define XtNoldXtermFKeys	"oldXtermFKeys"
#define XtNpointerColor		"pointerColor"
#define XtNpointerColorBackground	"pointerColorBackground"
#define XtNpointerShape		"pointerShape"
#define XtNprintAttributes	"printAttributes"
#define XtNprinterAutoClose	"printerAutoClose"
#define XtNprinterCommand	"printerCommand"
#define XtNprinterControlMode	"printerControlMode"
#define XtNprinterExtent	"printerExtent"
#define XtNprinterFormFeed	"printerFormFeed"
#define XtNresizeGravity	"resizeGravity"
#define XtNreverseWrap		"reverseWrap"
#define XtNrightScrollBar	"rightScrollBar"
#define XtNsaveLines		"saveLines"
#define XtNscrollBar		"scrollBar"
#define XtNscrollKey		"scrollKey"
#define XtNscrollLines		"scrollLines"
#define XtNscrollPos		"scrollPos"
#define XtNscrollTtyOutput	"scrollTtyOutput"
#define XtNsignalInhibit	"signalInhibit"
#define XtNtekGeometry		"tekGeometry"
#define XtNtekInhibit		"tekInhibit"
#define XtNtekSmall		"tekSmall"
#define XtNtekStartup		"tekStartup"
#define XtNtiteInhibit		"titeInhibit"
#define XtNunderLine		"underLine"
#define XtNvisualBell		"visualBell"
#define XtNxmcAttributes	"xmcAttributes"
#define XtNxmcGlitch		"xmcGlitch"
#define XtNxmcInline		"xmcInline"
#define XtNxmcMoveSGR		"xmcMoveSGR"

#ifdef ALLOWLOGGING
#define XtNlogFile		"logFile"
#define XtNlogInhibit		"logInhibit"
#define XtNlogging		"logging"
#endif

#ifdef NO_ACTIVE_ICON
#define XtNgeometry		"geometry"
#endif

#if OPT_HIGHLIGHT_COLOR
#define XtNhighlightColor	"highlightColor"
#endif

#define XtCAllowSendEvents	"AllowSendEvents"
#define XtCAlwaysHighlight	"AlwaysHighlight"
#define XtCAppcursorDefault	"AppcursorDefault"
#define XtCAppkeypadDefault	"AppkeypadDefault"
#define XtCAutoWrap		"AutoWrap"
#define XtCAwaitInput		"AwaitInput"
#define XtCBackarrowKey		"BackarrowKey"
#define XtCBellSuppressTime	"BellSuppressTime"
#define XtCBoldFont		"BoldFont"
#define XtCC132			"C132"
#define XtCCharClass		"CharClass"
#define XtCColorMode		"ColorMode"
#define XtCColumn		"Column"
#define XtCCurses		"Curses"
#define XtCCursorBlinkTime	"CursorBlinkTime"
#define XtCCutNewline		"CutNewline"
#define XtCCutToBeginningOfLine	"CutToBeginningOfLine"
#define XtCDecTerminalID	"DecTerminalID"
#define XtCDynamicColors	"DynamicColors"
#define XtCEightBitControl	"EightBitControl"
#define XtCEightBitInput	"EightBitInput"
#define XtCEightBitOutput	"EightBitOutput"
#define XtCHighlightSelection	"HighlightSelection"
#define XtCHpLowerleftBugCompat	"HpLowerleftBugCompat"
#define XtCJumpScroll		"JumpScroll"
#define XtCLoginShell		"LoginShell"
#define XtCMarginBell		"MarginBell"
#define XtCMultiClickTime	"MultiClickTime"
#define XtCMultiScroll		"MultiScroll"
#define XtCOldXtermFKeys	"OldXtermFKeys"
#define XtCPrintAttributes	"PrintAttributes"
#define XtCPrinterAutoClose	"PrinterAutoClose"
#define XtCPrinterCommand	"PrinterCommand"
#define XtCPrinterControlMode	"PrinterControlMode"
#define XtCPrinterExtent	"PrinterExtent"
#define XtCPrinterFormFeed	"PrinterFormFeed"
#define XtCResizeGravity	"ResizeGravity"
#define XtCReverseWrap		"ReverseWrap"
#define XtCRightScrollBar	"RightScrollBar"
#define XtCSaveLines		"SaveLines"
#define XtCScrollBar		"ScrollBar"
#define XtCScrollCond		"ScrollCond"
#define XtCScrollLines		"ScrollLines"
#define XtCScrollPos		"ScrollPos"
#define XtCSignalInhibit	"SignalInhibit"
#define XtCTekInhibit		"TekInhibit"
#define XtCTekSmall		"TekSmall"
#define XtCTekStartup		"TekStartup"
#define XtCTiteInhibit		"TiteInhibit"
#define XtCUnderLine		"UnderLine"
#define XtCVisualBell		"VisualBell"
#define XtCXmcAttributes	"XmcAttributes"
#define XtCXmcGlitch		"XmcGlitch"
#define XtCXmcInline		"XmcInline"
#define XtCXmcMoveSGR		"XmcMoveSGR"

#ifdef ALLOWLOGGING
#define XtCLogInhibit		"LogInhibit"
#define XtCLogfile		"Logfile"
#define XtCLogging		"Logging"
#endif

#ifdef NO_ACTIVE_ICON
#define XtCGeometry		"Geometry"
#endif

#define	doinput()		(bcnt-- > 0 ? *bptr++ : in_put())

static int nparam;
static ANSI reply;
static int param[NPARAM];

#ifdef UNUSED
static unsigned long ctotal;
static unsigned long ntotal;
#endif

static jmp_buf vtjmpbuf;

/* event handlers */
static void HandleBell PROTO_XT_ACTIONS_ARGS;
static void HandleIgnore PROTO_XT_ACTIONS_ARGS;
static void HandleKeymapChange PROTO_XT_ACTIONS_ARGS;
static void HandleSetFont PROTO_XT_ACTIONS_ARGS;
static void HandleVisualBell PROTO_XT_ACTIONS_ARGS;
#if OPT_ZICONBEEP
static void HandleMapUnmap PROTO_XT_EV_HANDLER_ARGS;
#endif

/*
 * NOTE: VTInitialize zeros out the entire ".screen" component of the
 * XtermWidget, so make sure to add an assignment statement in VTInitialize()
 * for each new ".screen" field added to this resource list.
 */

/* Defaults */
static  Boolean	defaultCOLORMODE   = DFT_COLORMODE;
static  Boolean	defaultFALSE	   = FALSE;
static  Boolean	defaultTRUE	   = TRUE;
static  int	defaultZERO        = 0;
static  int	defaultIntBorder   = DEFBORDER;
static  int	defaultSaveLines   = SAVELINES;
static	int	defaultScrollLines = SCROLLLINES;
static  int	defaultNMarginBell = N_MARGINBELL;
static  int	defaultMultiClickTime = MULTICLICKTIME;
static  int	defaultBellSuppressTime = BELLSUPPRESSMSEC;
static	int	default_DECID = DFT_DECID;
static	char *	_Font_Selected_ = "yes";  /* string is arbitrary */

#if OPT_BLINK_CURS
static  int	defaultBlinkTime = 0;
#endif

#if OPT_PRINT_COLORS
static  int	defaultONE = 1;
#endif

/*
 * Warning, the following must be kept under 1024 bytes or else some
 * compilers (particularly AT&T 6386 SVR3.2) will barf).  Workaround is to
 * declare a static buffer and copy in at run time (the the Athena text widget
 * does).  Yuck.
 */
static char defaultTranslations[] =
"\
          Shift <KeyPress> Prior:scroll-back(1,halfpage) \n\
           Shift <KeyPress> Next:scroll-forw(1,halfpage) \n\
         Shift <KeyPress> Select:select-cursor-start() select-cursor-end(PRIMARY , CUT_BUFFER0) \n\
         Shift <KeyPress> Insert:insert-selection(PRIMARY, CUT_BUFFER0) \n\
                ~Meta <KeyPress>:insert-seven-bit() \n\
                 Meta <KeyPress>:insert-eight-bit() \n\
                !Ctrl <Btn1Down>:popup-menu(mainMenu) \n\
           !Lock Ctrl <Btn1Down>:popup-menu(mainMenu) \n\
 !Lock Ctrl @Num_Lock <Btn1Down>:popup-menu(mainMenu) \n\
     ! @Num_Lock Ctrl <Btn1Down>:popup-menu(mainMenu) \n\
                ~Meta <Btn1Down>:select-start() \n\
              ~Meta <Btn1Motion>:select-extend() \n\
                !Ctrl <Btn2Down>:popup-menu(vtMenu) \n\
           !Lock Ctrl <Btn2Down>:popup-menu(vtMenu) \n\
 !Lock Ctrl @Num_Lock <Btn2Down>:popup-menu(vtMenu) \n\
     ! @Num_Lock Ctrl <Btn2Down>:popup-menu(vtMenu) \n\
          ~Ctrl ~Meta <Btn2Down>:ignore() \n\
            ~Ctrl ~Meta <Btn2Up>:insert-selection(PRIMARY, CUT_BUFFER0) \n\
                !Ctrl <Btn3Down>:popup-menu(fontMenu) \n\
           !Lock Ctrl <Btn3Down>:popup-menu(fontMenu) \n\
 !Lock Ctrl @Num_Lock <Btn3Down>:popup-menu(fontMenu) \n\
     ! @Num_Lock Ctrl <Btn3Down>:popup-menu(fontMenu) \n\
          ~Ctrl ~Meta <Btn3Down>:start-extend() \n\
              ~Meta <Btn3Motion>:select-extend()      \n\
                         <BtnUp>:select-end(PRIMARY, CUT_BUFFER0) \n\
                       <BtnDown>:bell(0) \
";

static XtActionsRec actionsList[] = {
    { "bell",			HandleBell },
    { "create-menu",		HandleCreateMenu },
    { "ignore", 		HandleIgnore },
    { "insert", 		HandleKeyPressed },  /* alias for insert-seven-bit */
    { "insert-seven-bit",	HandleKeyPressed },
    { "insert-eight-bit",	HandleEightBitKeyPressed },
    { "insert-selection",	HandleInsertSelection },
    { "keymap", 		HandleKeymapChange },
    { "popup-menu",		HandlePopupMenu },
    { "secure", 		HandleSecure },
    { "select-start",		HandleSelectStart },
    { "select-extend",		HandleSelectExtend },
    { "select-end",		HandleSelectEnd },
    { "select-set",		HandleSelectSet },
    { "select-cursor-start",	HandleKeyboardSelectStart },
    { "select-cursor-end",	HandleKeyboardSelectEnd },
    { "set-vt-font",		HandleSetFont },
    { "start-extend",		HandleStartExtend },
    { "start-cursor-extend",	HandleKeyboardStartExtend },
    { "string", 		HandleStringEvent },
    { "scroll-forw",		HandleScrollForward },
    { "scroll-back",		HandleScrollBack },
    /* menu actions */
    { "allow-send-events",	HandleAllowSends },
    { "set-visual-bell",	HandleSetVisualBell },
#ifdef ALLOWLOGGING
    { "set-logging",		HandleLogging },
#endif
    { "print", 			HandlePrint },
    { "redraw", 		HandleRedraw },
    { "send-signal",		HandleSendSignal },
    { "quit",			HandleQuit },
    { "set-8-bit-control",	Handle8BitControl },
    { "set-backarrow",		HandleBackarrow },
    { "set-sun-function-keys",	HandleSunFunctionKeys },
    { "set-scrollbar",		HandleScrollbar },
    { "set-jumpscroll", 	HandleJumpscroll },
    { "set-reverse-video",	HandleReverseVideo },
    { "set-autowrap",		HandleAutoWrap },
    { "set-reversewrap",	HandleReverseWrap },
    { "set-autolinefeed",	HandleAutoLineFeed },
    { "set-appcursor",		HandleAppCursor },
    { "set-appkeypad",		HandleAppKeypad },
    { "set-scroll-on-key",	HandleScrollKey },
    { "set-scroll-on-tty-output", HandleScrollTtyOutput },
    { "set-allow132",		HandleAllow132 },
    { "set-cursesemul", 	HandleCursesEmul },
    { "set-marginbell", 	HandleMarginBell },
    { "set-altscreen",		HandleAltScreen },
    { "soft-reset",		HandleSoftReset },
    { "hard-reset",		HandleHardReset },
    { "clear-saved-lines",	HandleClearSavedLines },
#if OPT_TEK4014
    { "set-terminal-type",	HandleSetTerminalType },
    { "set-visibility", 	HandleVisibility },
    { "set-tek-text",		HandleSetTekText },
    { "tek-page",		HandleTekPage },
    { "tek-reset",		HandleTekReset },
    { "tek-copy",		HandleTekCopy },
#endif
    { "visual-bell",		HandleVisualBell },
    { "dired-button",		DiredButton },
    { "vi-button",		ViButton },
};

static XtResource resources[] = {
{XtNfont, XtCFont, XtRString, sizeof(char *),
	XtOffsetOf(XtermWidgetRec, misc.f_n), XtRString,
	DEFFONT},
{XtNboldFont, XtCBoldFont, XtRString, sizeof(char *),
	XtOffsetOf(XtermWidgetRec, misc.f_b), XtRString,
	DEFBOLDFONT},
{XtNc132, XtCC132, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.c132),
	XtRBoolean, (XtPointer) &defaultFALSE},
{XtNcharClass, XtCCharClass, XtRString, sizeof(char *),
	XtOffsetOf(XtermWidgetRec, screen.charClass),
	XtRString, (XtPointer) NULL},
{XtNcurses, XtCCurses, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.curses),
	XtRBoolean, (XtPointer) &defaultFALSE},
{XtNhpLowerleftBugCompat, XtCHpLowerleftBugCompat, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.hp_ll_bc),
	XtRBoolean, (XtPointer) &defaultFALSE},
#if OPT_XMC_GLITCH
{XtNxmcGlitch, XtCXmcGlitch, XtRInt, sizeof(int),
	XtOffsetOf(XtermWidgetRec, screen.xmc_glitch),
        XtRString, "0"},
{XtNxmcAttributes, XtCXmcAttributes, XtRInt, sizeof(int),
	XtOffsetOf(XtermWidgetRec, screen.xmc_attributes),
        XtRString, "1"},
{XtNxmcInline, XtCXmcInline, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.xmc_inline),
	XtRBoolean, (XtPointer) &defaultFALSE},
{XtNxmcMoveSGR, XtCXmcMoveSGR, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.move_sgr_ok),
	XtRBoolean, (XtPointer) &defaultTRUE},
#endif
{XtNcutNewline, XtCCutNewline, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.cutNewline),
	XtRBoolean, (XtPointer) &defaultTRUE},
{XtNcutToBeginningOfLine, XtCCutToBeginningOfLine, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.cutToBeginningOfLine),
	XtRBoolean, (XtPointer) &defaultTRUE},
{XtNhighlightSelection,XtCHighlightSelection,XtRBoolean,
        sizeof(Boolean),XtOffsetOf(XtermWidgetRec, screen.highlight_selection),
        XtRBoolean, (XtPointer) &defaultFALSE},
{XtNbackground, XtCBackground, XtRPixel, sizeof(Pixel),
	XtOffsetOf(XtermWidgetRec, core.background_pixel),
	XtRString, "XtDefaultBackground"},
{XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
	XtOffsetOf(XtermWidgetRec, screen.foreground),
	XtRString, "XtDefaultForeground"},
{XtNcursorColor, XtCForeground, XtRPixel, sizeof(Pixel),
	XtOffsetOf(XtermWidgetRec, screen.cursorcolor),
	XtRString, "XtDefaultForeground"},
#if OPT_BLINK_CURS
{XtNcursorBlinkTime, XtCCursorBlinkTime, XtRInt, sizeof(int),
	XtOffsetOf(XtermWidgetRec, screen.cursor_blink),
        XtRInt, (XtPointer) &defaultBlinkTime},
#endif
{XtNeightBitInput, XtCEightBitInput, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.input_eight_bits),
	XtRBoolean, (XtPointer) &defaultTRUE},
{XtNeightBitOutput, XtCEightBitOutput, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.output_eight_bits),
	XtRBoolean, (XtPointer) &defaultTRUE},
{XtNeightBitControl, XtCEightBitControl, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.control_eight_bits),
	XtRBoolean, (XtPointer) &defaultFALSE},
{XtNgeometry,XtCGeometry, XtRString, sizeof(char *),
	XtOffsetOf(XtermWidgetRec, misc.geo_metry),
	XtRString, (XtPointer) NULL},
{XtNalwaysHighlight,XtCAlwaysHighlight,XtRBoolean,
        sizeof(Boolean),XtOffsetOf(XtermWidgetRec, screen.always_highlight),
        XtRBoolean, (XtPointer) &defaultFALSE},
{XtNappcursorDefault,XtCAppcursorDefault,XtRBoolean,
        sizeof(Boolean),XtOffsetOf(XtermWidgetRec, misc.appcursorDefault),
        XtRBoolean, (XtPointer) &defaultFALSE},
{XtNappkeypadDefault,XtCAppkeypadDefault,XtRBoolean,
        sizeof(Boolean),XtOffsetOf(XtermWidgetRec, misc.appkeypadDefault),
        XtRBoolean, (XtPointer) &defaultFALSE},
{XtNbackarrowKey, XtCBackarrowKey, XtRBoolean, sizeof(Boolean),
        XtOffsetOf(XtermWidgetRec, screen.backarrow_key),
        XtRBoolean, (XtPointer) &defaultTRUE},
{XtNbellSuppressTime, XtCBellSuppressTime, XtRInt, sizeof(int),
        XtOffsetOf(XtermWidgetRec, screen.bellSuppressTime),
        XtRInt, (XtPointer) &defaultBellSuppressTime},
{XtNtekGeometry,XtCGeometry, XtRString, sizeof(char *),
	XtOffsetOf(XtermWidgetRec, misc.T_geometry),
	XtRString, (XtPointer) NULL},
{XtNinternalBorder,XtCBorderWidth,XtRInt, sizeof(int),
	XtOffsetOf(XtermWidgetRec, screen.border),
	XtRInt, (XtPointer) &defaultIntBorder},
{XtNjumpScroll, XtCJumpScroll, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.jumpscroll),
	XtRBoolean, (XtPointer) &defaultTRUE},
{XtNoldXtermFKeys, XtCOldXtermFKeys, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.old_fkeys),
	XtRBoolean, (XtPointer) &defaultFALSE},
#ifdef ALLOWLOGGING
{XtNlogFile, XtCLogfile, XtRString, sizeof(char *),
	XtOffsetOf(XtermWidgetRec, screen.logfile),
	XtRString, (XtPointer) NULL},
{XtNlogging, XtCLogging, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, misc.log_on),
	XtRBoolean, (XtPointer) &defaultFALSE},
{XtNlogInhibit, XtCLogInhibit, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, misc.logInhibit),
	XtRBoolean, (XtPointer) &defaultFALSE},
#endif
{XtNloginShell, XtCLoginShell, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, misc.login_shell),
	XtRBoolean, (XtPointer) &defaultFALSE},
{XtNmarginBell, XtCMarginBell, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.marginbell),
	XtRBoolean, (XtPointer) &defaultFALSE},
{XtNpointerColor, XtCForeground, XtRPixel, sizeof(Pixel),
	XtOffsetOf(XtermWidgetRec, screen.mousecolor),
	XtRString, "XtDefaultForeground"},
{XtNpointerColorBackground, XtCBackground, XtRPixel, sizeof(Pixel),
	XtOffsetOf(XtermWidgetRec, screen.mousecolorback),
	XtRString, "XtDefaultBackground"},
{XtNpointerShape,XtCCursor, XtRCursor, sizeof(Cursor),
	XtOffsetOf(XtermWidgetRec, screen.pointer_cursor),
	XtRString, (XtPointer) "xterm"},
#ifdef OPT_PRINT_COLORS
{XtNprintAttributes,XtCPrintAttributes, XtRInt, sizeof(int),
	XtOffsetOf(XtermWidgetRec, screen.print_attributes),
	XtRInt, (XtPointer) &defaultONE},
#endif
{XtNprinterAutoClose,XtCPrinterAutoClose, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.printer_autoclose),
	XtRBoolean, (XtPointer) &defaultFALSE},
{XtNprinterControlMode, XtCPrinterControlMode, XtRInt, sizeof(int),
	XtOffsetOf(XtermWidgetRec, screen.printer_controlmode),
        XtRInt, (XtPointer) &defaultZERO},
{XtNprinterCommand,XtCPrinterCommand, XtRString, sizeof(String),
	XtOffsetOf(XtermWidgetRec, screen.printer_command),
	XtRString, (XtPointer) "lpr"},
{XtNprinterExtent,XtCPrinterExtent, XtRBoolean, sizeof(String),
	XtOffsetOf(XtermWidgetRec, screen.printer_extent),
	XtRBoolean, (XtPointer) &defaultFALSE},
{XtNprinterFormFeed,XtCPrinterFormFeed, XtRBoolean, sizeof(String),
	XtOffsetOf(XtermWidgetRec, screen.printer_formfeed),
	XtRBoolean, (XtPointer) &defaultFALSE},
{XtNmultiClickTime,XtCMultiClickTime, XtRInt, sizeof(int),
	XtOffsetOf(XtermWidgetRec, screen.multiClickTime),
	XtRInt, (XtPointer) &defaultMultiClickTime},
{XtNmultiScroll,XtCMultiScroll, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.multiscroll),
	XtRBoolean, (XtPointer) &defaultFALSE},
{XtNnMarginBell,XtCColumn, XtRInt, sizeof(int),
	XtOffsetOf(XtermWidgetRec, screen.nmarginbell),
	XtRInt, (XtPointer) &defaultNMarginBell},
{XtNreverseVideo,XtCReverseVideo,XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, misc.re_verse),
	XtRBoolean, (XtPointer) &defaultFALSE},
{XtNresizeGravity, XtCResizeGravity, XtRGravity, sizeof(XtGravity),
	XtOffsetOf(XtermWidgetRec, misc.resizeGravity),
	XtRImmediate, (XtPointer) SouthWestGravity},
{XtNreverseWrap,XtCReverseWrap, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, misc.reverseWrap),
	XtRBoolean, (XtPointer) &defaultFALSE},
{XtNautoWrap,XtCAutoWrap, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, misc.autoWrap),
	XtRBoolean, (XtPointer) &defaultTRUE},
{XtNsaveLines, XtCSaveLines, XtRInt, sizeof(int),
	XtOffsetOf(XtermWidgetRec, screen.savelines),
	XtRInt, (XtPointer) &defaultSaveLines},
{XtNscrollBar, XtCScrollBar, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, misc.scrollbar),
	XtRBoolean, (XtPointer) &defaultFALSE},
#ifdef SCROLLBAR_RIGHT
{XtNrightScrollBar, XtCRightScrollBar, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, misc.useRight),
	XtRBoolean, (XtPointer) &defaultFALSE},
#endif
{XtNscrollTtyOutput,XtCScrollCond, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.scrollttyoutput),
	XtRBoolean, (XtPointer) &defaultTRUE},
{XtNscrollKey, XtCScrollCond, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.scrollkey),
	XtRBoolean, (XtPointer) &defaultFALSE},
{XtNscrollLines, XtCScrollLines, XtRInt, sizeof(int),
	XtOffsetOf(XtermWidgetRec, screen.scrolllines),
	XtRInt, (XtPointer) &defaultScrollLines},
{XtNsignalInhibit,XtCSignalInhibit,XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, misc.signalInhibit),
	XtRBoolean, (XtPointer) &defaultFALSE},
#if OPT_TEK4014
{XtNtekInhibit, XtCTekInhibit, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, misc.tekInhibit),
	XtRBoolean, (XtPointer) &defaultFALSE},
{XtNtekSmall, XtCTekSmall, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, misc.tekSmall),
	XtRBoolean, (XtPointer) &defaultFALSE},
{XtNtekStartup, XtCTekStartup, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.TekEmu),
	XtRBoolean, (XtPointer) &defaultFALSE},
#endif
{XtNtiteInhibit, XtCTiteInhibit, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, misc.titeInhibit),
	XtRBoolean, (XtPointer) &defaultFALSE},
{XtNvisualBell, XtCVisualBell, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.visualbell),
	XtRBoolean, (XtPointer) &defaultFALSE},
{XtNallowSendEvents, XtCAllowSendEvents, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.allowSendEvents),
	XtRBoolean, (XtPointer) &defaultFALSE},
{XtNawaitInput, XtCAwaitInput, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.awaitInput),
	XtRBoolean, (XtPointer) &defaultFALSE},
{"font1", "Font1", XtRString, sizeof(String),
	XtOffsetOf(XtermWidgetRec, screen.menu_font_names[fontMenu_font1]),
	XtRString, (XtPointer) NULL},
{"font2", "Font2", XtRString, sizeof(String),
	XtOffsetOf(XtermWidgetRec, screen.menu_font_names[fontMenu_font2]),
	XtRString, (XtPointer) NULL},
{"font3", "Font3", XtRString, sizeof(String),
	XtOffsetOf(XtermWidgetRec, screen.menu_font_names[fontMenu_font3]),
	XtRString, (XtPointer) NULL},
{"font4", "Font4", XtRString, sizeof(String),
	XtOffsetOf(XtermWidgetRec, screen.menu_font_names[fontMenu_font4]),
	XtRString, (XtPointer) NULL},
{"font5", "Font5", XtRString, sizeof(String),
	XtOffsetOf(XtermWidgetRec, screen.menu_font_names[fontMenu_font5]),
	XtRString, (XtPointer) NULL},
{"font6", "Font6", XtRString, sizeof(String),
	XtOffsetOf(XtermWidgetRec, screen.menu_font_names[fontMenu_font6]),
	XtRString, (XtPointer) NULL},
#if OPT_INPUT_METHOD
{XtNinputMethod, XtCInputMethod, XtRString, sizeof(char*),
	XtOffsetOf(XtermWidgetRec, misc.input_method),
	XtRString, (XtPointer)NULL},
{XtNpreeditType, XtCPreeditType, XtRString, sizeof(char*),
	XtOffsetOf(XtermWidgetRec, misc.preedit_type),
	XtRString, (XtPointer)"Root"},
{XtNopenIm, XtCOpenIm, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, misc.open_im),
	XtRImmediate, (XtPointer)TRUE},
#endif
#if OPT_ISO_COLORS
{XtNcolor0, XtCForeground, XtRPixel, sizeof(Pixel),
	XtOffsetOf(XtermWidgetRec, screen.Acolors[COLOR_0]),
	XtRString, "XtDefaultForeground"},
{XtNcolor1, XtCForeground, XtRPixel, sizeof(Pixel),
	XtOffsetOf(XtermWidgetRec, screen.Acolors[COLOR_1]),
	XtRString, "XtDefaultForeground"},
{XtNcolor2, XtCForeground, XtRPixel, sizeof(Pixel),
	XtOffsetOf(XtermWidgetRec, screen.Acolors[COLOR_2]),
	XtRString, "XtDefaultForeground"},
{XtNcolor3, XtCForeground, XtRPixel, sizeof(Pixel),
	XtOffsetOf(XtermWidgetRec, screen.Acolors[COLOR_3]),
	XtRString, "XtDefaultForeground"},
{XtNcolor4, XtCForeground, XtRPixel, sizeof(Pixel),
	XtOffsetOf(XtermWidgetRec, screen.Acolors[COLOR_4]),
	XtRString, "XtDefaultForeground"},
{XtNcolor5, XtCForeground, XtRPixel, sizeof(Pixel),
	XtOffsetOf(XtermWidgetRec, screen.Acolors[COLOR_5]),
	XtRString, "XtDefaultForeground"},
{XtNcolor6, XtCForeground, XtRPixel, sizeof(Pixel),
	XtOffsetOf(XtermWidgetRec, screen.Acolors[COLOR_6]),
	XtRString, "XtDefaultForeground"},
{XtNcolor7, XtCForeground, XtRPixel, sizeof(Pixel),
	XtOffsetOf(XtermWidgetRec, screen.Acolors[COLOR_7]),
	XtRString, "XtDefaultForeground"},
{XtNcolor8, XtCForeground, XtRPixel, sizeof(Pixel),
	XtOffsetOf(XtermWidgetRec, screen.Acolors[COLOR_8]),
	XtRString, "XtDefaultForeground"},
{XtNcolor9, XtCForeground, XtRPixel, sizeof(Pixel),
	XtOffsetOf(XtermWidgetRec, screen.Acolors[COLOR_9]),
	XtRString, "XtDefaultForeground"},
{XtNcolor10, XtCForeground, XtRPixel, sizeof(Pixel),
	XtOffsetOf(XtermWidgetRec, screen.Acolors[COLOR_10]),
	XtRString, "XtDefaultForeground"},
{XtNcolor11, XtCForeground, XtRPixel, sizeof(Pixel),
	XtOffsetOf(XtermWidgetRec, screen.Acolors[COLOR_11]),
	XtRString, "XtDefaultForeground"},
{XtNcolor12, XtCForeground, XtRPixel, sizeof(Pixel),
	XtOffsetOf(XtermWidgetRec, screen.Acolors[COLOR_12]),
	XtRString, "XtDefaultForeground"},
{XtNcolor13, XtCForeground, XtRPixel, sizeof(Pixel),
	XtOffsetOf(XtermWidgetRec, screen.Acolors[COLOR_13]),
	XtRString, "XtDefaultForeground"},
{XtNcolor14, XtCForeground, XtRPixel, sizeof(Pixel),
	XtOffsetOf(XtermWidgetRec, screen.Acolors[COLOR_14]),
	XtRString, "XtDefaultForeground"},
{XtNcolor15, XtCForeground, XtRPixel, sizeof(Pixel),
	XtOffsetOf(XtermWidgetRec, screen.Acolors[COLOR_15]),
	XtRString, "XtDefaultForeground"},
{XtNcolorBD, XtCForeground, XtRPixel, sizeof(Pixel),
	XtOffsetOf(XtermWidgetRec, screen.Acolors[COLOR_BD]),
	XtRString, "XtDefaultForeground"},
{XtNcolorBL, XtCForeground, XtRPixel, sizeof(Pixel),
	XtOffsetOf(XtermWidgetRec, screen.Acolors[COLOR_BL]),
	XtRString, "XtDefaultForeground"},
{XtNcolorUL, XtCForeground, XtRPixel, sizeof(Pixel),
	XtOffsetOf(XtermWidgetRec, screen.Acolors[COLOR_UL]),
	XtRString, "XtDefaultForeground"},
{XtNcolorMode, XtCColorMode, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.colorMode),
	XtRBoolean, (XtPointer) &defaultCOLORMODE},
{XtNcolorULMode, XtCColorMode, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.colorULMode),
	XtRBoolean, (XtPointer) &defaultFALSE},
{XtNcolorBDMode, XtCColorMode, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.colorBDMode),
	XtRBoolean, (XtPointer) &defaultFALSE},
{XtNcolorBLMode, XtCColorMode, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.colorBLMode),
	XtRBoolean, (XtPointer) &defaultFALSE},
{XtNcolorAttrMode, XtCColorMode, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.colorAttrMode),
	XtRBoolean, (XtPointer) &defaultFALSE},
{XtNboldColors, XtCColorMode, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.boldColors),
	XtRBoolean, (XtPointer) &defaultTRUE},
#endif /* OPT_ISO_COLORS */
{XtNdynamicColors, XtCDynamicColors, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, misc.dynamicColors),
	XtRBoolean, (XtPointer) &defaultTRUE},
#if OPT_HIGHLIGHT_COLOR
{XtNhighlightColor, XtCForeground, XtRPixel, sizeof(Pixel),
	XtOffsetOf(XtermWidgetRec, screen.highlightcolor),
	XtRString, "XtDefaultForeground"},
#endif /* OPT_HIGHLIGHT_COLOR */
{XtNunderLine, XtCUnderLine, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.underline),
	XtRBoolean, (XtPointer) &defaultTRUE},
{XtNdecTerminalID, XtCDecTerminalID, XtRInt, sizeof(int),
	XtOffsetOf(XtermWidgetRec, screen.terminal_id),
	XtRInt, (XtPointer) &default_DECID},
#ifndef NO_ACTIVE_ICON
{"activeIcon", "ActiveIcon", XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, misc.active_icon),
	XtRString, "false"},
{"iconFont", "IconFont", XtRFontStruct, sizeof(XFontStruct),
	XtOffsetOf(XtermWidgetRec, screen.fnt_icon),
	XtRString, (XtPointer)XtExtdefaultfont},
{"iconBorderWidth", XtCBorderWidth, XtRInt, sizeof(int),
	XtOffsetOf(XtermWidgetRec, misc.icon_border_width),
	XtRString, "2"},
{"iconBorderColor", XtCBorderColor, XtRPixel, sizeof(Pixel),
	XtOffsetOf(XtermWidgetRec, misc.icon_border_pixel),
	XtRString, XtExtdefaultbackground},
#endif /* NO_ACTIVE_ICON */
};

static void VTClassInit (void);
static void VTInitialize (Widget wrequest, Widget wnew, ArgList args, Cardinal *num_args);
static void VTRealize (Widget w, XtValueMask *valuemask, XSetWindowAttributes *values);
static void VTExpose (Widget w, XEvent *event, Region region);
static void VTResize (Widget w);
static void VTDestroy (Widget w);
static Boolean VTSetValues (Widget cur, Widget request, Widget new, ArgList args, Cardinal *num_args);

#if OPT_I18N_SUPPORT && OPT_INPUT_METHOD
static void VTInitI18N (void);
#endif

static WidgetClassRec xtermClassRec = {
  {
/* core_class fields */
    /* superclass	  */	(WidgetClass) &widgetClassRec,
    /* class_name	  */	"VT100",
    /* widget_size	  */	sizeof(XtermWidgetRec),
    /* class_initialize   */    VTClassInit,
    /* class_part_initialize */ NULL,
    /* class_inited       */	FALSE,
    /* initialize	  */	VTInitialize,
    /* initialize_hook    */    NULL,
    /* realize		  */	VTRealize,
    /* actions		  */	actionsList,
    /* num_actions	  */	XtNumber(actionsList),
    /* resources	  */	resources,
    /* num_resources	  */	XtNumber(resources),
    /* xrm_class	  */	NULLQUARK,
    /* compress_motion	  */	TRUE,
    /* compress_exposure  */	FALSE,
    /* compress_enterleave */   TRUE,
    /* visible_interest	  */	FALSE,
    /* destroy		  */	VTDestroy,
    /* resize		  */	VTResize,
    /* expose		  */	VTExpose,
    /* set_values	  */	VTSetValues,
    /* set_values_hook    */    NULL,
    /* set_values_almost  */    NULL,
    /* get_values_hook    */    NULL,
    /* accept_focus	  */	NULL,
    /* version            */    XtVersion,
    /* callback_offsets   */    NULL,
    /* tm_table           */    defaultTranslations,
    /* query_geometry     */    XtInheritQueryGeometry,
    /* display_accelerator*/    XtInheritDisplayAccelerator,
    /* extension          */    NULL
  }
};

WidgetClass xtermWidgetClass = (WidgetClass)&xtermClassRec;

#if OPT_ISO_COLORS
/*
 * The terminal's foreground and background colors are set via two mechanisms:
 *	text (cur_foreground, cur_background values that are passed down to
 *		XDrawImageString and XDrawString)
 *	area (X11 graphics context used in XClearArea and XFillRectangle)
 */
void SGR_Foreground(int color)
{
	register TScreen *screen = &term->screen;
	Pixel	fg;

	/* FIXME HideCursor(); */
	if (color >= 0) {
		term->flags |= FG_COLOR;
	} else {
		term->flags &= ~FG_COLOR;
	}
	fg = getXtermForeground(term->flags, color);
	term->cur_foreground = color;

	XSetForeground(screen->display, NormalGC(screen), fg);
	XSetBackground(screen->display, ReverseGC(screen), fg);

	if (NormalGC(screen) != NormalBoldGC(screen)) {
		XSetForeground(screen->display, NormalBoldGC(screen), fg);
		XSetBackground(screen->display, ReverseBoldGC(screen), fg);
	}
}

void SGR_Background(int color)
{
	register TScreen *screen = &term->screen;
	Pixel	bg;

	if (color >= 0) {
		term->flags |= BG_COLOR;
	} else {
		term->flags &= ~BG_COLOR;
	}
	bg = getXtermBackground(term->flags, color);
	term->cur_background = color;

	XSetBackground(screen->display, NormalGC(screen), bg);
	XSetForeground(screen->display, ReverseGC(screen), bg);

	if (NormalGC(screen) != NormalBoldGC(screen)) {
		XSetBackground(screen->display, NormalBoldGC(screen), bg);
		XSetForeground(screen->display, ReverseBoldGC(screen), bg);
	}
}

/* Invoked after updating bold/underline flags, computes the extended color
 * index to use for foreground.  (See also 'extract_fg()').
 */
static void
setExtendedFG(void)
{
	int fg = term->sgr_foreground;

	if (term->screen.colorAttrMode
	 || (fg < 0)) {
		if (term->screen.colorULMode && (term->flags & UNDERLINE))
			fg = COLOR_UL;

		if (term->screen.colorBDMode && (term->flags & BOLD))
			fg = COLOR_BD;

		if (term->screen.colorBLMode && (term->flags & BLINK))
			fg = COLOR_BL;
	}

	/* This implements the IBM PC-style convention of 8-colors, with one
	 * bit for bold, thus mapping the 0-7 codes to 8-15.  It won't make
	 * much sense for 16-color applications, but we keep it to retain
	 * compatiblity with ANSI-color applications.
	 */
#if OPT_PC_COLORS
	if (term->screen.boldColors
	 && (fg >= 0)
	 && (fg < 8)
	 && (term->flags & BOLD))
		fg |= 8;
#endif

	SGR_Foreground(fg);
}

static void
reset_SGR_Foreground(void)
{
	term->sgr_foreground = -1;
	setExtendedFG();
}

static void
reset_SGR_Colors(void)
{
	reset_SGR_Foreground();
	SGR_Background(-1);
}
#endif /* OPT_ISO_COLORS */

void resetCharsets(TScreen *screen)
{
	screen->gsets[0] = 'B';			/* ASCII_G		*/
	screen->gsets[1] = '0';			/* line drawing		*/
	screen->gsets[2] = 'B';			/* DEC supplemental.	*/
	screen->gsets[3] = 'B';
	screen->curgl = 0;			/* G0 => GL.		*/
	screen->curgr = 2;			/* G2 => GR.		*/
	screen->curss = 0;			/* No single shift.	*/
}

static void VTparse(void)
{
	/* Buffer for processing strings (e.g., OSC ... ST) */
	static Char *string_area;
	static size_t string_size, string_used;

#if OPT_VT52_MODE
	static Bool vt52_cup = FALSE;
#endif

	Const PARSE_T *groundtable = ansi_table;
	register TScreen *screen = &term->screen;
	register Const PARSE_T *parsestate;
	register unsigned int c;
	register unsigned char *cp;
	register int row, col, top, bot, scstype, count;
	Bool private_function;	/* distinguish private-mode from standard */
	int string_mode;	/* nonzero iff we're processing a string */
	int lastchar;		/* positive iff we had a graphic character */

	/* We longjmp back to this point in VTReset() */
	(void)setjmp(vtjmpbuf);
#if OPT_VT52_MODE
	groundtable = screen->ansi_level ? ansi_table : vt52_table;
#else
	groundtable = ansi_table;
#endif
	parsestate = groundtable;
	scstype = 0;
	private_function = False;
	string_mode = 0;
	lastchar = -1;

	for( ; ; ) {
            int thischar = -1;
	    c = doinput();

	    /* Intercept characters for printer controller mode */
	    if (screen->printer_controlmode == 2) {
		if ((c = xtermPrinterControl(c)) == 0)
		    continue;
	    }

	    /* Accumulate string for APC, DCS, PM, OSC, SOS controls */
	    if (parsestate == sos_table) {
		if (string_size == 0) {
			string_area = (Char *)malloc(string_size = 256);
		} else if (string_used+1 >= string_size) {
			string_size += string_size;
			string_area = (Char *)realloc(string_area, string_size);
		}
		string_area[string_used++] = c;
	    } else if (parsestate != esc_table) {
		/* if we were accumulating, we're not any more */
	    	string_mode = 0;
		string_used = 0;
	    }

	    /*
	     * VT52 is a little ugly in the one place it has a parameterized
	     * control sequence, since the parameter falls after the character
	     * that denotes the type of sequence.
	     */
#if OPT_VT52_MODE
	    if (vt52_cup) {
		param[nparam++] = (c & 0x7f) - 32;
		if (nparam < 2)
			continue;
		vt52_cup = FALSE;
		if((row = param[0]) < 0)
			row = 0;
		if((col = param[1]) < 0)
			col = 0;
		CursorSet(screen, row, col, term->flags);
		parsestate = vt52_table;
		param[0] = 0;
		param[1] = 0;
		continue;
	    }
#endif

	    TRACE(("parse %d -> %d\n", c, parsestate[c]))
	    switch (parsestate[c]) {
		 case CASE_PRINT:
			/* printable characters */
			top = bcnt > TEXT_BUF_SIZE ? TEXT_BUF_SIZE : bcnt;
			cp = bptr;
			*--bptr = c;
			while(top > 0 && isprint(*cp & 0x7f)) {
#if OPT_VT52_MODE
				/*
				 * Strip output text to 7-bits for VT52.  We
				 * should do this for VT100 also (which is a
				 * 7-bit device), but since xterm has been
				 * doing this for so long we shouldn't change
				 * this behavior.
				 */
				if (screen->ansi_level < 1)
					*cp &= 0x7f;
#endif
				top--;
				bcnt--;
				cp++;
			}
			if(screen->curss) {
				thischar = *bptr;
				dotext(screen,
				 screen->gsets[(int)(screen->curss)],
				 	bptr, bptr + 1);
				bptr += 1;
				screen->curss = 0;
			}
			if(bptr < cp) {
				thischar = cp[-1];
				dotext(screen,
				 screen->gsets[(int)(screen->curgl)],
				 	bptr, cp);
			}
			bptr = cp;
			break;

		 case CASE_GROUND_STATE:
			/* exit ignore mode */
			parsestate = groundtable;
			break;

		 case CASE_IGNORE_STATE:
			/* Ies: ignore anything else */
			parsestate = igntable;
			break;

		 case CASE_IGNORE_ESC:
			/* Ign: escape */
			parsestate = iestable;
			break;

		 case CASE_IGNORE:
			/* Ignore character */
			break;

		 case CASE_ENQ:
			for (count = 0; xterm_name[count] != 0; count++)
				unparseputc(xterm_name[count], screen->respond);
			break;

		 case CASE_BELL:
			if (string_mode == OSC) {
				if (string_used)
					string_area[--string_used] = '\0';
				do_osc(string_area, string_used);
				parsestate = groundtable;
			} else {
				/* bell */
				Bell(XkbBI_TerminalBell,0);
			}
			break;

		 case CASE_BS:
			/* backspace */
			CursorBack(screen, 1);
			break;

		 case CASE_CR:
			/* carriage return */
			CarriageReturn(screen);
			parsestate = groundtable;
			break;

		 case CASE_ESC:
			/* escape */
			if_OPT_VT52_MODE(screen,{
				parsestate = vt52_esc_table;
				break;})
			parsestate = esc_table;
			break;

#if OPT_VT52_MODE
		 case CASE_VT52_CUP:
			vt52_cup = TRUE;
			nparam = 0;
			break;
#endif

		 case CASE_VMOT:
			/*
			 * form feed, line feed, vertical tab
			 */
			xtermAutoPrint(c);
			Index(screen, 1);
			if (term->flags & LINEFEED)
				CarriageReturn(screen);
			do_xevents();
			parsestate = groundtable;
			break;

		 case CASE_CBT:
			/* cursor backward tabulation */
			if((count = param[0]) == DEFAULT)
				count = 1;
			while ((count-- > 0)
			  &&   (TabToPrevStop()));
			parsestate = groundtable;
			break;

		 case CASE_CHT:
			/* cursor forward tabulation */
			if((count = param[0]) == DEFAULT)
				count = 1;
			while ((count-- > 0)
			  &&   (TabToNextStop()));
			parsestate = groundtable;
			break;

		 case CASE_TAB:
			/* tab */
			TabToNextStop();
			break;

		 case CASE_SI:
			screen->curgl = 0;
			parsestate = groundtable;
			break;

		 case CASE_SO:
			screen->curgl = 1;
			parsestate = groundtable;
			break;

		 case CASE_DECDHL:
			xterm_DECDHL(c == '3');
			parsestate = groundtable;
			break;

		 case CASE_DECSWL:
			xterm_DECSWL();
			parsestate = groundtable;
			break;

		 case CASE_DECDWL:
			xterm_DECDWL();
			parsestate = groundtable;
			break;

		 case CASE_SCR_STATE:
			/* enter scr state */
			parsestate = scrtable;
			break;

		 case CASE_SCS0_STATE:
			/* enter scs state 0 */
			scstype = 0;
			parsestate = scstable;
			break;

		 case CASE_SCS1_STATE:
			/* enter scs state 1 */
			scstype = 1;
			parsestate = scstable;
			break;

		 case CASE_SCS2_STATE:
			/* enter scs state 2 */
			scstype = 2;
			parsestate = scstable;
			break;

		 case CASE_SCS3_STATE:
			/* enter scs state 3 */
			scstype = 3;
			parsestate = scstable;
			break;

		 case CASE_ESC_IGNORE:
			/* unknown escape sequence */
			parsestate = eigtable;
			break;

		 case CASE_ESC_DIGIT:
			/* digit in csi or dec mode */
			if((row = param[nparam - 1]) == DEFAULT)
				row = 0;
			param[nparam - 1] = 10 * row + (c - '0');
			break;

		 case CASE_ESC_SEMI:
			/* semicolon in csi or dec mode */
			if (nparam < NPARAM)
			    param[nparam++] = DEFAULT;
			break;

		 case CASE_DEC_STATE:
			/* enter dec mode */
			parsestate = dec_table;
			break;

		 case CASE_DEC2_STATE:
			/* enter dec2 mode */
			parsestate = dec2_table;
			break;

		 case CASE_DEC3_STATE:
			/* enter dec3 mode */
			parsestate = dec3_table;
			break;

		 case CASE_ICH:
			/* ICH */
			if((row = param[0]) < 1)
				row = 1;
			InsertChar(screen, row);
			parsestate = groundtable;
			break;

		 case CASE_CUU:
			/* CUU */
			if((row = param[0]) < 1)
				row = 1;
			CursorUp(screen, row);
			parsestate = groundtable;
			break;

		 case CASE_CUD:
			/* CUD */
			if((row = param[0]) < 1)
				row = 1;
			CursorDown(screen, row);
			parsestate = groundtable;
			break;

		 case CASE_CUF:
			/* CUF */
			if((col = param[0]) < 1)
				col = 1;
			CursorForward(screen, col);
			parsestate = groundtable;
			break;

		 case CASE_CUB:
			/* CUB */
			if((col = param[0]) < 1)
				col = 1;
			CursorBack(screen, col);
			parsestate = groundtable;
			break;

		 case CASE_CUP:
			/* CUP | HVP */
			if_OPT_XMC_GLITCH(screen,{
				Jump_XMC(screen);
			})
			if((row = param[0]) < 1)
				row = 1;
			if(nparam < 2 || (col = param[1]) < 1)
				col = 1;
			CursorSet(screen, row-1, col-1, term->flags);
			parsestate = groundtable;
			break;

		 case CASE_VPA:
			if((row = param[0]) < 1)
				row = 1;
			CursorSet(screen, row-1, screen->cur_col, term->flags);
			parsestate = groundtable;
			break;

		 case CASE_HPA:
			/* HPA | CHA */
			if((col = param[0]) < 1)
				col = 1;
			CursorSet(screen, screen->cur_row, col-1, term->flags);
			parsestate = groundtable;
			break;

		 case CASE_HP_BUGGY_LL:
			/* Some HP-UX applications have the bug that they
			   assume ESC F goes to the lower left corner of
			   the screen, regardless of what terminfo says. */
			if (screen->hp_ll_bc)
			    CursorSet(screen, screen->max_row, 0, term->flags);
			parsestate = groundtable;
			break;

		 case CASE_ED:
			/* ED */
			do_erase_display(screen, param[0], OFF_PROTECT);
			parsestate = groundtable;
			break;

		 case CASE_EL:
			/* EL */
			do_erase_line(screen, param[0], OFF_PROTECT);
			parsestate = groundtable;
			break;

		 case CASE_ECH:
			/* ECH */
			ClearRight(screen, param[0] < 1 ? 1 : param[0]);
			parsestate = groundtable;
			break;

		 case CASE_IL:
			/* IL */
			if((row = param[0]) < 1)
				row = 1;
			InsertLine(screen, row);
			parsestate = groundtable;
			break;

		 case CASE_DL:
			/* DL */
			if((row = param[0]) < 1)
				row = 1;
			DeleteLine(screen, row);
			parsestate = groundtable;
			break;

		 case CASE_DCH:
			/* DCH */
			if((row = param[0]) < 1)
				row = 1;
			DeleteChar(screen, row);
			parsestate = groundtable;
			break;

		 case CASE_TRACK_MOUSE:
			if (screen->send_mouse_pos == 3
			 || nparam > 1) {
				/* Track mouse as long as in window and between
				 * specified rows
				 */
				TrackMouse(param[0],
					param[2]-1, param[1]-1,
					param[3]-1, param[4]-2);
			} else {
				/* SD as per DEC vt400 documentation */
				if((count = param[0]) < 1)
					count = 1;
				RevScroll(screen, count);
				do_xevents();
			}
			parsestate = groundtable;
			break;

		 case CASE_DECID:
			if_OPT_VT52_MODE(screen,{
				unparseputc(ESC,  screen->respond);
				unparseputc('/',  screen->respond);
				unparseputc('Z',  screen->respond);
				parsestate = groundtable;
				break;
				})
			param[0] = -1;		/* Default ID parameter */
			/* FALLTHRU */
		 case CASE_DA1:
			/* DA1 */
			if (param[0] <= 0) {	/* less than means DEFAULT */
			    count = 0;
			    reply.a_type   = CSI;
			    reply.a_pintro = '?';

			    /* The first param corresponds to the highest
			     * operating level (i.e., service level) of the
			     * emulation.  A DEC terminal can be setup to
			     * respond with a different DA response, but
			     * there's no control sequence that modifies this.
			     * We set it via a resource.
			     */
			    if (screen->terminal_id < 200) {
				switch (screen->terminal_id) {
				case 102:
				    reply.a_param[count++] = 6; /* VT102 */
				    break;
				case 101:
				    reply.a_param[count++] = 1; /* VT101 */
				    reply.a_param[count++] = 0; /* no options */
				    break;
				default: /* VT100 */
				    reply.a_param[count++] = 1; /* VT100 */
				    reply.a_param[count++] = 2; /* AVO */
				    break;
				}
			    } else {
				reply.a_param[count++] = 60 + screen->terminal_id/100;
				reply.a_param[count++] = 1; /* 132-columns */
				/* reply.a_param[count++] = 2; NO printer */
				reply.a_param[count++] = 6; /* selective-erase */
				reply.a_param[count++] = 8; /* user-defined-keys */
				reply.a_param[count++] = 15; /* technical characters */
			    }
			    reply.a_nparam = count;
			    reply.a_inters = 0;
			    reply.a_final  = 'c';
			    unparseseq(&reply, screen->respond);
			}
			parsestate = groundtable;
			break;

		 case CASE_DA2:
			/* DA2 */
			if ((screen->terminal_id >= 200)
			 && (param[0] <= 0)) {	/* less than means DEFAULT */
				count = 0;
				reply.a_type   = CSI;
				reply.a_pintro = '>';

				reply.a_param[count++] = 1; /* VT220 */
				reply.a_param[count++] = 0; /* Version */
				reply.a_param[count++] = 0; /* options (none) */
				reply.a_nparam = count;
				reply.a_inters = 0;
				reply.a_final  = 'c';
				unparseseq(&reply, screen->respond);
			}
			parsestate = groundtable;
			break;

		 case CASE_DECRPTUI:
			/* DECRPTUI */
			if ((screen->terminal_id >= 400)
			 && (param[0] <= 0)) {	/* less than means DEFAULT */
				unparseputc1(DCS, screen->respond);
				unparseputc('!',  screen->respond);
				unparseputc('|',  screen->respond);
				unparseputc('0',  screen->respond);
				unparseputc1(ST,  screen->respond);
			}
			parsestate = groundtable;
			break;

		 case CASE_TBC:
			/* TBC */
			if ((row = param[0]) <= 0) /* less than means default */
				TabClear(term->tabs, screen->cur_col);
			else if (row == 3)
				TabZonk(term->tabs);
			parsestate = groundtable;
			break;

		 case CASE_SET:
			/* SET */
			ansi_modes(term, bitset);
			parsestate = groundtable;
			break;

		 case CASE_RST:
			/* RST */
			ansi_modes(term, bitclr);
			parsestate = groundtable;
			break;

		 case CASE_SGR:
			/* SGR */
			for (row=0; row<nparam; ++row) {
				if_OPT_XMC_GLITCH(screen,{
					Mark_XMC(screen,param[row]);
				})
				switch (param[row]) {
				 case DEFAULT:
				 case 0:
					term->flags &=
						~(INVERSE|BOLD|BLINK|UNDERLINE|INVISIBLE);
					if_OPT_ISO_COLORS(screen,{reset_SGR_Colors();})
					break;
				 case 1:	/* Bold			*/
					term->flags |= BOLD;
					if_OPT_ISO_COLORS(screen,{setExtendedFG();})
					break;
				 case 5:	/* Blink		*/
					term->flags |= BLINK;
					if_OPT_ISO_COLORS(screen,{setExtendedFG();})
					break;
				 case 4:	/* Underscore		*/
					term->flags |= UNDERLINE;
					if_OPT_ISO_COLORS(screen,{setExtendedFG();})
					break;
				 case 7:
					term->flags |= INVERSE;
					break;
				 case 8:
					term->flags |= INVISIBLE;
					break;
				 case 24:
					term->flags &= ~UNDERLINE;
					if_OPT_ISO_COLORS(screen,{setExtendedFG();})
					break;
				 case 22: /* reset 'bold' */
					term->flags &= ~BOLD;
					if_OPT_ISO_COLORS(screen,{setExtendedFG();})
					break;
				 case 25: /* reset 'blink' */
					term->flags &= ~BLINK;
					if_OPT_ISO_COLORS(screen,{setExtendedFG();})
					break;
				 case 27:
					term->flags &= ~INVERSE;
					break;
				 case 28:
					term->flags &= ~INVISIBLE;
					break;
				 case 30:
				 case 31:
				 case 32:
				 case 33:
				 case 34:
				 case 35:
				 case 36:
				 case 37:
					if_OPT_ISO_COLORS(screen,{
					  term->sgr_foreground = (param[row] - 30);
					  setExtendedFG();
					})
					break;
				 case 39:
					if_OPT_ISO_COLORS(screen,{
					  reset_SGR_Foreground();
					})
					break;
				 case 40:
				 case 41:
				 case 42:
				 case 43:
				 case 44:
				 case 45:
				 case 46:
				 case 47:
					if_OPT_ISO_COLORS(screen,{
					  SGR_Background(param[row] - 40);
					})
					break;
				 case 49:
					if_OPT_ISO_COLORS(screen,{
					  SGR_Background(-1);
					})
					break;
				 case 90:
				 case 91:
				 case 92:
				 case 93:
				 case 94:
				 case 95:
				 case 96:
				 case 97:
					if_OPT_AIX_COLORS(screen,{
					  term->sgr_foreground = (param[row] - 90 + 8);
					  setExtendedFG();
					})
					break;
				 case 100:
#if !OPT_AIX_COLORS
					if_OPT_ISO_COLORS(screen,{
					  reset_SGR_Foreground();
					  SGR_Background(-1);
					})
					break;
#endif
				 case 101:
				 case 102:
				 case 103:
				 case 104:
				 case 105:
				 case 106:
				 case 107:
					if_OPT_AIX_COLORS(screen,{
					  SGR_Background(param[row] - 100 + 8);
					})
					break;
				}
			}
			parsestate = groundtable;
			break;

			/* DSR (except for the '?') is a superset of CPR */
		 case CASE_DSR:
			private_function = True;

			/* FALLTHRU */
		 case CASE_CPR:
			count = 0;
			reply.a_type   = CSI;
			reply.a_pintro = private_function ? '?' : 0;
			reply.a_inters = 0;
			reply.a_final  = 'n';

			switch (param[0]) {
			case 5:
				/* operating status */
				reply.a_param[count++] = 0; /* (no malfunction ;-) */
				break;
			case 6:
				/* CPR */
				/* DECXCPR (with page=0) */
				reply.a_param[count++] = screen->cur_row + 1;
				reply.a_param[count++] = screen->cur_col + 1;
				reply.a_final  = 'R';
				break;
			case 15:
				/* printer status */
				reply.a_param[count++] = 13; /* FIXME: implement printer */
				break;
			case 25:
				/* UDK status */
				reply.a_param[count++] = 20; /* UDK always unlocked */
				break;
			case 26:
				/* keyboard status */
				reply.a_param[count++] = 27;
				reply.a_param[count++] = 1; /* North American */
				if (screen->terminal_id >= 400) {
					reply.a_param[count++] = 0; /* ready */
					reply.a_param[count++] = 0; /* LK201 */
				}
				break;
			}

			if ((reply.a_nparam = count) != 0)
				unparseseq(&reply, screen->respond);

			parsestate = groundtable;
			private_function = False;
			break;

		 case CASE_MC:
			xtermMediaControl(param[0], FALSE);
			parsestate = groundtable;
			break;

		 case CASE_DEC_MC:
			xtermMediaControl(param[0], TRUE);
			parsestate = groundtable;
			break;

		 case CASE_HP_MEM_LOCK:
		 case CASE_HP_MEM_UNLOCK:
			if(screen->scroll_amt)
			    FlushScroll(screen);
			if (parsestate[c] == CASE_HP_MEM_LOCK)
			    screen->top_marg = screen->cur_row;
			else
			    screen->top_marg = 0;
			parsestate = groundtable;
			break;

		 case CASE_DECSTBM:
			/* DECSTBM - set scrolling region */
			if((top = param[0]) < 1)
				top = 1;
			if(nparam < 2 || (bot = param[1]) == DEFAULT
			   || bot > screen->max_row + 1
			   || bot == 0)
				bot = screen->max_row+1;
			if (bot > top) {
				if(screen->scroll_amt)
					FlushScroll(screen);
				screen->top_marg = top-1;
				screen->bot_marg = bot-1;
				CursorSet(screen, 0, 0, term->flags);
			}
			parsestate = groundtable;
			break;

		 case CASE_DECREQTPARM:
			/* DECREQTPARM */
			if (screen->terminal_id < 200) { /* VT102 */
			    if ((row = param[0]) == DEFAULT)
				row = 0;
			    if (row == 0 || row == 1) {
				reply.a_type = CSI;
				reply.a_pintro = 0;
				reply.a_nparam = 7;
				reply.a_param[0] = row + 2;
				reply.a_param[1] = 1;	/* no parity */
				reply.a_param[2] = 1;	/* eight bits */
				reply.a_param[3] = 112;	/* transmit 9600 baud */
				reply.a_param[4] = 112;	/* receive 9600 baud */
				reply.a_param[5] = 1;	/* clock multiplier ? */
				reply.a_param[6] = 0;	/* STP flags ? */
				reply.a_inters = 0;
				reply.a_final  = 'x';
				unparseseq(&reply, screen->respond);
			    }
			}
			parsestate = groundtable;
			break;

		 case CASE_DECSET:
			/* DECSET */
			dpmodes(term, bitset);
			parsestate = groundtable;
#if OPT_TEK4014
			if(screen->TekEmu)
				return;
#endif
			break;

		 case CASE_DECRST:
			/* DECRST */
			dpmodes(term, bitclr);
#if OPT_VT52_MODE
			if (screen->ansi_level == 0)
				groundtable = vt52_table;
			else if (screen->terminal_id >= 100)
				groundtable = ansi_table;
#endif
			parsestate = groundtable;
			break;

		 case CASE_DECALN:
			/* DECALN */
			if(screen->cursor_state)
				HideCursor();
			for(row = screen->max_row ; row >= 0 ; row--) {
				bzero(SCRN_BUF_ATTRS(screen, row),
				 col = screen->max_col + 1);
				for(cp = SCRN_BUF_CHARS(screen, row) ; col > 0 ; col--)
					*cp++ = (unsigned char) 'E';
			}
			ScrnRefresh(screen, 0, 0, screen->max_row + 1,
			 screen->max_col + 1, False);
			parsestate = groundtable;
			break;

		 case CASE_GSETS:
			screen->gsets[scstype] = c;
			parsestate = groundtable;
			break;

		 case CASE_DECSC:
			/* DECSC */
			CursorSave(term, &screen->sc);
			parsestate = groundtable;
			break;

		 case CASE_DECRC:
			/* DECRC */
			CursorRestore(term, &screen->sc);
			if_OPT_ISO_COLORS(screen,{setExtendedFG();})
			parsestate = groundtable;
			break;

		 case CASE_DECKPAM:
			/* DECKPAM */
			term->keyboard.flags |= MODE_DECKPAM;
			update_appkeypad();
			parsestate = groundtable;
			break;

		 case CASE_DECKPNM:
			/* DECKPNM */
			term->keyboard.flags &= ~MODE_DECKPAM;
			update_appkeypad();
			parsestate = groundtable;
			break;

		 case CASE_CSI_QUOTE_STATE:
			parsestate = csi_quo_table;
		 	break;

			/* the ANSI conformance levels are noted in the
			 * vt400 user's manual (I assume they're the non-DEC
			 * equivalents of DECSCL - T.Dickey)
			 */
		 case CASE_ANSI_LEVEL_1:
			if (screen->terminal_id >= 100) {
				screen->ansi_level = 1;
				show_8bit_control(False);
#if OPT_VT52_MODE
				groundtable =
				parsestate = ansi_table;
#endif
			}
			break;
		 case CASE_ANSI_LEVEL_2:
			if (screen->terminal_id >= 200)
				screen->ansi_level = 2;
			break;
		 case CASE_ANSI_LEVEL_3:
			if (screen->terminal_id >= 300)
				screen->ansi_level = 3;
			break;

		 case CASE_DECSCL:
			if (param[0] >= 61 && param[0] <= 63) {
				screen->ansi_level = param[0] - 60;
				if (param[0] > 61) {
					if (param[1] == 1)
						show_8bit_control(False);
					else if (param[1] == 0 || param[1] == 2)
						show_8bit_control(True);
				}
			}
			parsestate = groundtable;
		 	break;

		 case CASE_DECSCA:
			screen->protected_mode = DEC_PROTECT;
			if (param[0] <= 0 || param[0] == 2)
				term->flags &= ~PROTECTED;
			else if (param[0] == 1)
				term->flags |= PROTECTED;
			parsestate = groundtable;
		 	break;

		 case CASE_DECSED:
			/* DECSED */
			do_erase_display(screen, param[0], DEC_PROTECT);
			parsestate = groundtable;
		 	break;

		 case CASE_DECSEL:
			/* DECSEL */
			do_erase_line(screen, param[0], DEC_PROTECT);
			parsestate = groundtable;
		 	break;

		 case CASE_ST:
			if (!string_used)
				break;
			string_area[--string_used] = '\0';
			switch (string_mode) {
			case APC:
				/* ignored */
				break;
			case DCS:
				do_dcs(string_area, string_used);
				break;
			case OSC:
				do_osc(string_area, string_used);
				break;
			case PM:
				/* ignored */
				break;
			case SOS:
				/* ignored */
				break;
			}
			parsestate = groundtable;
		 	break;

		 case CASE_SOS:
			/* Start of String */
			string_mode = SOS;
			parsestate = sos_table;
			break;

		 case CASE_PM:
			/* Privacy Message */
			string_mode = PM;
			parsestate = sos_table;
			break;

		 case CASE_DCS:
			/* Device Control String */
			string_mode = DCS;
			parsestate = sos_table;
		 	break;

		 case CASE_APC:
			/* Application Program Command */
			string_mode = APC;
			parsestate = sos_table;
		 	break;

		 case CASE_SPA:
			screen->protected_mode = ISO_PROTECT;
			term->flags |= PROTECTED;
			parsestate = groundtable;
		 	break;

		 case CASE_EPA:
			term->flags &= ~PROTECTED;
			parsestate = groundtable;
		 	break;

		 case CASE_SU:
			/* SU */
			if((count = param[0]) < 1)
				count = 1;
			Scroll(screen, count);
			parsestate = groundtable;
			break;

		 case CASE_SD:
			/* SD as per ISO 6429 */
			if((count = param[0]) < 1)
				count = 1;
			RevScroll(screen, count);
			do_xevents();
			parsestate = groundtable;
			break;

		 case CASE_IND:
			/* IND */
			Index(screen, 1);
			do_xevents();
			parsestate = groundtable;
			break;

		 case CASE_CPL:
			/* cursor prev line */
			CursorPrevLine(screen, param[0]);
			parsestate = groundtable;
			break;

		 case CASE_CNL:
			/* cursor next line */
			CursorNextLine(screen, param[0]);
			parsestate = groundtable;
			break;

		 case CASE_NEL:
			/* NEL */
			Index(screen, 1);
			CarriageReturn(screen);
			do_xevents();
			parsestate = groundtable;
			break;

		 case CASE_HTS:
			/* HTS */
			TabSet(term->tabs, screen->cur_col);
			parsestate = groundtable;
			break;

		 case CASE_RI:
			/* RI */
			RevIndex(screen, 1);
			parsestate = groundtable;
			break;

		 case CASE_SS2:
			/* SS2 */
			screen->curss = 2;
			parsestate = groundtable;
			break;

		 case CASE_SS3:
			/* SS3 */
			screen->curss = 3;
			parsestate = groundtable;
			break;

		 case CASE_CSI_STATE:
			/* enter csi state */
			nparam = 1;
			param[0] = DEFAULT;
			parsestate = csi_table;
			break;

		 case CASE_ESC_SP_STATE:
			/* esc space */
			parsestate = esc_sp_table;
			break;

		 case CASE_CSI_EX_STATE:
			/* esc exclamation */
			parsestate = csi_ex_table;
			break;

		 case CASE_S7C1T:
			show_8bit_control(False);
			parsestate = groundtable;
		 	break;

		 case CASE_S8C1T:
#if OPT_VT52_MODE
			if (screen->ansi_level <= 1)
				break;
#endif
			show_8bit_control(True);
			parsestate = groundtable;
		 	break;

		 case CASE_OSC:
			/* Operating System Command */
			parsestate = sos_table;
			string_mode = OSC;
			break;

		 case CASE_RIS:
			/* RIS */
			VTReset(TRUE, TRUE);
			parsestate = groundtable;
			break;

		 case CASE_DECSTR:
			/* DECSTR */
			VTReset(FALSE, FALSE);
			parsestate = groundtable;
			break;

		 case CASE_REP:
			/* REP */
			if (lastchar >= 0 && isprint(lastchar)) {
			    Char repeated[2];
			    count = (param[0] < 1) ? 1 : param[0];
			    repeated[0] = lastchar;
			    while (count-- > 0) {
				dotext(screen,
					screen->gsets[(int)(screen->curgl)],
					repeated, repeated+1);
			    }
			}
			parsestate = groundtable;
			break;

		 case CASE_LS2:
			/* LS2 */
			screen->curgl = 2;
			parsestate = groundtable;
			break;

		 case CASE_LS3:
			/* LS3 */
			screen->curgl = 3;
			parsestate = groundtable;
			break;

		 case CASE_LS3R:
			/* LS3R */
			screen->curgr = 3;
			parsestate = groundtable;
			break;

		 case CASE_LS2R:
			/* LS2R */
			screen->curgr = 2;
			parsestate = groundtable;
			break;

		 case CASE_LS1R:
			/* LS1R */
			screen->curgr = 1;
			parsestate = groundtable;
			break;

		 case CASE_XTERM_SAVE:
			savemodes(term);
			parsestate = groundtable;
			break;

		 case CASE_XTERM_RESTORE:
			restoremodes(term);
			parsestate = groundtable;
			break;

		 case CASE_XTERM_WINOPS:
			window_ops(term);
			parsestate = groundtable;
			break;
	    }
	    if (parsestate == groundtable)
		    lastchar = thischar;
	}
}

static char *v_buffer;		/* pointer to physical buffer */
static char *v_bufstr = NULL;	/* beginning of area to write */
static char *v_bufptr;		/* end of area to write */
static char *v_bufend;		/* end of physical buffer */
#define	ptymask()	(v_bufptr > v_bufstr ? pty_mask : 0)

/* Write data to the pty as typed by the user, pasted with the mouse,
   or generated by us in response to a query ESC sequence. */

int
v_write(int f, char *d, int len)
{
	int riten;
	int c = len;

	if (v_bufstr == NULL  &&  len > 0) {
	        v_buffer = XtMalloc(len);
		v_bufstr = v_buffer;
		v_bufptr = v_buffer;
		v_bufend = v_buffer + len;
	}
#ifdef DEBUG
	if (debug) {
	    fprintf(stderr, "v_write called with %d bytes (%d left over)",
		    len, v_bufptr - v_bufstr);
	    if (len > 1  &&  len < 10) fprintf(stderr, " \"%.*s\"", len, d);
	    fprintf(stderr, "\n");
	}
#endif

#ifndef AMOEBA
	if (!FD_ISSET (f, &pty_mask))
		return(write(f, d, len));
#else
	if (term->screen.respond != f)
		return(write(f, d, len));
#endif

	/*
	 * Append to the block we already have.
	 * Always doing this simplifies the code, and
	 * isn't too bad, either.  If this is a short
	 * block, it isn't too expensive, and if this is
	 * a long block, we won't be able to write it all
	 * anyway.
	 */

	if (len > 0) {
	    if (v_bufend < v_bufptr + len) { /* we've run out of room */
		if (v_bufstr != v_buffer) {
		    /* there is unused space, move everything down */
		    /* possibly overlapping memmove here */
#ifdef DEBUG
		    if (debug)
			fprintf(stderr, "moving data down %d\n",
				v_bufstr - v_buffer);
#endif
		    memmove( v_buffer, v_bufstr, v_bufptr - v_bufstr);
		    v_bufptr -= v_bufstr - v_buffer;
		    v_bufstr = v_buffer;
		}
		if (v_bufend < v_bufptr + len) {
		    /* still won't fit: get more space */
		    /* Don't use XtRealloc because an error is not fatal. */
		    int size = v_bufptr - v_buffer; /* save across realloc */
		    v_buffer = realloc(v_buffer, size + len);
		    if (v_buffer) {
#ifdef DEBUG
			if (debug)
			    fprintf(stderr, "expanded buffer to %d\n",
				    size + len);
#endif
			v_bufstr = v_buffer;
			v_bufptr = v_buffer + size;
			v_bufend = v_bufptr + len;
		    } else {
			/* no memory: ignore entire write request */
			fprintf(stderr, "%s: cannot allocate buffer space\n",
				xterm_name);
			v_buffer = v_bufstr; /* restore clobbered pointer */
			c = 0;
		    }
		}
	    }
	    if (v_bufend >= v_bufptr + len) {
		/* new stuff will fit */
		memmove( v_bufptr, d, len);
		v_bufptr += len;
	    }
	}

	/*
	 * Write out as much of the buffer as we can.
	 * Be careful not to overflow the pty's input silo.
	 * We are conservative here and only write
	 * a small amount at a time.
	 *
	 * If we can't push all the data into the pty yet, we expect write
	 * to return a non-negative number less than the length requested
	 * (if some data written) or -1 and set errno to EAGAIN,
	 * EWOULDBLOCK, or EINTR (if no data written).
	 *
	 * (Not all systems do this, sigh, so the code is actually
	 * a little more forgiving.)
	 */

#define MAX_PTY_WRITE 128	/* 1/2 POSIX minimum MAX_INPUT */

	if (v_bufptr > v_bufstr) {
#ifndef AMOEBA
	    riten = write(f, v_bufstr, v_bufptr - v_bufstr <= MAX_PTY_WRITE ?
			  	       v_bufptr - v_bufstr : MAX_PTY_WRITE);
	    if (riten < 0)
#else
	    riten = v_bufptr - v_bufstr <= MAX_PTY_WRITE ?
		    v_bufptr - v_bufstr : MAX_PTY_WRITE;
	    if (cb_puts(term->screen.tty_inq, v_bufstr, riten) != 0)
#endif /* AMOEBA */
	    {
#ifdef DEBUG
		if (debug) perror("write");
#endif
		riten = 0;
	    }
#ifdef DEBUG
	    if (debug)
		fprintf(stderr, "write called with %d, wrote %d\n",
			v_bufptr - v_bufstr <= MAX_PTY_WRITE ?
			v_bufptr - v_bufstr : MAX_PTY_WRITE,
			riten);
#endif
	    v_bufstr += riten;
	    if (v_bufstr >= v_bufptr) /* we wrote it all */
		v_bufstr = v_bufptr = v_buffer;
	}

	/*
	 * If we have lots of unused memory allocated, return it
	 */
	if (v_bufend - v_bufptr > 1024) { /* arbitrary hysteresis */
	    /* save pointers across realloc */
	    int start = v_bufstr - v_buffer;
	    int size = v_bufptr - v_buffer;
	    int allocsize = size ? size : 1;

	    v_buffer = realloc(v_buffer, allocsize);
	    if (v_buffer) {
		v_bufstr = v_buffer + start;
		v_bufptr = v_buffer + size;
		v_bufend = v_buffer + allocsize;
#ifdef DEBUG
		if (debug)
		    fprintf(stderr, "shrunk buffer to %d\n", allocsize);
#endif
	    } else {
		/* should we print a warning if couldn't return memory? */
		v_buffer = v_bufstr - start; /* restore clobbered pointer */
	    }
	}
	return(c);
}

static fd_set select_mask;
static fd_set write_mask;
static int pty_read_bytes;

static int
in_put(void)
{
    register TScreen *screen = &term->screen;
    register int i;
    static struct timeval select_timeout;

    for( ; ; ) {
#ifndef AMOEBA
	if (FD_ISSET (screen->respond, &select_mask) && eventMode == NORMAL)
#else
	if ((bcnt = cb_full(screen->tty_outq)) > 0 && eventMode == NORMAL)
#endif
	{
#ifdef ALLOWLOGGING
	    if (screen->logging)
		FlushLog(screen);
#endif
#ifndef AMOEBA
	    bcnt = read(screen->respond, (char *)(bptr = VTbuffer), BUF_SIZE);
#else
	    bptr = VTbuffer;
	    if ((bcnt = cb_gets(screen->tty_outq, bptr, bcnt, BUF_SIZE)) == 0) {
		errno = EIO;
		bcnt = -1;
	    }
#endif
	    if (bcnt <= 0) {
/*
 * Yes, I know this is a majorly f*ugly hack, however it seems to be
 * necessary for Solaris x86.   DWH 11/15/94
 * Dunno why though..
 */
#if defined(i386) && defined(SVR4) && defined(sun)
		if (errno == EIO || errno == 0 )
#else
		if (errno == EIO)
#endif
		    Cleanup (0);
		else if (!E_TEST(errno))
		    Panic(
			  "input: read returned unexpected error (%d)\n",
			  errno);
	    } else if (bcnt == 0)
#if defined(MINIX) || defined(__EMX__)
		Cleanup(0);
#else
		Panic("input: read returned zero\n", 0);
#endif
	    else {
		/* read from pty was successful */
		if (!screen->output_eight_bits) {
		    register int bc = bcnt;
		    register Char *b = bptr;

		    for (; bc > 0; bc--, b++) {
			*b &= (Char) 0x7f;
		    }
		}
		if ( screen->scrollWidget && screen->scrollttyoutput &&
		     screen->topline < 0)
		    WindowScroll(screen, 0);  /* Scroll to bottom */
		pty_read_bytes += bcnt;
		/* stop speed reading at some point to look for X stuff */
		/* (4096 is just a random large number.) */
		if (pty_read_bytes > 4096)
		    FD_CLR (screen->respond, &select_mask);
		break;
	    }
	}
	pty_read_bytes = 0;
	/* update the screen */
	if (screen->scroll_amt)
	    FlushScroll(screen);
	if (screen->cursor_set && (screen->cursor_col != screen->cur_col
				   || screen->cursor_row != screen->cur_row)) {
	    if (screen->cursor_state)
		HideCursor();
	    ShowCursor();
	} else if (screen->cursor_set != screen->cursor_state) {
	    if (screen->cursor_set)
		ShowCursor();
	    else
		HideCursor();
	}

	XFlush(screen->display); /* always flush writes before waiting */

#ifndef AMOEBA
	/* Update the masks and, unless X events are already in the queue,
	   wait for I/O to be possible. */
	XFD_COPYSET (&Select_mask, &select_mask);
	if (v_bufptr > v_bufstr) {
	    XFD_COPYSET (&pty_mask, &write_mask);
	} else
	    FD_ZERO (&write_mask);
	select_timeout.tv_sec = 0;
	/*
	 * if there's either an XEvent or an XtTimeout pending, just take
	 * a quick peek, i.e. timeout from the select() immediately.  If
	 * there's nothing pending, let select() block a little while, but
	 * for a shorter interval than the arrow-style scrollbar timeout.
	 * The blocking is optional, because it tends to increase the load
	 * on the host.
	 */
	if (XtAppPending(app_con)
#if OPT_BLINK_CURS
	 || (screen->cursor_blink > 0
	  && (screen->select || screen->always_highlight))
	 || screen->cursor_state == BLINKED_OFF
#endif
	 )
		select_timeout.tv_usec = 0;
	else
		select_timeout.tv_usec = 50000;
	i = select(max_plus1, &select_mask, &write_mask, NULL,
			(select_timeout.tv_usec == 0) || screen->awaitInput
			? &select_timeout
			: NULL);
	if (i < 0) {
	    if (errno != EINTR)
		SysError(ERROR_SELECT);
	    continue;
	}

	/* if there is room to write more data to the pty, go write more */
	if (FD_ISSET (screen->respond, &write_mask)) {
	    v_write(screen->respond, (char *)0, 0); /* flush buffer */
	}

	/* if there are X events already in our queue, it
	   counts as being readable */
	if (XtAppPending(app_con) ||
	    FD_ISSET (ConnectionNumber(screen->display), &select_mask)) {
	    xevents();
	}
#else  /* AMOEBA */
	i = _X11TransAmSelect(ConnectionNumber(screen->display), 1);
	/* if there are X events already in our queue,
	   it counts as being readable */
	if (XtAppPending(app_con) || i > 0) {
	    xevents();
	    continue;
	} else if (i < 0) {
	    extern int exiting;
	    if (errno != EINTR && !exiting)
		SysError(ERROR_SELECT);
	}
	if (cb_full(screen->tty_outq) <= 0)
	    SleepMainThread();
#endif /* AMOENA */

    }
    bcnt--;
    return(*bptr++);
}

/*
 * process a string of characters according to the character set indicated
 * by charset.  worry about end of line conditions (wraparound if selected).
 */
void
dotext(
	register TScreen *screen,
	int	charset,
	Char	*buf,		/* start of characters to process */
	Char	*ptr)		/* end */
{
	register int	len;
	register int	n;
	register int	next_col;

	if (!xtermCharSets(buf, ptr, charset))
		return;

	if_OPT_XMC_GLITCH(screen,{
		register Char	*s;
		if (charset != '?')
			for (s=buf; s<ptr; ++s)
				if (*s == XMC_GLITCH)
					*s = XMC_GLITCH+1;
	})

	len = ptr - buf;
	ptr = buf;
	while (len > 0) {
		n = screen->max_col - screen->cur_col +1;
		if (n <= 1) {
			if (screen->do_wrap && (term->flags & WRAPAROUND)) {
			    /* mark that we had to wrap this line */
			    ScrnSetWrapped(screen, screen->cur_row);
			    xtermAutoPrint('\n');
			    Index(screen, 1);
			    screen->cur_col = 0;
			    screen->do_wrap = 0;
			    n = screen->max_col+1;
			} else
			    n = 1;
		}
		if (len < n)
			n = len;
		next_col = screen->cur_col + n;

		WriteText(screen, ptr, n);

		/*
		 * the call to WriteText updates screen->cur_col.
		 * If screen->cur_col != next_col, we must have
		 * hit the right margin, so set the do_wrap flag.
		 */
		screen->do_wrap = (screen->cur_col < next_col);
		len -= n;
		ptr += n;
	}
}

#if OPT_ZICONBEEP
/* Flag icon name with "*** "  on window output when iconified.
 * I'd like to do something like reverse video, but I don't
 * know how to tell this to window managers in general.
 *
 * mapstate can be IsUnmapped, !IsUnmapped, or -1;
 * -1 means no change; the other two are set by event handlers
 * and indicate a new mapstate.  !IsMapped is done in the handler.
 * we worry about IsUnmapped when output occurs.  -IAN!
 */
static int mapstate = -1;
#include <X11/Shell.h>
#endif /* OPT_ZICONBEEP */

/*
 * write a string str of length len onto the screen at
 * the current cursor position.  update cursor position.
 */
static void
WriteText(
	register TScreen *screen,
	register Char	*str,
	register int	len)
{
	unsigned flags	= term->flags;
	int	fg_bg = makeColorPair(term->cur_foreground, term->cur_background);
	GC	currentGC;

	TRACE(("WriteText (%2d,%2d) (%d) %3d:%.*s\n",
		screen->cur_row,
		screen->cur_col,
		curXtermChrSet(screen->cur_row),
		len, len, str))

	if(screen->cur_row - screen->topline <= screen->max_row) {
		if(screen->cursor_state)
			HideCursor();

		if (flags & INSERT)
			InsertChar(screen, len);
		if (!AddToRefresh(screen)) {
			/* make sure that the correct GC is current */
			currentGC = updatedXtermGC(screen, flags, fg_bg, False);

			if(screen->scroll_amt)
				FlushScroll(screen);

			if (flags & INVISIBLE)
				memset(str, ' ', len);

			TRACE(("%s @%d, calling drawXtermText (%d,%d)\n",
				__FILE__, __LINE__,
				screen->cur_col,
				screen->cur_row))
			drawXtermText(screen, flags, currentGC,
				CurCursorX(screen, screen->cur_row, screen->cur_col),
				CursorY(screen, screen->cur_row),
				curXtermChrSet(screen->cur_row),
				str, len);

			resetXtermGC(screen, flags, False);

			/*
			 * The following statements compile data to compute the
			 * average number of characters written on each call to
			 * XText.  The data may be examined via the use of a
			 * "hidden" escape sequence.
			 */
#ifdef UNUSED
			ctotal += len;
			++ntotal;
#endif
		}
	}
	ScreenWrite(screen, str, flags, fg_bg, len);
	CursorForward(screen, len);
#if OPT_ZICONBEEP
	/* Flag icon name with "***"  on window output when iconified.
	 */
	if( zIconBeep && mapstate == IsUnmapped && ! zIconBeep_flagged) {
	    static char *icon_name;
	    static Arg args[] = {
		{ XtNiconName, (XtArgVal) &icon_name }
	    };

	    icon_name = NULL;
	    XtGetValues(toplevel,args,XtNumber(args));

	    if( icon_name != NULL ) {
		zIconBeep_flagged = True;
		Changename(icon_name);
	    }
	    if (zIconBeep > 0)
		XBell( XtDisplay(toplevel), zIconBeep );
	}
	mapstate = -1;
#endif /* OPT_ZICONBEEP */
}

#if OPT_ZICONBEEP
/* Flag icon name with "***"  on window output when iconified.
 */
static void HandleMapUnmap(
	Widget w GCC_UNUSED,
	XtPointer closure GCC_UNUSED,
	XEvent *event,
	Boolean *cont GCC_UNUSED)
{
    static char *icon_name;
    static Arg args[] = {
            { XtNiconName, (XtArgVal) &icon_name }
    };

    TRACE(("event %d\n", event->type))

    switch( event->type ){
    case MapNotify:
	if( zIconBeep_flagged ) {
	    zIconBeep_flagged = False;
	    icon_name = NULL;
	    XtGetValues(toplevel,args,XtNumber(args));
	    if( icon_name != NULL ) {
		char	*buf = malloc(strlen(icon_name) + 1);
		if (buf == NULL) {
			zIconBeep_flagged = True;
			return;
		}
		strcpy(buf, icon_name + 4);
		Changename(buf);
		free(buf);
	    }
	}
	mapstate = !IsUnmapped;
	break;
    case UnmapNotify:
	mapstate = IsUnmapped;
	break;
    }
}
#endif /* OPT_ZICONBEEP */

/*
 * process ANSI modes set, reset
 */
static void
ansi_modes(
	XtermWidget	termw,
	void (*func) (unsigned *p, unsigned mask))
{
	register int	i;

	for (i=0; i<nparam; ++i) {
		switch (param[i]) {
		case 2:			/* KAM (if set, keyboard locked	*/
			(*func)(&termw->keyboard.flags, MODE_KAM);
			break;

		case 4:			/* IRM				*/
			(*func)(&termw->flags, INSERT);
			break;

		case 12:		/* SRM (if set, local echo	*/
			(*func)(&termw->keyboard.flags, MODE_SRM);
			break;

		case 20:		/* LNM				*/
			(*func)(&termw->flags, LINEFEED);
			update_autolinefeed();
			break;
		}
	}
}

/*
 * process DEC private modes set, reset
 */
static void
dpmodes(
	XtermWidget	termw,
	void (*func) (unsigned *p, unsigned mask))
{
	register TScreen	*screen	= &termw->screen;
	register int	i, j;

	for (i=0; i<nparam; ++i) {
		switch (param[i]) {
		case 1:			/* DECCKM			*/
			(*func)(&termw->keyboard.flags, MODE_DECCKM);
			update_appcursor();
			break;
		case 2:			/* ANSI/VT52 mode		*/
			if (func == bitset) {	/* ANSI (VT100)	*/
				resetCharsets(screen);
				if_OPT_VT52_MODE(screen,{
					screen->ansi_level = 1;})
			}
#if OPT_VT52_MODE
			else if (screen->terminal_id >= 100) {	/* VT52 */
				screen->ansi_level = 0;
				param[0] = 0;
				param[1] = 0;
				resetCharsets(screen);
			}
#endif
			break;
		case 3:			/* DECCOLM			*/
			if(screen->c132) {
				ClearScreen(screen);
				CursorSet(screen, 0, 0, termw->flags);
				if((j = func == bitset ? 132 : 80) !=
				 ((termw->flags & IN132COLUMNS) ? 132 : 80) ||
				 j != screen->max_col + 1)
					RequestResize(termw, -1, j, TRUE);
				(*func)(&termw->flags, IN132COLUMNS);
			}
			break;
		case 4:			/* DECSCLM (slow scroll)	*/
			if (func == bitset) {
				screen->jumpscroll = 0;
				if (screen->scroll_amt)
					FlushScroll(screen);
			} else
				screen->jumpscroll = 1;
			(*func)(&termw->flags, SMOOTHSCROLL);
			update_jumpscroll();
			break;
		case 5:			/* DECSCNM			*/
			j = termw->flags;
			(*func)(&termw->flags, REVERSE_VIDEO);
			if ((termw->flags ^ j) & REVERSE_VIDEO)
				ReverseVideo(termw);
			/* update_reversevideo done in RevVid */
			break;

		case 6:			/* DECOM			*/
			(*func)(&termw->flags, ORIGIN);
			CursorSet(screen, 0, 0, termw->flags);
			break;

		case 7:			/* DECAWM			*/
			(*func)(&termw->flags, WRAPAROUND);
			update_autowrap();
			break;
		case 8:			/* DECARM			*/
			/* ignore autorepeat */
			break;
		case 9:			/* MIT bogus sequence		*/
			if(func == bitset)
				screen->send_mouse_pos = 1;
			else
				screen->send_mouse_pos = 0;
			break;
		case 18:		/* DECPFF: print form feed */
		        if(func == bitset)
			        screen->printer_formfeed = ON;
			else
			        screen->printer_formfeed = OFF;
			break;
		case 19:		/* DECPEX: print extent */
		        if(func == bitset)
			        screen->printer_extent = ON;
			else
			        screen->printer_extent = OFF;
			break;
		case 25:		/* DECTCEM: Show/hide cursor (VT200) */
		        if(func == bitset)
			        screen->cursor_set = ON;
			else
			        screen->cursor_set = OFF;
			break;
		case 38:		/* DECTEK			*/
#if OPT_TEK4014
			if(func == bitset && !(screen->inhibit & I_TEK)) {
#ifdef ALLOWLOGGING
				if(screen->logging) {
					FlushLog(screen);
					screen->logstart = Tbuffer;
				}
#endif
				screen->TekEmu = TRUE;
			}
#endif
			break;
		case 40:		/* 132 column mode		*/
			screen->c132 = (func == bitset);
			update_allow132();
			break;
		case 41:		/* curses hack			*/
			screen->curses = (func == bitset);
			update_cursesemul();
			break;
		case 42:		/* DECNRCM national charset (VT220) */
			(*func)(&termw->flags, NATIONAL);
			break;
		case 44:		/* margin bell			*/
			screen->marginbell = (func == bitset);
			if(!screen->marginbell)
				screen->bellarmed = -1;
			update_marginbell();
			break;
		case 45:		/* reverse wraparound	*/
			(*func)(&termw->flags, REVERSEWRAP);
			update_reversewrap();
			break;
#ifdef ALLOWLOGGING
		case 46:		/* logging		*/
#ifdef ALLOWLOGFILEONOFF
			/*
			 * if this feature is enabled, logging may be
			 * enabled and disabled via escape sequences.
			 */
			if(func == bitset)
				StartLog(screen);
			else
				CloseLog(screen);
#else
			Bell(XkbBI_Info,0);
			Bell(XkbBI_Info,0);
#endif /* ALLOWLOGFILEONOFF */
			break;
#endif
		case 1047:
		case 47:		/* alternate buffer */
			if (!termw->misc.titeInhibit) {
			    if(func == bitset) {
				ToAlternate(screen);
			    } else {
				if (screen->alternate
				 && (param[i] == 1047))
				    ClearScreen(screen);
				FromAlternate(screen);
			    }
			}
			break;
		case 66:	/* DECNKM */
			/* FIXME: VT300 numeric keypad */
			break;
		case 67:	/* DECBKM */
			/* back-arrow mapped to backspace or delete(D)*/
			(*func)(&termw->keyboard.flags, MODE_DECBKM);
			update_decbkm();
			break;
		case 1000:		/* xterm bogus sequence		*/
			if(func == bitset)
				screen->send_mouse_pos = 2;
			else
				screen->send_mouse_pos = 0;
			break;
		case 1001:		/* xterm sequence w/hilite tracking */
			if(func == bitset)
				screen->send_mouse_pos = 3;
			else
				screen->send_mouse_pos = 0;
			break;
		case 1048:
			if (!termw->misc.titeInhibit) {
		        	if(func == bitset)
					CursorSave(termw, &screen->sc);
				else
					CursorRestore(termw, &screen->sc);
			}
			break;
		}
	}
}

/*
 * process xterm private modes save
 */
static void
savemodes(XtermWidget termw)
{
	register TScreen	*screen	= &termw->screen;
	register int i;

	for (i = 0; i < nparam; i++) {
		switch (param[i]) {
		case 1:			/* DECCKM			*/
			DoSM(DP_DECCKM, termw->keyboard.flags & MODE_DECCKM);
			break;
		case 3:			/* DECCOLM			*/
			if(screen->c132)
			    DoSM(DP_DECCOLM, termw->flags & IN132COLUMNS);
			break;
		case 4:			/* DECSCLM (slow scroll)	*/
			DoSM(DP_DECSCLM, termw->flags & SMOOTHSCROLL);
			break;
		case 5:			/* DECSCNM			*/
			DoSM(DP_DECSCNM, termw->flags & REVERSE_VIDEO);
			break;
		case 6:			/* DECOM			*/
			DoSM(DP_DECOM, termw->flags & ORIGIN);
			break;

		case 7:			/* DECAWM			*/
			DoSM(DP_DECAWM, termw->flags & WRAPAROUND);
			break;
		case 8:			/* DECARM			*/
			/* ignore autorepeat */
			break;
		case 9:			/* mouse bogus sequence */
			DoSM(DP_X_X10MSE, screen->send_mouse_pos);
			break;
		case 40:		/* 132 column mode		*/
			DoSM(DP_X_DECCOLM, screen->c132);
			break;
		case 41:		/* curses hack			*/
			DoSM(DP_X_MORE, screen->curses);
			break;
		case 44:		/* margin bell			*/
			DoSM(DP_X_MARGIN, screen->marginbell);
			break;
		case 45:		/* reverse wraparound	*/
			DoSM(DP_X_REVWRAP, termw->flags & REVERSEWRAP);
			break;
#ifdef ALLOWLOGGING
		case 46:		/* logging		*/
			DoSM(DP_X_LOGGING, screen->logging);
			break;
#endif
		case 1047:		/* alternate buffer		*/
		case 47:		/* alternate buffer		*/
			DoSM(DP_X_ALTSCRN, screen->alternate);
			break;
		case 1000:		/* mouse bogus sequence		*/
		case 1001:
			DoSM(DP_X_MOUSE, screen->send_mouse_pos);
			break;
		case 1048:
			if (!termw->misc.titeInhibit) {
				CursorSave(termw, &screen->sc);
			}
			break;
		}
	}
}

/*
 * process xterm private modes restore
 */
static void
restoremodes(XtermWidget termw)
{
	register TScreen	*screen	= &termw->screen;
	register int i, j;

	for (i = 0; i < nparam; i++) {
		switch (param[i]) {
		case 1:			/* DECCKM			*/
			bitcpy(&termw->keyboard.flags,
				screen->save_modes[DP_DECCKM], MODE_DECCKM);
			update_appcursor();
			break;
		case 3:			/* DECCOLM			*/
			if(screen->c132) {
				ClearScreen(screen);
				CursorSet(screen, 0, 0, termw->flags);
				if((j = (screen->save_modes[DP_DECCOLM] & IN132COLUMNS)
				 ? 132 : 80) != ((termw->flags & IN132COLUMNS)
				 ? 132 : 80) || j != screen->max_col + 1)
					RequestResize(termw, -1, j, TRUE);
				bitcpy(&termw->flags,
					screen->save_modes[DP_DECCOLM],
					IN132COLUMNS);
			}
			break;
		case 4:			/* DECSCLM (slow scroll)	*/
			if (screen->save_modes[DP_DECSCLM] & SMOOTHSCROLL) {
				screen->jumpscroll = 0;
				if (screen->scroll_amt)
					FlushScroll(screen);
			} else
				screen->jumpscroll = 1;
			bitcpy(&termw->flags, screen->save_modes[DP_DECSCLM], SMOOTHSCROLL);
			update_jumpscroll();
			break;
		case 5:			/* DECSCNM			*/
			if((screen->save_modes[DP_DECSCNM] ^ termw->flags) & REVERSE_VIDEO) {
				bitcpy(&termw->flags, screen->save_modes[DP_DECSCNM], REVERSE_VIDEO);
				ReverseVideo(termw);
				/* update_reversevideo done in RevVid */
			}
			break;
		case 6:			/* DECOM			*/
			bitcpy(&termw->flags, screen->save_modes[DP_DECOM], ORIGIN);
			CursorSet(screen, 0, 0, termw->flags);
			break;

		case 7:			/* DECAWM			*/
			bitcpy(&termw->flags, screen->save_modes[DP_DECAWM], WRAPAROUND);
			update_autowrap();
			break;
		case 8:			/* DECARM			*/
			/* ignore autorepeat */
			break;
		case 9:			/* MIT bogus sequence		*/
			DoRM(DP_X_X10MSE, screen->send_mouse_pos);
			break;
		case 40:		/* 132 column mode		*/
			DoRM(DP_X_DECCOLM, screen->c132);
			update_allow132();
			break;
		case 41:		/* curses hack			*/
			DoRM(DP_X_MORE, screen->curses);
			update_cursesemul();
			break;
		case 44:		/* margin bell			*/
			if((DoRM(DP_X_MARGIN, screen->marginbell)) == 0)
				screen->bellarmed = -1;
			update_marginbell();
			break;
		case 45:		/* reverse wraparound	*/
			bitcpy(&termw->flags, screen->save_modes[DP_X_REVWRAP], REVERSEWRAP);
			update_reversewrap();
			break;
#ifdef ALLOWLOGGING
		case 46:		/* logging		*/
#ifdef ALLOWLOGFILEONOFF
			if(screen->save_modes[DP_X_LOGGING])
				StartLog(screen);
			else
				CloseLog(screen);
#endif /* ALLOWLOGFILEONOFF */
			/* update_logging done by StartLog and CloseLog */
			break;
#endif
		case 1047:		/* alternate buffer */
		case 47:		/* alternate buffer */
			if (!termw->misc.titeInhibit) {
			    if(screen->save_modes[DP_X_ALTSCRN])
				ToAlternate(screen);
			    else
				FromAlternate(screen);
			    /* update_altscreen done by ToAlt and FromAlt */
			}
			break;
		case 1000:		/* mouse bogus sequence		*/
		case 1001:
			DoRM(DP_X_MOUSE, screen->send_mouse_pos);
			break;
		case 1048:
			if (!termw->misc.titeInhibit) {
				CursorRestore(termw, &screen->sc);
			}
			break;
		}
	}
}

/*
 * Report window label (icon or title) in dtterm protocol
 * ESC ] code label ESC backslash
 */
static void
report_win_label(
	TScreen	*screen,
	int code,
	XTextProperty *text,
	Status ok)
{
	char **list;
	int length = 0;

	reply.a_type = ESC;
	unparseputc(ESC, screen->respond);
	unparseputc(']', screen->respond);
	unparseputc(code, screen->respond);

	if (ok) {
		if (XTextPropertyToStringList(text, &list, &length)) {
			int n, c;
			for (n = 0; n < length; n++) {
				char *s = list[n];
				while ((c = *s++) != '\0')
					unparseputc(c, screen->respond);
			}
			XFreeStringList(list);
		}
		if (text->value != 0)
			XFree(text->value);
	}

	unparseputc(ESC, screen->respond);
	unparseputc('\\', screen->respond);
}

/*
 * Window operations (from CDE dtterm description)
 */
static void
window_ops(XtermWidget termw)
{
	register TScreen	*screen	= &termw->screen;
	XWindowChanges values;
	XWindowAttributes win_attrs;
	XTextProperty text;
	unsigned int value_mask;
	Position x, y;

	switch (param[0]) {
	case 1:		/* Restore (de-iconify) window */
		XMapWindow(screen->display,
			VShellWindow);
		break;

	case 2:		/* Minimize (iconify) window */
		XIconifyWindow(screen->display,
			VShellWindow,
			DefaultScreen(screen->display));
		break;

	case 3:		/* Move the window to the given position */
		values.x   = param[1];
		values.y   = param[2];
		value_mask = (CWX | CWY);
		XReconfigureWMWindow(
			screen->display,
			VShellWindow,
			DefaultScreen(screen->display),
			value_mask,
			&values);
		break;

	case 4:		/* Resize the window to given size in pixels */
		RequestResize(termw, param[1], param[2], FALSE);
		break;

	case 5:		/* Raise the window to the front of the stack */
		XRaiseWindow(screen->display, VShellWindow);
		break;

	case 6:		/* Lower the window to the bottom of the stack */
		XLowerWindow(screen->display, VShellWindow);
		break;

	case 7:		/* Refresh the window */
		Redraw();
		break;

	case 8:		/* Resize the text-area, in characters */
		RequestResize(termw, param[1], param[2], TRUE);
		break;

	case 11:	/* Report the window's state */
		XGetWindowAttributes(screen->display,
			VWindow(screen),
			&win_attrs);
		reply.a_type = CSI;
		reply.a_pintro = 0;
		reply.a_nparam = 1;
		reply.a_param[0] = (win_attrs.map_state == IsViewable) ? 1 : 2;
		reply.a_inters = 0;
		reply.a_final  = 't';
		unparseseq(&reply, screen->respond);
		break;

	case 13:	/* Report the window's position */
		XtTranslateCoords(toplevel, 0, 0, &x, &y);
		reply.a_type = CSI;
		reply.a_pintro = 0;
		reply.a_nparam = 3;
		reply.a_param[0] = 3;
		reply.a_param[1] = x;
		reply.a_param[2] = y;
		reply.a_inters = 0;
		reply.a_final  = 't';
		unparseseq(&reply, screen->respond);
		break;

	case 14:	/* Report the window's size in pixels */
		XGetWindowAttributes(screen->display,
			VWindow(screen),
			&win_attrs);
		reply.a_type = CSI;
		reply.a_pintro = 0;
		reply.a_nparam = 3;
		reply.a_param[0] = 4;
		/*FIXME: find if dtterm uses
		 *	win_attrs.height or Height
		 *	win_attrs.width  or Width
		 */
		reply.a_param[1] = Height(screen);
		reply.a_param[2] = Width(screen);
		reply.a_inters = 0;
		reply.a_final  = 't';
		unparseseq(&reply, screen->respond);
		break;

	case 18:	/* Report the text's size in characters */
		reply.a_type = CSI;
		reply.a_pintro = 0;
		reply.a_nparam = 3;
		reply.a_param[0] = 8;
		reply.a_param[1] = screen->max_row + 1;
		reply.a_param[2] = screen->max_col + 1;
		reply.a_inters = 0;
		reply.a_final  = 't';
		unparseseq(&reply, screen->respond);
		break;

	case 20:	/* Report the icon's label */
		report_win_label(screen, 'L', &text,
			XGetWMIconName(
				screen->display,
				VShellWindow,
				&text));
		break;

	case 21:	/* Report the window's title */
		report_win_label(screen, 'l', &text,
			XGetWMName(
				screen->display,
				VShellWindow,
				&text));
		break;

	default: /* DECSLPP (24, 25, 36, 48, 72, 144) */
		if (param[0] >= 24)
			RequestResize(termw, param[0], -1, TRUE);
		break;
	}
}

/*
 * set a bit in a word given a pointer to the word and a mask.
 */
static void bitset(unsigned *p, unsigned mask)
{
	*p |= mask;
}

/*
 * clear a bit in a word given a pointer to the word and a mask.
 */
static void bitclr(unsigned *p, unsigned mask)
{
	*p &= ~mask;
}

/*
 * Copy bits from one word to another, given a mask
 */
static void bitcpy(unsigned *p, unsigned q, unsigned mask)
{
	bitclr(p, mask);
	bitset(p, q & mask);
}

void
unparseputc1(int c, int fd)
{
	if (c >= 0x80 && c <= 0x9F) {
		if (!term->screen.control_eight_bits) {
			unparseputc(ESC, fd);
			c -= 0x40;
		}
	}
	unparseputc(c, fd);
}

void
unparseseq(register ANSI *ap, int fd)
{
	register int	c;
	register int	i;
	register int	inters;

	unparseputc1(c = ap->a_type, fd);
	if (c==ESC || c==DCS || c==CSI || c==OSC || c==PM || c==APC || c==SS3) {
		if (ap->a_pintro != 0)
			unparseputc((char) ap->a_pintro, fd);
		for (i=0; i<ap->a_nparam; ++i) {
			if (i != 0)
				unparseputc(';', fd);
			unparseputn((unsigned int) ap->a_param[i], fd);
		}
		inters = ap->a_inters;
		for (i=3; i>=0; --i) {
			c = (inters >> (8*i)) & 0xff;
			if (c != 0)
				unparseputc(c, fd);
		}
		unparseputc((char) ap->a_final, fd);
	}
}

static void
unparseputn(unsigned int n, int fd)
{
	unsigned int	q;

	q = n/10;
	if (q != 0)
		unparseputn(q, fd);
	unparseputc((char) ('0' + (n%10)), fd);
}

void
unparseputc(int c, int fd)
{
	Char	buf[2];
	register int i = 1;

#ifdef AMOEBA
	if (ttypreprocess(c)) return;
#endif
	if((buf[0] = c) == '\r' && (term->flags & LINEFEED)) {
		buf[1] = '\n';
		i++;
	}
	v_write(fd, (char *)buf, i);

	/* If send/receive mode is reset, we echo characters locally */
	if ((term->keyboard.flags & MODE_SRM) == 0) {
		register TScreen *screen = &term->screen;
		dotext(screen, screen->gsets[(int)(screen->curgl)], buf, buf+i);
	}
}

void
ToggleAlternate(register TScreen *screen)
{
	if (screen->alternate)
		FromAlternate(screen);
	else
		ToAlternate(screen);
}

static void
ToAlternate(register TScreen *screen)
{
	if(screen->alternate)
		return;
	if(!screen->altbuf)
		screen->altbuf = Allocate(screen->max_row + 1, screen->max_col
		 + 1, &screen->abuf_address);
	SwitchBufs(screen);
	screen->alternate = TRUE;
	update_altscreen();
}

static void
FromAlternate(register TScreen *screen)
{
	if(!screen->alternate)
		return;
	screen->alternate = FALSE;
	SwitchBufs(screen);
	update_altscreen();
}

static void
SwitchBufs(register TScreen *screen)
{
	register int rows, top;

	if(screen->cursor_state)
		HideCursor();

	rows = screen->max_row + 1;
	SwitchBufPtrs(screen);
	TrackText(0, 0, 0, 0);	/* remove any highlighting */

	if((top = -screen->topline) < rows) {
		if(screen->scroll_amt)
			FlushScroll(screen);
		if(top == 0)
			XClearWindow(screen->display, TextWindow(screen));
		else
			XClearArea(
			    screen->display,
			    TextWindow(screen),
			    (int) OriginX(screen),
			    (int) top * FontHeight(screen) + screen->border,
			    (unsigned) Width(screen),
			    (unsigned) (rows - top) * FontHeight(screen),
			    FALSE);
	}
	ScrnRefresh(screen, 0, 0, rows, screen->max_col + 1, False);
}

/* swap buffer line pointers between alt and regular screens */
void
SwitchBufPtrs(register TScreen *screen)
{
    size_t len = ScrnPointers(screen, screen->max_row + 1);

    memcpy ( (char *)screen->save_ptr, (char *)screen->visbuf,   len);
    memcpy ( (char *)screen->visbuf,   (char *)screen->altbuf,   len);
    memcpy ( (char *)screen->altbuf,   (char *)screen->save_ptr, len);
}

void
VTRun(void)
{
	register TScreen *screen = &term->screen;
#if OPT_TEK4014
	register int i;
#endif

	if (!screen->Vshow) {
	    set_vt_visibility (TRUE);
	}
	update_vttekmode();
	update_vtshow();
	update_tekshow();
	set_vthide_sensitivity();

	if (screen->allbuf == NULL) VTallocbuf ();

	screen->cursor_state = OFF;
	screen->cursor_set = ON;
	StartBlinking(screen);

	bcnt = 0;
	bptr = VTbuffer;
#if OPT_TEK4014
	while(Tpushb > Tpushback) {
		*bptr++ = *--Tpushb;
		bcnt++;
	}
	bcnt += (i = Tbcnt);
	for( ; i > 0 ; i--)
		*bptr++ = *Tbptr++;
	bptr = VTbuffer;
#endif
	if(!setjmp(VTend))
		VTparse();
	StopBlinking(screen);
	HideCursor();
	screen->cursor_set = OFF;
}

/*ARGSUSED*/
static void VTExpose(
	Widget w GCC_UNUSED,
	XEvent *event,
	Region region GCC_UNUSED)
{
	register TScreen *screen = &term->screen;

#ifdef DEBUG
	if(debug)
		fputs("Expose\n", stderr);
#endif	/* DEBUG */
	if (event->type == Expose)
		HandleExposure (screen, event);
}

static void VTGraphicsOrNoExpose (XEvent *event)
{
	register TScreen *screen = &term->screen;
	if (screen->incopy <= 0) {
		screen->incopy = 1;
		if (screen->scrolls > 0)
			screen->scrolls--;
	}
	if (event->type == GraphicsExpose)
	  if (HandleExposure (screen, event))
		screen->cursor_state = OFF;
	if ((event->type == NoExpose)
	 || ((XGraphicsExposeEvent *)event)->count == 0) {
		if (screen->incopy <= 0 && screen->scrolls > 0)
			screen->scrolls--;
		if (screen->scrolls)
			screen->incopy = -1;
		else
			screen->incopy = 0;
	}
}

/*ARGSUSED*/
static void VTNonMaskableEvent (
	Widget w GCC_UNUSED,
	XtPointer closure GCC_UNUSED,
	XEvent *event,
	Boolean *cont GCC_UNUSED)
{
    switch (event->type) {
       case GraphicsExpose:
       case NoExpose:
	  VTGraphicsOrNoExpose (event);
	  break;
      }
}




static void VTResize(Widget w)
{
    if (XtIsRealized(w))
      ScreenResize (&term->screen, term->core.width, term->core.height,
		    &term->flags);
}


static void RequestResize(
	XtermWidget termw,
	int rows,
	int cols,
	int text)
{
	register TScreen	*screen	= &termw->screen;
	Dimension replyWidth, replyHeight;
	Dimension askedWidth, askedHeight;
	XtGeometryResult status;
	XWindowAttributes attrs;

	askedWidth  = cols;
	askedHeight = rows;

	if (askedHeight == 0
	 || askedWidth  == 0) {
		XGetWindowAttributes(XtDisplay(termw),
			RootWindowOfScreen(XtScreen(termw)), &attrs);
	}

	if (text) {
		if (rows != 0) {
			if (rows < 0)
				askedHeight = screen->max_row + 1;
			askedHeight *= FontHeight(screen);
			askedHeight += (2 * screen->border);
		}

		if (cols != 0) {
			if (cols < 0)
				askedWidth = screen->max_col + 1;
			askedWidth  *= FontWidth(screen);
			askedWidth  += (2 * screen->border) + Scrollbar(screen);
		}

	} else {
		if (rows < 0)
			askedHeight = FullHeight(screen);
		if (cols < 0)
			askedWidth = FullWidth(screen);
	}

	if (rows == 0)
		askedHeight = attrs.height;
	if (cols == 0)
		askedWidth  = attrs.width;

	status = XtMakeResizeRequest (
	    (Widget) termw,
	     askedWidth,  askedHeight,
	    &replyWidth, &replyHeight);

	if (status == XtGeometryYes ||
	    status == XtGeometryDone) {
	    ScreenResize (&termw->screen,
			  replyWidth,
			  replyHeight,
			  &termw->flags);
	    XSync(screen->display, FALSE);	/* synchronize */
	    if(XtAppPending(app_con))
		xevents();
	}
}

extern Atom wm_delete_window;	/* for ICCCM delete window */

static String xterm_trans =
    "<ClientMessage>WM_PROTOCOLS: DeleteWindow()\n\
     <MappingNotify>: KeyboardMapping()\n";

int VTInit (void)
{
    register TScreen *screen = &term->screen;
    Widget vtparent = term->core.parent;

    XtRealizeWidget (vtparent);
    XtOverrideTranslations(vtparent, XtParseTranslationTable(xterm_trans));
    (void) XSetWMProtocols (XtDisplay(vtparent), XtWindow(vtparent),
			    &wm_delete_window, 1);

    if (screen->allbuf == NULL) VTallocbuf ();
    return (1);
}

static void VTallocbuf (void)
{
    register TScreen *screen = &term->screen;
    int nrows = screen->max_row + 1;

    /* allocate screen buffer now, if necessary. */
    if (screen->scrollWidget)
      nrows += screen->savelines;
    screen->allbuf = Allocate (nrows, screen->max_col + 1,
     &screen->sbuf_address);
    if (screen->scrollWidget)
      screen->visbuf = &screen->allbuf[MAX_PTRS * screen->savelines];
    else
      screen->visbuf = screen->allbuf;
    return;
}

static void VTClassInit (void)
{
    XtAddConverter(XtRString, XtRGravity, XmuCvtStringToGravity,
		   (XtConvertArgList) NULL, (Cardinal) 0);
}


/* ARGSUSED */
static void VTInitialize (
	Widget wrequest,
	Widget wnew,
	ArgList args GCC_UNUSED,
	Cardinal *num_args GCC_UNUSED)
{
   XtermWidget request = (XtermWidget) wrequest;
   XtermWidget new     = (XtermWidget) wnew;
   int i;
#if OPT_ISO_COLORS
   Boolean color_ok;
#endif

   /* Zero out the entire "screen" component of "new" widget, then do
    * field-by-field assigment of "screen" fields that are named in the
    * resource list.
    */
   bzero ((char *) &new->screen, sizeof(new->screen));

   /* dummy values so that we don't try to Realize the parent shell with height
    * or width of 0, which is illegal in X.  The real size is computed in the
    * xtermWidget's Realize proc, but the shell's Realize proc is called first,
    * and must see a valid size.
    */
   new->core.height = new->core.width = 1;

   /*
    * The definition of -rv now is that it changes the definition of
    * XtDefaultForeground and XtDefaultBackground.  So, we no longer
    * need to do anything special.
    */
   new->screen.display = new->core.screen->display;

   /*
    * We use the default foreground/background colors to compare/check if a
    * color-resource has been set.
    */
#define MyBlackPixel(dpy) BlackPixel(dpy,DefaultScreen(dpy))
#define MyWhitePixel(dpy) WhitePixel(dpy,DefaultScreen(dpy))

   if (request->misc.re_verse) {
	new->dft_foreground = MyWhitePixel(new->screen.display);
	new->dft_background = MyBlackPixel(new->screen.display);
   } else {
	new->dft_foreground = MyBlackPixel(new->screen.display);
	new->dft_background = MyWhitePixel(new->screen.display);
   }

   new->screen.c132 = request->screen.c132;
   new->screen.curses = request->screen.curses;
   new->screen.hp_ll_bc = request->screen.hp_ll_bc;
#if OPT_XMC_GLITCH
   new->screen.xmc_glitch = request->screen.xmc_glitch;
   new->screen.xmc_attributes = request->screen.xmc_attributes;
   new->screen.xmc_inline = request->screen.xmc_inline;
   new->screen.move_sgr_ok = request->screen.move_sgr_ok;
#endif
   new->screen.foreground = request->screen.foreground;
   new->screen.cursorcolor = request->screen.cursorcolor;
#if OPT_BLINK_CURS
   new->screen.cursor_blink = request->screen.cursor_blink;
#endif
   new->screen.border = request->screen.border;
   new->screen.jumpscroll = request->screen.jumpscroll;
   new->screen.old_fkeys = request->screen.old_fkeys;
#ifdef ALLOWLOGGING
   new->screen.logfile = request->screen.logfile;
#endif
   new->screen.marginbell = request->screen.marginbell;
   new->screen.mousecolor = request->screen.mousecolor;
   new->screen.mousecolorback = request->screen.mousecolorback;
   new->screen.multiscroll = request->screen.multiscroll;
   new->screen.nmarginbell = request->screen.nmarginbell;
   new->screen.savelines = request->screen.savelines;
   new->screen.scrolllines = request->screen.scrolllines;
   new->screen.scrollttyoutput = request->screen.scrollttyoutput;
   new->screen.scrollkey = request->screen.scrollkey;
   new->screen.terminal_id = request->screen.terminal_id;
   if (new->screen.terminal_id < MIN_DECID)
       new->screen.terminal_id = MIN_DECID;
   if (new->screen.terminal_id > MAX_DECID)
       new->screen.terminal_id = MAX_DECID;
   new->screen.ansi_level = (new->screen.terminal_id / 100);
   new->screen.visualbell = request->screen.visualbell;
#if OPT_TEK4014
   new->screen.TekEmu = request->screen.TekEmu;
#endif
   new->misc.re_verse = request->misc.re_verse;
   new->screen.multiClickTime = request->screen.multiClickTime;
   new->screen.bellSuppressTime = request->screen.bellSuppressTime;
   new->screen.charClass = request->screen.charClass;
   new->screen.cutNewline = request->screen.cutNewline;
   new->screen.cutToBeginningOfLine = request->screen.cutToBeginningOfLine;
   new->screen.highlight_selection = request->screen.highlight_selection;
   new->screen.always_highlight = request->screen.always_highlight;
   new->screen.pointer_cursor = request->screen.pointer_cursor;

   new->screen.printer_command = request->screen.printer_command;
   new->screen.printer_autoclose = request->screen.printer_autoclose;
   new->screen.printer_extent = request->screen.printer_extent;
   new->screen.printer_formfeed = request->screen.printer_formfeed;
   new->screen.printer_controlmode = request->screen.printer_controlmode;
#ifdef OPT_PRINT_COLORS
   new->screen.print_attributes = request->screen.print_attributes;
#endif

   new->screen.input_eight_bits = request->screen.input_eight_bits;
   new->screen.output_eight_bits = request->screen.output_eight_bits;
   new->screen.control_eight_bits = request->screen.control_eight_bits;
   new->screen.backarrow_key = request->screen.backarrow_key;
   new->screen.allowSendEvents = request->screen.allowSendEvents;
#ifndef NO_ACTIVE_ICON
   new->screen.fnt_icon = request->screen.fnt_icon;
#endif /* NO_ACTIVE_ICON */
   new->misc.titeInhibit = request->misc.titeInhibit;
   new->misc.dynamicColors = request->misc.dynamicColors;
   for (i = fontMenu_font1; i <= fontMenu_lastBuiltin; i++) {
       new->screen.menu_font_names[i] = request->screen.menu_font_names[i];
   }
   /* set default in realize proc */
   new->screen.menu_font_names[fontMenu_fontdefault] = NULL;
   new->screen.menu_font_names[fontMenu_fontescape] = NULL;
   new->screen.menu_font_names[fontMenu_fontsel] = NULL;
   new->screen.menu_font_number = fontMenu_fontdefault;

#if OPT_ISO_COLORS
   new->screen.boldColors    = request->screen.boldColors;
   new->screen.colorAttrMode = request->screen.colorAttrMode;
   new->screen.colorBDMode   = request->screen.colorBDMode;
   new->screen.colorBLMode   = request->screen.colorBLMode;
   new->screen.colorMode     = request->screen.colorMode;
   new->screen.colorULMode   = request->screen.colorULMode;

   for (i = 0, color_ok = False; i < MAXCOLORS; i++) {
       new->screen.Acolors[i] = request->screen.Acolors[i];
       if (new->screen.Acolors[i] != new->dft_foreground
        && new->screen.Acolors[i] != request->screen.foreground
        && new->screen.Acolors[i] != request->core.background_pixel)
	   color_ok = True;
   }

   /* If none of the colors are anything other than the foreground or
    * background, we'll assume this isn't color, no matter what the colorMode
    * resource says.  (There doesn't seem to be any good way to determine if
    * the resource lookup failed versus the user having misconfigured this).
    */
   if (!color_ok)
	new->screen.colorMode = False;

   new->num_ptrs = new->screen.colorMode ? 3 : 2;
   new->sgr_foreground = -1;
#endif /* OPT_ISO_COLORS */

#if OPT_HIGHLIGHT_COLOR
   new->screen.highlightcolor = request->screen.highlightcolor;
#endif

#if OPT_DEC_CHRSET
   new->num_ptrs = 5;
#endif

   new->screen.underline = request->screen.underline;

   new->cur_foreground = 0;
   new->cur_background = 0;

   new->keyboard.flags = MODE_SRM;
   if (new->screen.backarrow_key)
	   new->keyboard.flags |= MODE_DECBKM;

   /* look for focus related events on the shell, because we need
    * to care about the shell's border being part of our focus.
    */
   XtAddEventHandler(XtParent(new), EnterWindowMask, FALSE,
		HandleEnterWindow, (Opaque)NULL);
   XtAddEventHandler(XtParent(new), LeaveWindowMask, FALSE,
		HandleLeaveWindow, (Opaque)NULL);
   XtAddEventHandler(XtParent(new), FocusChangeMask, FALSE,
		HandleFocusChange, (Opaque)NULL);
   XtAddEventHandler((Widget)new, 0L, TRUE,
		VTNonMaskableEvent, (Opaque)NULL);
   XtAddEventHandler((Widget)new, PropertyChangeMask, FALSE,
		     HandleBellPropertyChange, (Opaque)NULL);

#if OPT_ZICONBEEP
   /* Flag icon name with "***"  on window output when iconified.
    * Put in a handler that will tell us when we get Map/Unmap events.
    */
   if ( zIconBeep )
       XtAddEventHandler(XtParent(new), StructureNotifyMask, FALSE,
			 HandleMapUnmap, (Opaque)NULL);
#endif /* OPT_ZICONBEEP */

   new->screen.bellInProgress = FALSE;

   set_character_class (new->screen.charClass);

   /* create it, but don't realize it */
   ScrollBarOn (new, TRUE, FALSE);

   /* make sure that the resize gravity acceptable */
   if ( new->misc.resizeGravity != NorthWestGravity &&
        new->misc.resizeGravity != SouthWestGravity) {
       Cardinal nparams = 1;

       XtAppWarningMsg(app_con, "rangeError", "resizeGravity", "XTermError",
		       "unsupported resizeGravity resource value (%d)",
		       (String *) &(new->misc.resizeGravity), &nparams);
       new->misc.resizeGravity = SouthWestGravity;
   }

#ifndef NO_ACTIVE_ICON
   new->screen.whichVwin = &new->screen.fullVwin;
#if OPT_TEK4014
   new->screen.whichTwin = &new->screen.fullTwin;
#endif
#endif /* NO_ACTIVE_ICON */

   return;
}


static void VTDestroy (Widget w)
{
    XtFree(((XtermWidget)w)->screen.selection);
}

/*ARGSUSED*/
static void VTRealize (
	Widget w,
	XtValueMask *valuemask,
	XSetWindowAttributes *values)
{
	unsigned int width, height;
	register TScreen *screen = &term->screen;
	int xpos, ypos, pr;
	XSizeHints		sizehints;
	int scrollbar_width;

	TabReset (term->tabs);

	screen->menu_font_names[fontMenu_fontdefault] = term->misc.f_n;
	screen->fnt_norm = screen->fnt_bold = NULL;
	if (!LoadNewFont(screen, term->misc.f_n, term->misc.f_b, False, 0)) {
	    if (XmuCompareISOLatin1(term->misc.f_n, "fixed") != 0) {
		fprintf (stderr,
		     "%s:  unable to open font \"%s\", trying \"fixed\"....\n",
		     xterm_name, term->misc.f_n);
		(void) LoadNewFont (screen, "fixed", NULL, False, 0);
		screen->menu_font_names[fontMenu_fontdefault] = "fixed";
	    }
	}

	/* really screwed if we couldn't open default font */
	if (!screen->fnt_norm) {
	    fprintf (stderr, "%s:  unable to locate a suitable font\n",
		     xterm_name);
	    Exit (1);
	}

	/* making cursor */
	if (!screen->pointer_cursor)
	  screen->pointer_cursor = make_colored_cursor(XC_xterm,
						       screen->mousecolor,
						       screen->mousecolorback);
	else
	  recolor_cursor (screen->pointer_cursor,
			  screen->mousecolor, screen->mousecolorback);

	scrollbar_width = (term->misc.scrollbar ?
			   screen->scrollWidget->core.width /* +
			   screen->scrollWidget->core.border_width */ : 0);

	/* set defaults */
	xpos = 1; ypos = 1; width = 80; height = 24;
	pr = XParseGeometry (term->misc.geo_metry, &xpos, &ypos,
			     &width, &height);
	screen->max_col = (width - 1);	/* units in character cells */
	screen->max_row = (height - 1);	/* units in character cells */
	update_font_info (&term->screen, False);

	width = screen->fullVwin.fullwidth;
	height = screen->fullVwin.fullheight;

	if ((pr & XValue) && (XNegative&pr))
	  xpos += DisplayWidth(screen->display, DefaultScreen(screen->display))
			- width - (term->core.parent->core.border_width * 2);
	if ((pr & YValue) && (YNegative&pr))
	  ypos += DisplayHeight(screen->display,DefaultScreen(screen->display))
			- height - (term->core.parent->core.border_width * 2);

	/* set up size hints for window manager; min 1 char by 1 char */
	sizehints.base_width = 2 * screen->border + scrollbar_width;
	sizehints.base_height = 2 * screen->border;
	sizehints.width_inc = FontWidth(screen);
	sizehints.height_inc = FontHeight(screen);
	sizehints.min_width = sizehints.base_width + sizehints.width_inc;
	sizehints.min_height = sizehints.base_height + sizehints.height_inc;
	sizehints.flags = (PBaseSize|PMinSize|PResizeInc);
	sizehints.x = xpos;
	sizehints.y = ypos;
	if ((XValue&pr) || (YValue&pr)) {
	    sizehints.flags |= USSize|USPosition;
	    sizehints.flags |= PWinGravity;
	    switch (pr & (XNegative | YNegative)) {
	      case 0:
		sizehints.win_gravity = NorthWestGravity;
		break;
	      case XNegative:
		sizehints.win_gravity = NorthEastGravity;
		break;
	      case YNegative:
		sizehints.win_gravity = SouthWestGravity;
		break;
	      default:
		sizehints.win_gravity = SouthEastGravity;
		break;
	    }
	} else {
	    /* set a default size, but do *not* set position */
	    sizehints.flags |= PSize;
	}
	sizehints.width = width;
	sizehints.height = height;
	if ((WidthValue&pr) || (HeightValue&pr))
	  sizehints.flags |= USSize;
	else sizehints.flags |= PSize;

	(void) XtMakeResizeRequest((Widget) term,
				   (Dimension)width, (Dimension)height,
				   &term->core.width, &term->core.height);

	/* XXX This is bogus.  We are parsing geometries too late.  This
	 * is information that the shell widget ought to have before we get
	 * realized, so that it can do the right thing.
	 */
        if (sizehints.flags & USPosition)
	    XMoveWindow (XtDisplay(term), term->core.parent->core.window,
			 sizehints.x, sizehints.y);

	XSetWMNormalHints (XtDisplay(term), term->core.parent->core.window,
			   &sizehints);
	XFlush (XtDisplay(term));	/* get it out to window manager */

	/* use ForgetGravity instead of SouthWestGravity because translating
	   the Expose events for ConfigureNotifys is too hard */
	values->bit_gravity = term->misc.resizeGravity == NorthWestGravity ?
	    NorthWestGravity : ForgetGravity;
	term->screen.fullVwin.window = term->core.window =
	  XCreateWindow(XtDisplay(term), XtWindow(term->core.parent),
		term->core.x, term->core.y,
		term->core.width, term->core.height, term->core.border_width,
		(int) term->core.depth,
		InputOutput, CopyFromParent,
		*valuemask|CWBitGravity, values);

#ifndef NO_ACTIVE_ICON
	if (term->misc.active_icon && screen->fnt_icon) {
	    int iconX=0, iconY=0;
	    Widget shell = term->core.parent;
	    unsigned long mask;
	    XGCValues xgcv;

	    XtVaGetValues(shell, XtNiconX, &iconX, XtNiconY, &iconY, NULL);
	    screen->iconVwin.f_width = screen->fnt_icon->max_bounds.width;
	    screen->iconVwin.f_height = (screen->fnt_icon->ascent +
					 screen->fnt_icon->descent);

	    screen->iconVwin.width =
		(screen->max_col + 1) * screen->iconVwin.f_width;
	    screen->iconVwin.fullwidth = screen->iconVwin.width +
		2 * screen->border;

	    screen->iconVwin.height =
		(screen->max_row + 1) * screen->iconVwin.f_height;
	    screen->iconVwin.fullheight = screen->iconVwin.height +
		2 * screen->border;

	    /* since only one client is permitted to select for Button
	     * events, we have to let the window manager get 'em...
	     */
	    values->event_mask &= ~(ButtonPressMask|ButtonReleaseMask);
	    values->border_pixel = term->misc.icon_border_pixel;

	    screen->iconVwin.window =
		XCreateWindow(XtDisplay(term),
			      RootWindowOfScreen(XtScreen(shell)),
			      iconX, iconY,
			      screen->iconVwin.fullwidth,
			      screen->iconVwin.fullheight,
			      term->misc.icon_border_width,
			      (int) term->core.depth,
			      InputOutput, CopyFromParent,
			      *valuemask|CWBitGravity|CWBorderPixel,
			      values);
	    XtVaSetValues(shell, XtNiconWindow, screen->iconVwin.window, NULL);
	    XtRegisterDrawable(XtDisplay(term), screen->iconVwin.window, w);

	    mask = (GCFont | GCForeground | GCBackground |
		    GCGraphicsExposures | GCFunction);

	    xgcv.font = screen->fnt_icon->fid;
	    xgcv.foreground = screen->foreground;
	    xgcv.background = term->core.background_pixel;
	    xgcv.graphics_exposures = TRUE;	/* default */
	    xgcv.function = GXcopy;

	    screen->iconVwin.normalGC =
		screen->iconVwin.normalboldGC =
		    XtGetGC(shell, mask, &xgcv);

	    xgcv.foreground = term->core.background_pixel;
	    xgcv.background = screen->foreground;

	    screen->iconVwin.reverseGC =
		screen->iconVwin.reverseboldGC =
		    XtGetGC(shell, mask, &xgcv);
	}
	else {
	    term->misc.active_icon = False;
	}
#endif /* NO_ACTIVE_ICON */

#if OPT_I18N_SUPPORT && OPT_INPUT_METHOD
	VTInitI18N();
#else
	term->screen.xic = NULL;
#endif

	set_cursor_gcs (screen);

	/* Reset variables used by ANSI emulation. */

	resetCharsets(screen);

	XDefineCursor(screen->display, VShellWindow, screen->pointer_cursor);

        screen->cur_col = screen->cur_row = 0;
	screen->max_col = Width(screen)/screen->fullVwin.f_width - 1;
	screen->top_marg = 0;
	screen->bot_marg = screen->max_row = Height(screen) /
				screen->fullVwin.f_height - 1;

	screen->sc.row = screen->sc.col = screen->sc.flags = 0;

	/* Mark screen buffer as unallocated.  We wait until the run loop so
	   that the child process does not fork and exec with all the dynamic
	   memory it will never use.  If we were to do it here, the
	   swap space for new process would be huge for huge savelines. */
#if OPT_TEK4014
	if (!tekWidget)			/* if not called after fork */
#endif
	  screen->visbuf = screen->allbuf = NULL;

	screen->do_wrap = 0;
	screen->scrolls = screen->incopy = 0;
	set_vt_box (screen);

	screen->savedlines = 0;

	if (term->misc.scrollbar) {
		screen->fullVwin.scrollbar = 0;
		ScrollBarOn (term, FALSE, TRUE);
	}
	CursorSave (term, &screen->sc);
	return;
}

#if OPT_I18N_SUPPORT && OPT_INPUT_METHOD
static void VTInitI18N(void)
{
    unsigned	i;
    char       *p,
	       *s,
	       *t,
	       *ns,
	       *end,
	  	buf[32];
    XIM		xim = (XIM) NULL;
    XIMStyles  *xim_styles;
    XIMStyle	input_style = 0;
    Boolean	found;

    term->screen.xic = NULL;

    if (!term->misc.open_im) return;

    if (!term->misc.input_method || !*term->misc.input_method) {
	if ((p = XSetLocaleModifiers("@im=none")) != NULL && *p)
	    xim = XOpenIM(XtDisplay(term), NULL, NULL, NULL);
    } else {
	s = term->misc.input_method;
	i = 5 + strlen(s);
	t = MyStackAlloc(i, buf);
	if (t == NULL)
	    SysError(ERROR_VINIT);

	for(ns = s; ns && *s;) {
	    while (*s && isspace(*s)) s++;
	    if (!*s) break;
	    if ((ns = end = strchr(s, ',')) == 0)
		end = s + strlen(s);
	    while ((end != s) && isspace(end[-1])) end--;

	    if (end != s) {
		strcpy(t, "@im=");
		strncat(t, s, end - s);

		if ((p = XSetLocaleModifiers(t)) != 0 && *p
		    && (xim = XOpenIM(XtDisplay(term), NULL, NULL, NULL)) != 0)
		    break;

	    }
	    s = ns + 1;
	}
	MyStackFree(t, buf);
    }

    if (xim == NULL && (p = XSetLocaleModifiers("@im=none")) != NULL && *p)
	xim = XOpenIM(XtDisplay(term), NULL, NULL, NULL);

    if (!xim) {
	fprintf(stderr, "Failed to open input method\n");
	return;
    }

    if (XGetIMValues(xim, XNQueryInputStyle, &xim_styles, NULL)
        || !xim_styles) {
	fprintf(stderr, "input method doesn't support any style\n");
        XCloseIM(xim);
        return;
    }

    found = False;
    for(s = term->misc.preedit_type; s && !found;) {
	while (*s && isspace(*s)) s++;
	if (!*s) break;
	if ((ns = end = strchr(s, ',')) != 0)
	    ns++;
	else
	    end = s + strlen(s);
	while ((end != s) && isspace(end[-1])) end--;

	if (end != s) {	/* just in case we have a spurious comma */
	    if (!strncmp(s, "OverTheSpot", end - s)) {
		input_style = (XIMPreeditPosition | XIMStatusArea);
	    } else if (!strncmp(s, "OffTheSpot", end - s)) {
		input_style = (XIMPreeditArea | XIMStatusArea);
	    } else if (!strncmp(s, "Root", end - s)) {
		input_style = (XIMPreeditNothing | XIMStatusNothing);
	    }
	    for (i = 0; (unsigned short)i < xim_styles->count_styles; i++) {
		if (input_style == xim_styles->supported_styles[i]) {
		    found = True;
		    break;
		}
	    }
	}

	s = ns;
    }
    XFree(xim_styles);

    if (!found) {
	fprintf(stderr, "input method doesn't support my preedit type\n");
	XCloseIM(xim);
	return;
    }

    /*
     * This program only understands the Root preedit_style yet
     * Then misc.preedit_type should default to:
     *		"OverTheSpot,OffTheSpot,Root"
     *
     *	/MaF
     */
    if (input_style != (XIMPreeditNothing | XIMStatusNothing)) {
	fprintf(stderr,"This program only supports the 'Root' preedit type\n");
	XCloseIM(xim);
	return;
    }

    term->screen.xic = XCreateIC(xim, XNInputStyle, input_style,
				      XNClientWindow, term->core.window,
				      XNFocusWindow, term->core.window,
				      NULL);

    if (!term->screen.xic) {
	fprintf(stderr,"Failed to create input context\n");
	XCloseIM(xim);
    }

    return;
}
#endif


static Boolean VTSetValues (
	Widget cur,
	Widget request GCC_UNUSED,
	Widget new,
	ArgList args GCC_UNUSED,
	Cardinal *num_args GCC_UNUSED)
{
    XtermWidget curvt = (XtermWidget) cur;
    XtermWidget newvt = (XtermWidget) new;
    Boolean refresh_needed = FALSE;
    Boolean fonts_redone = FALSE;

    if(curvt->core.background_pixel != newvt->core.background_pixel
       || curvt->screen.foreground != newvt->screen.foreground
       || curvt->screen.menu_font_names[curvt->screen.menu_font_number]
          != newvt->screen.menu_font_names[newvt->screen.menu_font_number]
       || curvt->misc.f_n != newvt->misc.f_n) {
	if(curvt->misc.f_n != newvt->misc.f_n)
	    newvt->screen.menu_font_names[fontMenu_fontdefault] = newvt->misc.f_n;
	if (LoadNewFont(&newvt->screen,
			newvt->screen.menu_font_names[curvt->screen.menu_font_number],
			newvt->screen.menu_font_names[curvt->screen.menu_font_number],
			TRUE, newvt->screen.menu_font_number)) {
	    /* resizing does the redisplay, so don't ask for it here */
	    refresh_needed = TRUE;
	    fonts_redone = TRUE;
	} else
	    if(curvt->misc.f_n != newvt->misc.f_n)
		newvt->screen.menu_font_names[fontMenu_fontdefault] = curvt->misc.f_n;
    }
    if(!fonts_redone
       && curvt->screen.cursorcolor != newvt->screen.cursorcolor) {
	set_cursor_gcs(&newvt->screen);
	refresh_needed = TRUE;
    }
    if(curvt->misc.re_verse != newvt->misc.re_verse) {
	newvt->flags ^= REVERSE_VIDEO;
	ReverseVideo(newvt);
	newvt->misc.re_verse = !newvt->misc.re_verse; /* ReverseVideo toggles */
	refresh_needed = TRUE;
    }
    if(curvt->screen.mousecolor != newvt->screen.mousecolor
       || curvt->screen.mousecolorback != newvt->screen.mousecolorback) {
	recolor_cursor (newvt->screen.pointer_cursor,
			newvt->screen.mousecolor,
			newvt->screen.mousecolorback);
	refresh_needed = TRUE;
    }
    if (curvt->misc.scrollbar != newvt->misc.scrollbar) {
	if (newvt->misc.scrollbar) {
	    ScrollBarOn (newvt, FALSE, FALSE);
	} else {
	    ScrollBarOff (&newvt->screen);
	}
	update_scrollbar();
    }

    return refresh_needed;
}

/*
 * Shows cursor at new cursor position in screen.
 */
void
ShowCursor(void)
{
	register TScreen *screen = &term->screen;
	register int x, y, flags;
	Char	c;
	Char	fg_bg = 0;
	GC	currentGC;
	Boolean	in_selection;
	Pixel fg_pix;
	Pixel bg_pix;
	Pixel tmp;
#if OPT_HIGHLIGHT_COLOR
	Pixel hi_pix = screen->highlightcolor;
#endif

	if (screen->cursor_state == BLINKED_OFF)
		return;

	if (eventMode != NORMAL) return;

	if (screen->cur_row - screen->topline > screen->max_row)
		return;

	screen->cursor_row = screen->cur_row;
	screen->cursor_col = screen->cur_col;

	c     = SCRN_BUF_CHARS(screen, screen->cursor_row)[screen->cursor_col];
	flags = SCRN_BUF_ATTRS(screen, screen->cursor_row)[screen->cursor_col];

#ifndef NO_ACTIVE_ICON
	if (IsIcon(screen)) {
	    screen->cursor_state = ON;
	    return;
	}
#endif /* NO_ACTIVE_ICON */

	if (c == 0)
		c = ' ';

	/*
	 * Compare the current cell to the last set of colors used for the
	 * cursor and update the GC's if needed.
	 */
#if OPT_ISO_COLORS
	if_OPT_ISO_COLORS(screen,{
	    fg_bg = SCRN_BUF_COLOR(screen, screen->cursor_row)[screen->cursor_col];
	})
#endif
	fg_pix = getXtermForeground(flags,extract_fg(fg_bg,flags));
	bg_pix = getXtermBackground(flags,extract_bg(fg_bg));

	if (screen->cur_row > screen->endHRow ||
	    (screen->cur_row == screen->endHRow &&
	     screen->cur_col >= screen->endHCol) ||
	    screen->cur_row < screen->startHRow ||
	    (screen->cur_row == screen->startHRow &&
	     screen->cur_col < screen->startHCol))
	    in_selection = False;
	else
	    in_selection = True;

	/* This is like updatedXtermGC(), except that we have to worry about
	 * whether the window has focus, since in that case we want just an
	 * outline for the cursor.
	 */
	if(screen->select || screen->always_highlight) {
		if (( (flags & INVERSE) && !in_selection) ||
		    (!(flags & INVERSE) &&  in_selection)){
		    /* text is reverse video */
		    if (screen->cursorGC) {
			currentGC = screen->cursorGC;
		    } else {
			if (flags & (BOLD|BLINK)) {
				currentGC = NormalBoldGC(screen);
			} else {
				currentGC = NormalGC(screen);
			}
		    }
#if OPT_HIGHLIGHT_COLOR
		    if (hi_pix != screen->foreground
		     && hi_pix != fg_pix
		     && hi_pix != bg_pix
		     && hi_pix != term->dft_foreground) {
			bg_pix = fg_pix;
			fg_pix = hi_pix;
		    }
#endif
		    EXCHANGE(fg_pix, bg_pix, tmp)
		} else { /* normal video */
		    if (screen->reversecursorGC) {
			currentGC = screen->reversecursorGC;
		    } else {
			if (flags & (BOLD|BLINK)) {
				currentGC = ReverseBoldGC(screen);
			} else {
				currentGC = ReverseGC(screen);
			}
		    }
		}
		if (screen->cursorcolor == term->dft_foreground) {
			XSetForeground(screen->display, currentGC, bg_pix);
			XSetBackground(screen->display, currentGC, fg_pix);
		}
	} else { /* not selected */
		if (( (flags & INVERSE) && !in_selection) ||
		    (!(flags & INVERSE) &&  in_selection)) {
		    /* text is reverse video */
			currentGC = ReverseGC(screen);
		} else { /* normal video */
			currentGC = NormalGC(screen);
		}
		if (screen->cursorcolor == term->dft_foreground) {
			XSetForeground(screen->display, currentGC, fg_pix);
			XSetBackground(screen->display, currentGC, bg_pix);
		}
	}

	TRACE(("%s @%d, calling drawXtermText\n", __FILE__, __LINE__))

	drawXtermText(screen, flags, currentGC,
		x = CurCursorX(screen, screen->cur_row, screen->cur_col),
		y = CursorY(screen, screen->cur_row),
		curXtermChrSet(screen->cur_row),
		&c, 1);

	if (!screen->select && !screen->always_highlight) {
		screen->box->x = x;
		screen->box->y = y;
		XDrawLines (screen->display, TextWindow(screen),
			    screen->cursoroutlineGC ? screen->cursoroutlineGC
			    			    : currentGC,
			    screen->box, NBOX, CoordModePrevious);
	}
	screen->cursor_state = ON;
}

/*
 * hide cursor at previous cursor position in screen.
 */
void
HideCursor(void)
{
	register TScreen *screen = &term->screen;
	GC	currentGC;
	register int flags, fg_bg = 0;
	Char c;
	Boolean	in_selection;

	if (screen->cursor_state == OFF)	/* FIXME */
		return;
	if(screen->cursor_row - screen->topline > screen->max_row)
		return;

	c     = SCRN_BUF_CHARS(screen, screen->cursor_row)[screen->cursor_col];
	flags = SCRN_BUF_ATTRS(screen, screen->cursor_row)[screen->cursor_col];

	if_OPT_ISO_COLORS(screen,{
	    fg_bg = SCRN_BUF_COLOR(screen, screen->cursor_row)[screen->cursor_col];
	})

#ifndef NO_ACTIVE_ICON
	if (IsIcon(screen)) {
	    screen->cursor_state = OFF;
	    return;
	}
#endif /* NO_ACTIVE_ICON */


	if (screen->cursor_row > screen->endHRow ||
	    (screen->cursor_row == screen->endHRow &&
	     screen->cursor_col >= screen->endHCol) ||
	    screen->cursor_row < screen->startHRow ||
	    (screen->cursor_row == screen->startHRow &&
	     screen->cursor_col < screen->startHCol))
	    in_selection = False;
	else
	    in_selection = True;

	currentGC = updatedXtermGC(screen, flags, fg_bg, in_selection);

	if (c == 0)
		c = ' ';

	TRACE(("%s @%d, calling drawXtermText\n", __FILE__, __LINE__))
	drawXtermText(screen, flags, currentGC,
		CursorX(screen, screen->cursor_col),
		CursorY(screen, screen->cursor_row),
		curXtermChrSet(screen->cur_row),
		&c, 1);

	screen->cursor_state = OFF;
	resetXtermGC(screen, flags, in_selection);
}

#if OPT_BLINK_CURS
static void
StartBlinking(TScreen *screen)
{
	if (screen->cursor_blink > 0
	 && screen->cursor_timer == 0) {
		unsigned long half = screen->cursor_blink / 2;
		if (half == 0)		/* wow! */
			half = 1;	/* let's humor him anyway */
		screen->cursor_timer = XtAppAddTimeOut(
			app_con,
			half,
			BlinkCursor,
			screen);
	}
}

static void
StopBlinking(TScreen *screen)
{
	if (screen->cursor_blink > 0)
		XtRemoveTimeOut(screen->cursor_timer);
	screen->cursor_timer = 0;
}

/*
 * Blink the cursor by alternately showing/hiding cursor.  We leave the timer
 * running all the time (even though that's a little inefficient) to make the
 * logic simple.
 */
static void
BlinkCursor(XtPointer closure, XtIntervalId* id)
{
	TScreen *screen = (TScreen *)closure;

	screen->cursor_timer = 0;
	if (screen->cursor_state == ON) {
		if(screen->select || screen->always_highlight) {
			HideCursor();
			if (screen->cursor_state == OFF)
				screen->cursor_state = BLINKED_OFF;
		}
	} else if (screen->cursor_state == BLINKED_OFF) {
		screen->cursor_state = OFF;
		ShowCursor();
		if (screen->cursor_state == OFF)
			screen->cursor_state = BLINKED_OFF;
	}
	StartBlinking(screen);
	xevents();
}
#endif /* OPT_BLINK_CURS */

/*
 * Implement soft or hard (full) reset of the VTxxx emulation.  There are a
 * couple of differences from real DEC VTxxx terminals (to avoid breaking
 * applications which have come to rely on xterm doing this):
 *
 *	+ autowrap mode should be reset (instead it's reset to the resource
 *	  default).
 *	+ the popup menu offers a choice of resetting the savedLines, or not.
 *	  (but the control sequence does this anyway).
 */
void
VTReset(Bool full, Bool saved)
{
	register TScreen *screen = &term->screen;

	if (saved) {
		screen->savedlines = 0;
		ScrollBarDrawThumb(screen->scrollWidget);
	}

	/* make cursor visible */
	screen->cursor_set = ON;

	/* reset scrolling region */
	screen->top_marg = 0;
	screen->bot_marg = screen->max_row;

	bitclr(&term->flags, ORIGIN);

	if_OPT_ISO_COLORS(screen,{reset_SGR_Colors();})

	/* Reset character-sets to initial state */
	resetCharsets(screen);

	/* Reset DECSCA */
	bitclr(&term->flags, PROTECTED);
	screen->protected_mode = OFF_PROTECT;

	if (full) {	/* RIS */
		TabReset (term->tabs);
		term->keyboard.flags = MODE_SRM;
		if (term->screen.backarrow_key)
			term->keyboard.flags |= MODE_DECBKM;
		update_appcursor();
		update_appkeypad();
		update_decbkm();
		show_8bit_control(False);
		reset_decudk();

		FromAlternate(screen);
		ClearScreen(screen);
		screen->cursor_state = OFF;
		if (term->flags & REVERSE_VIDEO)
			ReverseVideo(term);

		term->flags = term->initflags;
		update_reversevideo();
		update_autowrap();
		update_reversewrap();
		update_autolinefeed();

		screen->jumpscroll = !(term->flags & SMOOTHSCROLL);
		update_jumpscroll();

		if(screen->c132 && (term->flags & IN132COLUMNS)) {
		        Dimension junk;
			XtMakeResizeRequest(
			    (Widget) term,
			    (Dimension) 80*FontWidth(screen)
				+ 2 * screen->border + Scrollbar(screen),
			    (Dimension) FontHeight(screen)
			        * (screen->max_row + 1) + 2 * screen->border,
			    &junk, &junk);
			XSync(screen->display, FALSE);	/* synchronize */
			if(XtAppPending(app_con))
				xevents();
		}
		CursorSet(screen, 0, 0, term->flags);
		CursorSave(term, &screen->sc);
	} else {	/* DECSTR */
		/*
		 * There's a tiny difference, to accommodate usage of xterm.
		 * We reset autowrap to the resource values rather than turning
		 * it off.
		 *
		 * FIXME: also reset DECNKM when it's implemented.
		 */
		term->keyboard.flags &= ~(MODE_DECCKM|MODE_KAM);
		bitcpy(&term->flags, term->initflags, WRAPAROUND|REVERSEWRAP);
		bitclr(&term->flags, INSERT|INVERSE|BOLD|BLINK|UNDERLINE|INVISIBLE);
		if_OPT_ISO_COLORS(screen,{reset_SGR_Colors();})
		update_appcursor();
		update_autowrap();
		update_reversewrap();

		CursorSave(term, &screen->sc);
		screen->sc.row =
		screen->sc.col = 0;
	}
	longjmp(vtjmpbuf, 1);	/* force ground state in parser */
}



/*
 * set_character_class - takes a string of the form
 *
 *                 low[-high]:val[,low[-high]:val[...]]
 *
 * and sets the indicated ranges to the indicated values.
 */
static int
set_character_class (register char *s)
{
    register int i;			/* iterator, index into s */
    int len;				/* length of s */
    int acc;				/* accumulator */
    int low, high;			/* bounds of range [0..127] */
    int base;				/* 8, 10, 16 (octal, decimal, hex) */
    int numbers;			/* count of numbers per range */
    int digits;				/* count of digits in a number */
    static char *errfmt = "%s:  %s in range string \"%s\" (position %d)\n";

    if (!s || !s[0]) return -1;

    base = 10;				/* in case we ever add octal, hex */
    low = high = -1;			/* out of range */

    for (i = 0, len = strlen (s), acc = 0, numbers = digits = 0;
	 i < len; i++) {
	char c = s[i];

	if (isspace(c)) {
	    continue;
	} else if (isdigit(c)) {
	    acc = acc * base + (c - '0');
	    digits++;
	    continue;
	} else if (c == '-') {
	    low = acc;
	    acc = 0;
	    if (digits == 0) {
		fprintf (stderr, errfmt, ProgramName, "missing number", s, i);
		return (-1);
	    }
	    digits = 0;
	    numbers++;
	    continue;
	} else if (c == ':') {
	    if (numbers == 0)
	      low = acc;
	    else if (numbers == 1)
	      high = acc;
	    else {
		fprintf (stderr, errfmt, ProgramName, "too many numbers",
			 s, i);
		return (-1);
	    }
	    digits = 0;
	    numbers++;
	    acc = 0;
	    continue;
	} else if (c == ',') {
	    /*
	     * now, process it
	     */

	    if (high < 0) {
		high = low;
		numbers++;
	    }
	    if (numbers != 2) {
		fprintf (stderr, errfmt, ProgramName, "bad value number",
			 s, i);
	    } else if (SetCharacterClassRange (low, high, acc) != 0) {
		fprintf (stderr, errfmt, ProgramName, "bad range", s, i);
	    }

	    low = high = -1;
	    acc = 0;
	    digits = 0;
	    numbers = 0;
	    continue;
	} else {
	    fprintf (stderr, errfmt, ProgramName, "bad character", s, i);
	    return (-1);
	}				/* end if else if ... else */

    }

    if (low < 0 && high < 0) return (0);

    /*
     * now, process it
     */

    if (high < 0) high = low;
    if (numbers < 1 || numbers > 2) {
	fprintf (stderr, errfmt, ProgramName, "bad value number", s, i);
    } else if (SetCharacterClassRange (low, high, acc) != 0) {
	fprintf (stderr, errfmt, ProgramName, "bad range", s, i);
    }

    return (0);
}

/* ARGSUSED */
static void HandleKeymapChange(
	Widget w,
	XEvent *event GCC_UNUSED,
	String *params,
	Cardinal *param_count)
{
    static XtTranslations keymap, original;
    static XtResource key_resources[] = {
	{ XtNtranslations, XtCTranslations, XtRTranslationTable,
	      sizeof(XtTranslations), 0, XtRTranslationTable, (XtPointer)NULL}
    };
    char mapName[1000];
    char mapClass[1000];
    char* pmapName;
    char* pmapClass;
    size_t len;

    if (*param_count != 1) return;

    if (original == NULL) original = w->core.tm.translations;

    if (strcmp(params[0], "None") == 0) {
	XtOverrideTranslations(w, original);
	return;
    }

    len = strlen (params[0]) + 7;

    pmapName  = MyStackAlloc(len, mapName);
    pmapClass = MyStackAlloc(len, mapClass);
    if (pmapName == NULL
     || pmapClass == NULL)
	SysError(ERROR_KMMALLOC1);

    (void) sprintf( pmapName, "%sKeymap", params[0] );
    (void) strcpy( pmapClass, pmapName );
    if (islower(pmapClass[0])) pmapClass[0] = toupper(pmapClass[0]);
    XtGetSubresources( w, (XtPointer)&keymap, pmapName, pmapClass,
		       key_resources, (Cardinal)1, NULL, (Cardinal)0 );
    if (keymap != NULL)
	XtOverrideTranslations(w, keymap);

    MyStackFree(pmapName,  mapName);
    MyStackFree(pmapClass, mapClass);
}


/* ARGSUSED */
static void HandleBell(
	Widget w,
	XEvent *event GCC_UNUSED,
	String *params,		/* [0] = volume */
	Cardinal *param_count)	/* 0 or 1 */
{
    int percent = (*param_count) ? atoi(params[0]) : 0;

#ifdef XKB
    int which= XkbBI_TerminalBell;
    XkbStdBell(XtDisplay(w),XtWindow(w),percent,which);
#else
    XBell( XtDisplay(w), percent );
#endif
}


/* ARGSUSED */
static void HandleVisualBell(
	Widget w GCC_UNUSED,
	XEvent *event GCC_UNUSED,
	String *params GCC_UNUSED,
	Cardinal *param_count GCC_UNUSED)
{
    VisualBell();
}


/* ARGSUSED */
static void HandleIgnore(
	Widget w,
	XEvent *event,
	String *params GCC_UNUSED,
	Cardinal *param_count GCC_UNUSED)
{
    /* do nothing, but check for funny escape sequences */
    (void) SendMousePosition(w, event);
}


/* ARGSUSED */
static void
DoSetSelectedFont(
	Widget w GCC_UNUSED,
	XtPointer client_data GCC_UNUSED,
	Atom *selection GCC_UNUSED,
	Atom *type,
	XtPointer value,
	unsigned long *length GCC_UNUSED,
	int *format)
{
    char *val = (char *)value;
    int len;
    if (*type != XA_STRING  ||  *format != 8) {
	Bell(XkbBI_MinorError,0);
	return;
    }
    len = strlen(val);
    if (len > 0) {
	if (val[len-1] == '\n') val[len-1] = '\0';
	/* Do some sanity checking to avoid sending a long selection
	   back to the server in an OpenFont that is unlikely to succeed.
	   XLFD allows up to 255 characters and no control characters;
	   we are a little more liberal here. */
	if (len > 1000  ||  strchr(val, '\n'))
	    return;
	if (!LoadNewFont (&term->screen, val, NULL, True, fontMenu_fontsel))
	    Bell(XkbBI_MinorError,0);
    }
}

void FindFontSelection (char *atom_name, Bool justprobe)
{
    static AtomPtr *atoms;
    static int atomCount = 0;
    AtomPtr *pAtom;
    int a;
    Atom target;

    if (!atom_name) atom_name = "PRIMARY";

    for (pAtom = atoms, a = atomCount; a; a--, pAtom++) {
	if (strcmp(atom_name, XmuNameOfAtom(*pAtom)) == 0) break;
    }
    if (!a) {
	atoms = (AtomPtr*) XtRealloc ((char *)atoms,
				      sizeof(AtomPtr)*(atomCount+1));
	*(pAtom = &atoms[atomCount++]) = XmuMakeAtom(atom_name);
    }

    target = XmuInternAtom(XtDisplay(term), *pAtom);
    if (justprobe) {
	term->screen.menu_font_names[fontMenu_fontsel] =
	  XGetSelectionOwner(XtDisplay(term), target) ? _Font_Selected_ : NULL;
    } else {
	XtGetSelectionValue((Widget)term, target, XA_STRING,
			    DoSetSelectedFont, NULL,
			    XtLastTimestampProcessed(XtDisplay(term)));
    }
    return;
}


/* ARGSUSED */
static void
HandleSetFont(
	Widget w GCC_UNUSED,
	XEvent *event GCC_UNUSED,
	String *params,
	Cardinal *param_count)
{
    int fontnum;
    char *name1 = NULL, *name2 = NULL;

    if (*param_count == 0) {
	fontnum = fontMenu_fontdefault;
    } else {
	Cardinal maxparams = 1;		/* total number of params allowed */

	switch (params[0][0]) {
	  case 'd': case 'D': case '0':
	    fontnum = fontMenu_fontdefault; break;
	  case '1':
	    fontnum = fontMenu_font1; break;
	  case '2':
	    fontnum = fontMenu_font2; break;
	  case '3':
	    fontnum = fontMenu_font3; break;
	  case '4':
	    fontnum = fontMenu_font4; break;
	  case '5':
	    fontnum = fontMenu_font5; break;
	  case '6':
	    fontnum = fontMenu_font6; break;
	  case 'e': case 'E':
	    fontnum = fontMenu_fontescape; maxparams = 3; break;
	  case 's': case 'S':
	    fontnum = fontMenu_fontsel; maxparams = 2; break;
	  default:
	    Bell(XkbBI_MinorError,0);
	    return;
	}
	if (*param_count > maxparams) {	 /* see if extra args given */
	    Bell(XkbBI_MinorError,0);
	    return;
	}
	switch (*param_count) {		/* assign 'em */
	  case 3:
	    name2 = params[2];
	    /* FALLTHRU */
	  case 2:
	    name1 = params[1];
	    break;
	}
    }

    SetVTFont (fontnum, True, name1, name2);
}


void SetVTFont (
	int i,
	Bool doresize,
	char *name1,
	char *name2)
{
    TScreen *screen = &term->screen;

    if (i < 0 || i >= NMENUFONTS) {
	Bell(XkbBI_MinorError,0);
	return;
    }
    if (i == fontMenu_fontsel) {	/* go get the selection */
	FindFontSelection (name1, False);  /* name1 = atom, name2 is ignored */
	return;
    }
    if (!name1) name1 = screen->menu_font_names[i];
    if (!LoadNewFont(screen, name1, name2, doresize, i)) {
	Bell(XkbBI_MinorError,0);
    }
    return;
}

static int
LoadNewFont (
	TScreen *screen,
	char *nfontname,
	char *bfontname,
	Bool doresize,
	int fontnum)
{
    XFontStruct *nfs = NULL, *bfs = NULL;
    XGCValues xgcv;
    unsigned long mask;
    GC new_normalGC = NULL, new_normalboldGC = NULL;
    GC new_reverseGC = NULL, new_reverseboldGC = NULL;
    Pixel new_normal, new_revers;
    char *tmpname = NULL;
    Boolean proportional = False;

    if (!nfontname) return 0;

    if (fontnum == fontMenu_fontescape &&
	nfontname != screen->menu_font_names[fontnum]) {
	tmpname = (char *) malloc (strlen(nfontname) + 1);
	if (!tmpname) return 0;
	strcpy (tmpname, nfontname);
    }

    if (!(nfs = XLoadQueryFont (screen->display, nfontname))) goto bad;
    if (nfs->ascent + nfs->descent == 0  ||  nfs->max_bounds.width == 0)
	goto bad;		/* can't use a 0-sized font */

    if (!(bfontname &&
	  (bfs = XLoadQueryFont (screen->display, bfontname))))
      bfs = nfs;
    else
	if (bfs->ascent + bfs->descent == 0  ||  bfs->max_bounds.width == 0)
	    goto bad;		/* can't use a 0-sized font */

    if (nfs->min_bounds.width != nfs->max_bounds.width
     || bfs->min_bounds.width != bfs->max_bounds.width
     || nfs->min_bounds.width != bfs->min_bounds.width
     || nfs->max_bounds.width != bfs->max_bounds.width) {
	TRACE(("Proportional font!\n"))
	proportional = True;
    }

    mask = (GCFont | GCForeground | GCBackground | GCGraphicsExposures |
	    GCFunction);

    new_normal = getXtermForeground(term->flags, term->cur_foreground);
    new_revers = getXtermBackground(term->flags, term->cur_background);

    xgcv.font = nfs->fid;
    xgcv.foreground = new_normal;
    xgcv.background = new_revers;
    xgcv.graphics_exposures = TRUE;	/* default */
    xgcv.function = GXcopy;

    new_normalGC = XtGetGC((Widget)term, mask, &xgcv);
    if (!new_normalGC) goto bad;

    if (nfs == bfs) {			/* there is no bold font */
	new_normalboldGC = new_normalGC;
    } else {
	xgcv.font = bfs->fid;
	new_normalboldGC = XtGetGC((Widget)term, mask, &xgcv);
	if (!new_normalboldGC) goto bad;
    }

    xgcv.font = nfs->fid;
    xgcv.foreground = new_revers;
    xgcv.background = new_normal;
    new_reverseGC = XtGetGC((Widget)term, mask, &xgcv);
    if (!new_reverseGC) goto bad;

    if (nfs == bfs) {			/* there is no bold font */
	new_reverseboldGC = new_reverseGC;
    } else {
	xgcv.font = bfs->fid;
	new_reverseboldGC = XtGetGC((Widget)term, mask, &xgcv);
	if (!new_reverseboldGC) goto bad;
    }

    if (NormalGC(screen) != NormalBoldGC(screen))
	XtReleaseGC ((Widget) term, NormalBoldGC(screen));
    XtReleaseGC ((Widget) term, NormalGC(screen));
    if (ReverseGC(screen) != ReverseBoldGC(screen))
	XtReleaseGC ((Widget) term, ReverseBoldGC(screen));
    XtReleaseGC ((Widget) term, ReverseGC(screen));
    NormalGC(screen) = new_normalGC;
    NormalBoldGC(screen) = new_normalboldGC;
    ReverseGC(screen) = new_reverseGC;
    ReverseBoldGC(screen) = new_reverseboldGC;

    /* If we're switching fonts, free the old ones.  Otherwise we'll leak the
     * memory that is associated with the old fonts. The XLoadQueryFont call
     * allocates a new XFontStruct.
     */
    if (screen->fnt_bold != 0
     && screen->fnt_bold != screen->fnt_norm)
    	XFreeFont(screen->display, screen->fnt_bold);
    if (screen->fnt_norm != 0)
    	XFreeFont(screen->display, screen->fnt_norm);

    screen->fnt_norm = nfs;
    screen->fnt_bold = bfs;
    screen->fnt_prop = proportional;

    screen->enbolden = (nfs == bfs);
    set_menu_font (False);
    screen->menu_font_number = fontnum;
    set_menu_font (True);
    if (tmpname) {			/* if setting escape or sel */
	if (screen->menu_font_names[fontnum])
	  free (screen->menu_font_names[fontnum]);
	screen->menu_font_names[fontnum] = tmpname;
	if (fontnum == fontMenu_fontescape) {
	    set_sensitivity (term->screen.fontMenu,
			     fontMenuEntries[fontMenu_fontescape].widget,
			     TRUE);
	}
    }
    set_cursor_gcs (screen);
    update_font_info (screen, doresize);
    return 1;

  bad:
    if (tmpname) free (tmpname);
    if (new_normalGC)
      XtReleaseGC ((Widget) term, screen->fullVwin.normalGC);
    if (new_normalGC && new_normalGC != new_normalboldGC)
      XtReleaseGC ((Widget) term, new_normalboldGC);
    if (new_reverseGC)
      XtReleaseGC ((Widget) term, new_reverseGC);
    if (new_reverseGC && new_reverseGC != new_reverseboldGC)
      XtReleaseGC ((Widget) term, new_reverseboldGC);
    if (nfs) XFreeFont (screen->display, nfs);
    if (bfs && nfs != bfs) XFreeFont (screen->display, bfs);
    return 0;
}


static void
update_font_info (TScreen *screen, Bool doresize)
{
    int i, j, width, height, scrollbar_width;

    screen->fullVwin.f_width = screen->fnt_norm->max_bounds.width;
    screen->fullVwin.f_height = (screen->fnt_norm->ascent +
				 screen->fnt_norm->descent);
    scrollbar_width = (term->misc.scrollbar ?
		       screen->scrollWidget->core.width +
		       screen->scrollWidget->core.border_width : 0);
    i = 2 * screen->border + scrollbar_width;
    j = 2 * screen->border;
    width = (screen->max_col + 1) * screen->fullVwin.f_width + i;
    height = (screen->max_row + 1) * screen->fullVwin.f_height + j;
    screen->fullVwin.fullwidth = width;
    screen->fullVwin.fullheight = height;
    screen->fullVwin.width = width - i;
    screen->fullVwin.height = height - j;

    if (doresize) {
	if (VWindow(screen)) {
	    XClearWindow (screen->display, VWindow(screen));
	}
	DoResizeScreen (term);		/* set to the new natural size */
	if (screen->scrollWidget)
	    ResizeScrollBar (screen);
	Redraw ();
    }
    set_vt_box (screen);
}

static void
set_vt_box (TScreen *screen)
{
	XPoint	*vp;

	vp = &VTbox[1];
	(vp++)->x = FontWidth(screen) - 1;
	(vp++)->y = FontHeight(screen) - 1;
	(vp++)->x = -(FontWidth(screen) - 1);
	vp->y = -(FontHeight(screen) - 1);
	screen->box = VTbox;
}

void
set_cursor_gcs (TScreen *screen)
{
    XGCValues xgcv;
    XtGCMask mask;
    Pixel cc = screen->cursorcolor;
    Pixel fg = screen->foreground;
    Pixel bg = term->core.background_pixel;
    GC new_cursorGC = NULL;
    GC new_cursorFillGC = NULL;
    GC new_reversecursorGC = NULL;
    GC new_cursoroutlineGC = NULL;

    /*
     * Let's see, there are three things that have "color":
     *
     *     background
     *     text
     *     cursorblock
     *
     * And, there are four situation when drawing a cursor, if we decide
     * that we like have a solid block of cursor color with the letter
     * that it is highlighting shown in the background color to make it
     * stand out:
     *
     *     selected window, normal video - background on cursor
     *     selected window, reverse video - foreground on cursor
     *     unselected window, normal video - foreground on background
     *     unselected window, reverse video - background on foreground
     *
     * Since the last two are really just normalGC and reverseGC, we only
     * need two new GC's.  Under monochrome, we get the same effect as
     * above by setting cursor color to foreground.
     */

#if OPT_ISO_COLORS
    /*
     * If we're using ANSI colors, the functions manipulating the SGR code will
     * use the same GC's.  To avoid having the cursor change color, we use the
     * Xlib calls rather than the Xt calls.
     *
     * Use the colorMode value to determine which we'll do (the TextWindow may
     * not be set before the widget's realized, so it's tested separately).
     */
    if(screen->colorMode) {
	if (TextWindow(screen) != 0 && (cc != bg) && (cc != fg)) {
	    /* we might have a colored foreground/background later */
	    xgcv.font = screen->fnt_norm->fid;
	    mask = (GCForeground | GCBackground | GCFont);
	    xgcv.foreground = fg;
	    xgcv.background = cc;
	    new_cursorGC = XCreateGC (screen->display, TextWindow(screen), mask, &xgcv);

	    xgcv.foreground = cc;
	    xgcv.background = fg;
	    new_cursorFillGC = XCreateGC (screen->display, TextWindow(screen), mask, &xgcv);

	    if (screen->always_highlight) {
		new_reversecursorGC = (GC) 0;
		new_cursoroutlineGC = (GC) 0;
	    } else {
		xgcv.foreground = bg;
		xgcv.background = cc;
		new_reversecursorGC = XCreateGC (screen->display, TextWindow(screen), mask, &xgcv);
		xgcv.foreground = cc;
		xgcv.background = bg;
		new_cursoroutlineGC = XCreateGC (screen->display, TextWindow(screen), mask, &xgcv);
	    }
	}
    } else
#endif
    if (cc != fg && cc != bg) {
	/* we have a colored cursor */
	xgcv.font = screen->fnt_norm->fid;
	mask = (GCForeground | GCBackground | GCFont);

	xgcv.foreground = fg;
	xgcv.background = cc;
	new_cursorGC = XtGetGC ((Widget) term, mask, &xgcv);

	xgcv.foreground = cc;
	xgcv.background = fg;
	new_cursorFillGC = XtGetGC ((Widget) term, mask, &xgcv);

	if (screen->always_highlight) {
	    new_reversecursorGC = (GC) 0;
	    new_cursoroutlineGC = (GC) 0;
	} else {
	    xgcv.foreground = bg;
	    xgcv.background = cc;
	    new_reversecursorGC = XtGetGC ((Widget) term, mask, &xgcv);
	    xgcv.foreground = cc;
	    xgcv.background = bg;
	    new_cursoroutlineGC = XtGetGC ((Widget) term, mask, &xgcv);
	}
    }

#if OPT_ISO_COLORS
    if(screen->colorMode)
    {
	if (screen->cursorGC)
	    XFreeGC (screen->display, screen->cursorGC);
	if (screen->fillCursorGC)
	    XFreeGC (screen->display, screen->fillCursorGC);
	if (screen->reversecursorGC)
	    XFreeGC (screen->display, screen->reversecursorGC);
	if (screen->cursoroutlineGC)
	    XFreeGC (screen->display, screen->cursoroutlineGC);
    }
    else
#endif
    {
	if (screen->cursorGC)
	    XtReleaseGC ((Widget)term, screen->cursorGC);
	if (screen->fillCursorGC)
	    XtReleaseGC ((Widget)term, screen->fillCursorGC);
	if (screen->reversecursorGC)
	    XtReleaseGC ((Widget)term, screen->reversecursorGC);
	if (screen->cursoroutlineGC)
	    XtReleaseGC ((Widget)term, screen->cursoroutlineGC);
    }

    screen->cursorGC        = new_cursorGC;
    screen->fillCursorGC    = new_cursorFillGC;
    screen->reversecursorGC = new_reversecursorGC;
    screen->cursoroutlineGC = new_cursoroutlineGC;
}
