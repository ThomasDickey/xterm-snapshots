/* $XFree86: xc/programs/xterm/xterm.h,v 3.49 1999/11/19 13:55:24 hohndel Exp $ */

/************************************************************

Copyright 1999 by Thomas E. Dickey <dickey@clark.net>

                        All Rights Reserved

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name(s) of the above copyright
holders shall not be used in advertising or otherwise to promote the
sale, use or other dealings in this Software without prior written
authorization.

********************************************************/

/*
 * Common/useful definitions for XTERM application.
 *
 * This is also where we put the fallback definitions if we do not build using
 * the configure script.
 */
#ifndef	included_xterm_h
#define	included_xterm_h

#ifdef HAVE_CONFIG_H
#include <xtermcfg.h>
#endif

#ifndef GCC_UNUSED
#define GCC_UNUSED /* nothing */
#endif

#include <X11/Xos.h>

#ifndef HAVE_CONFIG_H

#ifndef HAVE_X11_DECKEYSYM_H
#define HAVE_X11_DECKEYSYM_H 1
#endif

#ifndef DFT_TERMTYPE
#define DFT_TERMTYPE "xterm"
#endif

#ifndef X_NOT_POSIX
#define HAVE_WAITPID 1
#define HAVE_SYS_WAIT_H 1
#define HAVE_UNISTD_H 1
#endif

#ifndef X_NOT_STDC_ENV
#define HAVE_STDLIB_H 1
#define DECL_ERRNO 1
#else
#define size_t int
#define time_t long
#endif

#if defined(CSRG_BASED) || defined(__GNU__)
#define USE_POSIX_TERMIOS 1
#endif

#ifdef USE_POSIX_TERMIOS
#define HAVE_TERMIOS_H 1
#define HAVE_TCGETATTR 1
#endif

#if defined(UTMP)
#define HAVE_UTMP 1
#endif

#if (defined(__MVS__) || defined(SVR4) || defined(SCO325)) && !defined(__CYGWIN__)
#define UTMPX_FOR_UTMP 1
#endif

#ifndef ISC
#define HAVE_UTMP_UT_HOST 1
#endif

#if defined(UTMPX_FOR_UTMP) && !(defined(__MVS__) || defined(__hpux))
#define HAVE_UTMP_UT_SESSION 1
#endif

#if !(defined(linux) && (!defined(__GLIBC__) || (__GLIBC__ < 2))) && !defined(SVR4)
#define ut_xstatus ut_exit.e_exit
#endif

#if defined(SVR4) || defined(SCO325) || (defined(linux) && defined(__GLIBC__) && (__GLIBC__ >= 2) && !(defined(__powerpc__) && (__GLIBC__ == 2) && (__GLIBC_MINOR__ == 0)))
#define HAVE_UTMP_UT_XTIME 1
#endif

#if defined(linux) || defined(__CYGWIN__)
#define USE_LASTLOG
#define HAVE_LASTLOG_H
#elif defined(BSD) && (BSD >= 199103)
#define USE_LASTLOG
#endif

#if defined(AIXV3) || defined(CRAY) || defined(SCO) || defined(SVR4) || (defined(SYSV) && defined(i386)) || defined(__MVS__) || defined(__hpux) || defined(__osf__) || defined(linux) || defined(macII)
#define USE_SYSV_UTMP
#endif

#if defined(__GNU__) || defined(__MVS__)
#define USE_TTY_GROUP
#endif

#if defined(__MVS__)
#undef ut_xstatus
#define ut_name ut_user
#define ut_xstatus ut_exit.ut_e_exit
#define ut_xtime ut_tv.tv_sec
#endif

#if defined(ut_xstatus)
#define HAVE_UTMP_UT_XSTATUS 1
#endif

#endif /* HAVE_CONFIG_H */

/***====================================================================***/

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#else
extern char *calloc();
extern char *getenv();
extern char *malloc();
extern char *realloc();
extern void exit();
extern void free();
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#include <errno.h>
#if defined(DECL_ERRNO) && !defined(errno)
extern int errno;
#endif

