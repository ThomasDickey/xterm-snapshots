/*
 *	$XConsortium: util.c,v 1.31 91/06/20 18:34:47 gildea Exp $
 *	$XFree86: xc/programs/xterm/util.c,v 3.5 1996/05/06 06:01:27 dawes Exp $
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

#include "ptyx.h"
#include "data.h"
#include "error.h"
#include "menu.h"

#include <stdio.h>

#include "xterm.h"

extern Bool waiting_for_initial_map;

static int handle_translated_exposure PROTO((TScreen *screen, int rect_x, int rect_y, unsigned int rect_width, unsigned int rect_height));
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
		XClearArea (
		    screen->display,
		    TextWindow(screen),
		    (int) screen->border + screen->scrollbar,
		    (int) refreshtop * FontHeight(screen) + screen->border,
		    (unsigned) Width(screen),
		    (unsigned) refreshheight * FontHeight(screen),
		    FALSE);
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
		XClearArea (
		   screen->display,
		   TextWindow(screen),
		   (int) screen->border + screen->scrollbar,
		   (int) refreshtop * FontHeight(screen) + screen->border,
		   (unsigned) Width(screen),
		   (unsigned) refreshheight * FontHeight(screen),
		   FALSE);
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
	if(refreshheight > 0)
		XClearArea (
		    screen->display,
		    TextWindow(screen),
		    (int) screen->border + screen->scrollbar,
		    (int) refreshtop * FontHeight(screen) + screen->border,
		    (unsigned) Width(screen),
		    (unsigned) refreshheight * FontHeight(screen),
		    FALSE);
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
	if(refreshheight > 0)
		XClearArea (
		    screen->display,
		    TextWindow(screen),
		    (int) screen->border + screen->scrollbar,
		    (int) refreshtop * FontHeight(screen) + screen->border,
		    (unsigned) Width(screen),
		    (unsigned) refreshheight * FontHeight(screen),
		    FALSE);
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
	if(refreshheight > 0)
		XClearArea (
		    screen->display,
		    TextWindow(screen),
		    (int) screen->border + screen->scrollbar,
		    (int) refreshtop * FontHeight(screen) + screen->border,
		    (unsigned) Width(screen),
		    (unsigned) refreshheight * FontHeight(screen),
		    FALSE);
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
        register int cx, cy;

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
	
		cx = CursorX (screen, screen->cur_col);
		cy = CursorY (screen, screen->cur_row);

		useCurBackground(TRUE);
		XFillRectangle(
		    screen->display,
		    TextWindow(screen), 
		    screen->reverseGC,
		    cx, cy,
		    (unsigned) n * FontWidth(screen), (unsigned) FontHeight(screen));
		useCurBackground(FALSE);
	    }
	}
	/* adjust screen->buf */
	ScrnInsertChar(screen->buf, screen->cur_row, screen->cur_col, n,
			screen->max_col + 1);
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
	
		useCurBackground(TRUE);
		XFillRectangle
		    (screen->display, TextWindow(screen),
		     screen->reverseGC,
		     screen->border + screen->scrollbar
		       + Width(screen) - n*FontWidth(screen),
		     CursorY (screen, screen->cur_row), n * FontWidth(screen),
		     FontHeight(screen));
		useCurBackground(FALSE);
	    }
	}
	/* adjust screen->buf */
	ScrnDeleteChar (screen->buf, screen->cur_row, screen->cur_col, n,
			screen->max_col + 1);

}

/*
 * Clear from cursor position to beginning of display, inclusive.
 */
void
ClearAbove (screen)
register TScreen *screen;
{
	register top, height;

	if(screen->cursor_state)
		HideCursor();
	if((top = -screen->topline) <= screen->max_row) {
		if(screen->scroll_amt)
			FlushScroll(screen);
		if((height = screen->cur_row + top) > screen->max_row)
			height = screen->max_row;
		if((height -= top) > 0) {
			useCurBackground(TRUE);
			XClearArea(screen->display, TextWindow(screen),
			 screen->border + screen->scrollbar, top *
			 FontHeight(screen) + screen->border,
			 Width(screen), height * FontHeight(screen), FALSE);
			useCurBackground(FALSE);
		}

		if(screen->cur_row - screen->topline <= screen->max_row)
			ClearLeft(screen);
	}
	ClearBufRows(screen, 0, screen->cur_row - 1);
}

/*
 * Clear from cursor position to end of display, inclusive.
 */
