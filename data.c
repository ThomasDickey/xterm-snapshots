/*
 *	$XConsortium: data.c,v 1.12 95/04/05 19:58:47 kaleb Exp $
 *	$XFree86: xc/programs/xterm/data.c,v 3.2.4.1 1997/05/23 09:24:37 dawes Exp $
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

#include "ptyx.h"		/* gets Xt stuff, too */

#if XtSpecificationRelease >= 6
#include <X11/Xpoll.h>
#endif

#include "data.h"
#include <setjmp.h>

XPoint T_boxlarge[NBOX] = {
	{0, 0},
	{8, 0},
	{0, 14},
	{-8, 0},
	{0, -14},
};
XPoint T_box2[NBOX] = {
	{0, 0},
	{7, 0},
	{0, 12},
	{-7, 0},
	{0, -12},
};
XPoint T_box3[NBOX] = {
	{0, 0},
	{5, 0},
	{0, 12},
	{-5, 0},
	{0, -12},
};
XPoint T_boxsmall[NBOX] = {
	{0, 0},
	{5, 0},
	{0, 9},
	{-5, 0},
	{0, -9},
};
jmp_buf Tekend;
int Tbcnt = 0;
Char *Tbuffer;
Char *Tbptr;
TekLink *TekRefresh;
Char *Tpushb;
Char *Tpushback;
int Ttoggled = 0;
int bcnt = 0;
Char buffer[BUF_SIZE];
Char *bptr = buffer;
jmp_buf VTend;
XPoint VTbox[NBOX] = {
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
};

#ifdef DEBUG
int debug = 0; 		/* true causes error messages to be displayed */
#endif	/* DEBUG */

XtAppContext app_con;
XtermWidget term;		/* master data structure for client */
char *xterm_name;	/* argv[0] */
Boolean sunFunctionKeys;

int am_slave = 0;	/* set to 1 if running as a slave process */
int max_plus1;
fd_set Select_mask;
fd_set X_mask;
fd_set pty_mask;
char *ptydev;
char *ttydev;
#ifdef ALLOWLOGGING
char log_def_name[] = "XtermLog.XXXXX";
#endif
int T_lastx = -1;
int T_lasty = -1;

int waitingForTrackInfo = 0;
EventMode eventMode = NORMAL;

GC visualBellGC;

int VTgcFontMask = GCFont;
int TEKgcFontMask = GCFont;

TekWidget tekWidget;