/*
 * FIXME:  Toggling logging from xterm hangs under Linux 2.0.29 with libc5 if
 * we use 'waitpid()', while 'wait()' seems to work properly.
 */
#ifdef linux
#undef HAVE_WAITPID
#endif

/***====================================================================***/

#include <proto.h>
#include <ptyx.h>

#if (XtSpecificationRelease >= 6) && !defined(NO_XPOLL_H)
#include <X11/Xpoll.h>
#else
#define Select(n,r,w,e,t) select(n,(fd_set*)r,(fd_set*)w,(fd_set*)e,(struct timeval *)t)
#define XFD_COPYSET(src,dst) bcopy((src)->fds_bits, (dst)->fds_bits, sizeof(fd_set))
#ifdef __MVS__
#include <sys/time.h>
#endif
#endif

#ifdef USE_SYS_SELECT_H

#include <sys/types.h>

#if defined(AIXV3) && defined(NFDBITS)
#undef NFDBITS	/* conflict between X11/Xpoll.h and sys/select.h */
#endif

#include <sys/select.h>

#endif

#include <setjmp.h>

/***====================================================================***/

#define XtNallowSendEvents	"allowSendEvents"
#define XtNalwaysHighlight	"alwaysHighlight"
#define XtNanswerbackString	"answerbackString"
#define XtNappcursorDefault	"appcursorDefault"
#define XtNappkeypadDefault	"appkeypadDefault"
#define XtNautoWrap		"autoWrap"
#define XtNawaitInput		"awaitInput"
#define XtNbackarrowKey		"backarrowKey"
#define XtNbellSuppressTime	"bellSuppressTime"
#define XtNboldColors		"boldColors"
#define XtNboldFont		"boldFont"
#define XtNboldMode		"boldMode"
#define XtNc132			"c132"
#define XtNcacheDoublesize	"cacheDoublesize"
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
#define XtNcursorBlink		"cursorBlink"
#define XtNcursorColor		"cursorColor"
#define XtNcursorOffTime	"cursorOffTime"
#define XtNcursorOnTime		"cursorOnTime"
#define XtNcutNewline		"cutNewline"
#define XtNcutToBeginningOfLine	"cutToBeginningOfLine"
#define XtNdecTerminalID	"decTerminalID"
#define XtNdynamicColors	"dynamicColors"
#define XtNeightBitControl	"eightBitControl"
#define XtNeightBitInput	"eightBitInput"
#define XtNeightBitOutput	"eightBitOutput"
#define XtNfontDoublesize	"fontDoublesize"
#define XtNhighlightColor	"highlightColor"
#define XtNhighlightSelection	"highlightSelection"
#define XtNhpLowerleftBugCompat	"hpLowerleftBugCompat"
#define XtNinternalBorder	"internalBorder"
#define XtNjumpScroll		"jumpScroll"
#define XtNkeyboardDialect	"keyboardDialect"
#define XtNlogFile		"logFile"
#define XtNlogInhibit		"logInhibit"
#define XtNlogging		"logging"
#define XtNloginShell		"loginShell"
#define XtNmarginBell		"marginBell"
#define XtNmenuBar		"menuBar"
#define XtNmenuHeight		"menuHeight"
#define XtNmetaSendsEscape	"metaSendsEscape"
#define XtNmultiClickTime	"multiClickTime"
#define XtNmultiScroll		"multiScroll"
#define XtNnMarginBell		"nMarginBell"
#define XtNnumLock		"numLock"
#define XtNoldXtermFKeys	"oldXtermFKeys"
#define XtNpointerColor		"pointerColor"
#define XtNpointerColorBackground "pointerColorBackground"
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
#define XtNshiftKeys		"shiftKeys"
#define XtNsignalInhibit	"signalInhibit"
#define XtNtekGeometry		"tekGeometry"
#define XtNtekInhibit		"tekInhibit"
#define XtNtekSmall		"tekSmall"
#define XtNtekStartup		"tekStartup"
#define XtNtiteInhibit		"titeInhibit"
#define XtNtrimSelection	"trimSelection"
#define XtNunderLine		"underLine"
#define XtNutf8			"utf8"
#define XtNvisualBell		"visualBell"
#define XtNwideChars		"wideChars"
#define XtNxmcAttributes	"xmcAttributes"
#define XtNxmcGlitch		"xmcGlitch"
#define XtNxmcInline		"xmcInline"
#define XtNxmcMoveSGR		"xmcMoveSGR"

