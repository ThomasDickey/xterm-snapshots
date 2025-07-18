/* $XTermId: xterm.h,v 1.968 2025/06/23 23:31:45 tom Exp $ */

/*
 * Copyright 1999-2024,2025 by Thomas E. Dickey
 *
 *                         All Rights Reserved
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name(s) of the above copyright
 * holders shall not be used in advertising or otherwise to promote the
 * sale, use or other dealings in this Software without prior written
 * authorization.
 */

/*
 * Common/useful definitions for XTERM application.
 *
 * This is also where we put the fallback definitions if we do not build using
 * the configure script.
 */
#ifndef included_xterm_h
#define included_xterm_h
/* *INDENT-OFF* */

#ifdef HAVE_CONFIG_H
#include <xtermcfg.h>
#endif

#undef bcopy
#include <X11/Xos.h>

#ifndef HAVE_CONFIG_H

#define HAVE_LIB_XAW 1

#ifdef CSRG_BASED
/* Get definition of BSD */
#include <sys/param.h>
#endif

#ifndef DFT_TERMTYPE
#define DFT_TERMTYPE "xterm"
#endif

#ifndef X_NOT_POSIX
#define HAVE_WAITPID 1
#define HAVE_SYS_WAIT_H 1
#define HAVE_UNISTD_H 1
#endif

#define HAVE_STDLIB_H 1

#if defined(sun)
/* errno is declared in <errno.h> */
#else
#define DECL_ERRNO 1
#endif

#undef DECL_PTSNAME		/* ptsname() is normally in stdlib.h */

#ifndef NOPUTENV
#define HAVE_PUTENV 1
#endif

#if defined(CSRG_BASED) || defined(__GNU__) || defined(__minix)
#define USE_POSIX_TERMIOS 1
#endif

#ifdef __FreeBSD__
#if __FreeBSD_version >= 900000
#define USE_SYSV_UTMP 1
#define UTMPX_FOR_UTMP 1
#define HAVE_UTMP_UT_HOST 1
#define HAVE_UTMP_UT_XTIME 1
#define ut_name ut_user
#define ut_xtime ut_tv.tv_sec
#endif
#endif

#ifdef __NetBSD__
#if __NetBSD_Version__ >= 106030000	/* 1.6C */
#define BSD_UTMPX 1
#define ut_xtime ut_tv.tv_sec
#endif
#endif

#if defined(hpux) && !defined(__hpux)
#define __hpux 1		/* HPUX 11.0 does not define this */
#endif

#if !defined(__SCO__) && (defined(SCO) || defined(sco) || defined(SCO325))
#define __SCO__ 1
#endif

#ifdef USE_POSIX_TERMIOS
#define HAVE_TERMIOS_H 1
#define HAVE_TCGETATTR 1
#endif

#if defined(__SCO__) || defined(__UNIXWARE__) || defined(__minix)
#define USE_TERMCAP 1
#endif

#if defined(UTMP)
#define HAVE_UTMP 1
#endif

#if (defined(SVR4) || defined(__SCO__) || defined(BSD_UTMPX)) && !defined(__CYGWIN__)
#define UTMPX_FOR_UTMP 1
#endif

#if !defined(ISC) && !defined(__QNX__)
#define HAVE_UTMP_UT_HOST 1
#endif

#if defined(UTMPX_FOR_UTMP) && !(defined(__hpux) || defined(__FreeBSD__))
#define HAVE_UTMP_UT_SESSION 1
#endif

#if !(defined(__linux__) && (!defined(__GLIBC__) || (__GLIBC__ < 2))) && !defined(SVR4) && !defined(__FreeBSD__)
#define ut_xstatus ut_exit.e_exit
#endif

#if defined(SVR4) || defined(__SCO__) || defined(BSD_UTMPX) || (defined(__linux__) && defined(__GLIBC__) && (__GLIBC__ >= 2) && !(defined(__powerpc__) && (__GLIBC__ == 2) && (__GLIBC_MINOR__ == 0)))
#define HAVE_UTMP_UT_XTIME 1
#endif

#if defined(__linux__) || defined(__CYGWIN__)
#define USE_LASTLOG
#define HAVE_LASTLOG_H
#define USE_STRUCT_LASTLOG
#elif defined(BSD) && (BSD >= 199103)
#ifdef BSD_UTMPX
#define USE_LASTLOGX
#elif defined(USE_SYSV_UTMP)
#else
#define USE_LASTLOG
#define USE_STRUCT_LASTLOG
#endif
#endif

#if defined(__OpenBSD__)
#define DEFDELETE_DEL True
#define DEF_BACKARO_ERASE True
#define DEF_INITIAL_ERASE True
#endif

#if defined(__SCO__) || defined(__UNIXWARE__)
#define DEFDELETE_DEL True
#define OPT_SCO_FUNC_KEYS 1
#endif

#if defined(__SCO__) || defined(SVR4) || defined(_POSIX_VERSION) || defined(__QNX__) || defined(__hpux) || (defined(BSD) && (BSD >= 199103)) || defined(__CYGWIN__)
#define USE_POSIX_WAIT
#endif

#if defined(AIXV3) || defined(CRAY) || defined(__SCO__) || defined(SVR4) || (defined(SYSV) && defined(i386)) || defined(__hpux) || defined(__osf__) || defined(__linux__) || defined(macII) || defined(BSD_UTMPX)
#define USE_SYSV_UTMP
#endif

#if defined(__GNU__) || defined(__osf__)
#define USE_TTY_GROUP
#endif

#if defined(__CYGWIN__)
#define HAVE_NCURSES_TERM_H 1
#endif

#ifdef __osf__
#define TTY_GROUP_NAME "terminal"
#endif

#if defined(ut_xstatus)
#define HAVE_UTMP_UT_XSTATUS 1
#endif

#if defined(XKB)
#define HAVE_XKB_BELL_EXT 1
#endif

#if (defined(SVR4) && !defined(__CYGWIN__)) || defined(__linux__) || (defined(BSD) && (BSD >= 199103))
#define HAVE_POSIX_SAVED_IDS
#endif

#if defined(__linux__) || defined(__GLIBC__) || (defined(SYSV) && (defined(CRAY) || defined(macII) || defined(__hpux) || defined(__osf__) || defined(__sgi))) || !(defined(SYSV) || defined(__QNX__) || defined(__INTERIX))
#define HAVE_INITGROUPS
#endif

#if defined(__linux__)
#define HAVE_PTY_H
#endif

#if !defined(USG) && !defined(__minix)
#define HAVE_SETITIMER 1
#else
#define HAVE_SETITIMER 0
#endif

#if defined(_POSIX_VERSION) || defined(SVR4) || defined(__convex__) || defined(__SCO__) || defined(__QNX__)
#define HAVE_SETSID 1
#endif

#endif /* HAVE_CONFIG_H */

#ifndef HAVE_X11_DECKEYSYM_H
#define HAVE_X11_DECKEYSYM_H 1
#endif

#ifndef HAVE_X11_SUNKEYSYM_H
#define HAVE_X11_SUNKEYSYM_H 1
#endif

#ifndef HAVE_X11_XF86KEYSYM_H
#define HAVE_X11_XF86KEYSYM_H 0
#endif

#if defined(HAVE_X11_EXTENSIONS_XDBE_H) && defined(HAVE_XDBESWAPBUFFERS)
#define USE_DOUBLE_BUFFER 1
#else
#define USE_DOUBLE_BUFFER 0
#endif

/***====================================================================***/

/* if compiling with gcc -ansi -pedantic, we must fix POSIX definitions */
#if defined(SVR4) && defined(sun)
#ifndef __EXTENSIONS__
#define __EXTENSIONS__ 1
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 1
#endif
#endif

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
#undef HAVE_UNISTD_H
#endif

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#include <errno.h>
#if defined(DECL_ERRNO) && !defined(errno)
extern int errno;
#endif

#include <X11/Xlocale.h>

#ifdef HAVE_STDNORETURN_H
#include <stdnoreturn.h>
#undef GCC_NORETURN
#define GCC_NORETURN STDC_NORETURN
#endif

/*
 * FIXME:  Toggling logging from xterm hangs under Linux 2.0.29 with libc5 if
 * we use 'waitpid()', while 'wait()' seems to work properly.
 */
#ifdef __linux__
#undef HAVE_WAITPID
#endif

#ifndef OPT_WIDE_CHARS
#define OPT_WIDE_CHARS 0
#endif

#if OPT_WIDE_CHARS
#define HIDDEN_CHAR 0xffff
#endif

/***====================================================================***/

#define PROTO_XT_ACTIONS_ARGS \
	(Widget w, XEvent *event, String *params, Cardinal *num_params)

#define PROTO_XT_CALLBACK_ARGS \
	(Widget gw, XtPointer closure, XtPointer data)

#define PROTO_XT_CVT_SELECT_ARGS \
	(Widget w, Atom *selection, Atom *target, Atom *type, XtPointer *value, unsigned long *length, int *format)

#define PROTO_XT_EV_HANDLER_ARGS \
	(Widget w, XtPointer closure, XEvent *event, Boolean *cont)

#define PROTO_XT_SEL_CB_ARGS \
	(Widget w, XtPointer client_data, Atom *selection, Atom *type, XtPointer value, unsigned long *length, int *format)

#include <ptyx.h>

/***====================================================================***/

#ifndef GCC_PRINTFLIKE
#ifdef _X_ATTRIBUTE_PRINTF
#define GCC_PRINTFLIKE(f,n)	_X_ATTRIBUTE_PRINTF(f,n)
#else
#define GCC_PRINTFLIKE(f,n)	/* nothing */
#endif
#endif

#ifndef GCC_UNUSED
#ifdef _X_UNUSED
#define GCC_UNUSED		_X_UNUSED
#else
#define GCC_UNUSED		/* nothing */
#endif
#endif

#ifndef GCC_NORETURN
#ifdef _X_NORETURN
#define GCC_NORETURN		_X_NORETURN
#else
#define GCC_NORETURN		/* nothing */
#endif
#endif

/***====================================================================***/

#if defined(__GNUC__) && defined(_FORTIFY_SOURCE)
#define USE_IGNORE_RC
#define IGNORE_RC(func) ignore_unused = (int) func
#else
#define IGNORE_RC(func) (void) func
#endif /* gcc workarounds */
#if (XtSpecificationRelease >= 6) && !defined(NO_XPOLL_H) && !defined(sun)
#include <X11/Xpoll.h>
#define USE_XPOLL_H 1
#else
#define Select(n,r,w,e,t) select(n,(fd_set*)r,(fd_set*)w,(fd_set*)e,(struct timeval *)t)
#define XFD_COPYSET(src,dst) memcpy((dst)->fds_bits, (src)->fds_bits, sizeof(fd_set))
#endif

#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

/* these may be needed for sig_atomic_t */
#include <sys/types.h>
#include <signal.h>

#ifdef USE_SYS_SELECT_H

#if defined(USE_XPOLL_H) && defined(AIXV3) && defined(NFDBITS)
#undef NFDBITS			/* conflict between X11/Xpoll.h and sys/select.h */
#endif

#include <sys/select.h>

#endif /* USE_SYS_SELECT_H */

#include <setjmp.h>

#if !(defined(__linux__) && defined(__USE_GNU)) && !defined(__hpux) && !defined(_ALL_SOURCE) && !defined(__osf__)
extern char **environ;
#endif

#ifndef _Xconst
#define _Xconst const		/* Solaris 7 workaround */
#endif /* _Xconst */

#define XK_Fn(n)	(XK_F1 + (n) - 1)

#define Maybe		2

#define ALLOC_STRING(name) \
	if (name != NULL) \
	    name = x_strdup(name)
#define FREE_STRING(name) \
	    free_string(name)

/* strftime format and length of the result */
#define FMT_TIMESTAMP ".%Y.%m.%d.%H.%M.%S"
#define LEN_TIMESTAMP sizeof(".YYYY.MM.DD.hh.mm.ss")

/***====================================================================***/

#if OPT_TRACE
#undef NDEBUG			/* turn on assert's */
#else
#ifndef NDEBUG
#define NDEBUG			/* not debugging, don't do assert's */
#endif
#endif

#include <trace.h>

#if OPT_TRACE && !defined(DEBUG)
#define DEBUG 1
#endif

