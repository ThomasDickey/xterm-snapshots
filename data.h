/*
 *	$XConsortium: data.h /main/13 1996/11/24 17:35:40 rws $
 *	$XFree86: xc/programs/xterm/data.h,v 3.22 1999/10/13 04:21:44 dawes Exp $
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

#ifndef included_data_h
#define included_data_h 1

#include <xterm.h>

extern Widget toplevel;

extern XtAppContext app_con;

#if OPT_TEK4014
extern Char *Tpushb;
extern Char *Tpushback;
extern PtyData *Tbuffer;
extern TekLink *TekRefresh;
extern TekWidget tekWidget;
extern Widget tekshellwidget;
extern int TEKgcFontMask;
extern int T_lastx;
extern int T_lasty;
extern int Ttoggled;
extern jmp_buf Tekend;
#endif

#ifdef ALLOWLOGGING
extern char log_def_name[];
#endif
extern char *ptydev;
extern char *ttydev;
extern char *xterm_name;
extern Boolean sunFunctionKeys;
extern int hold_screen;

#if OPT_HP_FUNC_KEYS
extern Boolean hpFunctionKeys;
#endif

#if OPT_ZICONBEEP 
extern int zIconBeep; 
extern Boolean zIconBeep_flagged; 
#endif 

#if OPT_SAME_NAME 
extern Boolean sameName; 
#endif 

#if OPT_SUNPC_KBD
extern Boolean sunKeyboard;
#endif

extern PtyData VTbuffer;
extern int am_slave;
extern int max_plus1;
extern jmp_buf VTend;

#ifdef DEBUG
extern int debug;
#endif	/* DEBUG */

extern fd_set Select_mask;
extern fd_set X_mask;
extern fd_set pty_mask;

extern int waitingForTrackInfo;

extern EventMode eventMode;

extern XtermWidget term;

#ifdef NO_XKBSTDBELL
#undef XKB
#endif

#ifdef XKB
#include <X11/extensions/XKBbells.h>
#else
#define	XkbBI_Info			0
#define	XkbBI_MinorError		1
#define	XkbBI_MajorError		2
#define	XkbBI_TerminalBell		9
#define	XkbBI_MarginBell		10
#endif

#if OPT_WIDE_CHARS
extern const unsigned short dec2ucs[32];
#endif

#endif /* included_data_h */