#define XtCAllowSendEvents	"AllowSendEvents"
#define XtCAlwaysHighlight	"AlwaysHighlight"
#define XtCAnswerbackString	"AnswerbackString"
#define XtCAppcursorDefault	"AppcursorDefault"
#define XtCAppkeypadDefault	"AppkeypadDefault"
#define XtCAutoWrap		"AutoWrap"
#define XtCAwaitInput		"AwaitInput"
#define XtCBackarrowKey		"BackarrowKey"
#define XtCBellSuppressTime	"BellSuppressTime"
#define XtCBoldFont		"BoldFont"
#define XtCBoldMode		"BoldMode"
#define XtCC132			"C132"
#define XtCCacheDoublesize	"CacheDoublesize"
#define XtCCharClass		"CharClass"
#define XtCColorMode		"ColorMode"
#define XtCColumn		"Column"
#define XtCCurses		"Curses"
#define XtCCursorBlink		"CursorBlink"
#define XtCCursorOffTime	"CursorOffTime"
#define XtCCursorOnTime		"CursorOnTime"
#define XtCCutNewline		"CutNewline"
#define XtCCutToBeginningOfLine	"CutToBeginningOfLine"
#define XtCDecTerminalID	"DecTerminalID"
#define XtCDynamicColors	"DynamicColors"
#define XtCEightBitControl	"EightBitControl"
#define XtCEightBitInput	"EightBitInput"
#define XtCEightBitOutput	"EightBitOutput"
#define XtCFontDoublesize	"FontDoublesize"
#define XtCHighlightSelection	"HighlightSelection"
#define XtCHpLowerleftBugCompat	"HpLowerleftBugCompat"
#define XtCJumpScroll		"JumpScroll"
#define XtCKeyboardDialect	"KeyboardDialect"
#define XtCLogInhibit		"LogInhibit"
#define XtCLogfile		"Logfile"
#define XtCLogging		"Logging"
#define XtCLoginShell		"LoginShell"
#define XtCMarginBell		"MarginBell"
#define XtCMenuBar		"MenuBar"
#define XtCMenuHeight		"MenuHeight"
#define XtCMetaSendsEscape	"MetaSendsEscape"
#define XtCMultiClickTime	"MultiClickTime"
#define XtCMultiScroll		"MultiScroll"
#define XtCNumLock		"NumLock"
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
#define XtCShiftKeys		"ShiftKeys"
#define XtCSignalInhibit	"SignalInhibit"
#define XtCTekInhibit		"TekInhibit"
#define XtCTekSmall		"TekSmall"
#define XtCTekStartup		"TekStartup"
#define XtCTiteInhibit		"TiteInhibit"
#define XtCTrimSelection	"TrimSelection"
#define XtCUnderLine		"UnderLine"
#define XtCUtf8			"Utf8"
#define XtCVisualBell		"VisualBell"
#define XtCWideChars		"WideChars"
#define XtCXmcAttributes	"XmcAttributes"
#define XtCXmcGlitch		"XmcGlitch"
#define XtCXmcInline		"XmcInline"
#define XtCXmcMoveSGR		"XmcMoveSGR"

#ifdef NO_ACTIVE_ICON
#define XtNgeometry		"geometry"
#define XtCGeometry		"Geometry"
#endif

/***====================================================================***/