#ifdef DEBUG
#define if_DEBUG(code) if(debug) code
#else
#define if_DEBUG(code) /*nothing*/
#endif

#define DEBUG_MSG(text) if_DEBUG({ IGNORE_RC(write(2, text, sizeof(text) - 1)); })

/***====================================================================***/

#define XtNactiveIcon		"activeIcon"
#define XtNallowBoldFonts	"allowBoldFonts"
#define XtNallowC1Printable	"allowC1Printable"
#define XtNallowColorOps	"allowColorOps"
#define XtNallowFontOps		"allowFontOps"
#define XtNallowMouseOps	"allowMouseOps"
#define XtNallowPasteControls	"allowPasteControls"
#define XtNallowScrollLock	"allowScrollLock"
#define XtNallowSendEvents	"allowSendEvents"
#define XtNallowTcapOps		"allowTcapOps"
#define XtNallowTitleOps	"allowTitleOps"
#define XtNallowWindowOps	"allowWindowOps"
#define XtNaltIsNotMeta		"altIsNotMeta"
#define XtNaltSendsEscape	"altSendsEscape"
#define XtNalternateScroll	"alternateScroll"
#define XtNalwaysBoldMode	"alwaysBoldMode"
#define XtNalwaysHighlight	"alwaysHighlight"
#define XtNalwaysUseMods	"alwaysUseMods"
#define XtNanswerbackString	"answerbackString"
#define XtNappcursorDefault	"appcursorDefault"
#define XtNappkeypadDefault	"appkeypadDefault"
#define XtNassumeAllChars	"assumeAllChars"
#define XtNautoScrollLock	"autoScrollLock"
#define XtNautoWrap		"autoWrap"
#define XtNawaitInput		"awaitInput"
#define XtNbackarrowKey		"backarrowKey"
#define XtNbackarrowKeyIsErase	"backarrowKeyIsErase"
#define XtNbellIsUrgent		"bellIsUrgent"
#define XtNbellOnReset		"bellOnReset"
#define XtNbellSuppressTime	"bellSuppressTime"
#define XtNboldColors		"boldColors"
#define XtNboldFont		"boldFont"
#define XtNboldMode		"boldMode"
#define XtNbrokenLinuxOSC	"brokenLinuxOSC"
#define XtNbrokenSelections	"brokenSelections"
#define XtNbrokenStringTerm	"brokenStringTerm"
#define XtNbuffered		"buffered"
#define XtNbufferedFPS		"bufferedFPS"
#define XtNc132			"c132"
#define XtNcacheDoublesize	"cacheDoublesize"
#define XtNcdXtraScroll		"cdXtraScroll"
#define XtNcharClass		"charClass"
#define XtNchecksumExtension	"checksumExtension"
#define XtNcjkWidth		"cjkWidth"
#define XtNcolorAttrMode	"colorAttrMode"
#define XtNcolorBDMode		"colorBDMode"
#define XtNcolorBLMode		"colorBLMode"
#define XtNcolorEvents		"colorEvents"
#define XtNcolorITMode		"colorITMode"
#define XtNcolorInnerBorder	"colorInnerBorder"
#define XtNcolorMode		"colorMode"
#define XtNcolorRVMode		"colorRVMode"
#define XtNcolorULMode		"colorULMode"
#define XtNcombiningChars	"combiningChars"
#define XtNctrlFKeys		"ctrlFKeys"
#define XtNcurses		"curses"
#define XtNcursorBar		"cursorBar"
#define XtNcursorBlink		"cursorBlink"
#define XtNcursorBlinkXOR	"cursorBlinkXOR"
#define XtNcursorColor		"cursorColor"
#define XtNcursorOffTime	"cursorOffTime"
#define XtNcursorOnTime		"cursorOnTime"
#define XtNcursorTheme		"cursorTheme"
#define XtNcursorUnderLine	"cursorUnderLine"
#define XtNcutNewline		"cutNewline"
#define XtNcutToBeginningOfLine	"cutToBeginningOfLine"
#define XtNdecGraphicsID	"decGraphicsID"
#define XtNdecTerminalID	"decTerminalID"
#define XtNdefaultString	"defaultString"
#define XtNdeleteIsDEL		"deleteIsDEL"
#define XtNdirectColor		"directColor"
#define XtNdisallowedColorOps	"disallowedColorOps"
#define XtNdisallowedFontOps	"disallowedFontOps"
#define XtNdisallowedMouseOps	"disallowedMouseOps"
#define XtNdisallowedPasteControls "disallowedPasteControls"
#define XtNdisallowedTcapOps	"disallowedTcapOps"
#define XtNdisallowedWindowOps	"disallowedWindowOps"
#define XtNdynamicColors	"dynamicColors"
#define XtNeightBitControl	"eightBitControl"
#define XtNeightBitInput	"eightBitInput"
#define XtNeightBitMeta		"eightBitMeta"
#define XtNeightBitOutput	"eightBitOutput"
#define XtNeightBitSelectTypes	"eightBitSelectTypes"
#define XtNeraseSavedLines	"eraseSavedLines"
#define XtNfaceName		"faceName"
#define XtNfaceNameDoublesize	"faceNameDoublesize"
#define XtNfaceSize		"faceSize"
#define XtNfaintIsRelative	"faintIsRelative"
#define XtNfastScroll		"fastScroll"
#define XtNfont1		"font1"
#define XtNfont2		"font2"
#define XtNfont3		"font3"
#define XtNfont4		"font4"
#define XtNfont5		"font5"
#define XtNfont6		"font6"
#define XtNfont7		"font7"
#define XtNfontDoublesize	"fontDoublesize"
#define XtNfontWarnings		"fontWarnings"
#define XtNforceBoxChars	"forceBoxChars"
#define XtNforcePackedFont	"forcePackedFont"
#define XtNforceXftHeight	"forceXftHeight"
#define XtNformatCursorKeys	"formatCursorKeys"
#define XtNformatFunctionKeys	"formatFunctionKeys"
#define XtNformatKeypadKeys	"formatKeypadKeys"
#define XtNformatModifierKeys	"formatModifierKeys"
#define XtNformatOtherKeys	"formatOtherKeys"
#define XtNformatSpecialKeys	"formatSpecialKeys"
#define XtNformatStringKeys	"formatStringKeys"
#define XtNfreeBoldBox		"freeBoldBox"
#define XtNfullscreen		"fullscreen"
#define XtNhighlightColor	"highlightColor"
#define XtNhighlightColorMode	"highlightColorMode"
#define XtNhighlightReverse	"highlightReverse"
#define XtNhighlightSelection	"highlightSelection"
#define XtNhighlightTextColor	"highlightTextColor"
#define XtNhold			"hold"
#define XtNhpFunctionKeys	"hpFunctionKeys"
#define XtNhpLowerleftBugCompat	"hpLowerleftBugCompat"
#define XtNi18nSelections	"i18nSelections"
#define XtNiconBorderColor	"iconBorderColor"
#define XtNiconBorderWidth	"iconBorderWidth"
#define XtNiconFont		"iconFont"
#define XtNiconGeometry		"iconGeometry"
#define XtNiconHint		"iconHint"
#define XtNincrementalGraphics  "incrementalGraphics"
#define XtNindicatorFormat	"indicatorFormat"
#define XtNinitialFont		"initialFont"
#define XtNinternalBorder	"internalBorder"
#define XtNitalicULMode		"italicULMode"
#define XtNjumpScroll		"jumpScroll"
#define XtNkeepClipboard	"keepClipboard"
#define XtNkeepSelection	"keepSelection"
#define XtNkeyboardDialect	"keyboardDialect"
#define XtNkeyboardType		"keyboardType"
#define XtNlimitFontHeight	"limitFontHeight"
#define XtNlimitFontWidth	"limitFontWidth"
#define XtNlimitFontsets	"limitFontsets"
#define XtNlimitResize		"limitResize"
#define XtNlimitResponse	"limitResponse"
#define XtNlocale		"locale"
#define XtNlocaleFilter		"localeFilter"
#define XtNlogFile		"logFile"
#define XtNlogInhibit		"logInhibit"
#define XtNlogging		"logging"
#define XtNloginShell		"loginShell"
#define XtNmarginBell		"marginBell"
#define XtNmaxBufSize		"maxBufSize"
#define XtNmaxGraphicSize	"maxGraphicSize"
#define XtNmaxStringParse	"maxStringParse"
#define XtNmaximized		"maximized"
#define XtNmenuBar		"menuBar"	/* internal */
#define XtNmenuHeight		"menuHeight"
#define XtNmenuLocale		"menuLocale"
#define XtNmessages		"messages"
#define XtNmetaSendsEscape	"metaSendsEscape"
#define XtNminBufSize		"minBufSize"
#define XtNmkSamplePass		"mkSamplePass"
#define XtNmkSampleSize		"mkSampleSize"
#define XtNmkWidth		"mkWidth"
#define XtNmodifyCursorKeys	"modifyCursorKeys"
#define XtNmodifyFunctionKeys	"modifyFunctionKeys"
#define XtNmodifyKeyboard	"modifyKeyboard"
#define XtNmodifyKeypadKeys	"modifyKeypadKeys"
#define XtNmodifyModifierKeys	"modifyModifierKeys"
#define XtNmodifyOtherKeys	"modifyOtherKeys"
#define XtNmodifySpecialKeys	"modifySpecialKeys"
#define XtNmodifyStringKeys	"modifyStringKeys"
#define XtNmultiClickTime	"multiClickTime"
#define XtNmultiScroll		"multiScroll"
#define XtNnMarginBell		"nMarginBell"
#define XtNnextEventDelay	"nextEventDelay"
#define XtNnotMapped		"notMapped"
#define XtNnumColorRegisters	"numColorRegisters"
#define XtNnumLock		"numLock"
#define XtNoldXtermFKeys	"oldXtermFKeys"
#define XtNomitTranslation	"omitTranslation"
#define XtNpointerColor		"pointerColor"
#define XtNpointerColorBackground "pointerColorBackground"
#define XtNpointerFont		"pointerFont"
#define XtNpointerMode		"pointerMode"
#define XtNpointerShape		"pointerShape"
#define XtNpopOnBell		"popOnBell"
#define XtNprecompose		"precompose"
#define XtNpreferLatin1		"preferLatin1"
#define XtNprintAttributes	"printAttributes"
#define XtNprintFileImmediate	"printFileImmediate"
#define XtNprintFileOnXError	"printFileOnXError"
#define XtNprintModeImmediate	"printModeImmediate"
#define XtNprintModeOnXError	"printModeOnXError"
#define XtNprintOptsImmediate	"printOptsImmediate"
#define XtNprintOptsOnXError	"printOptsOnXError"
#define XtNprintRawChars	"printRawChars"
#define XtNprinterAutoClose	"printerAutoClose"
#define XtNprinterCommand	"printerCommand"
#define XtNprinterControlMode	"printerControlMode"
#define XtNprinterExtent	"printerExtent"
#define XtNprinterFormFeed	"printerFormFeed"
#define XtNprinterNewLine	"printerNewLine"
#define XtNprivateColorRegisters "privateColorRegisters"
#define XtNptyHandshake		"ptyHandshake"
#define XtNptyInitialErase	"ptyInitialErase"
#define XtNptySttySize		"ptySttySize"
#define XtNquietGrab		"quietGrab"
#define XtNregisDefaultFont	"regisDefaultFont"
#define XtNregisScreenSize	"regisScreenSize"
#define XtNrenderFont		"renderFont"
#define XtNreportCClass		"reportCClass"
#define XtNreportColors		"reportColors"
#define XtNreportFonts		"reportFonts"
#define XtNreportIcons		"reportIcons"
#define XtNreportXRes		"reportXRes"
#define XtNresizeByPixel	"resizeByPixel"
#define XtNresizeGravity	"resizeGravity"
#define XtNretryInputMethod	"retryInputMethod"
#define XtNreverseWrap		"reverseWrap"
#define XtNrightScrollBar	"rightScrollBar"
#define XtNsameName		"sameName"
#define XtNsaveLines		"saveLines"
#define XtNscaleHeight		"scaleHeight"
#define XtNscoFunctionKeys	"scoFunctionKeys"
#define XtNscrollBar		"scrollBar"
#define XtNscrollBarBorder	"scrollBarBorder"
#define XtNscrollKey		"scrollKey"
#define XtNscrollLines		"scrollLines"
#define XtNscrollTtyOutput	"scrollTtyOutput"
#define XtNselectToClipboard	"selectToClipboard"
#define XtNsessionMgt		"sessionMgt"
#define XtNshiftEscape		"shiftEscape"
#define XtNshiftFonts		"shiftFonts"
#define XtNshowBlinkAsBold	"showBlinkAsBold"
#define XtNshowMissingGlyphs	"showMissingGlyphs"
#define XtNshowWrapMarks	"showWrapMarks"
#define XtNsignalInhibit	"signalInhibit"
#define XtNsixelScrolling	"sixelScrolling"
#define XtNsixelScrollsRight	"sixelScrollsRight"
#define XtNsunFunctionKeys	"sunFunctionKeys"
#define XtNsunKeyboard		"sunKeyboard"
#define XtNtcapFunctionKeys	"tcapFunctionKeys"
#define XtNtekGeometry		"tekGeometry"
#define XtNtekInhibit		"tekInhibit"
#define XtNtekSmall		"tekSmall"
#define XtNtekStartup		"tekStartup"
#define XtNtermName		"termName"
#define XtNtiXtraScroll		"tiXtraScroll"
#define XtNtiteInhibit		"titeInhibit"
#define XtNtitleModes		"titleModes"
#define XtNtoolBar		"toolBar"
#define XtNtrimSelection	"trimSelection"
#define XtNttyModes		"ttyModes"
#define XtNunderLine		"underLine"
#define XtNuseBorderClipping	"useBorderClipping"
#define XtNuseClipping		"useClipping"
#define XtNuseInsertMode	"useInsertMode"
#define XtNutf8			"utf8"
#define XtNutf8Fonts		"utf8Fonts"
#define XtNutf8Latin1		"utf8Latin1"
#define XtNutf8SelectTypes	"utf8SelectTypes"
#define XtNutf8Title		"utf8Title"
#define XtNutf8Weblike		"utf8Weblike"
#define XtNutmpDisplayId	"utmpDisplayId"
#define XtNutmpInhibit		"utmpInhibit"
#define XtNvalidShells		"validShells"
#define XtNveryBoldColors	"veryBoldColors"
#define XtNvisualBell		"visualBell"
#define XtNvisualBellDelay	"visualBellDelay"
#define XtNvisualBellLine	"visualBellLine"
#define XtNvt100Graphics	"vt100Graphics"
#define XtNwaitForMap		"waitForMap"
#define XtNwideBoldFont		"wideBoldFont"
#define XtNwideChars		"wideChars"
#define XtNwideFont		"wideFont"
#define XtNxftMaxGlyphMemory	"xftMaxGlyphMemory"
#define XtNxftMaxUnrefFonts	"xftMaxUnrefFonts"
#define XtNxftTrackMemUsage	"xftTrackMemUsage"
#define XtNximFont		"ximFont"
#define XtNxmcAttributes	"xmcAttributes"	/* ncurses-testing */
#define XtNxmcGlitch		"xmcGlitch"	/* ncurses-testing */
#define XtNxmcInline		"xmcInline"	/* ncurses-testing */
#define XtNxmcMoveSGR		"xmcMoveSGR"	/* ncurses-testing */
#define XtNzIconBeep		"zIconBeep"
#define XtNzIconTitleFormat	"zIconTitleFormat"

