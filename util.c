/*
 *	$XConsortium: util.c /main/33 1996/12/01 23:47:10 swick $
 *	$XFree86: xc/programs/xterm/util.c,v 3.13.2.2 1997/05/23 12:19:50 dawes Exp $
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

/* util.c */

#ifdef HAVE_CONFIG_H
#include <xtermcfg.h>
#endif

#include "ptyx.h"
#include "data.h"
#include "error.h"
#include "menu.h"

#include <stdio.h>

#include "xterm.h"

extern Bool waiting_for_initial_map;

static int ClearInLine PROTO((TScreen *screen, int row, int col, int len));
static int handle_translated_exposure PROTO((TScreen *screen, int rect_x, int rect_y, unsigned int rect_width, unsigned int rect_height));
static void ClearAbove PROTO((TScreen *screen));
static void ClearBelow PROTO((TScreen *screen));
static void ClearLeft PROTO((TScreen *screen));
static void ClearLine PROTO((TScreen *screen));
static void CopyWait PROTO((TScreen *screen));
static void copy_area PROTO((TScreen *screen, int src_x, int src_y, unsigned int width, unsigned int height, int dest_x, int dest_y));
static void horizontal_copy_area PROTO((TScreen *screen, int firstchar, int nchars, int amount));
static void vertical_copy_area PROTO((TScreen *screen, int firstline, int nlines, int amount));

/*
 * These routines are used for the jump scroll feature
 */
void
FlushScroll(screen)
register TScreen *screen;
{
	register int i;
	register int shift = -screen->topline;
	register int bot = screen->max_row - shift;
	register int refreshtop;
	register int refreshheight;
	register int scrolltop;
	register int scrollheight;

	if(screen->cursor_state)
		HideCursor();
	if(screen->scroll_amt > 0) {
		refreshheight = screen->refresh_amt;
		scrollheight = screen->bot_marg - screen->top_marg -
		 refreshheight + 1;
		if((refreshtop = screen->bot_marg - refreshheight + 1 + shift) >
		 (i = screen->max_row - screen->scroll_amt + 1))
			refreshtop = i;
		if(screen->scrollWidget && !screen->alternate
		 && screen->top_marg == 0) {
			scrolltop = 0;
			if((scrollheight += shift) > i)
				scrollheight = i;
			if((i = screen->bot_marg - bot) > 0 &&
			 (refreshheight -= i) < screen->scroll_amt)
				refreshheight = screen->scroll_amt;
			if((i = screen->savedlines) < screen->savelines) {
				if((i += screen->scroll_amt) >
				  screen->savelines)
					i = screen->savelines;
				screen->savedlines = i;
				ScrollBarDrawThumb(screen->scrollWidget);
			}
		} else {
			scrolltop = screen->top_marg + shift;
			if((i = bot - (screen->bot_marg - screen->refresh_amt +
			 screen->scroll_amt)) > 0) {
				if(bot < screen->bot_marg)
					refreshheight = screen->scroll_amt + i;
			} else {
				scrollheight += i;
				refreshheight = screen->scroll_amt;
				if((i = screen->top_marg + screen->scroll_amt -
				 1 - bot) > 0) {
					refreshtop += i;
					refreshheight -= i;
				}
			}
		}
	} else {
		refreshheight = -screen->refresh_amt;
		scrollheight = screen->bot_marg - screen->top_marg -
		 refreshheight + 1;
		refreshtop = screen->top_marg + shift;
		scrolltop = refreshtop + refreshheight;
		if((i = screen->bot_marg - bot) > 0)
			scrollheight -= i;
		if((i = screen->top_marg + refreshheight - 1 - bot) > 0)
			refreshheight -= i;
	}
	scrolling_copy_area(screen, scrolltop+screen->scroll_amt,
			    scrollheight, screen->scroll_amt);
	ScrollSelection(screen, -(screen->scroll_amt));
	screen->scroll_amt = 0;
	screen->refresh_amt = 0;
	if(refreshheight > 0) {
		ClearCurBackground(screen,
		    (int) refreshtop * FontHeight(screen) + screen->border,
		    (int) screen->border + Scrollbar(screen),
		    (unsigned) refreshheight * FontHeight(screen),
		    (unsigned) Width(screen));
		ScrnRefresh(screen, refreshtop, 0, refreshheight,
		    screen->max_col + 1, False);
	}
}

int
AddToRefresh(screen)
register TScreen *screen;
{
	register int amount = screen->refresh_amt;
	register int row = screen->cur_row;

	if(amount == 0)
		return(0);
	if(amount > 0) {
		register int bottom;

		if(row == (bottom = screen->bot_marg) - amount) {
			screen->refresh_amt++;
			return(1);
		}
		return(row >= bottom - amount + 1 && row <= bottom);
	} else {
		register int top;

		amount = -amount;
		if(row == (top = screen->top_marg) + amount) {
			screen->refresh_amt--;
			return(1);
		}
		return(row <= top + amount - 1 && row >= top);
	}
}

/* 
 * scrolls the screen by amount lines, erases bottom, doesn't alter 
 * cursor position (i.e. cursor moves down amount relative to text).
 * All done within the scrolling region, of course. 
 * requires: amount > 0
 */