#ifdef	__cplusplus
extern "C" {
#endif

/* Tekproc.c */
extern int TekInit (void);
extern int TekPtyData (void);
extern void ChangeTekColors (TScreen *screen, ScrnColors *pNew);
extern void TCursorToggle (int toggle);
extern void TekCopy (void);
extern void TekEnqMouse (int c);
extern void TekExpose (Widget w, XEvent *event, Region region);
extern void TekGINoff (void);
extern void TekReverseVideo (TScreen *screen);
extern void TekRun (void);
extern void TekSetFontSize (int newitem);
extern void TekSimulatePageButton (Bool reset);
extern void dorefresh (void);

/* button.c */
extern Boolean SendMousePosition (Widget w, XEvent* event);
extern int SetCharacterClassRange (int low, int high, int value);
extern void DiredButton               PROTO_XT_ACTIONS_ARGS;
extern void DisownSelection (XtermWidget termw);
extern void HandleGINInput            PROTO_XT_ACTIONS_ARGS;
extern void HandleInsertSelection     PROTO_XT_ACTIONS_ARGS;
extern void HandleKeyboardSelectEnd   PROTO_XT_ACTIONS_ARGS;
extern void HandleKeyboardSelectStart PROTO_XT_ACTIONS_ARGS;
extern void HandleKeyboardStartExtend PROTO_XT_ACTIONS_ARGS;
extern void HandleSecure              PROTO_XT_ACTIONS_ARGS;
extern void HandleSelectEnd           PROTO_XT_ACTIONS_ARGS;
extern void HandleSelectExtend        PROTO_XT_ACTIONS_ARGS;
extern void HandleSelectSet           PROTO_XT_ACTIONS_ARGS;
extern void HandleSelectStart         PROTO_XT_ACTIONS_ARGS;
extern void HandleStartExtend         PROTO_XT_ACTIONS_ARGS;
extern void ResizeSelection (TScreen *screen, int rows, int cols);
extern void ScrollSelection (TScreen* screen, int amount);
extern void TrackMouse (int func, int startrow, int startcol, int firstrow, int lastrow);
extern void TrackText (int frow, int fcol, int trow, int tcol);
extern void ViButton                  PROTO_XT_ACTIONS_ARGS;

#if OPT_DEC_LOCATOR
extern Boolean SendLocatorPosition (Widget w, XEvent* event);
extern void CheckLocatorPosition (Widget w, XEvent *event);
extern void GetLocatorPosition (XtermWidget w);
extern void InitLocatorFilter (XtermWidget w);
#endif	/* OPT_DEC_LOCATOR */

/* charproc.c */
extern int VTInit (void);
extern int v_write (int f, Char *d, int len);
extern void FindFontSelection (char *atom_name, Bool justprobe);
extern void HideCursor (void);
extern void ShowCursor (void);
extern void SwitchBufPtrs (TScreen *screen);
extern void ToggleAlternate (TScreen *screen);
extern void VTReset (int full, int saved);
extern void VTRun (void);
extern void dotext (TScreen *screen, int charset, IChar *buf, Cardinal len);
extern void resetCharsets (TScreen *screen);
extern void set_cursor_gcs (TScreen *screen);
extern void unparseputc (int c, int fd);
extern void unparseputc1 (int c, int fd);
extern void unparseputs (char *s, int fd);
extern void unparseseq (ANSI *ap, int fd);

#if OPT_BLINK_CURS
extern void ToggleCursorBlink(TScreen *screen);
#endif

#if OPT_ISO_COLORS
extern void SGR_Background (int color);
extern void SGR_Foreground (int color);
#endif

/* charsets.c */
extern unsigned xtermCharSetIn (unsigned code, int charset);
extern int xtermCharSetOut (IChar *buf, IChar *ptr, char charset);

/* cursor.c */
extern void CarriageReturn (TScreen *screen);
extern void CursorBack (TScreen *screen, int  n);
extern void CursorDown (TScreen *screen, int  n);
extern void CursorForward (TScreen *screen, int  n);
extern void CursorNextLine (TScreen *screen, int count);
extern void CursorPrevLine (TScreen *screen, int count);
extern void CursorRestore (XtermWidget tw);
extern void CursorSave (XtermWidget tw);
extern void CursorSet (TScreen *screen, int row, int col, unsigned flags);
extern void CursorUp (TScreen *screen, int  n);
extern void Index (TScreen *screen, int amount);
extern void RevIndex (TScreen *screen, int amount);

/* doublechr.c */
extern void xterm_DECDHL (Bool top);
extern void xterm_DECSWL (void);
extern void xterm_DECDWL (void);
#if OPT_DEC_CHRSET
extern int xterm_Double_index(unsigned chrset, unsigned flags);
extern GC xterm_DoubleGC(unsigned chrset, unsigned flags, GC old_gc);
#endif

/* input.c */
extern void Input (TKeyboard *keyboard, TScreen *screen, XKeyEvent *event, Bool eightbit);
extern void StringInput (TScreen *screen, char *string, size_t nbytes);

#if OPT_NUM_LOCK
extern void VTInitModifiers(void);
#endif

#if OPT_WIDE_CHARS
extern int convertFromUTF8(unsigned long c, Char *strbuf);
#endif

/* main.c */
#ifndef __EMX__
extern int main (int argc, char **argv);
#else
extern int main (int argc, char **argv,char **envp);
#endif

extern int GetBytesAvailable (int fd);
extern int kill_process_group (int pid, int sig);
extern int nonblocking_wait (void);
extern void first_map_occurred (void);

#ifdef SIGNAL_T
extern SIGNAL_T Exit (int n);
#endif

/* menu.c */
extern void do_hangup          PROTO_XT_CALLBACK_ARGS;
extern void show_8bit_control  (Bool value);

/* misc.c */
extern Cursor make_colored_cursor (unsigned cursorindex, unsigned long fg, unsigned long bg);
extern char *SysErrorMsg (int n);
extern char *strindex (char *s1, char *s2);
extern char *udk_lookup (int keycode, int *len);
extern int XStrCmp (char *s1, char *s2);
extern int xerror (Display *d, XErrorEvent *ev);
extern int xioerror (Display *dpy);
extern void Bell (int which, int percent);
extern void ChangeXprop (char *name);
extern void Changename (char *name);
extern void Changetitle (char *name);
extern void Cleanup (int code);
extern void Error (int i);
extern void HandleBellPropertyChange PROTO_XT_EV_HANDLER_ARGS;
extern void HandleEightBitKeyPressed PROTO_XT_ACTIONS_ARGS;
extern void HandleEnterWindow        PROTO_XT_EV_HANDLER_ARGS;
extern void HandleFocusChange        PROTO_XT_EV_HANDLER_ARGS;
extern void HandleInterpret          PROTO_XT_ACTIONS_ARGS;
extern void HandleKeyPressed         PROTO_XT_ACTIONS_ARGS;
extern void HandleLeaveWindow        PROTO_XT_EV_HANDLER_ARGS;
extern void HandleStringEvent        PROTO_XT_ACTIONS_ARGS;
extern void Panic (char *s, int a);
extern void Redraw (void);
extern void ReverseOldColors (void);
extern void Setenv (char *var, char *value);
extern void SysError (int i);
extern void VisualBell (void);
extern void creat_as (int uid, int gid, char *pathname, int mode);
extern void do_dcs (Char *buf, size_t len);
extern void do_osc (Char *buf, int len, int final);
extern void do_xevents (void);
extern void end_tek_mode (void);
extern void end_vt_mode (void);
extern void hide_tek_window (void);
extern void hide_vt_window (void);
extern void reset_decudk (void);
extern void set_tek_visibility (Boolean on);
extern void set_vt_visibility (Boolean on);
extern void switch_modes (Bool tovt);
extern void xevents (void);
extern void xt_error (String message);

#if OPT_MAXIMIZE
extern int QueryMaximize (TScreen *screen, unsigned *width, unsigned *height);
extern void HandleDeIconify          PROTO_XT_ACTIONS_ARGS;
extern void HandleIconify            PROTO_XT_ACTIONS_ARGS;
extern void HandleMaximize           PROTO_XT_ACTIONS_ARGS;
extern void HandleRestoreSize        PROTO_XT_ACTIONS_ARGS;
extern void RequestMaximize (XtermWidget termw, int maximize);
#endif

#ifdef ALLOWLOGGING
extern void StartLog (TScreen *screen);
extern void CloseLog (TScreen *screen);
extern void FlushLog (TScreen *screen);
#else
#define FlushLog(screen) /*nothing*/
#endif

/* print.c */
extern int xtermPrinterControl (int chr);
extern void xtermAutoPrint (int chr);
extern void xtermMediaControl (int param, int private_seq);
extern void xtermPrintScreen (Boolean use_DECPEX);

/* ptydata.c */
extern int getPtyData (TScreen *screen, fd_set *select_mask, PtyData *data);
extern unsigned usedPtyData(PtyData *data);
extern void initPtyData (PtyData *data);

#define nextPtyData(data) ((data)->cnt)--, (*((data)->ptr)++)
#define morePtyData(data) ((data)->cnt > 0)

#if OPT_WIDE_CHARS
extern Char * convertToUTF8(Char *lp, unsigned c);
extern void writePtyData(int f, IChar *d, unsigned len);
#else
#define writePtyData(f,d,len) v_write(f,d,len)
#endif

/* screen.c */
extern Bool non_blank_line (ScrnBuf sb, int row, int col, int len);
extern ScrnBuf Allocate (int nrow, int ncol, Char **addr);
extern int ScreenResize (TScreen *screen, int width, int height, unsigned *flags);
extern size_t ScrnPointers (TScreen *screen, size_t len);
extern void ClearBufRows (TScreen *screen, int first, int last);
extern void ScreenWrite (TScreen *screen, PAIRED_CHARS(Char *str, Char *str2), unsigned flags, unsigned cur_fg_bg, int length);
extern void ScrnDeleteChar (TScreen *screen, int n, int size);
extern void ScrnDeleteLine (TScreen *screen, ScrnBuf sb, int n, int last, int size, int where);
extern void ScrnInsertChar (TScreen *screen, int n, int size);
extern void ScrnInsertLine (TScreen *screen, ScrnBuf sb, int last, int where, int n, int size);
extern void ScrnRefresh (TScreen *screen, int toprow, int leftcol, int nrows, int ncols, int force);

#define ScrnClrWrapped(screen, row) \
	SCRN_BUF_FLAGS(screen, row + screen->topline) = \
		(Char *)((long)SCRN_BUF_FLAGS(screen, row + screen->topline) & ~ LINEWRAPPED)

#define ScrnSetWrapped(screen, row) \
	SCRN_BUF_FLAGS(screen, row + screen->topline) = \
		(Char *)(((long)SCRN_BUF_FLAGS(screen, row + screen->topline) | LINEWRAPPED))

#define ScrnTstWrapped(screen, row) \
	(((long)SCRN_BUF_FLAGS(screen, row + screen->topline) & LINEWRAPPED) != 0)

/* scrollbar.c */
extern void DoResizeScreen (XtermWidget xw);
extern void HandleScrollBack PROTO_XT_ACTIONS_ARGS;
extern void HandleScrollForward PROTO_XT_ACTIONS_ARGS;
extern void ResizeScrollBar (TScreen *screen);
extern void ScrollBarDrawThumb (Widget scrollWidget);
extern void ScrollBarOff (TScreen *screen);
extern void ScrollBarOn (XtermWidget xw, int init, int doalloc);
extern void ScrollBarReverseVideo (Widget scrollWidget);
extern void ToggleScrollBar (XtermWidget w);
extern void WindowScroll (TScreen *screen, int top);

/* tabs.c */
extern Boolean TabToNextStop (void);
extern Boolean TabToPrevStop (void);
extern int TabNext (Tabs tabs, int col);
extern int TabPrev (Tabs tabs, int col);
extern void TabClear (Tabs tabs, int col);
extern void TabReset (Tabs tabs);
extern void TabSet (Tabs tabs, int col);
extern void TabZonk (Tabs	tabs);

/* util.c */
extern GC updatedXtermGC (TScreen *screen, int flags, int fg_bg, Bool hilite);
extern int AddToRefresh (TScreen *screen);
extern int HandleExposure (TScreen *screen, XEvent *event);
extern int char2lower(int ch);
extern int drawXtermText (TScreen *screen, unsigned flags, GC gc, int x, int y, int chrset, PAIRED_CHARS(Char *text, Char *text2), Cardinal len);
extern void ChangeAnsiColors (XtermWidget tw);
extern void ChangeColors (XtermWidget tw, ScrnColors *pNew);
extern void ClearRight (TScreen *screen, int n);
extern void ClearScreen (TScreen *screen);
extern void DeleteChar (TScreen *screen, int n);
extern void DeleteLine (TScreen *screen, int n);
extern void FlushScroll (TScreen *screen);
extern void GetColors (XtermWidget tw, ScrnColors *pColors);
extern void InsertChar (TScreen *screen, int n);
extern void InsertLine (TScreen *screen, int n);
extern void RevScroll (TScreen *screen, int amount);
extern void ReverseVideo (XtermWidget termw);
extern void Scroll (TScreen *screen, int amount);
extern void do_erase_display (TScreen *screen, int param, int mode);
extern void do_erase_line (TScreen *screen, int param, int mode);
extern void recolor_cursor (Cursor cursor, unsigned long fg, unsigned long bg);
extern void resetXtermGC (TScreen *screen, int flags, Bool hilite);
extern void scrolling_copy_area (TScreen *screen, int firstline, int nlines, int amount);

#if OPT_ISO_COLORS

extern Pixel getXtermBackground (int flags, int color);
extern Pixel getXtermForeground (int flags, int color);
extern int extract_fg (unsigned color, unsigned flags);
extern unsigned makeColorPair (int fg, int bg);
extern void ClearCurBackground (TScreen *screen, int top, int left, unsigned height, unsigned width);

#define xtermColorPair() makeColorPair(term->sgr_foreground, term->cur_background)

#define getXtermForeground(flags, color) \
	(((flags) & FG_COLOR) && ((color) >= 0) \
			? term->screen.Acolors[color] \
			: term->screen.foreground)

#define getXtermBackground(flags, color) \
	(((flags) & BG_COLOR) && ((color) >= 0) \
			? term->screen.Acolors[color] \
			: term->core.background_pixel)

#if OPT_EXT_COLORS
#define extract_bg(color) ((int)((color) & 0xff))
#else
#define extract_bg(color) ((int)((color) & 0xf))
#endif

#else /* !OPT_ISO_COLORS */

#define ClearCurBackground(screen, top, left, height, width) \
	XClearArea (screen->display, VWindow(screen), \
		left, top, width, height, FALSE)

#define extract_fg(color, flags) term->cur_foreground
#define extract_bg(color) term->cur_background

		/* FIXME: Reverse-Video? */
#define getXtermBackground(flags, color) term->core.background_pixel
#define getXtermForeground(flags, color) term->screen.foreground
#define makeColorPair(fg, bg) 0
#define xtermColorPair() 0

#endif	/* OPT_ISO_COLORS */

#if OPT_DEC_CHRSET
#define curXtermChrSet(row) \
	((CSET_DOUBLE(SCRN_ROW_CSET((&term->screen), row))) \
		? SCRN_ROW_CSET((&term->screen), row) \
		: (term->screen).cur_chrset)
#else
#define curXtermChrSet(row) 0
#endif

#if OPT_WIDE_CHARS
extern unsigned getXtermCell (TScreen *screen, int row, int col);
extern void putXtermCell (TScreen *screen, int row, int col, int ch);
#else
#define getXtermCell(screen,row,col) SCRN_BUF_CHARS(screen, row)[col]
#define putXtermCell(screen,row,col,ch) SCRN_BUF_CHARS(screen, row)[col] = ch
#endif

#if OPT_XMC_GLITCH
extern void Mark_XMC (TScreen *screen, int param);
extern void Jump_XMC (TScreen *screen);
extern void Resolve_XMC (TScreen *screen);
#endif

#ifdef	__cplusplus
	}
#endif

#endif	/* included_xterm_h */
