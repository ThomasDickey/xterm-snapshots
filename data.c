/*
 *	$Xorg: data.c,v 1.3 2000/08/17 19:55:08 cpqbld Exp $
 */

/* $XFree86: xc/programs/xterm/data.c,v 3.21 2001/01/17 23:46:36 dawes Exp $ */

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
int hold_screen;

#if OPT_ZICONBEEP
int zIconBeep;  /* non-zero means beep; see charproc.c for details -IAN! */
Boolean zIconBeep_flagged; /* True if the icon name has been changed */
#endif /* OPT_ZICONBEEP */

#if OPT_SAME_NAME
Boolean sameName;	/* Don't change the title or icon name if it
			is the same.  This prevents flicker on the screen at
			the cost of an extra request to the server */
#endif

int am_slave = -1;	/* set to file-descriptor if we're a slave process */
int max_plus1;
#ifdef VMS
int Select_mask;
int X_mask;
int pty_mask;
#else /* VMS */
fd_set Select_mask;
fd_set X_mask;
fd_set pty_mask;
#endif /* VMS */
char *ptydev;
char *ttydev;
#ifdef ALLOWLOGGING
char log_def_name[] = "XtermLog.XXXXXX";
#endif

int waitingForTrackInfo = 0;
EventMode eventMode = NORMAL;

#if OPT_WIDE_CHARS
char defaultUTF8[2] = { '0', 0 };

const unsigned short dec2ucs[32] = {
	0x25ae		/* black vertical rectangle			*/,
	0x25c6		/* black diamond				*/,
	0x2592		/* medium shade					*/,
	0x2409		/* symbol for horizontal tabulation		*/,
	0x240c		/* symbol for form feed				*/,
	0x240d		/* symbol for carriage return			*/,
	0x240a		/* symbol for line feed				*/,
	0x00b0		/* degree sign					*/,
	0x00b1		/* plus-minus sign				*/,
	0x2424		/* symbol for newline				*/,
	0x240b		/* symbol for vertical tabulation		*/,
	0x2518		/* box drawings light up and left		*/,
	0x2510		/* box drawings light down and left		*/,
	0x250c		/* box drawings light down and right		*/,
	0x2514		/* box drawings light up and right		*/,
	0x253c		/* box drawings light vertical and horizontal	*/,
	UCS_REPL	/* replacement character			*/,
	UCS_REPL	/* replacement character			*/,
	0x2500		/* box drawings light horizontal		*/,
	UCS_REPL	/* replacement character			*/,
	UCS_REPL	/* replacement character			*/,
	0x251c		/* box drawings light vertical and right	*/,
	0x2524		/* box drawings light vertical and left		*/,
	0x2534		/* box drawings light up and horizontal		*/,
	0x252c		/* box drawings light down and horizontal	*/,
	0x2502		/* box drawings light vertical			*/,
	0x2264		/* less-than or equal to			*/,
	0x2265		/* greater-than or equal to			*/,
	0x03c0		/* greek small letter pi			*/,
	0x2260		/* not equal to					*/,
	0x00a3		/* pound sign					*/,
	0x00b7		/* middle dot					*/
};
#endif