void
Scroll(screen, amount)
register TScreen *screen;
register int amount;
{
	register int i = screen->bot_marg - screen->top_marg + 1;
	register int shift;
	register int bot;
	register int refreshtop = 0;
	register int refreshheight;
	register int scrolltop;
	register int scrollheight;

	if(screen->cursor_state)
		HideCursor();
	if (amount > i)
		amount = i;
    if(screen->jumpscroll) {
	if(screen->scroll_amt > 0) {
		if(screen->refresh_amt + amount > i)
			FlushScroll(screen);
		screen->scroll_amt += amount;
		screen->refresh_amt += amount;
	} else {
		if(screen->scroll_amt < 0)
			FlushScroll(screen);
		screen->scroll_amt = amount;
		screen->refresh_amt = amount;
	}
	refreshheight = 0;
    } else {
	ScrollSelection(screen, -(amount));
	if (amount == i) {
		ClearScreen(screen);
		return;
	}
	shift = -screen->topline;
	bot = screen->max_row - shift;
	scrollheight = i - amount;
	refreshheight = amount;
	if((refreshtop = screen->bot_marg - refreshheight + 1 + shift) >
	 (i = screen->max_row - refreshheight + 1))
		refreshtop = i;
	if(screen->scrollWidget && !screen->alternate
	 && screen->top_marg == 0) {
		scrolltop = 0;
		if((scrollheight += shift) > i)
			scrollheight = i;
		if((i = screen->savedlines) < screen->savelines) {
			if((i += amount) > screen->savelines)
				i = screen->savelines;
			screen->savedlines = i;
			ScrollBarDrawThumb(screen->scrollWidget);
		}
	} else {
		scrolltop = screen->top_marg + shift;
		if((i = screen->bot_marg - bot) > 0) {
			scrollheight -= i;
			if((i = screen->top_marg + amount - 1 - bot) >= 0) {
				refreshtop += i;
				refreshheight -= i;
			}
		}
	}

	if (screen->multiscroll && amount == 1 &&
	    screen->topline == 0 && screen->top_marg == 0 &&
	    screen->bot_marg == screen->max_row) {
	    if (screen->incopy < 0 && screen->scrolls == 0)
		CopyWait(screen);
	    screen->scrolls++;
	}
	scrolling_copy_area(screen, scrolltop+amount, scrollheight, amount);
	if(refreshheight > 0) {
		ClearCurBackground(screen,
		    (int) refreshtop * FontHeight(screen) + screen->border,
		    (int) screen->border + Scrollbar(screen),
		    (unsigned) refreshheight * FontHeight(screen),
		    (unsigned) Width(screen));
		if(refreshheight > shift)
			refreshheight = shift;
	}
    }
	if(screen->scrollWidget && !screen->alternate && screen->top_marg == 0)
		ScrnDeleteLine(screen->allbuf, screen->bot_marg +
		 screen->savelines, 0, amount, screen->max_col + 1);
	else
		ScrnDeleteLine(screen->buf, screen->bot_marg, screen->top_marg,
		 amount, screen->max_col + 1);
	if(refreshheight > 0)
		ScrnRefresh(screen, refreshtop, 0, refreshheight,
		 screen->max_col + 1, False);
}


/*
 * Reverse scrolls the screen by amount lines, erases top, doesn't alter
 * cursor position (i.e. cursor moves up amount relative to text).
 * All done within the scrolling region, of course.
 * Requires: amount > 0
 */
void
RevScroll(screen, amount)
register TScreen *screen;
register int amount;
{
	register int i = screen->bot_marg - screen->top_marg + 1;
	register int shift;
	register int bot;
	register int refreshtop;
	register int refreshheight;
	register int scrolltop;
	register int scrollheight;

	if(screen->cursor_state)
		HideCursor();
	if (amount > i)
		amount = i;
    if(screen->jumpscroll) {
	if(screen->scroll_amt < 0) {
		if(-screen->refresh_amt + amount > i)
			FlushScroll(screen);
		screen->scroll_amt -= amount;
		screen->refresh_amt -= amount;
	} else {
		if(screen->scroll_amt > 0)
			FlushScroll(screen);
		screen->scroll_amt = -amount;
		screen->refresh_amt = -amount;
	}
    } else {
	shift = -screen->topline;
	bot = screen->max_row - shift;
	refreshheight = amount;
	scrollheight = screen->bot_marg - screen->top_marg -
	 refreshheight + 1;
	refreshtop = screen->top_marg + shift;
	scrolltop = refreshtop + refreshheight;
	if((i = screen->bot_marg - bot) > 0)
		scrollheight -= i;
	if((i = screen->top_marg + refreshheight - 1 - bot) > 0)
		refreshheight -= i;

	if (screen->multiscroll && amount == 1 &&
	    screen->topline == 0 && screen->top_marg == 0 &&
	    screen->bot_marg == screen->max_row) {
	    if (screen->incopy < 0 && screen->scrolls == 0)
		CopyWait(screen);
	    screen->scrolls++;
	}
	scrolling_copy_area(screen, scrolltop-amount, scrollheight, -amount);
	if(refreshheight > 0) {
		ClearCurBackground(screen,
		    (int) refreshtop * FontHeight(screen) + screen->border,
		    (int) screen->border + Scrollbar(screen),
		    (unsigned) refreshheight * FontHeight(screen),
		    (unsigned) Width(screen));
	}
    }
	ScrnInsertLine (screen->buf, screen->bot_marg, screen->top_marg,
			amount, screen->max_col + 1);
}

/*
 * If cursor not in scrolling region, returns.  Else,
 * inserts n blank lines at the cursor's position.  Lines above the
 * bottom margin are lost.
 */
void
InsertLine (screen, n)
register TScreen *screen;
register int n;
{
	register int i;
	register int shift;
	register int bot;
	register int refreshtop;
	register int refreshheight;
	register int scrolltop;
	register int scrollheight;

	if (screen->cur_row < screen->top_marg ||
	 screen->cur_row > screen->bot_marg)
		return;
	if(screen->cursor_state)
		HideCursor();
	screen->do_wrap = 0;
	if (n > (i = screen->bot_marg - screen->cur_row + 1))
		n = i;
    if(screen->jumpscroll) {
	if(screen->scroll_amt <= 0 &&
	 screen->cur_row <= -screen->refresh_amt) {
		if(-screen->refresh_amt + n > screen->max_row + 1)
			FlushScroll(screen);
		screen->scroll_amt -= n;
		screen->refresh_amt -= n;
	} else if(screen->scroll_amt)
		FlushScroll(screen);
    }
    if(!screen->scroll_amt) {
	shift = -screen->topline;
	bot = screen->max_row - shift;
	refreshheight = n;
	scrollheight = screen->bot_marg - screen->cur_row - refreshheight + 1;
	refreshtop = screen->cur_row + shift;
	scrolltop = refreshtop + refreshheight;
	if((i = screen->bot_marg - bot) > 0)
		scrollheight -= i;
	if((i = screen->cur_row + refreshheight - 1 - bot) > 0)
		refreshheight -= i;
	vertical_copy_area(screen, scrolltop-n, scrollheight, -n);
	if(refreshheight > 0) {
		ClearCurBackground(screen,
		    (int) refreshtop * FontHeight(screen) + screen->border,
		    (int) screen->border + Scrollbar(screen),
		    (unsigned) refreshheight * FontHeight(screen),
		    (unsigned) Width(screen));
	}
    }
	/* adjust screen->buf */
	ScrnInsertLine(screen->buf, screen->bot_marg, screen->cur_row, n,
			screen->max_col + 1);
}