void
ClearBelow (screen)
register TScreen *screen;
{
	register top;

	ClearRight(screen);
	if((top = screen->cur_row - screen->topline) <= screen->max_row) {
		if(screen->scroll_amt)
			FlushScroll(screen);
		if(++top <= screen->max_row) {
			useCurBackground(TRUE);
			XClearArea(screen->display, TextWindow(screen),
			 screen->border + screen->scrollbar, top *
			 FontHeight(screen) + screen->border,
			 Width(screen), (screen->max_row - top + 1) *
			 FontHeight(screen), FALSE);
			useCurBackground(FALSE);
		}
	}
	ClearBufRows(screen, screen->cur_row + 1, screen->max_row);
}

/* 
 * Clear last part of cursor's line, inclusive.
 */
void
ClearRight (screen)
register TScreen *screen;
{
	int	len = (screen->max_col - screen->cur_col + 1);
	ScrnBuf	buf = screen->buf;
	Char *attrs = BUF_ATTRS(buf, screen->cur_row) + screen->cur_col;

	if(screen->cursor_state)
		HideCursor();
	screen->do_wrap = 0;
	if(screen->cur_row - screen->topline <= screen->max_row) {
	    if(!AddToRefresh(screen)) {
	if(screen->scroll_amt)
		FlushScroll(screen);
		useCurBackground(TRUE);
		XFillRectangle(screen->display, TextWindow(screen),
		  screen->reverseGC,
		 CursorX(screen, screen->cur_col),
		 CursorY(screen, screen->cur_row),
		 Width(screen) - screen->cur_col * FontWidth(screen),
		 FontHeight(screen));
		useCurBackground(FALSE);
	    }
	}
	bzero(BUF_CHARS(buf, screen->cur_row) + screen->cur_col, len);
	memset(attrs, term->flags & (FG_COLOR|BG_COLOR), len);
	memset(BUF_FORES(buf, screen->cur_row) + screen->cur_col,
		(term->flags & FG_COLOR)
		? term->cur_foreground : 0, len);
	memset(BUF_BACKS(buf, screen->cur_row) + screen->cur_col,
		(term->flags & BG_COLOR)
		? term->cur_background : 0, len);

	/* with the right part cleared, we can't be wrapping */
	attrs[0] &= ~LINEWRAPPED;
}

/*
 * Clear first part of cursor's line, inclusive.
 */
void
ClearLeft (screen)
    register TScreen *screen;
{
	int len = screen->cur_col + 1;
	int flags = CHARDRAWN | (term->flags & (FG_COLOR|BG_COLOR));

	if(screen->cursor_state)
		HideCursor();
	screen->do_wrap = 0;
	if(screen->cur_row - screen->topline <= screen->max_row) {
	    if(!AddToRefresh(screen)) {
		if(screen->scroll_amt)
			FlushScroll(screen);
		useCurBackground(TRUE);
		XFillRectangle (screen->display, TextWindow(screen),
		     screen->reverseGC,
		     screen->border + screen->scrollbar,
		      CursorY (screen, screen->cur_row),
		     len * FontWidth(screen),
		     FontHeight(screen));
		useCurBackground(FALSE);
	    }
	}
	
	memset(SCRN_BUF_CHARS(screen, screen->cur_row), ' ',   len);
	memset(SCRN_BUF_ATTRS(screen, screen->cur_row), flags, len);
	memset(SCRN_BUF_FORES(screen, screen->cur_row),
		flags & FG_COLOR ? term->cur_foreground : 0, len);
	memset(SCRN_BUF_BACKS(screen, screen->cur_row),
		flags & BG_COLOR ? term->cur_background : 0, len);

}

/* 
 * Erase the cursor's line.
 */