#define XtCActiveIcon		"ActiveIcon"
#define XtCAllowBoldFonts	"AllowBoldFonts"
#define XtCAllowC1Printable	"AllowC1Printable"
#define XtCAllowColorOps	"AllowColorOps"
#define XtCAllowFontOps		"AllowFontOps"
#define XtCAllowMouseOps	"AllowMouseOps"
#define XtCAllowPasteControls	"AllowPasteControls"
#define XtCAllowScrollLock	"AllowScrollLock"
#define XtCAllowSendEvents	"AllowSendEvents"
#define XtCAllowTcapOps		"AllowTcapOps"
#define XtCAllowTitleOps	"AllowTitleOps"
#define XtCAllowWindowOps	"AllowWindowOps"
#define XtCAltIsNotMeta		"AltIsNotMeta"
#define XtCAltSendsEscape	"AltSendsEscape"
#define XtCAlwaysBoldMode	"AlwaysBoldMode"
#define XtCAlwaysHighlight	"AlwaysHighlight"
#define XtCAlwaysUseMods	"AlwaysUseMods"
#define XtCAnswerbackString	"AnswerbackString"
#define XtCAppcursorDefault	"AppcursorDefault"
#define XtCAppkeypadDefault	"AppkeypadDefault"
#define XtCAssumeAllChars	"AssumeAllChars"
#define XtCAutoScrollLock	"AutoScrollLock"
#define XtCAutoWrap		"AutoWrap"
#define XtCAwaitInput		"AwaitInput"
#define XtCBackarrowKey		"BackarrowKey"
#define XtCBackarrowKeyIsErase	"BackarrowKeyIsErase"
#define XtCBellIsUrgent		"BellIsUrgent"
#define XtCBellOnReset		"BellOnReset"
#define XtCBellSuppressTime	"BellSuppressTime"
#define XtCBoldFont		"BoldFont"
#define XtCBoldMode		"BoldMode"
#define XtCBrokenLinuxOSC	"BrokenLinuxOSC"
#define XtCBrokenSelections	"BrokenSelections"
#define XtCBrokenStringTerm	"BrokenStringTerm"
#define XtCBuffered		"Buffered"
#define XtCBufferedFPS		"BufferedFPS"
#define XtCC132			"C132"
#define XtCCacheDoublesize	"CacheDoublesize"
#define XtCCdXtraScroll		"CdXtraScroll"
#define XtCCharClass		"CharClass"
#define XtCChecksumExtension	"ChecksumExtension"
#define XtCCjkWidth		"CjkWidth"
#define XtCColorAttrMode	"ColorAttrMode"
#define XtCColorEvents		"ColorEvents"
#define XtCColorInnerBorder	"ColorInnerBorder"
#define XtCColorMode		"ColorMode"
#define XtCColumn		"Column"
#define XtCCombiningChars	"CombiningChars"
#define XtCCtrlFKeys		"CtrlFKeys"
#define XtCCurses		"Curses"
#define XtCCursorBar		"CursorBar"
#define XtCCursorBlink		"CursorBlink"
#define XtCCursorBlinkXOR	"CursorBlinkXOR"
#define XtCCursorOffTime	"CursorOffTime"
#define XtCCursorOnTime		"CursorOnTime"
#define XtCCursorTheme		"CursorTheme"
#define XtCCursorUnderLine	"CursorUnderLine"
#define XtCCutNewline		"CutNewline"
#define XtCCutToBeginningOfLine	"CutToBeginningOfLine"
#define XtCDecGraphicsID	"DecGraphicsID"
#define XtCDecTerminalID	"DecTerminalID"
#define XtCDefaultString	"DefaultString"
#define XtCDeleteIsDEL		"DeleteIsDEL"
#define XtCDirectColor		"DirectColor"
#define XtCDisallowedColorOps	"DisallowedColorOps"
#define XtCDisallowedFontOps	"DisallowedFontOps"
#define XtCDisallowedMouseOps	"DisallowedMouseOps"
#define XtCDisallowedPasteControls "DisallowedPasteControls"
#define XtCDisallowedTcapOps	"DisallowedTcapOps"
#define XtCDisallowedWindowOps	"DisallowedWindowOps"
#define XtCDynamicColors	"DynamicColors"
#define XtCEightBitControl	"EightBitControl"
#define XtCEightBitInput	"EightBitInput"
#define XtCEightBitMeta		"EightBitMeta"
#define XtCEightBitOutput	"EightBitOutput"
#define XtCEightBitSelectTypes	"EightBitSelectTypes"
#define XtCEraseSavedLines	"EraseSavedLines"
#define XtCFaceName		"FaceName"
#define XtCFaceNameDoublesize	"FaceNameDoublesize"
#define XtCFaceSize		"FaceSize"
#define XtCFaintIsRelative	"FaintIsRelative"
#define XtCFastScroll		"FastScroll"
#define XtCFont1		"Font1"
#define XtCFont2		"Font2"
#define XtCFont3		"Font3"
#define XtCFont4		"Font4"
#define XtCFont5		"Font5"
#define XtCFont6		"Font6"
#define XtCFont7		"Font7"
#define XtCFontDoublesize	"FontDoublesize"
#define XtCFontWarnings		"FontWarnings"
#define XtCForceBoxChars	"ForceBoxChars"
#define XtCForcePackedFont	"ForcePackedFont"
#define XtCForceXftHeight	"ForceXftHeight"
#define XtCFormatCursorKeys	"FormatCursorKeys"
#define XtCFormatFunctionKeys	"FormatFunctionKeys"
#define XtCFormatKeypadKeys	"FormatKeypadKeys"
#define XtCFormatModifierKeys	"FormatModifierKeys"
#define XtCFormatOtherKeys	"FormatOtherKeys"
#define XtCFormatSpecialKeys	"FormatSpecialKeys"
#define XtCFormatStringKeys	"FormatStringKeys"
#define XtCFreeBoldBox		"FreeBoldBox"
#define XtCFullscreen		"Fullscreen"
#define XtCHighlightColorMode	"HighlightColorMode"
#define XtCHighlightReverse	"HighlightReverse"
#define XtCHighlightSelection	"HighlightSelection"
#define XtCHold			"Hold"
#define XtCHpFunctionKeys	"HpFunctionKeys"
#define XtCHpLowerleftBugCompat	"HpLowerleftBugCompat"
#define XtCI18nSelections	"I18nSelections"
#define XtCIconFont		"IconFont"
#define XtCIconGeometry		"IconGeometry"
#define XtCIconHint		"IconHint"
#define XtCIncrementalGraphics  "IncrementalGraphics"
#define XtCIndicatorFormat	"IndicatorFormat"
#define XtCInitialFont		"InitialFont"
#define XtCInternalBorder	"InternalBorder"
#define XtCJumpScroll		"JumpScroll"
#define XtCKeepClipboard	"KeepClipboard"
#define XtCKeepSelection	"KeepSelection"
#define XtCKeyboardDialect	"KeyboardDialect"
#define XtCKeyboardType		"KeyboardType"
#define XtCLimitFontHeight	"LimitFontHeight"
#define XtCLimitFontWidth	"LimitFontWidth"
#define XtCLimitFontsets	"LimitFontsets"
#define XtCLimitResize		"LimitResize"
#define XtCLimitResponse	"LimitResponse"
#define XtCLocale		"Locale"
#define XtCLocaleFilter		"LocaleFilter"
#define XtCLogInhibit		"LogInhibit"
#define XtCLogfile		"Logfile"
#define XtCLogging		"Logging"
#define XtCLoginShell		"LoginShell"
#define XtCMarginBell		"MarginBell"
#define XtCMaxBufSize		"MaxBufSize"
#define XtCMaxGraphicSize	"MaxGraphicSize"
#define XtCMaxStringParse	"MaxStringParse"
#define XtCMaximized		"Maximized"
#define XtCMenuBar		"MenuBar"	/* internal */
#define XtCMenuHeight		"MenuHeight"
#define XtCMenuLocale		"MenuLocale"
#define XtCMessages		"Messages"
#define XtCMetaSendsEscape	"MetaSendsEscape"
#define XtCMinBufSize		"MinBufSize"
#define XtCMkSamplePass		"MkSamplePass"
#define XtCMkSampleSize		"MkSampleSize"
#define XtCMkWidth		"MkWidth"
#define XtCModifyCursorKeys	"ModifyCursorKeys"
#define XtCModifyFunctionKeys	"ModifyFunctionKeys"
#define XtCModifyKeyboard	"ModifyKeyboard"
#define XtCModifyKeypadKeys	"ModifyKeypadKeys"
#define XtCModifyModifierKeys	"ModifyModifierKeys"
#define XtCModifyOtherKeys	"ModifyOtherKeys"
#define XtCModifySpecialKeys	"ModifySpecialKeys"
#define XtCModifyStringKeys	"ModifyStringKeys"
#define XtCMultiClickTime	"MultiClickTime"
#define XtCMultiScroll		"MultiScroll"
#define XtCNextEventDelay	"NextEventDelay"
#define XtCNotMapped		"NotMapped"
#define XtCNumColorRegisters	"NumColorRegisters"
#define XtCNumLock		"NumLock"
#define XtCOldXtermFKeys	"OldXtermFKeys"
#define XtCOmitTranslation	"OmitTranslation"
#define XtCPointerFont		"PointerFont"
#define XtCPointerMode		"PointerMode"
#define XtCPopOnBell		"PopOnBell"
#define XtCPrecompose		"Precompose"
#define XtCPreferLatin1		"PreferLatin1"
#define XtCPrintAttributes	"PrintAttributes"
#define XtCPrintFileImmediate	"PrintFileImmediate"
#define XtCPrintFileOnXError	"PrintFileOnXError"
#define XtCPrintModeImmediate	"PrintModeImmediate"
#define XtCPrintModeOnXError	"PrintModeOnXError"
#define XtCPrintOptsImmediate	"PrintOptsImmediate"
#define XtCPrintOptsOnXError	"PrintOptsOnXError"
#define XtCPrintRawChars	"PrintRawChars"
#define XtCPrinterAutoClose	"PrinterAutoClose"
#define XtCPrinterCommand	"PrinterCommand"
#define XtCPrinterControlMode	"PrinterControlMode"
#define XtCPrinterExtent	"PrinterExtent"
#define XtCPrinterFormFeed	"PrinterFormFeed"
#define XtCPrinterNewLine	"PrinterNewLine"
#define XtCPrivateColorRegisters "PrivateColorRegisters"
#define XtCPtyHandshake		"PtyHandshake"
#define XtCPtyInitialErase	"PtyInitialErase"
#define XtCPtySttySize		"PtySttySize"
#define XtCQuietGrab		"QuietGrab"
#define XtCRegisDefaultFont	"RegisDefaultFont"
#define XtCRegisScreenSize	"RegisScreenSize"
#define XtCRenderFont		"RenderFont"
#define XtCReportCClass		"ReportCClass"
#define XtCReportColors		"ReportColors"
#define XtCReportFonts		"ReportFonts"
#define XtCReportIcons		"ReportIcons"
#define XtCReportXRes		"ReportXRes"
#define XtCResizeByPixel	"ResizeByPixel"
#define XtCResizeGravity	"ResizeGravity"
#define XtCRetryInputMethod	"RetryInputMethod"
#define XtCReverseWrap		"ReverseWrap"
#define XtCRightScrollBar	"RightScrollBar"
#define XtCSameName		"SameName"
#define XtCSaveLines		"SaveLines"
#define XtCScaleHeight		"ScaleHeight"
#define XtCScoFunctionKeys	"ScoFunctionKeys"
#define XtCScrollBar		"ScrollBar"
#define XtCScrollBarBorder	"ScrollBarBorder"
#define XtCScrollCond		"ScrollCond"
#define XtCScrollLines		"ScrollLines"
#define XtCSelectToClipboard	"SelectToClipboard"
#define XtCSessionMgt		"SessionMgt"
#define XtCShiftEscape		"ShiftEscape"
#define XtCShiftFonts		"ShiftFonts"
#define XtCShowBlinkAsBold	"ShowBlinkAsBold"
#define XtCShowMissingGlyphs	"ShowMissingGlyphs"
#define XtCShowWrapMarks	"ShowWrapMarks"
#define XtCSignalInhibit	"SignalInhibit"
#define XtCSixelScrolling	"SixelScrolling"
#define XtCSixelScrollsRight	"SixelScrollsRight"
#define XtCSunFunctionKeys	"SunFunctionKeys"
#define XtCSunKeyboard		"SunKeyboard"
#define XtCTcapFunctionKeys	"TcapFunctionKeys"
#define XtCTekInhibit		"TekInhibit"
#define XtCTekSmall		"TekSmall"
#define XtCTekStartup		"TekStartup"
#define XtCTermName		"TermName"
#define XtCTiXtraScroll		"TiXtraScroll"
#define XtCTiteInhibit		"TiteInhibit"
#define XtCTitleModes		"TitleModes"
#define XtCToolBar		"ToolBar"
#define XtCTrimSelection	"TrimSelection"
#define XtCTtyModes		"TtyModes"
#define XtCUnderLine		"UnderLine"
#define XtCUseBorderClipping	"UseBorderClipping"
#define XtCUseClipping		"UseClipping"
#define XtCUseInsertMode	"UseInsertMode"
#define XtCUtf8			"Utf8"
#define XtCUtf8Fonts		"Utf8Fonts"
#define XtCUtf8Latin1		"Utf8Latin1"
#define XtCUtf8SelectTypes	"Utf8SelectTypes"
#define XtCUtf8Title		"Utf8Title"
#define XtCUtf8Weblike		"Utf8Weblike"
#define XtCUtmpDisplayId	"UtmpDisplayId"
#define XtCUtmpInhibit		"UtmpInhibit"
#define XtCVT100Graphics	"VT100Graphics"
#define XtCValidShells		"ValidShells"
#define XtCVeryBoldColors	"VeryBoldColors"
#define XtCVisualBell		"VisualBell"
#define XtCVisualBellDelay	"VisualBellDelay"
#define XtCVisualBellLine	"VisualBellLine"
#define XtCWaitForMap		"WaitForMap"
#define XtCWideBoldFont		"WideBoldFont"
#define XtCWideChars		"WideChars"
#define XtCWideFont		"WideFont"
#define XtCXftMaxGlyphMemory	"XftMaxGlyphMemory"
#define XtCXftMaxUnrefFonts	"XftMaxUnrefFonts"
#define XtCXftTrackMemUsage	"XftTrackMemUsage"
#define XtCXimFont		"XimFont"
#define XtCXmcAttributes	"XmcAttributes"	/* ncurses-testing */
#define XtCXmcGlitch		"XmcGlitch"	/* ncurses-testing */
#define XtCXmcInline		"XmcInline"	/* ncurses-testing */
#define XtCXmcMoveSGR		"XmcMoveSGR"	/* ncurses-testing */
#define XtCZIconBeep		"ZIconBeep"
#define XtCZIconTitleFormat	"ZIconTitleFormat"