/*
 * If cursor not in scrolling region, returns.  Else, deletes n lines
 * at the cursor's position, lines added at bottom margin are blank.
 */
void
DeleteLine(screen, n)
register TScreen *screen;
register int n;
{
	register int i;
	register int shift;
	register int bot;
	register int refreshtop;
	register int refreshheight;
	register int scrolltop;
	register int scrollheight;

	if (screen->cur_row < screen->top_marg ||
	 screen->cur_row > screen->bot_marg)
		return;
	if(screen->cursor_state)
		HideCursor();
	screen->do_wrap = 0;
	if (n > (i = screen->bot_marg - screen->cur_row + 1))
		n = i;
    if(screen->jumpscroll) {
	if(screen->scroll_amt >= 0 && screen->cur_row == screen->top_marg) {
		if(screen->refresh_amt + n > screen->max_row + 1)
			FlushScroll(screen);
		screen->scroll_amt += n;
		screen->refresh_amt += n;
	} else if(screen->scroll_amt)
		FlushScroll(screen);
    }
    if(!screen->scroll_amt) {

	shift = -screen->topline;
	bot = screen->max_row - shift;
	scrollheight = i - n;
	refreshheight = n;
	if((refreshtop = screen->bot_marg - refreshheight + 1 + shift) >
	 (i = screen->max_row - refreshheight + 1))
		refreshtop = i;
	if(screen->scrollWidget && !screen->alternate && screen->cur_row == 0) {
		scrolltop = 0;
		if((scrollheight += shift) > i)
			scrollheight = i;
		if((i = screen->savedlines) < screen->savelines) {
			if((i += n) > screen->savelines)
				i = screen->savelines;
			screen->savedlines = i;
			ScrollBarDrawThumb(screen->scrollWidget);
		}
	} else {
		scrolltop = screen->cur_row + shift;
		if((i = screen->bot_marg - bot) > 0) {
			scrollheight -= i;
			if((i = screen->cur_row + n - 1 - bot) >= 0) {
				refreshheight -= i;
			}
		}
	}
	vertical_copy_area(screen, scrolltop+n, scrollheight, n);
	if(refreshheight > 0) {
		ClearCurBackground(screen,
		    (int) refreshtop * FontHeight(screen) + screen->border,
		    (int) screen->border + Scrollbar(screen),
		    (unsigned) refreshheight * FontHeight(screen),
		    (unsigned) Width(screen));
	}
    }
	/* adjust screen->buf */
	if(screen->scrollWidget && !screen->alternate && screen->cur_row == 0)
		ScrnDeleteLine(screen->allbuf, screen->bot_marg +
		 screen->savelines, 0, n, screen->max_col + 1);
	else
		ScrnDeleteLine(screen->buf, screen->bot_marg, screen->cur_row,
		 n, screen->max_col + 1);
}

/*
 * Insert n blanks at the cursor's position, no wraparound
 */
void
InsertChar (screen, n)
    register TScreen *screen;
    register int n;
{
	if(screen->cursor_state)
		HideCursor();
	screen->do_wrap = 0;
	if(screen->cur_row - screen->topline <= screen->max_row) {
	    if(!AddToRefresh(screen)) {
		if(screen->scroll_amt)
			FlushScroll(screen);

		/*
		 * prevent InsertChar from shifting the end of a line over
		 * if it is being appended to
		 */
		if (non_blank_line (screen->buf, screen->cur_row, 
				    screen->cur_col, screen->max_col + 1))
		    horizontal_copy_area(screen, screen->cur_col,
					 screen->max_col+1 - (screen->cur_col+n),
					 n);

		FillCurBackground(
			screen,
			CursorX (screen, screen->cur_col),
			CursorY (screen, screen->cur_row),
			(unsigned) n * FontWidth(screen),
			(unsigned) FontHeight(screen));
	    }
	}
	/* adjust screen->buf */
	ScrnInsertChar(screen, n, screen->max_col + 1);
}

/*
 * Deletes n chars at the cursor's position, no wraparound.
 */
void
DeleteChar (screen, n)
    register TScreen *screen;
    register int	n;
{
	register int width;

	if(screen->cursor_state)
		HideCursor();
	screen->do_wrap = 0;
	if (n > (width = screen->max_col + 1 - screen->cur_col))
	  	n = width;
		
	if(screen->cur_row - screen->topline <= screen->max_row) {
	    if(!AddToRefresh(screen)) {
		if(screen->scroll_amt)
			FlushScroll(screen);
	
		horizontal_copy_area(screen, screen->cur_col+n,
				     screen->max_col+1 - (screen->cur_col+n),
				     -n);
	
		FillCurBackground (
			screen,
			Width(screen) + CursorX(screen, -n),
			CursorY (screen, screen->cur_row),
			n * FontWidth(screen),
			FontHeight(screen));
	    }
	}
	/* adjust screen->buf */
	ScrnDeleteChar (screen, n, screen->max_col + 1);
}

/*
 * Clear from cursor position to beginning of display, inclusive.
 */