void
ClearLine(screen)
register TScreen *screen;
{
	if(screen->cursor_state)
		HideCursor();
	screen->do_wrap = 0;
	if(screen->cur_row - screen->topline <= screen->max_row) {
	    if(!AddToRefresh(screen)) {
		if(screen->scroll_amt)
			FlushScroll(screen);
		useCurBackground(TRUE);
		XFillRectangle (screen->display, TextWindow(screen), 
		     screen->reverseGC,
		     screen->border + screen->scrollbar,
		      CursorY (screen, screen->cur_row),
		     Width(screen), FontHeight(screen));
		useCurBackground(FALSE);
	    }
	}
	bzero (SCRN_BUF_CHARS(screen, screen->cur_row), (screen->max_col + 1));
	bzero (SCRN_BUF_ATTRS(screen, screen->cur_row), (screen->max_col + 1));
	bzero (SCRN_BUF_FORES(screen, screen->cur_row), (screen->max_col + 1));
	bzero (SCRN_BUF_BACKS(screen, screen->cur_row), (screen->max_col + 1));
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
		useCurBackground(TRUE);
		XClearArea(screen->display, TextWindow(screen),
			 screen->border + screen->scrollbar, 
			 top * FontHeight(screen) + screen->border,	
		 	 Width(screen), (screen->max_row - top + 1) *
			 FontHeight(screen), FALSE);
		useCurBackground(FALSE);
	}
	ClearBufRows (screen, 0, screen->max_row);
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
	      screen->normalGC,
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
	int src_x = screen->border + screen->scrollbar;
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
	leftcol = (rect_x - screen->border - screen->scrollbar)
	    / FontWidth(screen);
	if(leftcol < 0)
		leftcol = 0;
	nrows = (rect_y + rect_height - 1 - screen->border) / 
		FontHeight(screen) - toprow + 1;
	ncols =
	 (rect_x + rect_width - 1 - screen->border - screen->scrollbar) /
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
	    XSetForeground(screen->display,screen->normalGC,fg);
	    XSetBackground(screen->display,screen->reverseGC,fg);
	    XSetForeground(screen->display,screen->normalboldGC,fg);
	    XSetBackground(screen->display,screen->reverseboldGC,fg);
	}

	if (COLOR_DEFINED(pNew,TEXT_BG)) {
	    Pixel	bg=	COLOR_VALUE(pNew,TEXT_BG);
	    tw->core.background_pixel=	bg;
	    XSetBackground(screen->display,screen->normalGC,bg);
	    XSetForeground(screen->display,screen->reverseGC,bg);
	    XSetBackground(screen->display,screen->normalboldGC,bg);
	    XSetForeground(screen->display,screen->reverseboldGC,bg);
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
	EXCHANGE( screen->Acolors[0], screen->Acolors[7],  tmp )
	EXCHANGE( screen->Acolors[8], screen->Acolors[15], tmp )

	tmp = termw->core.background_pixel;
	if(screen->cursorcolor == screen->foreground)
		screen->cursorcolor = tmp;
	termw->core.background_pixel = screen->foreground;
	screen->foreground = tmp;

	EXCHANGE( screen->mousecolor,    screen->mousecolorback, tmp )
	EXCHANGE( screen->normalGC,      screen->reverseGC,      tmpGC )
	EXCHANGE( screen->normalboldGC,  screen->reverseboldGC,  tmpGC )

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
			gc = screen->reverseboldGC;
		else
			gc = screen->reverseGC;

		XSetForeground(screen->display, gc, bg_pix);
		XSetBackground(screen->display, gc, fg_pix);

	} else {
		if (flags & BOLD)
			gc = screen->normalboldGC;
		else
			gc = screen->normalGC;

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
			gc = screen->reverseboldGC;
		else
			gc = screen->reverseGC;

		XSetForeground(screen->display, gc, bg_pix);
		XSetBackground(screen->display, gc, fg_pix);

	} else {
		if (flags & BOLD)
			gc = screen->normalboldGC;
		else
			gc = screen->normalGC;

		XSetForeground(screen->display, gc, fg_pix);
		XSetBackground(screen->display, gc, bg_pix);
	}
}

/* GET_FG */
Pixel
getXtermForeground(flags, color)
	int flags;
	int color;
{
	Pixel fg = (flags & FG_COLOR)
			? term->screen.Acolors[color]
			: term->screen.foreground;

	if (term->misc.re_verse && (fg == term->screen.original_fg))
		fg = term->screen.original_bg;

	return fg;
}

/* GET_BG */
Pixel
getXtermBackground(flags, color)
	int flags;
	int color;
{
	Pixel bg = (flags & BG_COLOR)
			? term->screen.Acolors[color]
			: term->core.background_pixel;

	if (term->misc.re_verse && (bg == term->screen.original_bg))
		bg = term->screen.original_fg;

	return bg;
}

/*
 * Update the screen's background (for XClearArea)
 *
 * If the argument is true, sets the window's background to the value set
 * in the current SGR background. Otherwise, reset to the window's default
 * background.
 */
void useCurBackground(Bool flag)
{
	TScreen *screen = &term->screen;
	int color = flag ? term->cur_background : -1;
	Pixel	bg = getXtermBackground(term->flags, color);

	XSetWindowBackground(screen->display, TextWindow(screen), bg);
}