#if defined(NO_ACTIVE_ICON) && !defined(XtNgeometry)
#define XtNgeometry		"geometry"
#define XtCGeometry		"Geometry"
#endif

#if OPT_COLOR_CLASS
#define XtCCursorColor		"CursorColor"
#define XtCPointerColor		"PointerColor"
#define XtCHighlightColor	"HighlightColor"
#define XtCHighlightTextColor	"HighlightTextColor"
#else
#define XtCCursorColor		XtCForeground
#define XtCPointerColor		XtCForeground
#define XtCHighlightColor	XtCForeground
#define XtCHighlightTextColor	XtCBackground
#endif

/***====================================================================***/

#ifdef __cplusplus
extern "C" {
#endif

struct XTERM_RESOURCE;

/* Tekproc.c */
#if OPT_TEK4014
extern TekWidget getTekWidget(Widget /* w */);
extern int TekGetFontSize (const char * /* param */);
extern int TekInit (void);
extern void ChangeTekColors (TekWidget /* tw */, TScreen * /* screen */, ScrnColors * /* pNew */);
extern void HandleGINInput             PROTO_XT_ACTIONS_ARGS;
extern void TCursorToggle (TekWidget /* tw */, int /* toggle */);
extern void TekCopy (TekWidget /* tw */);
extern void TekEnqMouse (TekWidget /* tw */, int /* c */);
extern void TekExpose (Widget /* w */, XEvent * /* event */, Region /* region */);
extern void TekGINoff (TekWidget /* tw */);
extern void TekRefresh (TekWidget /* tw */);
extern void TekRepaint (TekWidget /* xw */);
extern void TekReverseVideo (XtermWidget /* xw */, TekWidget /* tw */);
extern void TekRun (void);
extern void TekSetFontSize (TekWidget /* tw */, Bool /* fromMenu */, int /* newitem */);
extern void TekSetWinSize (TekWidget /* tw */);
extern void TekSimulatePageButton (TekWidget /* tw */, Bool /* reset */);
#endif

/* button.c */
#define	MotionOff( s, t ) if (!(screen->hide_pointer)) {		\
	    (s)->event_mask |= ButtonMotionMask;			\
	    (s)->event_mask &= ~PointerMotionMask;			\
	    XSelectInput(XtDisplay((t)), XtWindow((t)), (long) (s)->event_mask); }

#define	MotionOn( s, t ) {						\
	    (s)->event_mask &= ~ButtonMotionMask;			\
	    (s)->event_mask |= PointerMotionMask;			\
	    XSelectInput(XtDisplay((t)), XtWindow((t)), (long) (s)->event_mask); }

extern Bool SendMousePosition (XtermWidget /* w */, XEvent* /* event */);
extern XtermMouseModes okSendMousePos(XtermWidget /* xw */);
extern void DiredButton                PROTO_XT_ACTIONS_ARGS;
extern void DisownSelection (XtermWidget /* xw */);
extern void UnhiliteSelection (XtermWidget /* xw */);
extern void HandleCopySelection        PROTO_XT_ACTIONS_ARGS;
extern void HandleInsertSelection      PROTO_XT_ACTIONS_ARGS;
extern void HandleKeyboardSelectEnd    PROTO_XT_ACTIONS_ARGS;
extern void HandleKeyboardSelectExtend PROTO_XT_ACTIONS_ARGS;
extern void HandleKeyboardSelectStart  PROTO_XT_ACTIONS_ARGS;
extern void HandleKeyboardStartExtend  PROTO_XT_ACTIONS_ARGS;
extern void HandlePointerMotion        PROTO_XT_ACTIONS_ARGS;
extern void HandlePointerButton        PROTO_XT_ACTIONS_ARGS;
extern void HandleSelectEnd            PROTO_XT_ACTIONS_ARGS;
extern void HandleSelectExtend         PROTO_XT_ACTIONS_ARGS;
extern void HandleSelectSet            PROTO_XT_ACTIONS_ARGS;
extern void HandleSelectStart          PROTO_XT_ACTIONS_ARGS;
extern void HandleStartExtend          PROTO_XT_ACTIONS_ARGS;
extern void ResizeSelection (TScreen * /* screen */, int /* rows */, int /* cols */);
extern void ScrollSelection (TScreen * /* screen */, int /* amount */,  Bool /* always */);
extern void TrackMouse (XtermWidget /* xw */, int /* func */, const CELL * /* start */, int /* firstrow */, int /* lastrow */);
extern void ViButton                   PROTO_XT_ACTIONS_ARGS;

extern void UnmapSelections (XtermWidget /* xw */);
extern int xtermUtf8ToTextList (XtermWidget /* xw */, XTextProperty * /* text_prop */, char *** /* text_list */, int * /* text_list_count */);
extern void xtermButtonInit (XtermWidget /* xw */);

#if OPT_DEC_LOCATOR
extern void GetLocatorPosition (XtermWidget /* w */);
extern void InitLocatorFilter (XtermWidget /* w */);
#endif	/* OPT_DEC_LOCATOR */

#if OPT_FOCUS_EVENT
extern void SendFocusButton(XtermWidget /* xw */, XFocusChangeEvent * /* event */);
#else
#define SendFocusButton(xw, event) /* nothing */
#endif

#if OPT_PASTE64
extern void AppendToSelectionBuffer (TScreen * /* screen */, unsigned /* c */, String /* selection */);
extern void ClearSelectionBuffer (TScreen * /* screen */, String /* selection */);
extern void CompleteSelection (XtermWidget /* xw */, String * /* args */, Cardinal /* len */);
extern void xtermGetSelection (Widget /* w */, Time /* ev_time */, String * /* params */, Cardinal /* num_params */, Atom * /* targets */);
#endif

#if OPT_READLINE
extern void ReadLineButton             PROTO_XT_ACTIONS_ARGS;
#endif

#if OPT_REPORT_CCLASS
extern void report_char_class(XtermWidget);
#endif

#define IsAscii1(n)  (((n) >= 32 && (n) <= 126))