static void
ClearAbove (screen)
register TScreen *screen;
{
	if (screen->protected_mode != OFF_PROTECT) {
		register int row;
		for (row = 0; row <= screen->max_row; row++)
			ClearInLine(screen, row, 0, screen->max_col + 1);
	} else {
		register int top, height;

		if(screen->cursor_state)
			HideCursor();
		if((top = -screen->topline) <= screen->max_row) {
			if(screen->scroll_amt)
				FlushScroll(screen);
			if((height = screen->cur_row + top) > screen->max_row)
				height = screen->max_row;
			if((height -= top) > 0) {
				ClearCurBackground(screen,
				    top * FontHeight(screen) + screen->border,
				    screen->border + Scrollbar(screen),
				    height * FontHeight(screen),
				    Width(screen));
			}
		}
		ClearBufRows(screen, 0, screen->cur_row - 1);
	}

	if(screen->cur_row - screen->topline <= screen->max_row)
		ClearLeft(screen);
}

/*
 * Clear from cursor position to end of display, inclusive.
 */
static void
ClearBelow (screen)
register TScreen *screen;
{
	ClearRight(screen, -1);

	if (screen->protected_mode != OFF_PROTECT) {
		register int row;
		for (row = screen->cur_row + 1; row <= screen->max_row; row++)
			ClearInLine(screen, row, 0, screen->max_col + 1);
	} else {
		register int top;

		if((top = screen->cur_row - screen->topline) <= screen->max_row) {
			if(screen->scroll_amt)
				FlushScroll(screen);
			if(++top <= screen->max_row) {
				ClearCurBackground(screen,
				    top * FontHeight(screen) + screen->border,
				    screen->border + Scrollbar(screen),
				    (screen->max_row - top + 1) * FontHeight(screen),
				    Width(screen));
			}
		}
		ClearBufRows(screen, screen->cur_row + 1, screen->max_row);
	}
}

/*
 * Clear the given row, for the given range of columns, returning 1 if no
 * protected characters were found, 0 otherwise.
 */
static int
ClearInLine(screen, row, col, len)
	register TScreen *screen;
	int row;
	int col;
	int len;
{
	int rc = 1;
	int flags = TERM_COLOR_FLAGS;

	/*
	 * If we're clearing to the end of the line, we won't count this as
	 * "drawn" characters.  We'll only do cut/paste on "drawn" characters,
	 * so this has the effect of suppressing trailing blanks from a
	 * selection.
	 */
	if (col + len + 1 < screen->max_col)
		flags |= CHARDRAWN;

	/* If we've marked protected text on the screen, we'll have to
	 * check each time we do an erase.
	 */
	if (screen->protected_mode != OFF_PROTECT) {
		register int n;
		Char *attrs = SCRN_BUF_ATTRS(screen, row) + col;
		int saved_mode = screen->protected_mode;
		Bool done;

		/* disable this branch during recursion */
		screen->protected_mode = OFF_PROTECT;

		do {
			done = True;
			for (n = 0; n < len; n++) {
				if (attrs[n] & PROTECTED) {
					rc = 0; /* found a protected segment */
					if (n != 0)
						ClearInLine(screen, row, col, n);
					while ((n < len)
					   &&  (attrs[n] & PROTECTED))
						n++;
					done = False;
					break;
				}
			}
			/* setup for another segment, past the protected text */
			if (!done) {
				attrs += n;
				col += n;
				len -= n;
			}
		} while (!done);

		screen->protected_mode = saved_mode;
		if (len <= 0)
			return 0;
	}
	/* fall through to the final non-protected segment */

	if(screen->cursor_state)
		HideCursor();
	screen->do_wrap = 0;

	if (row - screen->topline <= screen->max_row) {
		if(!AddToRefresh(screen)) {
			if(screen->scroll_amt)
				FlushScroll(screen);
			FillCurBackground (
				screen,
				CursorX (screen, col),
				CursorY (screen, row),
				len * FontWidth(screen),
				FontHeight(screen));
		}
	}

	memset(SCRN_BUF_CHARS(screen, row) + col, ' ',   len);
	memset(SCRN_BUF_ATTRS(screen, row) + col, flags, len);

	if_OPT_ISO_COLORS(screen,{
		memset(SCRN_BUF_COLOR(screen, row) + col, xtermColorPair(), len);
	})

	return rc;
}

/* 
 * Clear the next n characters on the cursor's line, including the cursor's
 * position.
 */
void
ClearRight (screen, n)
register TScreen *screen;
int n;
{
	int	len = (screen->max_col - screen->cur_col + 1);

	if (n < 0)	/* the remainder of the line */
		n = screen->max_col + 1;
	if (n == 0)	/* default for 'ECH' */
		n = 1;

	if (len > n)
		len = n;

	(void) ClearInLine(screen, screen->cur_row, screen->cur_col, len);

	/* with the right part cleared, we can't be wrapping */
	BUF_ATTRS(screen->buf, screen->cur_row)[0] &= ~LINEWRAPPED;
}

/*
 * Clear first part of cursor's line, inclusive.
 */
static void
ClearLeft (screen)
register TScreen *screen;
{
	(void) ClearInLine(screen, screen->cur_row, 0, screen->cur_col + 1);
}

/* 
 * Erase the cursor's line.
 */
static void
ClearLine(screen)
register TScreen *screen;
{
	(void) ClearInLine(screen, screen->cur_row, 0, screen->max_col + 1);
}

void
ClearScreen(screen)
register TScreen *screen;
{
	register int top;

	if(screen->cursor_state)
		HideCursor();
	screen->do_wrap = 0;
	if((top = -screen->topline) <= screen->max_row) {
		if(screen->scroll_amt)
			FlushScroll(screen);
		ClearCurBackground(screen,
		    top * FontHeight(screen) + screen->border,	
		    screen->border + Scrollbar(screen), 
		    (screen->max_row - top + 1) * FontHeight(screen),
		    Width(screen));
	}
	ClearBufRows (screen, 0, screen->max_row);
}

/*
 * If we've written protected text DEC-style, and are issuing a non-DEC
 * erase, temporarily reset the protected_mode flag so that the erase will
 * ignore the protected flags.
 */
