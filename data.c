/*
 *	$XConsortium: data.c,v 1.12 95/04/05 19:58:47 kaleb Exp $
 *	$XFree86: xc/programs/xterm/data.c,v 3.14 1999/04/29 09:14:03 dawes Exp $
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

#ifdef HAVE_CONFIG_H
#include <xtermcfg.h>
#endif

#include <ptyx.h>		/* gets Xt stuff, too */
#include <data.h>

Widget toplevel;		/* top-most widget in xterm */

#if OPT_TEK4014
PtyData *Tbuffer;
Char *Tpushb;
Char *Tpushback;
TekLink *TekRefresh;
TekWidget tekWidget;
Widget tekshellwidget;
int TEKgcFontMask = GCFont;
int T_lastx = -1;
int T_lasty = -1;
int Ttoggled = 0;
jmp_buf Tekend;
#endif

PtyData VTbuffer;

jmp_buf VTend;

#ifdef DEBUG
int debug = 0; 		/* true causes error messages to be displayed */
#endif	/* DEBUG */

XtAppContext app_con;
XtermWidget term;	/* master data structure for client */
char *xterm_name;	/* argv[0] */
Boolean sunFunctionKeys;

#if OPT_HP_FUNC_KEYS
Boolean hpFunctionKeys;
#endif

#if OPT_ZICONBEEP
int zIconBeep;  /* non-zero means beep; see charproc.c for details -IAN! */
Boolean zIconBeep_flagged; /* True if the icon name has been changed */
#endif /* OPT_ZICONBEEP */

#if OPT_SAME_NAME
Boolean sameName;            /* Don't change the title or icon name if it
			is the same.  This prevents flicker on the screen at
			the cost of an extra request to the server */
#endif

#if OPT_SUNPC_KBD
Boolean sunKeyboard;
#endif

int am_slave = 0;	/* set to 1 if running as a slave process */
int max_plus1;
fd_set Select_mask;
fd_set X_mask;
fd_set pty_mask;
char *ptydev;
char *ttydev;
#ifdef ALLOWLOGGING
char log_def_name[] = "XtermLog.XXXXXX";
#endif

int waitingForTrackInfo = 0;
EventMode eventMode = NORMAL;