#if OPT_WIDE_CHARS
#define WideCells(n) (((IChar)(n) >= first_widechar) ? my_wcwidth((wchar_t) (n)) : 1)
#define isWideFrg(n) (((n) == HIDDEN_CHAR) || (WideCells((n)) == 2))
#define isWide(n)    (((IChar)(n) >= first_widechar) && isWideFrg(n))
#define CharWidth(screen, n) (((n) < 256) \
			      ? (IsLatin1(n) ? 1 : 0) \
			      : my_wcwidth((wchar_t) (n)))
#define IsLatin1(n)  (IsAscii1(n) || ((n) >= 160 && (n) <= 255))
#else
#define WideCells(n) 1
#define CharWidth(screen, n) (IsLatin1(n) ? 1 : 0)
#define IsLatin1(n)  (IsAscii1(n) || ((n) >= 160))
#endif

/* cachedCgs.c */
extern CgsEnum getCgsId(XtermWidget /*xw*/, VTwin * /*cgsWin*/, GC /*gc*/);
extern GC freeCgs(XtermWidget /*xw*/, VTwin * /*cgsWin*/, CgsEnum /*cgsId*/);
extern GC getCgsGC(XtermWidget /*xw*/, VTwin * /*cgsWin*/, CgsEnum /*cgsId*/);
extern Pixel getCgsBack(XtermWidget /*xw*/, VTwin * /*cgsWin*/, GC /*gc*/);
extern Pixel getCgsFore(XtermWidget /*xw*/, VTwin * /*cgsWin*/, GC /*gc*/);
extern XTermFonts * getCgsFont(XtermWidget /*xw*/, VTwin * /*cgsWin*/, GC /*gc*/);
extern void clrCgsFonts(XtermWidget /*xw*/, VTwin * /*cgsWin*/, XTermFonts * /*font*/);
extern void copyCgs(XtermWidget /*xw*/, VTwin * /*cgsWin*/, CgsEnum /*dstCgsId*/, CgsEnum /*srcCgsId*/);
extern void redoCgs(XtermWidget /*xw*/, Pixel /*fg*/, Pixel /*bg*/, CgsEnum /*cgsId*/);
extern void setCgsBack(XtermWidget /*xw*/, VTwin * /*cgsWin*/, CgsEnum /*cgsId*/, Pixel /*bg*/);
extern void setCgsCSet(XtermWidget /*xw*/, VTwin * /*cgsWin*/, CgsEnum /*cgsId*/, unsigned /*cset*/);
extern void setCgsFont(XtermWidget /*xw*/, VTwin * /*cgsWin*/, CgsEnum /*cgsId*/, XTermFonts * /*font*/);
extern void setCgsFont2(XtermWidget /*xw*/, VTwin * /*cgsWin*/, CgsEnum /*cgsId*/, XTermFonts * /*font*/, unsigned /*which*/);
extern void setCgsFore(XtermWidget /*xw*/, VTwin * /*cgsWin*/, CgsEnum /*cgsId*/, Pixel /*fg*/);
extern void setCgsLine(XtermWidget /*xw*/, VTwin * /*cgsWin*/, CgsEnum /*cgsId*/, Pixel /*fg*/);
extern void swapCgs(XtermWidget /*xw*/, VTwin * /*cgsWin*/, CgsEnum /*dstCgsId*/, CgsEnum /*srcCgsId*/);

#ifdef NO_LEAKS
extern void noleaks_cachedCgs (XtermWidget /* xw */);
#endif

/* charproc.c */
extern Bool CheckBufPtrs (TScreen * /* screen */);
extern Bool set_cursor_gcs (XtermWidget /* xw */);
extern char * vt100ResourceToString (XtermWidget /* xw */, const char * /* name */);
extern int VTInit (XtermWidget /* xw */);
extern Bool FindFontSelection (XtermWidget /* xw */, const char * /* atom_name */, Bool /* justprobe */);
extern void HideCursor (XtermWidget /* xw */);
extern void RestartBlinking(XtermWidget /* xw */);
extern void ShowCursor (XtermWidget /* xw */);
extern void SwitchBufPtrs (XtermWidget /* xw */, int /* toBuf */);
extern void ToggleAlternate (XtermWidget /* xw */);
extern void VTInitTranslations (void);
extern GCC_NORETURN void VTReset (XtermWidget /* xw */, int /* full */, int /* saved */);
extern void VTRun (XtermWidget /* xw */);
extern void dotext (XtermWidget /* xw */, DECNRCM_codes /* charset */, IChar * /* buf */, Cardinal /* len */);
extern void getKeymapResources(Widget /* w */, const char * /*mapName */, const char * /* mapClass */, const char * /* type */, void * /* result */, size_t /* size */);
extern void initBorderGC (XtermWidget /* xw */, VTwin * /* win */);
extern void lookupSelectUnit(XtermWidget /* xw */, Cardinal /* item */, String /* value */);
extern void releaseCursorGCs(XtermWidget /*xw*/);
extern void releaseWindowGCs(XtermWidget /*xw*/, VTwin * /*win*/);
extern void resetCharsets (TScreen * /* screen */);
extern void resetMargins (XtermWidget /* xw */);
extern void restoreCharsets (TScreen * /* screen */, const DECNRCM_codes * /* source */);
extern void saveCharsets (TScreen * /* screen */, DECNRCM_codes * /* target */);
extern void set_max_col(TScreen * /* screen */, int /* cols */);
extern void set_max_row(TScreen * /* screen */, int /* rows */);
extern void unparse_end (XtermWidget /* xw */);
extern void unparseputc (XtermWidget /* xw */, int /* c */);
extern void unparseputc1 (XtermWidget /* xw */, int /* c */);
extern void unparseputn (XtermWidget /* xw */, unsigned /* n */);
extern void unparseputs (XtermWidget /* xw */, const char * /* s */);
extern void unparseseq (XtermWidget /* xw */, ANSI * /* ap */);
extern void v_write (int /* f */, const Char * /* d */, size_t /* len */);
extern void xtermAddInput (Widget /* w */);
extern void xtermDecodeSCS (XtermWidget /* xw */, int /* which */, int /* sgroup */, int /* prefix */, int /* suffix */);

#if OPT_BLINK_CURS
extern void ToggleCursorBlink(XtermWidget /* xw */);
#endif

#if OPT_BLINK_TEXT
extern Bool LineHasBlinking(TScreen * /* screen */, CLineData * /* ld */);
#endif

#if OPT_INPUT_METHOD
extern TInput *lookupTInput (XtermWidget /* xw */, Widget /* w */);
#endif

#if OPT_ISO_COLORS
extern void SGR_Background (XtermWidget /* xw */, int /* color */);
extern void SGR_Foreground (XtermWidget /* xw */, int /* color */);
extern void setExtendedColors (XtermWidget /* xw */);
#endif

#if OPT_QUERY_ALLOW
extern void unparse_disallowed_ops (XtermWidget /* xw */, char * /* value */);
extern void unparse_allowable_ops (XtermWidget /* xw */, char * /* value */);
#endif

#ifdef NO_LEAKS
extern void noleaks_charproc (void);
#endif

/* charsets.c */
extern unsigned xtermCharSetIn (XtermWidget /* xw */, unsigned /* code */, DECNRCM_codes /* charset */);
extern Cardinal xtermCharSetOut (XtermWidget /* xw */, Cardinal /* length */, DECNRCM_codes /* charset */);
extern int xtermCharSetDec (XtermWidget /* xw */, IChar /* ch */, DECNRCM_codes /* cs */);

/* cursor.c */
extern int CursorCol (XtermWidget /* xw */);
extern int CursorRow (XtermWidget /* xw */);
extern void AdjustSavedCursor (XtermWidget /* xw */, int /* adjust */);
extern void CarriageReturn (XtermWidget /* xw */);
extern void CursorBack (XtermWidget /* xw */, int /* n */);
extern void CursorDown (TScreen * /* screen */, int /* n */);
extern void CursorForward (XtermWidget /* xw */, int /* n */);
extern void CursorNextLine (XtermWidget /* xw */, int /* count */);
extern void CursorPrevLine (XtermWidget /* xw */, int /* count */);
extern void CursorRestore2 (XtermWidget /* xw */, SavedCursor * /* sc */);
extern void CursorRestore (XtermWidget /* xw */);
extern void CursorSave2 (XtermWidget /* xw */, SavedCursor * /* sc */);
extern void CursorSave (XtermWidget /* xw */);
extern void CursorSet (TScreen * /* screen */, int /* row */, int /* col */, unsigned /* flags */);
extern void CursorUp (TScreen * /* screen */, int /* n */);
extern void RevIndex (XtermWidget /* xw */, int /* amount */);
extern void xtermIndex (XtermWidget /* xw */, int /* amount */);

#if OPT_TRACE
extern int set_cur_col(TScreen * /* screen */, int /* value */);
extern int set_cur_row(TScreen * /* screen */, int /* value */);
#else
#define set_cur_col(screen, value) screen->cur_col = value
#define set_cur_row(screen, value) \
	do { \
	    int row_value = value; \
	    if_STATUS_LINE(screen, { \
		row_value = LastRowNumber(screen); \
	    }); \
	    screen->cur_row = row_value; \
	} while (0)
#endif

/* cursorfont.c */
extern Cursor CreateAlternateCursorFontCursor(Display * /* dpy */, char const * /* cursorfont */, unsigned int /* which */);
extern int CursorFontIndexFromShapeName(char const * /* shapename */);

/* doublechr.c */
extern void xterm_DECDHL (XtermWidget /* xw */, Bool /* top */);
extern void xterm_DECSWL (XtermWidget /* xw */);
extern void xterm_DECDWL (XtermWidget /* xw */);
extern void xterm_ResetDouble(XtermWidget /* xw */);
#if OPT_DEC_CHRSET
extern GC xterm_DoubleGC(XTermDraw * /* params */, GC /* old_gc */, int * /* inxp */);
#if OPT_RENDERFONT
extern XTermXftFonts * xterm_DoubleFT(XTermDraw * /* params */, unsigned /* chrset */, unsigned /* attr_flags */);
extern void freeall_DoubleFT(XtermWidget /* xw */);
#endif
#endif

/* input.c */
extern unsigned xtermParamToState (XtermWidget /* xw */, unsigned /* param */);
extern unsigned xtermStateToParam (XtermWidget /* xw */, unsigned /* state */);
extern Bool xtermDeleteIsDEL (XtermWidget /* xw */);
extern void Input (XtermWidget /* xw */, XKeyEvent */* event */, Bool /* eightbit */);
extern void StringInput (XtermWidget /* xw */, const Char * /* string */, size_t /* nbytes */);

#if OPT_NUM_LOCK
extern void VTInitModifiers(XtermWidget /* xw */);
#else
#define VTInitModifiers(xw) /* nothing */
#endif

/* linedata.c */
extern LineData *getLineData(TScreen * /* screen */, int /* row */);
extern void copyLineData(LineData * /* dst */, CLineData * /* src */);
extern void initLineData(XtermWidget /* xw */);

extern CellData *newCellData(XtermWidget /* xw */, Cardinal /* count */);
extern void saveCellData(TScreen * /* screen */, CellData * /* data */, Cardinal /* cell */, CLineData * /* ld */, XTermRect * /* limits */, int /* column */);
extern void restoreCellData(TScreen * /* screen */, const CellData * /* data */, Cardinal /* cell */, LineData * /* ld */, XTermRect * /* limits */, int /* column */);

/* main.c */
#define ENVP_ARG /**/

extern int main (int /* argc */, char ** /* argv */ ENVP_ARG);
extern int xtermDisabledChar(void);
extern int GetBytesAvailable (Display * /* dpy */);
extern int kill_process_group (int /* pid */, int /* sig */);
extern int nonblocking_wait (void);
extern Atom CachedInternAtom(Display * /* display */, const char * /* name */);

extern int get_tty_erase(int /* fd */, int /* default_erase */, const char * /* tag */);
extern int get_tty_lnext(int /* fd */, int /* default_lnext */, const char * /* tag */);

#if OPT_PTY_HANDSHAKE
extern void first_map_occurred (void);
#else
#define first_map_occurred() /* nothing */
#endif

extern GCC_NORETURN void Exit (int /* n */);

#ifndef SIG_ATOMIC_T
#define SIG_ATOMIC_T int
#endif