void
do_erase_line(screen, param, mode)
	register TScreen *screen;
	int param;
	int mode;
{
	int saved_mode = screen->protected_mode;

	if (saved_mode == DEC_PROTECT
	 && saved_mode != mode)
	 	screen->protected_mode = OFF_PROTECT;

	switch (param) {
	case -1:	/* DEFAULT */
	case 0:
		ClearRight(screen, -1);
		break;
	case 1:
		ClearLeft(screen);
		break;
	case 2:
		ClearLine(screen);
		break;
	}
	screen->protected_mode = saved_mode;
}

/*
 * Just like 'do_erase_line()', except that this intercepts ED controls.  If we
 * clear the whole screen, we'll get the return-value from ClearInLine, and
 * find if there were any protected characters left.  If not, reset the
 * protected mode flag in the screen data (it's slower).
 */
void
do_erase_display(screen, param, mode)
	register TScreen *screen;
	int param;
	int mode;
{
	int saved_mode = screen->protected_mode;

	if (saved_mode == DEC_PROTECT
	 && saved_mode != mode)
	 	screen->protected_mode = OFF_PROTECT;

	switch (param) {
	case -1:	/* DEFAULT */
	case 0:
		if (screen->cur_row == 0
		 && screen->cur_col == 0) {
			screen->protected_mode = saved_mode;
			do_erase_display(screen, 2, mode);
			saved_mode = screen->protected_mode;
		} else
			ClearBelow(screen);
		break;

	case 1:
		if (screen->cur_row == screen->max_row
		 && screen->cur_col == screen->max_col) {
			screen->protected_mode = saved_mode;
			do_erase_display(screen, 2, mode);
			saved_mode = screen->protected_mode;
		} else
			ClearAbove(screen);
		break;

	case 2:
		/*
		 * We use 'ClearScreen()' throughout the remainder of the
		 * program for places where we don't care if the characters are
		 * protected or not.  So we modify the logic around this call
		 * on 'ClearScreen()' to handle protected characters.
		 */
		if (screen->protected_mode != OFF_PROTECT) {
			register int row;
			int rc = 1;
			for (row = 0; row <= screen->max_row; row++)
				rc &= ClearInLine(screen, row, 0, screen->max_col + 1);
			if (rc != 0)
				saved_mode = OFF_PROTECT;
		} else {
			ClearScreen(screen);
		}
		break;
	}
	screen->protected_mode = saved_mode;
}

static void
CopyWait(screen)
register TScreen *screen;
{
	XEvent reply;
	XEvent *rep = &reply;

	while (1) {
		XWindowEvent (screen->display, VWindow(screen), 
		  ExposureMask, &reply);
		switch (reply.type) {
		case Expose:
			HandleExposure (screen, &reply);
			break;
		case NoExpose:
		case GraphicsExpose:
			if (screen->incopy <= 0) {
				screen->incopy = 1;
				if (screen->scrolls > 0)
					screen->scrolls--;
			}
			if (reply.type == GraphicsExpose)
			    HandleExposure (screen, &reply);

			if ((reply.type == NoExpose) ||
			    ((XExposeEvent *)rep)->count == 0) {
			    if (screen->incopy <= 0 && screen->scrolls > 0)
				screen->scrolls--;
			    if (screen->scrolls == 0) {
				screen->incopy = 0;
				return;
			    }
			    screen->incopy = -1;
			}
			break;
		}
	}
}

/*
 * used by vertical_copy_area and and horizontal_copy_area
 */
static void
copy_area(screen, src_x, src_y, width, height, dest_x, dest_y)
    TScreen *screen;
    int src_x, src_y;
    unsigned int width, height;
    int dest_x, dest_y;
{
    /* wait for previous CopyArea to complete unless
       multiscroll is enabled and active */
    if (screen->incopy  &&  screen->scrolls == 0)
	CopyWait(screen);
    screen->incopy = -1;

    /* save for translating Expose events */
    screen->copy_src_x = src_x;
    screen->copy_src_y = src_y;
    screen->copy_width = width;
    screen->copy_height = height;
    screen->copy_dest_x = dest_x;
    screen->copy_dest_y = dest_y;

    XCopyArea(screen->display, 
	      TextWindow(screen), TextWindow(screen),
	      NormalGC(screen),
	      src_x, src_y, width, height, dest_x, dest_y);
}

/*
 * use when inserting or deleting characters on the current line
 */
static void
horizontal_copy_area(screen, firstchar, nchars, amount)
    TScreen *screen;
    int firstchar;		/* char pos on screen to start copying at */
    int nchars;
    int amount;			/* number of characters to move right */
{
    int src_x = CursorX(screen, firstchar);
    int src_y = CursorY(screen, screen->cur_row);

    copy_area(screen, src_x, src_y,
	      (unsigned)nchars*FontWidth(screen), FontHeight(screen),
	      src_x + amount*FontWidth(screen), src_y);
}

/*
 * use when inserting or deleting lines from the screen
 */
static void
vertical_copy_area(screen, firstline, nlines, amount)
    TScreen *screen;
    int firstline;		/* line on screen to start copying at */
    int nlines;
    int amount;			/* number of lines to move up (neg=down) */
{
    if(nlines > 0) {
	int src_x = screen->border + Scrollbar(screen);
	int src_y = firstline * FontHeight(screen) + screen->border;

	copy_area(screen, src_x, src_y,
		  (unsigned)Width(screen), nlines*FontHeight(screen),
		  src_x, src_y - amount*FontHeight(screen));
    }
}

/*
 * use when scrolling the entire screen
 */
void
scrolling_copy_area(screen, firstline, nlines, amount)
    TScreen *screen;
    int firstline;		/* line on screen to start copying at */
    int nlines;
    int amount;			/* number of lines to move up (neg=down) */
{

    if(nlines > 0) {
	vertical_copy_area(screen, firstline, nlines, amount);
    }
}

/*
 * Handler for Expose events on the VT widget.
 * Returns 1 iff the area where the cursor was got refreshed.
 */