#if OPT_WIDE_CHARS
extern unsigned first_widechar;
extern int (*my_wcwidth)(wchar_t);
#endif

/* menu.c */
extern void do_hangup          PROTO_XT_CALLBACK_ARGS;
extern void repairSizeHints    (void);
extern void show_8bit_control  (Bool /* value */);

/* misc.c */

#define TIMESTAMP_LEN 20	/* length of TIMESTAMP_FMT */

extern Bool AllocateTermColor(XtermWidget, ScrnColors *, int, const char *, Bool);
extern Boolean allocateBestRGB(XtermWidget /* xw */, XColor * /* def */);
extern Boolean validProgram(const char * /* pathname */);
extern Boolean xtermGetWinAttrs(Display * /* dpy */, Window /* win */, XWindowAttributes * /* attrs */);
extern Boolean xtermGetWinProp(Display * /* dpy */, Window /* win */, Atom /* property */, long /* long_offset */, long /* long_length */, Atom /* req_type */, Atom * /* actual_type_return */, int * /* actual_format_return */, unsigned long * /* nitems_return */, unsigned long * /* bytes_after_return */, unsigned char ** /* prop_return */);
extern Boolean xtermIsIconified (XtermWidget /* xw */);
extern Cursor make_colored_cursor (unsigned /* cursorindex */, unsigned long /* fg */, unsigned long /* bg */);
extern FILE * create_printfile(XtermWidget /* xw */, const char * /* suffix */);
extern OptionHelp * sortedOpts(OptionHelp *, XrmOptionDescRec *, Cardinal);
extern String xtermEnvLocale (void);
extern Widget xtermOpenApplication (XtAppContext * /* app_context_return */, String /* application_class */, XrmOptionDescRec */* options */, Cardinal /* num_options */, int * /* argc_in_out */, char **/* argv_in_out */, String * /* fallback_resources */, WidgetClass /* widget_class */, ArgList /* args */, Cardinal /* num_args */);
extern Window WMFrameWindow (XtermWidget /* xw */);
extern XtInputMask xtermAppPending (void);
extern XrmOptionDescRec * sortedOptDescs (const XrmOptionDescRec *, Cardinal);
extern XtermWidget getXtermWidget (Widget /* w */);
extern XVisualInfo *getVisualInfo (XtermWidget /* xw */);
extern char *udk_lookup (XtermWidget /* xw */, int /* keycode */, int * /* len */);
extern char *xtermEnvEncoding (void);
extern char *xtermFindShell (char * /* leaf */, Bool /* warning */);
extern char *xtermFormatSGR (XtermWidget /* xw */, char * /* target */, unsigned /* attrs */, int /* fg */, int /* bg */);
extern const char *SysErrorMsg (int /* n */);
extern const char *SysReasonMsg (int /* n */);
extern int ResetAnsiColorRequest (XtermWidget, char *, int);
extern int XStrCmp (char * /* s1 */, char * /* s2 */);
extern int creat_as (uid_t /* uid */, gid_t /* gid */, Bool /* append */, char * /* pathname */, unsigned /* mode */);
extern int getVisualDepth (XtermWidget /* xw */);
extern int ignore_x11_error(Display * /* dpy */, XErrorEvent * /* event */);
extern int open_userfile (uid_t /* uid */, gid_t /* gid */, char * /* path */, Bool /* append */);
extern int update_winsize (TScreen * /* screen */, int /* rows */, int /* cols */, int /* height */, int /* width */);
extern int xerror (Display * /* d */, XErrorEvent * /* ev */);
extern int xioerror (Display * /* dpy */);
extern int xtermClosestColor (XtermWidget /* xw */, int /* red */, int /* green */, int /* blue */);
extern int xtermResetIds (TScreen * /* screen */);
extern void Bell (XtermWidget /* xw */, int /* which */, int /* percent */);
extern void ChangeGroup(XtermWidget /* xw */, const char * /* attribute */, char * /* value */);
extern void ChangeIconName (XtermWidget /* xw */, char * /* name */);
extern void ChangeTitle (XtermWidget /* xw */, char * /* name */);
extern GCC_NORETURN void Cleanup (int /* code */);
extern void HandleBellPropertyChange   PROTO_XT_EV_HANDLER_ARGS;
extern void HandleEightBitKeyPressed   PROTO_XT_ACTIONS_ARGS;
extern void HandleEnterWindow          PROTO_XT_EV_HANDLER_ARGS;
extern void HandleFocusChange          PROTO_XT_EV_HANDLER_ARGS;
extern void HandleInterpret            PROTO_XT_ACTIONS_ARGS;
extern void HandleKeyPressed           PROTO_XT_ACTIONS_ARGS;
extern void HandleLeaveWindow          PROTO_XT_EV_HANDLER_ARGS;
extern void HandleSpawnTerminal        PROTO_XT_ACTIONS_ARGS;
extern void HandleStringEvent          PROTO_XT_ACTIONS_ARGS;
extern void NormalExit (void);
extern void Panic (const char * /* s */, int /* a */);
extern void Redraw (void);
extern void ReverseOldColors (XtermWidget /* xw */);
extern GCC_NORETURN void SysError (int /* i */);
extern void VisualBell (void);
extern void cleanup_colored_cursor (void);
extern void do_ansi_rqm (XtermWidget /* xw */, int /* nparam */, int * /* params */);
extern void do_dcs (XtermWidget /* xw */, Char * /* buf */, size_t /* len */);
extern void do_dec_rqm (XtermWidget /* xw */, int /* nparam */, int * /* params */);
extern void do_osc (XtermWidget /* xw */, Char * /* buf */, size_t /* len */, int /* final */);
extern void do_xevents (XtermWidget /* xw */);
extern void end_tek_mode (void);
extern void end_vt_mode (void);
extern void free_string(String value);
extern void hide_tek_window (void);
extern void hide_vt_window (void);
extern GCC_NORETURN void ice_error (IceConn /* iceConn */);
extern void init_colored_cursor (Display * /* dpy */);
extern void parse_ansi_params(ANSI * /* params */, const char ** /* string */);
extern void reset_decudk (XtermWidget /* xw */);
extern void set_tek_visibility (Bool /* on */);
extern void set_vt_visibility (Bool /* on */);
extern void switch_modes (Bool /* tovt */);
extern void timestamp_filename(char * /* dst */, const char * /* src */);
extern void xevents (XtermWidget /* xw */);
extern GCC_NORETURN void xt_error (String /* message */);
extern void xtermBell(XtermWidget /* xw */, int /* which */, int /* percent */);
extern void xtermCopyEnv (char ** /* oldenv */);
extern void xtermDisplayPointer (XtermWidget /* xw */);
extern void xtermDeiconify (XtermWidget /* xw */);
extern void xtermEmbedWindow (Window /* winToEmbedInfo */);
extern void xtermIconify (XtermWidget /* xw */);
extern void xtermLoadIcon (XtermWidget /* xw */, const char * /* icon_hint */);
extern void xtermPerror (const char * /*fmt*/,...) GCC_PRINTFLIKE(1,2);
extern void xtermSetenv (const char * /* var */, const char * /* value */);
extern void xtermSetupPointer (XtermWidget /* xw */, const char * /* theShape */);
extern void xtermSetWinSize (XtermWidget /* xw */);
extern void xtermShowPointer (XtermWidget /* xw */, Bool /* enable */);
extern void xtermUnsetenv (const char * /* var */);
extern void xtermWarning (const char * /*fmt*/,...) GCC_PRINTFLIKE(1,2);

extern Boolean xtermPopTitle(TScreen * /* screen */, int /* which */, SaveTitle * /* item */);
extern void xtermPushTitle(TScreen * /* screen */, int /* which */, SaveTitle * /* item */);
extern void xtermFreeTitle(SaveTitle *item);
extern void xtermReportTitleStack(XtermWidget /* xw */);

#if OPT_DABBREV
extern void HandleDabbrevExpand        PROTO_XT_ACTIONS_ARGS;
#endif

#if OPT_DIRECT_COLOR
extern int getDirectColor(XtermWidget /* xw */, int /* red */, int /* green */, int /* blue */);
#endif /* OPT_DIRECT_COLOR */

#if USE_DOUBLE_BUFFER
extern void xtermFlushDbe(XtermWidget /* xw */);
extern void xtermTimedDbe(XtermWidget /* xw */);
#define xtermNeedSwap(xw,why)	TScreenOf(xw)->needSwap |= (why)
#else
#define xtermFlushDbe(xw)	/* nothing */
#define xtermTimedDbe(xw)	/* nothing */
#define xtermNeedSwap(xw,why)	/* nothing */
#endif /* USE_DOUBLE_BUFFER */

#if OPT_EXEC_XTERM
extern char *ProcGetCWD(pid_t /* pid */);
#else
#define ProcGetCWD(pid) NULL
#endif

#if OPT_ISO_COLORS
extern Boolean AllocOneColor(XtermWidget /* xw */, XColor * /* def */);
extern Boolean QueryOneColor(XtermWidget /* xw */, XColor * /* def */);
#else
#define AllocOneColor(xw, def) ((def)->pixel = 0)
#define QueryOneColor(xw, def) ((def)->red = (def)->green = (def)->blue = 0)
#endif

#if OPT_MAXIMIZE
extern int QueryMaximize (XtermWidget /* xw */, unsigned * /* width */, unsigned * /* height */);
extern void HandleDeIconify            PROTO_XT_ACTIONS_ARGS;
extern void HandleIconify              PROTO_XT_ACTIONS_ARGS;
extern void HandleMaximize             PROTO_XT_ACTIONS_ARGS;
extern void HandleRestoreSize          PROTO_XT_ACTIONS_ARGS;
extern void RequestMaximize (XtermWidget /* xw */, int /* maximize */);
#endif

#if OPT_REPORT_ICONS
extern void report_icons(const char *fmt,...) GCC_PRINTFLIKE(1,2);
#define ReportIcons(params)	report_icons params
#else
#define ReportIcons(params)	TRACE(params)
#endif

#if OPT_SCROLL_LOCK
extern void GetScrollLock (TScreen * /* screen */);
extern void HandleScrollLock           PROTO_XT_ACTIONS_ARGS;
extern void ShowScrollLock (TScreen * /* screen */, Bool /* enable */);
extern void SetScrollLock (TScreen * /* screen */, Bool /* enable */);
extern void xtermShowLED (TScreen * /* screen */, Cardinal /* led_number */, Bool /* enable */);
extern void xtermClearLEDs (TScreen * /* screen */);
#else
#define ShowScrollLock(screen, enable) /* nothing */
#define SetScrollLock(screen, enable) /* nothing */
#define GetScrollLock(screen) /* nothing */
#endif

#if OPT_SELECTION_OPS
extern void HandleExecFormatted        PROTO_XT_ACTIONS_ARGS;
extern void HandleExecSelectable       PROTO_XT_ACTIONS_ARGS;
extern void HandleInsertFormatted      PROTO_XT_ACTIONS_ARGS;
extern void HandleInsertSelectable     PROTO_XT_ACTIONS_ARGS;
#endif

#if OPT_SESSION_MGT
extern void xtermCloseSession (void);
extern void xtermOpenSession (void);
extern void xtermUpdateRestartCommand(XtermWidget /* xw */);
#else
#define xtermCloseSession() /* nothing */
#define xtermOpenSession() /* nothing */
#define xtermUpdateRestartCommand(xw) /* nothing */
#endif

#if OPT_WIDE_CHARS
extern Bool xtermEnvUTF8(void);
#else
#define xtermEnvUTF8() False
#endif

#if OPT_XTERM_SGR
extern void xtermPushSGR (XtermWidget /* xw */, int /* value */);
extern void xtermPopSGR (XtermWidget /* xw */);
extern void xtermReportSGR (XtermWidget /* xw */, XTermRect * /* value */);
extern void xtermPushColors (XtermWidget /* xw */, int /* value */);
extern void xtermPopColors (XtermWidget /* xw */, int /* value */);
extern void xtermReportColors (XtermWidget /* xw */);
#endif

#ifdef ALLOWLOGGING
extern void StartLog (XtermWidget /* xw */);
extern void CloseLog (XtermWidget /* xw */);
extern void FlushLog (XtermWidget /* xw */);
#else
#define FlushLog(xw) /*nothing*/
#endif

/* print.c */
extern Bool xtermHasPrinter (XtermWidget /* xw */);
extern PrinterFlags *getPrinterFlags (XtermWidget /* xw */, String * /* params */, Cardinal * /* param_count */);
extern int xtermPrinterControl (XtermWidget /* xw */, int /* chr */);
extern void setPrinterControlMode (XtermWidget /* xw */, int /* mode */);
extern void xtermAutoPrint (XtermWidget /* xw */, unsigned /* chr */);
extern void xtermMediaControl (XtermWidget /* xw */, int /* param */, int /* private_seq */);
extern void xtermPrintScreen (XtermWidget /* xw */, Bool /* use_DECPEX */, PrinterFlags * /* p */);
extern void xtermPrintEverything (XtermWidget /* xw */, PrinterFlags * /* p */);
extern void xtermPrintImmediately (XtermWidget /* xw */, String /* filename */, int /* opts */, int /* attributes */);
extern void xtermPrintOnXError (XtermWidget /* xw */, int /* n */);

#if OPT_SCREEN_DUMPS
/* html.c */
extern void xtermDumpHtml (XtermWidget /* xw */);
extern char *PixelToCSSColor(XtermWidget /* xw */, Pixel /* p */);
/* svg.c */
extern void xtermDumpSvg (XtermWidget /* xw */);
#endif

/* ptydata.c */
#define PtySelect fd_set

extern Bool decodeUtf8 (TScreen * /* screen */, PtyData * /* data */);
extern int readPtyData (XtermWidget /* xw */, PtySelect * /* select_mask */, PtyData * /* data */);
extern void fillPtyData (XtermWidget /* xw */, PtyData * /* data */, const char * /* value */, size_t /* length */);
extern void initPtyData (PtyData ** /* data */);
extern void trimPtyData (XtermWidget /* xw */, PtyData * /* data */);

#ifdef NO_LEAKS
extern void noleaks_ptydata ( void );
#endif

#if OPT_WIDE_CHARS
extern Boolean isValidUTF8 (Char * /* lp */);
extern Char *convertToUTF8 (Char * /* lp */, unsigned /* c */);
extern Char *convertFromUTF8 (Char * /* lp */, unsigned * /* cp */);
extern IChar nextPtyData (TScreen * /* screen */, PtyData * /* data */);
extern PtyData * fakePtyData (PtyData * /* result */, Char * /* next */, Char * /* last */);
extern void switchPtyData (TScreen * /* screen */, int /* f */);
extern void writePtyData (int /* f */, IChar * /* d */, size_t /* len */);

#define morePtyData(screen, data) \
	(((data)->last > (data)->next) \
	 ? (((screen)->utf8_inparse && !(data)->utf_size) \
	    ? decodeUtf8(screen, data) \
	    : True) \
	 : False)

#define skipPtyData(data, result) \
	do { \
	    result = (data)->utf_data; \
	    (data)->next += (data)->utf_size; \
	    (data)->utf_size = 0; \
	} while (0)
#else
#define morePtyData(screen, data) ((data)->last > (data)->next)
#define nextPtyData(screen, data) (IChar) (*((data)->next++) & \
					   (screen->output_eight_bits \
					    ? 0xff \
					    : 0x7f))
#define writePtyData(f,d,len) v_write(f,d,len)
#endif

/* screen.c */

/*
 * See http://standards.freedesktop.org/wm-spec/wm-spec-latest.html
 */
#define _NET_WM_STATE_REMOVE	0	/* remove/unset property */
#define _NET_WM_STATE_ADD	1	/* add/set property */
#define _NET_WM_STATE_TOGGLE	2	/* toggle property */

extern Bool non_blank_line (TScreen */* screen */, int /* row */, int /* col */, int /* len */);
extern Char * allocScrnData (TScreen * /* screen */, unsigned /* nrow */, unsigned /* ncol */, Bool /* bottom */);
extern ScrnBuf allocScrnBuf (XtermWidget /* xw */, unsigned /* nrow */, unsigned /* ncol */, ScrnPtr * /* addr */);
extern ScrnBuf scrnHeadAddr (TScreen * /* screen */, ScrnBuf /* base */, unsigned /* offset */);
extern size_t ScrnPointers (TScreen * /* screen */, size_t /* len */);
extern void ClearBufRows (XtermWidget /* xw */, int /* first */, int /* last */);
extern void ClearCells (XtermWidget /* xw */, int /* flags */, unsigned /* len */, int /* row */, int /* col */);
extern void CopyCells (TScreen * /* screen */, LineData * /* src */, LineData * /* dst */, int /* col */, int /* len */, Bool /* down */);
extern void FullScreen (XtermWidget /* xw */, int /* mode */);
extern void FreeMarkGCs (XtermWidget /* xw */);
extern void ResetHiddenHint (XtermWidget /* xw */);
extern void ScreenResize (XtermWidget /* xw */, int /* width */, int /* height */, unsigned * /* flags */);
extern void ScrnAllocBuf (XtermWidget /* xw */);
extern void ScrnClearCells (XtermWidget /* xw */, int /* row */, int /* col */, unsigned /* len */);
extern void ScrnDeleteChar (XtermWidget /* xw */, unsigned /* n */);
extern void ScrnDeleteCol (XtermWidget /* xw */, unsigned /* n */);
extern void ScrnDeleteLine (XtermWidget /* xw */, ScrnBuf /* sb */, int /* n */, int /* last */, unsigned /* where */);
extern void ScrnDisownSelection (XtermWidget /* xw */);
extern void ScrnFillRectangle (XtermWidget /* xw */, XTermRect *,  int /* value */,  DECNRCM_codes /* charset */, unsigned /* flags */, Bool /* keepColors */);
extern void ScrnInsertChar (XtermWidget /* xw */, unsigned /* n */);
extern void ScrnInsertCol (XtermWidget /* xw */, unsigned /* n */);
extern void ScrnInsertLine (XtermWidget /* xw */, ScrnBuf /* sb */, int /* last */, int /* where */, unsigned /* n */);
extern void ScrnRefresh (XtermWidget /* xw */, int /* toprow */, int /* leftcol */, int /* nrows */, int /* ncols */, Bool /* force */);
extern void ScrnUpdate (XtermWidget /* xw */, int /* toprow */, int /* leftcol */, int /* nrows */, int /* ncols */, Bool /* force */);
extern void ScrnWriteText (XtermWidget /* xw */, Cardinal /* offset */, Cardinal /* length */, unsigned /* flags */, CellColor /* cur_fg_bg */);
extern void ShowWrapMarks (XtermWidget /* xw */, int /* row */, CLineData * /* ld */);
extern void setupLineData (TScreen * /* screen */, ScrnBuf /* base */, Char * /* data */, unsigned /* nrow */, unsigned /* ncol */, Bool /* bottom */);
extern void xtermParseRect (XtermWidget /* xw */, int, int *, XTermRect *);

#if OPT_TRACE && OPT_TRACE_FLAGS
extern int  LineTstFlag(LineData /* ld */, int /* flag */);
extern void LineClrFlag(LineData /* ld */, int /* flag */);
extern void LineSetFlag(LineData /* ld */, int /* flag */);
#else

#define LineFlags(ld)         GetLineFlags(ld)

#define LineClrFlag(ld, flag) SetLineFlags(ld, (GetLineFlags(ld) & ~ (flag)))
#define LineSetFlag(ld, flag) SetLineFlags(ld, (GetLineFlags(ld) | (flag)))
#define LineTstFlag(ld, flag) ((GetLineFlags(ld) & flag) != 0)

#endif /* OPT_TRACE && OPT_TRACE_FLAGS */

#define LineClrBlinked(ld) LineClrFlag(ld, LINEBLINKED)
#define LineSetBlinked(ld) LineSetFlag(ld, LINEBLINKED)
#define LineTstBlinked(ld) LineTstFlag(ld, LINEBLINKED)

#define LineClrWrapped(ld) LineClrFlag(ld, LINEWRAPPED)
#define LineSetWrapped(ld) LineSetFlag(ld, LINEWRAPPED)
#define LineTstWrapped(ld) LineTstFlag(ld, LINEWRAPPED)

#define ScrnHaveSelection(screen) \
			((screen)->startH.row != (screen)->endH.row \
			|| (screen)->startH.col != (screen)->endH.col)

#define ScrnAreRowsInSelection(screen, first, last) \
	((last) >= (screen)->startH.row && (first) <= (screen)->endH.row)

#define ScrnIsRowInSelection(screen, line) \
	((line) >= (screen)->startH.row && (line) <= (screen)->endH.row)

#define ScrnHaveRowMargins(screen) \
			((screen)->top_marg != 0 \
			|| ((screen)->bot_marg != screen->max_row))

#define ScrnIsRowInMargins(screen, line) \
	((line) >= (screen)->top_marg && (line) <= (screen)->bot_marg)

#define ScrnHaveColMargins(screen) \
			((screen)->rgt_marg > (screen)->max_col)

#define ScrnIsColInMargins(screen, col) \
	((col) >= (screen)->lft_marg && (col) <= (screen)->rgt_marg)

/*
 * If the vertical scrolling margins are active, they will be something other
 * than the first/last row of the visible screen, as well as being distinct.
 */
#define IsTopBottomMode(xw)	(ScrnTopMargin(xw) < ScrnBottomMargin(xw))
#define ScrnTopMargin(xw)	TScreenOf(xw)->top_marg
#define ScrnBottomMargin(xw)	TScreenOf(xw)->bot_marg

/*
 * Left/right horizontal scrolling margins are only active when DECLRMM is.
 */
#define IsLeftRightMode(xw) ((xw)->flags & LEFT_RIGHT)
#define ScrnLeftMargin(xw)  (IsLeftRightMode(xw) \
			     ? TScreenOf(xw)->lft_marg \
			     : 0)
#define ScrnRightMargin(xw) (IsLeftRightMode(xw) \
			     ? TScreenOf(xw)->rgt_marg \
			     : MaxCols(TScreenOf(xw)) - 1)

#if OPT_DEC_RECTOPS
extern void ScrnCopyRectangle (XtermWidget /* xw */, XTermRect *, int, int *);
extern void ScrnMarkRectangle (XtermWidget /* xw */, XTermRect *, Bool, int, int *);
extern void ScrnWipeRectangle (XtermWidget /* xw */, XTermRect *);
extern void xtermCheckRect(XtermWidget /* xw */, int /* nparam */, int */* params */, int * /* result */);
#endif

#if OPT_WIDE_CHARS
extern void ChangeToWide(XtermWidget /* xw */);
#endif

/* scrollback.c */
extern LineData *getScrollback (TScreen * /* screen */, int /* row */);
extern LineData *addScrollback (TScreen * /* screen */);
extern void deleteScrollback (TScreen * /* screen */);

/* scrollbar.c */
extern void DoResizeScreen (XtermWidget /* xw */);
extern void HandleScrollBack           PROTO_XT_ACTIONS_ARGS;
extern void HandleScrollForward        PROTO_XT_ACTIONS_ARGS;
extern void HandleScrollTo             PROTO_XT_ACTIONS_ARGS;
extern void ResizeScrollBar (XtermWidget /* xw */);
extern void ScrollBarDrawThumb (XtermWidget /* xw */, int /* mode */);
extern void ScrollBarOff (XtermWidget /* xw */);
extern void ScrollBarOn (XtermWidget /* xw */, Bool /* init */);
extern void ScrollBarReverseVideo (Widget /* scrollWidget */);
extern void ToggleScrollBar (XtermWidget /* xw */);
extern void WindowScroll (XtermWidget /* xw */, int /* top */, Bool /* always */);

#ifdef SCROLLBAR_RIGHT
extern void updateRightScrollbar(XtermWidget /* xw */);
#else
#define updateRightScrollbar(xw) /* nothing */
#endif

/* tabs.c */
extern Bool TabToNextStop (XtermWidget /* xw */);
extern Bool TabToPrevStop (XtermWidget /* xw */);
extern void TabClear (Tabs /* tabs */, int /* col */);
extern void TabReset (Tabs /* tabs */);
extern void TabSet (Tabs /* tabs */, int /* col */);
extern void TabZonk (Tabs /* tabs */);
extern Bool TabIsSet (Tabs /* tabs */, int /* col */);