int
HandleExposure (screen, event)
    register TScreen *screen;
    register XEvent *event;
{
    register XExposeEvent *reply = (XExposeEvent *)event;

#ifndef NO_ACTIVE_ICON
    if (reply->window == screen->iconVwin.window)
	screen->whichVwin = &screen->iconVwin;
    else
	screen->whichVwin = &screen->fullVwin;
#endif /* NO_ACTIVE_ICON */

    /* if not doing CopyArea or if this is a GraphicsExpose, don't translate */
    if(!screen->incopy  ||  event->type != Expose)
	return handle_translated_exposure (screen, reply->x, reply->y,
					   reply->width, reply->height);
    else {
	/* compute intersection of area being copied with
	   area being exposed. */
	int both_x1 = Max(screen->copy_src_x, reply->x);
	int both_y1 = Max(screen->copy_src_y, reply->y);
	int both_x2 = Min(screen->copy_src_x+screen->copy_width,
			  reply->x+reply->width);
	int both_y2 = Min(screen->copy_src_y+screen->copy_height,
			  reply->y+reply->height);
	int value = 0;

	/* was anything copied affected? */
	if(both_x2 > both_x1  && both_y2 > both_y1) {
	    /* do the copied area */
	    value = handle_translated_exposure
		(screen, reply->x + screen->copy_dest_x - screen->copy_src_x,
		 reply->y + screen->copy_dest_y - screen->copy_src_y,
		 reply->width, reply->height);
	}
	/* was anything not copied affected? */
	if(reply->x < both_x1 || reply->y < both_y1
	   || reply->x+reply->width > both_x2
	   || reply->y+reply->height > both_y2)
	    value = handle_translated_exposure (screen, reply->x, reply->y,
						reply->width, reply->height);

	return value;
    }
}

/*
 * Called by the ExposeHandler to do the actual repaint after the coordinates
 * have been translated to allow for any CopyArea in progress.
 * The rectangle passed in is pixel coordinates.
 */
static int
handle_translated_exposure (screen, rect_x, rect_y, rect_width, rect_height)
    register TScreen *screen;
    register int rect_x, rect_y;
    register unsigned int rect_width, rect_height;
{
	register int toprow, leftcol, nrows, ncols;

	toprow = (rect_y - screen->border) / FontHeight(screen);
	if(toprow < 0)
		toprow = 0;
	leftcol = (rect_x - screen->border - Scrollbar(screen))
	    / FontWidth(screen);
	if(leftcol < 0)
		leftcol = 0;
	nrows = (rect_y + rect_height - 1 - screen->border) / 
		FontHeight(screen) - toprow + 1;
	ncols =
	 (rect_x + rect_width - 1 - screen->border - Scrollbar(screen)) /
			FontWidth(screen) - leftcol + 1;
	toprow -= screen->scrolls;
	if (toprow < 0) {
		nrows += toprow;
		toprow = 0;
	}
	if (toprow + nrows - 1 > screen->max_row)
		nrows = screen->max_row - toprow + 1;
	if (leftcol + ncols - 1 > screen->max_col)
		ncols = screen->max_col - leftcol + 1;

	if (nrows > 0 && ncols > 0) {
		ScrnRefresh (screen, toprow, leftcol, nrows, ncols, False);
		if (waiting_for_initial_map) {
		    first_map_occurred ();
		}
		if (screen->cur_row >= toprow &&
		    screen->cur_row < toprow + nrows &&
		    screen->cur_col >= leftcol &&
		    screen->cur_col < leftcol + ncols)
			return (1);

	}
	return (0);
}

/***====================================================================***/

void
GetColors(tw,pColors)
	XtermWidget tw;
	ScrnColors *pColors;
{
	register TScreen *screen = &tw->screen;

	pColors->which=	0;
	SET_COLOR_VALUE(pColors,TEXT_FG,	screen->foreground);
	SET_COLOR_VALUE(pColors,TEXT_BG,	tw->core.background_pixel);
	SET_COLOR_VALUE(pColors,TEXT_CURSOR,	screen->cursorcolor);
	SET_COLOR_VALUE(pColors,MOUSE_FG,	screen->mousecolor);
	SET_COLOR_VALUE(pColors,MOUSE_BG,	screen->mousecolorback);

	SET_COLOR_VALUE(pColors,TEK_FG,		screen->Tforeground);
	SET_COLOR_VALUE(pColors,TEK_BG,		screen->Tbackground);
}

void
ChangeColors(tw,pNew)
	XtermWidget tw;
	ScrnColors *pNew;
{
	register TScreen *screen = &tw->screen;
	Window tek = TWindow(screen);
	Bool	newCursor=	TRUE;

	if (COLOR_DEFINED(pNew,TEXT_BG)) {
	    tw->core.background_pixel=	COLOR_VALUE(pNew,TEXT_BG);
	}

	if (COLOR_DEFINED(pNew,TEXT_CURSOR)) {
	    screen->cursorcolor=	COLOR_VALUE(pNew,TEXT_CURSOR);
	}
	else if ((screen->cursorcolor == screen->foreground)&&
		 (COLOR_DEFINED(pNew,TEXT_FG))) {
	    screen->cursorcolor=	COLOR_VALUE(pNew,TEXT_FG);
	}
	else newCursor=	FALSE;

	if (COLOR_DEFINED(pNew,TEXT_FG)) {
	    Pixel	fg=	COLOR_VALUE(pNew,TEXT_FG);
	    screen->foreground=	fg;
	    XSetForeground(screen->display,NormalGC(screen),fg);
	    XSetBackground(screen->display,ReverseGC(screen),fg);
	    XSetForeground(screen->display,NormalBoldGC(screen),fg);
	    XSetBackground(screen->display,ReverseBoldGC(screen),fg);
	}

	if (COLOR_DEFINED(pNew,TEXT_BG)) {
	    Pixel	bg=	COLOR_VALUE(pNew,TEXT_BG);
	    tw->core.background_pixel=	bg;
	    XSetBackground(screen->display,NormalGC(screen),bg);
	    XSetForeground(screen->display,ReverseGC(screen),bg);
	    XSetBackground(screen->display,NormalBoldGC(screen),bg);
	    XSetForeground(screen->display,ReverseBoldGC(screen),bg);
	    XSetWindowBackground(screen->display, TextWindow(screen),
						  tw->core.background_pixel);
	}

	if (COLOR_DEFINED(pNew,MOUSE_FG)||(COLOR_DEFINED(pNew,MOUSE_BG))) {
	    if (COLOR_DEFINED(pNew,MOUSE_FG))
		screen->mousecolor=	COLOR_VALUE(pNew,MOUSE_FG);
	    if (COLOR_DEFINED(pNew,MOUSE_BG))
		screen->mousecolorback=	COLOR_VALUE(pNew,MOUSE_BG);

	    recolor_cursor (screen->pointer_cursor,
		screen->mousecolor, screen->mousecolorback);
	    recolor_cursor (screen->arrow,
		screen->mousecolor, screen->mousecolorback);
	    XDefineCursor(screen->display, TextWindow(screen),
					   screen->pointer_cursor);
	    if(tek)
		XDefineCursor(screen->display, tek, screen->arrow);
	}

	if ((tek)&&(COLOR_DEFINED(pNew,TEK_FG)||COLOR_DEFINED(pNew,TEK_BG))) {
	    ChangeTekColors(screen,pNew);
	}
	set_cursor_gcs(screen);
	XClearWindow(screen->display, TextWindow(screen));
	ScrnRefresh (screen, 0, 0, screen->max_row + 1,
	 screen->max_col + 1, False);
	if(screen->Tshow) {
	    XClearWindow(screen->display, tek);
	    TekExpose((Widget)NULL, (XEvent *)NULL, (Region)NULL);
	}
}

/***====================================================================***/

#define EXCHANGE(a,b,tmp) tmp = a; a = b; b = tmp;

void
ReverseVideo (termw)
	XtermWidget termw;
{
	register TScreen *screen = &termw->screen;
	GC tmpGC;
	Window tek = TWindow(screen);
	Pixel tmp;

	/*
	 * Swap SGR foreground and background colors.  By convention, these are
	 * the colors assigned to "black" (SGR #0) and "white" (SGR #7).  Also,
	 * SGR #8 and SGR #15 are the bold (or bright) versions of SGR #0 and
	 * #7, respectively.
	 *
	 * We don't swap colors that happen to match the screen's foreground
	 * and background because that tends to produce bizarre effects.
	 */
	if_OPT_ISO_COLORS(screen,{
		EXCHANGE( screen->Acolors[0], screen->Acolors[7],  tmp )
		EXCHANGE( screen->Acolors[8], screen->Acolors[15], tmp )
	})

	tmp = termw->core.background_pixel;
	if(screen->cursorcolor == screen->foreground)
		screen->cursorcolor = tmp;
	termw->core.background_pixel = screen->foreground;
	screen->foreground = tmp;

	EXCHANGE( screen->mousecolor,    screen->mousecolorback, tmp )
	EXCHANGE( NormalGC(screen),      ReverseGC(screen),      tmpGC )
	EXCHANGE( NormalBoldGC(screen),  ReverseBoldGC(screen),  tmpGC )

#ifndef NO_ACTIVE_ICON
	tmpGC = screen->iconVwin.normalGC;
	screen->iconVwin.normalGC = screen->iconVwin.reverseGC;
	screen->iconVwin.reverseGC = tmpGC;

	tmpGC = screen->iconVwin.normalboldGC;
	screen->iconVwin.normalboldGC = screen->iconVwin.reverseboldGC;
	screen->iconVwin.reverseboldGC = tmpGC;
#endif /* NO_ACTIVE_ICON */

	recolor_cursor (screen->pointer_cursor, 
			screen->mousecolor, screen->mousecolorback);
	recolor_cursor (screen->arrow,
			screen->mousecolor, screen->mousecolorback);

	termw->misc.re_verse = !termw->misc.re_verse;

	XDefineCursor(screen->display, TextWindow(screen), screen->pointer_cursor);
	if(tek)
		XDefineCursor(screen->display, tek, screen->arrow);

	if(screen->scrollWidget)
		ScrollBarReverseVideo(screen->scrollWidget);

	XSetWindowBackground(screen->display, TextWindow(screen), termw->core.background_pixel);

	/* the shell-window's background will be used in the first repainting
	 * on resizing
	 */
	XSetWindowBackground(screen->display, VShellWindow, termw->core.background_pixel);

	if(tek) {
	    TekReverseVideo(screen);
	}
	XClearWindow(screen->display, TextWindow(screen));
	ScrnRefresh (screen, 0, 0, screen->max_row + 1,
	 screen->max_col + 1, False);
	if(screen->Tshow) {
	    XClearWindow(screen->display, tek);
	    TekExpose((Widget)NULL, (XEvent *)NULL, (Region)NULL);
	}
	ReverseOldColors();
	update_reversevideo();
}

void
recolor_cursor (cursor, fg, bg)
    Cursor cursor;			/* X cursor ID to set */
    unsigned long fg, bg;		/* pixel indexes to look up */
{
    register TScreen *screen = &term->screen;
    register Display *dpy = screen->display;
    XColor colordefs[2];		/* 0 is foreground, 1 is background */

    colordefs[0].pixel = fg;
    colordefs[1].pixel = bg;
    XQueryColors (dpy, DefaultColormap (dpy, DefaultScreen (dpy)),
		  colordefs, 2);
    XRecolorCursor (dpy, cursor, colordefs, colordefs+1);
    return;
}

/*
 * Draws text with the specified combination of bold/underline
 */
void
drawXtermText(screen, flags, gc, x, y, text, len)
	register TScreen *screen;
	unsigned flags;
	GC gc;
	int x;
	int y;
	char *text;
	int len;
{
	y += FontAscent(screen);
	XDrawImageString(screen->display, TextWindow(screen), gc, 
		x, y,  text, len);
	if ((flags & BOLD) && screen->enbolden)
		XDrawString(screen->display, TextWindow(screen), gc,
			x+1, y,  text, len);
	if ((flags & UNDERLINE) && screen->underline) 
		XDrawLine(screen->display, TextWindow(screen), gc, 
			x, y+1, x + len * FontWidth(screen), y+1);
}

/*
 * Returns a GC, selected according to the font (reverse/bold/normal) that is
 * required for the current position (implied).  The GC is updated with the
 * current screen foreground and background colors.
 */