/* util.c */
extern Boolean AssignFgColor (XtermWidget /* xw */, Pixel /* bg */);
extern Boolean AssignBgColor (XtermWidget /* xw */, Pixel /* bg */);
extern Boolean isDefaultBackground (const char * /* name */);
extern Boolean isDefaultForeground (const char * /* name */);
extern CgsEnum whichXtermCgs (XtermWidget /* xw */, unsigned /* attr_flags */, Bool /* hilite */);
extern GC updatedXtermGC (XtermWidget /* xw */, unsigned /* flags */, CellColor /* fg_bg */, Bool /* hilite */);
extern Pixel getXtermBackground (XtermWidget /* xw */, unsigned /* flags */, int /* color */);
extern Pixel getXtermForeground (XtermWidget /* xw */, unsigned /* flags */, int /* color */);
extern char * xtermSetLocale (int /* category */, String /* after */);
extern int ClearInLine (XtermWidget /* xw */, int /* row */, int /* col */, unsigned /* len */);
extern int HandleExposure (XtermWidget /* xw */, XEvent * /* event */);
extern int dimRound (double /* value */);
extern int drawXtermText (const XTermDraw * /* param */, GC /* gc */, int /* x */, int /* y */, const IChar * /* text */, Cardinal /* len */);
extern int extendedBoolean (const char * /* value */, const FlagList * /* table */, Cardinal /* limit */);
extern void ChangeColors (XtermWidget /* xw */, ScrnColors * /* pNew */);
extern void ClearLine (XtermWidget /* xw */);
extern void ClearRight (XtermWidget /* xw */, int /* n */);
extern void ClearScreen (XtermWidget /* xw */);
extern void DeleteChar (XtermWidget /* xw */, unsigned /* n */);
extern void DeleteLine (XtermWidget /* xw */, int /* n */, Bool /* canSave */);
extern void FlushScroll (XtermWidget /* xw */);
extern void GetColors (XtermWidget /* xw */, ScrnColors * /* pColors */);
extern void InsertChar (XtermWidget /* xw */, unsigned /* n */);
extern void InsertLine (XtermWidget /* xw */, int /* n */);
extern void RevScroll (XtermWidget /* xw */, int /* amount */);
extern void ReverseVideo (XtermWidget /* xw */);
extern void WriteText (XtermWidget /* xw */, Cardinal /* offset */, Cardinal /* len */);
extern void decode_keyboard_type (XtermWidget /* xw */, struct XTERM_RESOURCE * /* rp */);
extern void decode_wcwidth (XtermWidget /* xw */);
extern void do_cd_xtra_scroll (XtermWidget /* xw */, int /* param */);
extern void do_erase_display (XtermWidget /* xw */, int /* param */, int /* mode */);
extern void do_erase_char (XtermWidget /* xw */, int /* param */, int /* mode */);
extern void do_erase_line (XtermWidget /* xw */, int /* param */, int /* mode */);
extern void do_ti_xtra_scroll (XtermWidget /* xw */);
extern void getXtermSizeHints (XtermWidget /* xw */);
extern void recolor_cursor (TScreen * /* screen */, Cursor /* cursor */, unsigned long /* fg */, unsigned long /* bg */);
extern void resetXtermGC (XtermWidget /* xw */, unsigned /* flags */, Bool /* hilite */);
extern void scrolling_copy_area (XtermWidget /* xw */, int /* firstline */, int /* nlines */, int /* amount */);
extern void set_keyboard_type (XtermWidget /* xw */, xtermKeyboardType /* type */, Bool /* set */);
extern void toggle_keyboard_type (XtermWidget /* xw */, xtermKeyboardType /* type */);
extern void update_keyboard_type (void);
extern void xtermClear (XtermWidget /* xw */);
extern void xtermClear2 (XtermWidget /* xw */, int /* x */, int /* y */, unsigned /* width */, unsigned /* height */);
extern void xtermColIndex (XtermWidget /* xw */, Bool /* toLeft */);
extern void xtermColScroll (XtermWidget /* xw */, int /* amount */, Bool /* toLeft */, int /* at_col */);
extern void xtermRepaint (XtermWidget /* xw */);
extern void xtermResetLocale (int /* category */, char * /* before */);
extern void xtermScroll (XtermWidget /* xw */, int /* amount */);
extern void xtermScrollLR (XtermWidget /* xw */, int /* amount */, Bool /* toLeft */);
extern void xtermSizeHints (XtermWidget /* xw */, int /* scrollbarWidth */);

struct Xinerama_geometry {
    int x;
    int y;
    unsigned w;
    unsigned h;
    int scr_x;
    int scr_y;
    int scr_w;
    int scr_h;
};
extern int XParseXineramaGeometry(Display * /* display */, char * /* parsestring */, struct Xinerama_geometry * /* ret */);

#if OPT_ISO_COLORS

extern Pixel extract_fg (XtermWidget /* xw */, CellColor /* color */, unsigned /* flags */);
extern Pixel extract_bg (XtermWidget /* xw */, CellColor /* color */, unsigned /* flags */);
extern CellColor makeColorPair (XtermWidget /* xw */);
extern void ClearCurBackground (XtermWidget /* xw */, int /* top */, int /* left */, unsigned /* height */, unsigned /* width */, unsigned /* fw */);

#define xtermColorPair(xw) makeColorPair(xw)

#define GET_COLOR_RES(xw, res) xtermGetColorRes(xw, &(res))
#define SET_COLOR_RES(res,color) (res)->value = color
#define EQL_COLOR_RES(res,color) (res)->value == color
#define T_COLOR(v,n) (v)->Tcolors[n].value
extern Pixel xtermGetColorRes(XtermWidget /* xw */, ColorRes * /* res */);

#define ExtractForeground(color) (unsigned) GetCellColorFG(color)
#define ExtractBackground(color) (unsigned) GetCellColorBG(color)

#if OPT_RENDERFONT
extern void discardRenderDraw(TScreen * /* screen */);
#else
#define discardRenderDraw(screen) /* nothing */
#endif

#if OPT_WIDE_ATTRS
#define MapToWideColorMode(fg, screen, flags) \
	(((screen)->colorITMode && ((flags) & ATR_ITALIC)) \
	 ? COLOR_IT \
	 : fg)
#else
#define MapToWideColorMode(fg, screen, flags) fg
#endif

#define MapToColorMode(fg, screen, flags) \
	(((screen)->colorBLMode && ((flags) & BLINK)) \
	 ? COLOR_BL \
	 : (((screen)->colorBDMode && ((flags) & BOLD)) \
	    ? COLOR_BD \
	    : (((screen)->colorULMode && ((flags) & UNDERLINE)) \
	       ? COLOR_UL \
	       : MapToWideColorMode(fg, screen, flags))))

#define checkVeryBoldAttr(flags, fg, code, attr) \
	if ((flags & FG_COLOR) != 0 \
	 && (screen->veryBoldColors & attr) == 0 \
	 && (flags & attr) != 0 \
	 && (fg == code)) \
		 UIntClr(flags, attr)

#if OPT_WIDE_ATTRS
#define checkVeryBoldWideAttr(flags, fg, it, atr) \
	    checkVeryBoldAttr(flags, fg, it, atr)
#else
#define checkVeryBoldWideAttr(flags, fg, it, atr) (void) flags
#endif

#define checkVeryBoldColors(flags, fg) \
	checkVeryBoldAttr(flags, fg, COLOR_RV, INVERSE); \
	checkVeryBoldAttr(flags, fg, COLOR_UL, UNDERLINE); \
	checkVeryBoldAttr(flags, fg, COLOR_BD, BOLD); \
	checkVeryBoldAttr(flags, fg, COLOR_BL, BLINK); \
	checkVeryBoldWideAttr(flags, fg, COLOR_IT, ATR_ITALIC)

#else /* !OPT_ISO_COLORS */

#define MapToColorMode(fg, screen, flags) fg

#define ClearCurBackground(xw, top, left, height, width, fw) \
	XClearArea (TScreenOf(xw)->display, \
		    VDrawable(TScreenOf(xw)), \
		    CursorX2(TScreenOf(xw), left, fw), \
		    CursorY2(TScreenOf(xw), top), \
		    ((width) * (unsigned) fw), \
		    ((height) * (unsigned) FontHeight(TScreenOf(xw))), \
		    False)

#define extract_fg(xw, color, flags) (unsigned) (xw)->cur_foreground
#define extract_bg(xw, color, flags) (unsigned) (xw)->cur_background

		/* FIXME: Reverse-Video? */
#define T_COLOR(v,n) (v)->Tcolors[n].value
#define xtermColorPair(xw) 0

#define checkVeryBoldColors(flags, fg) /* nothing */
#define discardRenderDraw(screen) /* nothing */

#endif	/* OPT_ISO_COLORS */

#define getXtermFG(xw, flags, color) getXtermForeground(xw, flags, color)
#define getXtermBG(xw, flags, color) getXtermBackground(xw, flags, color)

#if OPT_ZICONBEEP
extern void initZIconBeep(void);
extern void resetZIconBeep(XtermWidget /* xw */);
extern Boolean showZIconBeep(XtermWidget /* xw */, const char * /* name */);
#else
#define initZIconBeep() /* nothing */
#define resetZIconBeep(xw) /* nothing */
#define showZIconBeep(xw, name) False
#endif

#define XTERM_CELL(row,col)    getXtermCell(screen,     ROW2INX(screen, row), col)

extern unsigned getXtermCell (TScreen * /* screen */, int /* row */, int /* col */);
extern unsigned getXtermCombining(TScreen * /* screen */, int /* row */, int /* col */, int /* off */);
extern void putXtermCell (TScreen * /* screen */, int /* row */, int /* col */, int /* ch */);

#define IsCellCombined(screen, row, col) (getXtermCombining(screen, row, col, 0) != 0)

#if OPT_HIGHLIGHT_COLOR
#define isNotForeground(xw, fg, bg, sel) \
		(Boolean) ((sel) != T_COLOR(TScreenOf(xw), TEXT_FG) \
			   && (sel) != (fg) \
			   && (sel) != (bg) \
			   && (sel) != (xw)->dft_foreground)
#define isNotBackground(xw, fg, bg, sel) \
		(Boolean) ((sel) != T_COLOR(TScreenOf(xw), TEXT_BG) \
			   && (sel) != (fg) \
			   && (sel) != (bg) \
			   && (sel) != (xw)->dft_background)
#endif

#define setXtermLineAttributes(dpy, gc, width, style) \
	XSetLineAttributes(dpy, gc, (unsigned) width, style, CapProjecting, JoinMiter);

#define resetXtermLineAttributes(dpy, gc) \
	setXtermLineAttributes(dpy, gc, 0, LineSolid);

#if OPT_WIDE_CHARS
extern int DamagedCells(TScreen * /* screen */, unsigned /* n */, int * /* klp */, int * /* krp */, int /* row */, int /* col */);
extern int DamagedCurCells(TScreen * /* screen */, unsigned /* n */, int * /* klp */, int * /* krp */);
extern unsigned AsciiEquivs(unsigned /* ch */);
extern void addXtermCombining (TScreen * /* screen */, int /* row */, int /* col */, unsigned /* ch */);
extern void allocXtermChars(ScrnPtr * /* buffer */, Cardinal /* length */);
#endif

#if OPT_XMC_GLITCH
extern void Mark_XMC (XtermWidget /* xw */, int /* param */);
extern void Jump_XMC (XtermWidget /* xw */);
extern void Resolve_XMC (XtermWidget /* xw */);
#endif

#if OPT_WIDE_CHARS
unsigned visual_width(const IChar * /* str */, Cardinal /* len */);
#else
#define visual_width(a, b) (b)
#endif

#define BtoS(b)    ((b) ? "on" : "off")
#define MtoS(b)    (((b) == Maybe) ? "maybe" : BtoS(b))
#define NonNull(s) ((s) ? (s) : "<null>")

#define UIntSet(dst,bits) dst = dst | (unsigned) (bits)
#define UIntClr(dst,bits) dst = dst & (unsigned) ~(bits)
#define SIntClr(dst,bits) dst = (int) ((unsigned) dst & (unsigned) ~(bits))

#ifdef __cplusplus
	}
#endif
/* *INDENT-ON* */

#endif /* included_xterm_h */