GC
updatedXtermGC(screen, flags, fg, bg, hilite)
	register TScreen *screen;
	int flags;
	int fg;
	int bg;
	Bool hilite;
{
	Pixel fg_pix = getXtermForeground(flags,fg);
	Pixel bg_pix = getXtermBackground(flags,bg);
	GC gc;

	if ( (!hilite && (flags & INVERSE) != 0)
	  ||  (hilite && (flags & INVERSE) == 0) ) {
		if (flags & BOLD)
			gc = ReverseBoldGC(screen);
		else
			gc = ReverseGC(screen);

		XSetForeground(screen->display, gc, bg_pix);
		XSetBackground(screen->display, gc, fg_pix);

	} else {
		if (flags & BOLD)
			gc = NormalBoldGC(screen);
		else
			gc = NormalGC(screen);

		XSetForeground(screen->display, gc, fg_pix);
		XSetBackground(screen->display, gc, bg_pix);
	}
	return gc;
}

/*
 * Resets the foreground/background of the GC returned by 'updatedXtermGC()'
 * to the values that would be set in SGR_Foreground and SGR_Background. This
 * duplicates some logic, but only modifies 1/4 as many GC's.
 */
void
resetXtermGC(screen, flags, hilite)
	register TScreen *screen;
	int flags;
	Bool hilite;
{
	Pixel fg_pix = getXtermForeground(flags,term->cur_foreground);
	Pixel bg_pix = getXtermBackground(flags,term->cur_background);
	GC gc;

	if ( (!hilite && (flags & INVERSE) != 0)
	  ||  (hilite && (flags & INVERSE) == 0) ) {
		if (flags & BOLD)
			gc = ReverseBoldGC(screen);
		else
			gc = ReverseGC(screen);

		XSetForeground(screen->display, gc, bg_pix);
		XSetBackground(screen->display, gc, fg_pix);

	} else {
		if (flags & BOLD)
			gc = NormalBoldGC(screen);
		else
			gc = NormalGC(screen);

		XSetForeground(screen->display, gc, fg_pix);
		XSetBackground(screen->display, gc, bg_pix);
	}
}

#if OPT_ISO_COLORS
/*
 * Extract the foreground-color index from a one-byte color pair.  If we've got
 * BOLD or UNDERLINE color-mode active, those will be used unless we've got
 * an SGR foreground color active.
 */
unsigned
extract_fg (color, flags)
	unsigned color;
	unsigned flags;
{
	unsigned fg = (color >> 4) & 0xf;
	if (fg == extract_bg(color))
	{
		if (term->screen.colorULMode && (flags & UNDERLINE))
			fg = COLOR_UL;
		if (term->screen.colorBDMode && (flags & BOLD))
			fg = COLOR_BD;
	}
	return fg;
}

unsigned
extract_bg (color)
	unsigned color;
{
	return color & 0xf;
}

/*
 * Combine the current foreground and background into a single 8-bit number.
 * Note that we're storing the SGR foreground, since cur_foreground may be set
 * to COLOR_UL or COLOR_BD, which would make the code larger than 8 bits.
 *
 * FIXME: I'm using the coincidence of fg/bg values to unmask COLOR_UL/COLOR_BD,
 * which will require more work...
 */
unsigned
makeColorPair (fg, bg)
	int fg;
	int bg;
{
	unsigned my_bg = (bg >= 0) && (bg < 16) ? bg : 0;
	unsigned my_fg = (fg >= 0) && (fg < 16) ? fg : my_bg;
	return (my_fg << 4) | my_bg;
}

unsigned
xtermColorPair ()
{
	return makeColorPair(term->sgr_foreground, term->cur_background);
}

Pixel
getXtermForeground(flags, color)
	int flags;
	int color;
{
	Pixel fg = (flags & FG_COLOR) && (color >= 0)
			? term->screen.Acolors[color]
			: term->screen.foreground;

	return fg;
}

Pixel
getXtermBackground(flags, color)
	int flags;
	int color;
{
	Pixel bg = (flags & BG_COLOR) && (color >= 0)
			? term->screen.Acolors[color]
			: term->core.background_pixel;
	return bg;
}

/*
 * Update the screen's background (for XClearArea)
 *
 * If the argument is true, sets the window's background to the value set
 * in the current SGR background. Otherwise, reset to the window's default
 * background.
 */
void useCurBackground(flag)
	Bool flag;
{
	TScreen *screen = &term->screen;
	int color = flag ? term->cur_background : -1;
	Pixel	bg = getXtermBackground(term->flags, color);

	XSetWindowBackground(screen->display, TextWindow(screen), bg);
}

/*
 * Using the "current" SGR background, clear a rectangle.
 */
void ClearCurBackground(screen, top,left, height,width)
	register TScreen *screen;
	int top;
	int left;
	unsigned height;
	unsigned width;
{
	useCurBackground(TRUE);
	XClearArea (screen->display, TextWindow(screen),
		left, top, width, height, FALSE);
	useCurBackground(FALSE);
}
#endif /* OPT_ISO_COLORS */

#ifdef HAVE_CONFIG_H
#if USE_MY_MEMMOVE
char *	my_memmove(s1, s2, n)
	char *	s1;
	char *	s2;
	size_t	n;
{
	if (n != 0) {
		if ((s1+n > s2) && (s2+n > s1)) {
			static	char	*buffer;
			static	size_t	length;
			register int	j;
			if (length < n) {
				length = (n * 3) / 2;
				buffer = doalloc(buffer, length = n);
			}
			for (j = 0; j < n; j++)
				buffer[j] = s2[j];
			s2 = buffer;
		}
		while (n-- != 0)
			s1[n] = s2[n];
	}
	return s1;
}
#endif /* USE_MY_MEMMOVE */

#if !HAVE_STRERROR
char *my_strerror(n)
{
	extern char *sys_errlist[];
	extern int sys_nerr;
	if (n > 0 && n < sys_nerr)
		return sys_errlist[n];
	return "?";
}
#endif
#endif
