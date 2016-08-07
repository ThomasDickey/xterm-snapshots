/* $XTermId: util.c,v 1.313 2006/11/29 22:52:10 tom Exp $ */

/* $XFree86: xc/programs/xterm/util.c,v 3.98 2006/06/19 00:36:52 dickey Exp $ */

/*
 * Copyright 1999-2005,2006 by Thomas E. Dickey
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
 *
 *
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

#include <xterm.h>

#include <data.h>
#include <error.h>
#include <menu.h>
#include <fontutils.h>
#include <xstrings.h>

#include <stdio.h>
#include <ctype.h>
#include <assert.h>

#if OPT_WIDE_CHARS
#if defined(HAVE_WCHAR_H) && defined(HAVE_WCWIDTH)
#include <wchar.h>
#endif
#include <wcwidth.h>
#endif

static int ClearInLine(XtermWidget xw, int row, int col, unsigned len);
static int handle_translated_exposure(XtermWidget xw,
				      int rect_x,
				      int rect_y,
				      int rect_width,
				      int rect_height);
static void ClearLeft(XtermWidget xw);
static void CopyWait(XtermWidget xw);
static void horizontal_copy_area(XtermWidget xw,
				 int firstchar,
				 int nchars,
				 int amount);
static void vertical_copy_area(XtermWidget xw,
			       int firstline,
			       int nlines,
			       int amount);

#if OPT_WIDE_CHARS
int (*my_wcwidth) (wchar_t);
#endif

/*
 * These routines are used for the jump scroll feature
 */
void
FlushScroll(XtermWidget xw)
{
    TScreen *screen = &(xw->screen);
    int i;
    int shift = INX2ROW(screen, 0);
    int bot = screen->max_row - shift;
    int refreshtop;
    int refreshheight;
    int scrolltop;
    int scrollheight;

    if (screen->cursor_state)
	HideCursor();
    if (screen->scroll_amt > 0) {
	refreshheight = screen->refresh_amt;
	scrollheight = screen->bot_marg - screen->top_marg -
	    refreshheight + 1;
	if ((refreshtop = screen->bot_marg - refreshheight + 1 + shift) >
	    (i = screen->max_row - screen->scroll_amt + 1))
	    refreshtop = i;
	if (screen->scrollWidget && !screen->alternate
	    && screen->top_marg == 0) {
	    scrolltop = 0;
	    if ((scrollheight += shift) > i)
		scrollheight = i;
	    if ((i = screen->bot_marg - bot) > 0 &&
		(refreshheight -= i) < screen->scroll_amt)
		refreshheight = screen->scroll_amt;
	    if ((i = screen->savedlines) < screen->savelines) {
		if ((i += screen->scroll_amt) >
		    screen->savelines)
		    i = screen->savelines;
		screen->savedlines = i;
		ScrollBarDrawThumb(screen->scrollWidget);
	    }
	} else {
	    scrolltop = screen->top_marg + shift;
	    if ((i = bot - (screen->bot_marg - screen->refresh_amt +
			    screen->scroll_amt)) > 0) {
		if (bot < screen->bot_marg)
		    refreshheight = screen->scroll_amt + i;
	    } else {
		scrollheight += i;
		refreshheight = screen->scroll_amt;
		if ((i = screen->top_marg + screen->scroll_amt -
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
	if ((i = screen->bot_marg - bot) > 0)
	    scrollheight -= i;
	if ((i = screen->top_marg + refreshheight - 1 - bot) > 0)
	    refreshheight -= i;
    }
    scrolling_copy_area(xw, scrolltop + screen->scroll_amt,
			scrollheight, screen->scroll_amt);
    ScrollSelection(screen, -(screen->scroll_amt), False);
    screen->scroll_amt = 0;
    screen->refresh_amt = 0;
    if (refreshheight > 0) {
	ClearCurBackground(xw,
			   (int) refreshtop * FontHeight(screen) + screen->border,
			   (int) OriginX(screen),
			   (unsigned) refreshheight * FontHeight(screen),
			   (unsigned) Width(screen));
	ScrnRefresh(xw, refreshtop, 0, refreshheight,
		    MaxCols(screen), False);
    }
    return;
}

int
AddToRefresh(TScreen * screen)
{
    int amount = screen->refresh_amt;
    int row = screen->cur_row;

    if (amount == 0) {
	return (0);
    } else if (amount > 0) {
	int bottom;

	if (row == (bottom = screen->bot_marg) - amount) {
	    screen->refresh_amt++;
	    return (1);
	}
	return (row >= bottom - amount + 1 && row <= bottom);
    } else {
	int top;

	amount = -amount;
	if (row == (top = screen->top_marg) + amount) {
	    screen->refresh_amt--;
	    return (1);
	}
	return (row <= top + amount - 1 && row >= top);
    }
}

/*
 * If we're scrolling, leave the selection intact if possible.
 * If it will bump into one of the extremes of the saved-lines, truncate that.
 * If the selection is not contained within the scrolled region, clear it.
 */
static void
adjustHiliteOnFwdScroll(XtermWidget xw, int amount, Bool all_lines)
{
    TScreen *screen = &(xw->screen);
    int lo_row = (all_lines
		  ? (screen->bot_marg - screen->savelines)
		  : screen->top_marg);
    int hi_row = screen->bot_marg;

    TRACE2(("adjustSelection FWD %s by %d (%s)\n",
	    screen->alternate ? "alternate" : "normal",
	    amount,
	    all_lines ? "all" : "visible"));
    TRACE2(("  before highlite %d.%d .. %d.%d\n",
	    screen->startH.row,
	    screen->startH.col,
	    screen->endH.row,
	    screen->endH.col));
    TRACE2(("  margins %d..%d\n", screen->top_marg, screen->bot_marg));
    TRACE2(("  limits  %d..%d\n", lo_row, hi_row));

    if (screen->startH.row >= lo_row
	&& screen->startH.row - amount < lo_row) {
	/* truncate the selection because its start would move out of region */
	if (lo_row + amount <= screen->endH.row) {
	    TRACE2(("truncate selection by changing start %d.%d to %d.%d\n",
		    screen->startH.row,
		    screen->startH.col,
		    lo_row + amount,
		    0));
	    screen->startH.row = lo_row + amount;
	    screen->startH.col = 0;
	} else {
	    TRACE2(("deselect because %d.%d .. %d.%d shifted %d is outside margins %d..%d\n",
		    screen->startH.row,
		    screen->startH.col,
		    screen->endH.row,
		    screen->endH.col,
		    -amount,
		    lo_row,
		    hi_row));
	    ScrnDisownSelection(xw);
	}
    } else if (screen->startH.row <= hi_row && screen->endH.row > hi_row) {
	ScrnDisownSelection(xw);
    } else if (screen->startH.row < lo_row && screen->endH.row > lo_row) {
	ScrnDisownSelection(xw);
    }

    TRACE2(("  after highlite %d.%d .. %d.%d\n",
	    screen->startH.row,
	    screen->startH.col,
	    screen->endH.row,
	    screen->endH.col));
}

/*
 * This is the same as adjustHiliteOnFwdScroll(), but reversed.  In this case,
 * only the visible lines are affected.
 */
static void
adjustHiliteOnBakScroll(XtermWidget xw, int amount)
{
    TScreen *screen = &(xw->screen);
    int lo_row = screen->top_marg;
    int hi_row = screen->bot_marg;

    TRACE2(("adjustSelection BAK %s by %d (%s)\n",
	    screen->alternate ? "alternate" : "normal",
	    amount,
	    "visible"));
    TRACE2(("  before highlite %d.%d .. %d.%d\n",
	    screen->startH.row,
	    screen->startH.col,
	    screen->endH.row,
	    screen->endH.col));
    TRACE2(("  margins %d..%d\n", screen->top_marg, screen->bot_marg));

    if (screen->endH.row >= hi_row
	&& screen->endH.row + amount > hi_row) {
	/* truncate the selection because its start would move out of region */
	if (hi_row - amount >= screen->startH.row) {
	    TRACE2(("truncate selection by changing start %d.%d to %d.%d\n",
		    screen->startH.row,
		    screen->startH.col,
		    hi_row - amount,
		    0));
	    screen->endH.row = hi_row - amount;
	    screen->endH.col = 0;
	} else {
	    TRACE2(("deselect because %d.%d .. %d.%d shifted %d is outside margins %d..%d\n",
		    screen->startH.row,
		    screen->startH.col,
		    screen->endH.row,
		    screen->endH.col,
		    amount,
		    lo_row,
		    hi_row));
	    ScrnDisownSelection(xw);
	}
    } else if (screen->endH.row >= lo_row && screen->startH.row < lo_row) {
	ScrnDisownSelection(xw);
    } else if (screen->endH.row > hi_row && screen->startH.row > hi_row) {
	ScrnDisownSelection(xw);
    }

    TRACE2(("  after highlite %d.%d .. %d.%d\n",
	    screen->startH.row,
	    screen->startH.col,
	    screen->endH.row,
	    screen->endH.col));
}

/*
 * scrolls the screen by amount lines, erases bottom, doesn't alter
 * cursor position (i.e. cursor moves down amount relative to text).
 * All done within the scrolling region, of course.
 * requires: amount > 0
 */
void
xtermScroll(XtermWidget xw, int amount)
{
    TScreen *screen = &(xw->screen);
    int i = screen->bot_marg - screen->top_marg + 1;
    int shift;
    int bot;
    int refreshtop = 0;
    int refreshheight;
    int scrolltop;
    int scrollheight;
    Boolean scroll_all_lines = (screen->scrollWidget
				&& !screen->alternate
				&& screen->top_marg == 0);

    TRACE(("xtermScroll count=%d\n", amount));

    screen->cursor_busy += 1;
    screen->cursor_moved = True;

    if (screen->cursor_state)
	HideCursor();

    if (amount > i)
	amount = i;

    if (ScrnHaveSelection(screen))
	adjustHiliteOnFwdScroll(xw, amount, scroll_all_lines);

    if (screen->jumpscroll) {
	if (screen->scroll_amt > 0) {
	    if (screen->refresh_amt + amount > i)
		FlushScroll(xw);
	    screen->scroll_amt += amount;
	    screen->refresh_amt += amount;
	} else {
	    if (screen->scroll_amt < 0)
		FlushScroll(xw);
	    screen->scroll_amt = amount;
	    screen->refresh_amt = amount;
	}
	refreshheight = 0;
    } else {
	ScrollSelection(screen, -(amount), False);
	if (amount == i) {
	    ClearScreen(xw);
	    screen->cursor_busy -= 1;
	    return;
	}

	shift = INX2ROW(screen, 0);
	bot = screen->max_row - shift;
	scrollheight = i - amount;
	refreshheight = amount;

	if ((refreshtop = screen->bot_marg - refreshheight + 1 + shift) >
	    (i = screen->max_row - refreshheight + 1))
	    refreshtop = i;

	if (scroll_all_lines) {
	    scrolltop = 0;
	    if ((scrollheight += shift) > i)
		scrollheight = i;
	    if ((i = screen->savedlines) < screen->savelines) {
		if ((i += amount) > screen->savelines)
		    i = screen->savelines;
		screen->savedlines = i;
		ScrollBarDrawThumb(screen->scrollWidget);
	    }
	} else {
	    scrolltop = screen->top_marg + shift;
	    if ((i = screen->bot_marg - bot) > 0) {
		scrollheight -= i;
		if ((i = screen->top_marg + amount - 1 - bot) >= 0) {
		    refreshtop += i;
		    refreshheight -= i;
		}
	    }
	}

	if (screen->multiscroll && amount == 1 &&
	    screen->topline == 0 && screen->top_marg == 0 &&
	    screen->bot_marg == screen->max_row) {
	    if (screen->incopy < 0 && screen->scrolls == 0)
		CopyWait(xw);
	    screen->scrolls++;
	}

	scrolling_copy_area(xw, scrolltop + amount, scrollheight, amount);

	if (refreshheight > 0) {
	    ClearCurBackground(xw,
			       (int) refreshtop * FontHeight(screen) + screen->border,
			       (int) OriginX(screen),
			       (unsigned) refreshheight * FontHeight(screen),
			       (unsigned) Width(screen));
	    if (refreshheight > shift)
		refreshheight = shift;
	}
    }

    if (amount > 0) {
	if (scroll_all_lines) {
	    ScrnDeleteLine(xw,
			   screen->allbuf,
			   screen->bot_marg + screen->savelines,
			   0,
			   (unsigned) amount,
			   (unsigned) MaxCols(screen));
	} else {
	    ScrnDeleteLine(xw,
			   screen->visbuf,
			   screen->bot_marg,
			   screen->top_marg,
			   (unsigned) amount,
			   (unsigned) MaxCols(screen));
	}
    }

    if (refreshheight > 0) {
	ScrnRefresh(xw, refreshtop, 0, refreshheight,
		    MaxCols(screen), False);
    }

    screen->cursor_busy -= 1;
    return;
}

/*
 * Reverse scrolls the screen by amount lines, erases top, doesn't alter
 * cursor position (i.e. cursor moves up amount relative to text).
 * All done within the scrolling region, of course.
 * Requires: amount > 0
 */
void
RevScroll(XtermWidget xw, int amount)
{
    TScreen *screen = &(xw->screen);
    int i = screen->bot_marg - screen->top_marg + 1;
    int shift;
    int bot;
    int refreshtop;
    int refreshheight;
    int scrolltop;
    int scrollheight;

    TRACE(("RevScroll count=%d\n", amount));

    screen->cursor_busy += 1;
    screen->cursor_moved = True;

    if (screen->cursor_state)
	HideCursor();

    if (amount > i)
	amount = i;

    if (ScrnHaveSelection(screen))
	adjustHiliteOnBakScroll(xw, amount);

    if (screen->jumpscroll) {
	if (screen->scroll_amt < 0) {
	    if (-screen->refresh_amt + amount > i)
		FlushScroll(xw);
	    screen->scroll_amt -= amount;
	    screen->refresh_amt -= amount;
	} else {
	    if (screen->scroll_amt > 0)
		FlushScroll(xw);
	    screen->scroll_amt = -amount;
	    screen->refresh_amt = -amount;
	}
    } else {
	shift = INX2ROW(screen, 0);
	bot = screen->max_row - shift;
	refreshheight = amount;
	scrollheight = screen->bot_marg - screen->top_marg -
	    refreshheight + 1;
	refreshtop = screen->top_marg + shift;
	scrolltop = refreshtop + refreshheight;
	if ((i = screen->bot_marg - bot) > 0)
	    scrollheight -= i;
	if ((i = screen->top_marg + refreshheight - 1 - bot) > 0)
	    refreshheight -= i;

	if (screen->multiscroll && amount == 1 &&
	    screen->topline == 0 && screen->top_marg == 0 &&
	    screen->bot_marg == screen->max_row) {
	    if (screen->incopy < 0 && screen->scrolls == 0)
		CopyWait(xw);
	    screen->scrolls++;
	}

	scrolling_copy_area(xw, scrolltop - amount, scrollheight, -amount);

	if (refreshheight > 0) {
	    ClearCurBackground(xw,
			       (int) refreshtop * FontHeight(screen) + screen->border,
			       (int) OriginX(screen),
			       (unsigned) refreshheight * FontHeight(screen),
			       (unsigned) Width(screen));
	}
    }
    if (amount > 0) {
	ScrnInsertLine(xw,
		       screen->visbuf,
		       screen->bot_marg,
		       screen->top_marg,
		       (unsigned) amount,
		       (unsigned) MaxCols(screen));
    }
    screen->cursor_busy -= 1;
    return;
}

/*
 * If cursor not in scrolling region, returns.  Else,
 * inserts n blank lines at the cursor's position.  Lines above the
 * bottom margin are lost.
 */
void
InsertLine(XtermWidget xw, int n)
{
    TScreen *screen = &(xw->screen);
    int i;
    int shift;
    int bot;
    int refreshtop;
    int refreshheight;
    int scrolltop;
    int scrollheight;

    if (!ScrnIsLineInMargins(screen, INX2ROW(screen, screen->cur_row)))
	return;

    TRACE(("InsertLine count=%d\n", n));

    if (screen->cursor_state)
	HideCursor();

    if (ScrnHaveSelection(screen)
	&& ScrnAreLinesInSelection(screen, screen->top_marg, screen->bot_marg)) {
	ScrnDisownSelection(xw);
    }

    screen->do_wrap = 0;
    if (n > (i = screen->bot_marg - screen->cur_row + 1))
	n = i;
    if (screen->jumpscroll) {
	if (screen->scroll_amt <= 0 &&
	    screen->cur_row <= -screen->refresh_amt) {
	    if (-screen->refresh_amt + n > MaxRows(screen))
		FlushScroll(xw);
	    screen->scroll_amt -= n;
	    screen->refresh_amt -= n;
	} else if (screen->scroll_amt)
	    FlushScroll(xw);
    }
    if (!screen->scroll_amt) {
	shift = INX2ROW(screen, 0);
	bot = screen->max_row - shift;
	refreshheight = n;
	scrollheight = screen->bot_marg - screen->cur_row - refreshheight + 1;
	refreshtop = screen->cur_row + shift;
	scrolltop = refreshtop + refreshheight;
	if ((i = screen->bot_marg - bot) > 0)
	    scrollheight -= i;
	if ((i = screen->cur_row + refreshheight - 1 - bot) > 0)
	    refreshheight -= i;
	vertical_copy_area(xw, scrolltop - n, scrollheight, -n);
	if (refreshheight > 0) {
	    ClearCurBackground(xw,
			       (int) refreshtop * FontHeight(screen) + screen->border,
			       (int) OriginX(screen),
			       (unsigned) refreshheight * FontHeight(screen),
			       (unsigned) Width(screen));
	}
    }
    if (n > 0) {
	ScrnInsertLine(xw,
		       screen->visbuf,
		       screen->bot_marg,
		       screen->cur_row,
		       (unsigned) n,
		       (unsigned) MaxCols(screen));
    }
}

/*
 * If cursor not in scrolling region, returns.  Else, deletes n lines
 * at the cursor's position, lines added at bottom margin are blank.
 */
void
DeleteLine(XtermWidget xw, int n)
{
    TScreen *screen = &(xw->screen);
    int i;
    int shift;
    int bot;
    int refreshtop;
    int refreshheight;
    int scrolltop;
    int scrollheight;

    if (!ScrnIsLineInMargins(screen, INX2ROW(screen, screen->cur_row)))
	return;

    TRACE(("DeleteLine count=%d\n", n));

    if (screen->cursor_state)
	HideCursor();

    if (ScrnHaveSelection(screen)
	&& ScrnAreLinesInSelection(screen, screen->top_marg, screen->bot_marg)) {
	ScrnDisownSelection(xw);
    }

    screen->do_wrap = 0;
    if (n > (i = screen->bot_marg - screen->cur_row + 1))
	n = i;
    if (screen->jumpscroll) {
	if (screen->scroll_amt >= 0 && screen->cur_row == screen->top_marg) {
	    if (screen->refresh_amt + n > MaxRows(screen))
		FlushScroll(xw);
	    screen->scroll_amt += n;
	    screen->refresh_amt += n;
	} else if (screen->scroll_amt)
	    FlushScroll(xw);
    }
    if (!screen->scroll_amt) {

	shift = INX2ROW(screen, 0);
	bot = screen->max_row - shift;
	scrollheight = i - n;
	refreshheight = n;
	if ((refreshtop = screen->bot_marg - refreshheight + 1 + shift) >
	    (i = screen->max_row - refreshheight + 1))
	    refreshtop = i;
	if (screen->scrollWidget && !screen->alternate && screen->cur_row == 0) {
	    scrolltop = 0;
	    if ((scrollheight += shift) > i)
		scrollheight = i;
	    if ((i = screen->savedlines) < screen->savelines) {
		if ((i += n) > screen->savelines)
		    i = screen->savelines;
		screen->savedlines = i;
		ScrollBarDrawThumb(screen->scrollWidget);
	    }
	} else {
	    scrolltop = screen->cur_row + shift;
	    if ((i = screen->bot_marg - bot) > 0) {
		scrollheight -= i;
		if ((i = screen->cur_row + n - 1 - bot) >= 0) {
		    refreshheight -= i;
		}
	    }
	}
	vertical_copy_area(xw, scrolltop + n, scrollheight, n);
	if (refreshheight > 0) {
	    ClearCurBackground(xw,
			       (int) refreshtop * FontHeight(screen) + screen->border,
			       (int) OriginX(screen),
			       (unsigned) refreshheight * FontHeight(screen),
			       (unsigned) Width(screen));
	}
    }
    /* adjust screen->buf */
    if (n > 0) {
	if (screen->scrollWidget
	    && !screen->alternate
	    && screen->cur_row == 0)
	    ScrnDeleteLine(xw,
			   screen->allbuf,
			   screen->bot_marg + screen->savelines,
			   0,
			   (unsigned) n,
			   (unsigned) MaxCols(screen));
	else
	    ScrnDeleteLine(xw,
			   screen->visbuf,
			   screen->bot_marg,
			   screen->cur_row,
			   (unsigned) n,
			   (unsigned) MaxCols(screen));
    }
}

/*
 * Insert n blanks at the cursor's position, no wraparound
 */
void
InsertChar(XtermWidget xw, unsigned n)
{
    TScreen *screen = &(xw->screen);
    unsigned limit;
    int row = INX2ROW(screen, screen->cur_row);

    if (screen->cursor_state)
	HideCursor();

    TRACE(("InsertChar count=%d\n", n));

    if (ScrnHaveSelection(screen)
	&& ScrnIsLineInSelection(screen, row)) {
	ScrnDisownSelection(xw);
    }
    screen->do_wrap = 0;

    assert(screen->cur_col <= screen->max_col);
    limit = MaxCols(screen) - screen->cur_col;

    if (n > limit)
	n = limit;

    assert(n != 0);
    if (row <= screen->max_row) {
	if (!AddToRefresh(screen)) {
	    int col = MaxCols(screen) - n;
	    if (screen->scroll_amt)
		FlushScroll(xw);

#if OPT_DEC_CHRSET
	    if (CSET_DOUBLE(SCRN_BUF_CSETS(screen, screen->cur_row)[0])) {
		col = MaxCols(screen) / 2 - n;
	    }
#endif
	    /*
	     * prevent InsertChar from shifting the end of a line over
	     * if it is being appended to
	     */
	    if (non_blank_line(screen, screen->cur_row,
			       screen->cur_col, MaxCols(screen)))
		horizontal_copy_area(xw, screen->cur_col,
				     col - screen->cur_col,
				     (int) n);

	    ClearCurBackground(xw,
			       CursorY(screen, screen->cur_row),
			       CurCursorX(screen, screen->cur_row, screen->cur_col),
			       (unsigned) FontHeight(screen),
			       n * CurFontWidth(screen, screen->cur_row));
	}
    }
    /* adjust screen->buf */
    ScrnInsertChar(xw, n);
}

/*
 * Deletes n chars at the cursor's position, no wraparound.
 */
void
DeleteChar(XtermWidget xw, unsigned n)
{
    TScreen *screen = &(xw->screen);
    unsigned limit;
    int row = INX2ROW(screen, screen->cur_row);

    if (screen->cursor_state)
	HideCursor();

    TRACE(("DeleteChar count=%d\n", n));

    if (ScrnHaveSelection(screen)
	&& ScrnIsLineInSelection(screen, row)) {
	ScrnDisownSelection(xw);
    }
    screen->do_wrap = 0;

    assert(screen->cur_col <= screen->max_col);
    limit = MaxCols(screen) - screen->cur_col;

    if (n > limit)
	n = limit;

    assert(n != 0);
    if (row <= screen->max_row) {
	if (!AddToRefresh(screen)) {
	    int col = MaxCols(screen) - n;
	    if (screen->scroll_amt)
		FlushScroll(xw);

#if OPT_DEC_CHRSET
	    if (CSET_DOUBLE(SCRN_BUF_CSETS(screen, screen->cur_row)[0])) {
		col = MaxCols(screen) / 2 - n;
	    }
#endif
	    horizontal_copy_area(xw,
				 (int) (screen->cur_col + n),
				 col - screen->cur_col,
				 -((int) n));

	    ClearCurBackground(xw,
			       CursorY(screen, screen->cur_row),
			       CurCursorX(screen, screen->cur_row, col),
			       (unsigned) FontHeight(screen),
			       n * CurFontWidth(screen, screen->cur_row));
	}
    }
    if (n > 0) {
	/* adjust screen->buf */
	ScrnDeleteChar(xw, (unsigned) n);
    }
}

/*
 * Clear from cursor position to beginning of display, inclusive.
 */
static void
ClearAbove(XtermWidget xw)
{
    TScreen *screen = &(xw->screen);

    if (screen->protected_mode != OFF_PROTECT) {
	int row;
	unsigned len = MaxCols(screen);

	assert(screen->max_col >= 0);
	for (row = 0; row <= screen->max_row; row++)
	    ClearInLine(xw, row, 0, len);
    } else {
	int top, height;

	if (screen->cursor_state)
	    HideCursor();
	if ((top = INX2ROW(screen, 0)) <= screen->max_row) {
	    if (screen->scroll_amt)
		FlushScroll(xw);
	    if ((height = screen->cur_row + top) > screen->max_row)
		height = screen->max_row;
	    if ((height -= top) > 0) {
		ClearCurBackground(xw,
				   top * FontHeight(screen) + screen->border,
				   OriginX(screen),
				   (unsigned) (height * FontHeight(screen)),
				   (unsigned) (Width(screen)));
	    }
	}
	ClearBufRows(xw, 0, screen->cur_row - 1);
    }

    if (INX2ROW(screen, screen->cur_row) <= screen->max_row)
	ClearLeft(xw);
}

/*
 * Clear from cursor position to end of display, inclusive.
 */
static void
ClearBelow(XtermWidget xw)
{
    TScreen *screen = &(xw->screen);

    ClearRight(xw, -1);

    if (screen->protected_mode != OFF_PROTECT) {
	int row;
	unsigned len = MaxCols(screen);

	assert(screen->max_col >= 0);
	for (row = screen->cur_row + 1; row <= screen->max_row; row++)
	    ClearInLine(xw, row, 0, len);
    } else {
	int top;

	if ((top = INX2ROW(screen, screen->cur_row)) <= screen->max_row) {
	    if (screen->scroll_amt)
		FlushScroll(xw);
	    if (++top <= screen->max_row) {
		ClearCurBackground(xw,
				   top * FontHeight(screen) + screen->border,
				   OriginX(screen),
				   (unsigned) ((screen->max_row - top + 1)
					       * FontHeight(screen)),
				   (unsigned) (Width(screen)));
	    }
	}
	ClearBufRows(xw, screen->cur_row + 1, screen->max_row);
    }
}

/*
 * Clear the given row, for the given range of columns, returning 1 if no
 * protected characters were found, 0 otherwise.
 */
static int
ClearInLine(XtermWidget xw, int row, int col, unsigned len)
{
    TScreen *screen = &(xw->screen);
    int rc = 1;
    int flags = TERM_COLOR_FLAGS(xw);

    TRACE(("ClearInLine(row=%d, col=%d, len=%d) vs %d..%d\n",
	   row, col, len,
	   screen->startH.row,
	   screen->startH.col));

    if (ScrnHaveSelection(screen)
	&& ScrnIsLineInSelection(screen, row)) {
	ScrnDisownSelection(xw);
    }

    /*
     * If we're clearing to the end of the line, we won't count this as
     * "drawn" characters.  We'll only do cut/paste on "drawn" characters,
     * so this has the effect of suppressing trailing blanks from a
     * selection.
     */
    if (col + (int) len < MaxCols(screen)) {
	flags |= CHARDRAWN;
    } else {
	len = MaxCols(screen) - col;
    }

    /* If we've marked protected text on the screen, we'll have to
     * check each time we do an erase.
     */
    if (screen->protected_mode != OFF_PROTECT) {
	unsigned n;
	Char *attrs = SCRN_BUF_ATTRS(screen, row) + col;
	int saved_mode = screen->protected_mode;
	Bool done;

	/* disable this branch during recursion */
	screen->protected_mode = OFF_PROTECT;

	do {
	    done = True;
	    for (n = 0; n < len; n++) {
		if (attrs[n] & PROTECTED) {
		    rc = 0;	/* found a protected segment */
		    if (n != 0)
			ClearInLine(xw, row, col, n);
		    while ((n < len)
			   && (attrs[n] & PROTECTED))
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

    if (screen->cursor_state)
	HideCursor();
    screen->do_wrap = 0;

    if (INX2ROW(screen, row) <= screen->max_row) {
	if (!AddToRefresh(screen)) {
	    if (screen->scroll_amt)
		FlushScroll(xw);
	    ClearCurBackground(xw,
			       CursorY(screen, row),
			       CurCursorX(screen, row, col),
			       (unsigned) FontHeight(screen),
			       len * CurFontWidth(screen, row));
	}
    }

    if (len != 0) {
	memset(SCRN_BUF_CHARS(screen, row) + col, ' ', len);
	memset(SCRN_BUF_ATTRS(screen, row) + col, flags, len);

	if_OPT_EXT_COLORS(screen, {
	    memset(SCRN_BUF_FGRND(screen, row) + col,
		   xw->sgr_foreground, len);
	    memset(SCRN_BUF_BGRND(screen, row) + col,
		   xw->cur_background, len);
	});
	if_OPT_ISO_TRADITIONAL_COLORS(screen, {
	    memset(SCRN_BUF_COLOR(screen, row) + col,
		   (int) xtermColorPair(xw), len);
	});
	if_OPT_DEC_CHRSET({
	    memset(SCRN_BUF_CSETS(screen, row) + col,
		   curXtermChrSet(xw, screen->cur_row), len);
	});
	if_OPT_WIDE_CHARS(screen, {
	    int off;
	    for (off = OFF_WIDEC; off < MAX_PTRS; ++off) {
		memset(SCREEN_PTR(screen, row, off) + col, 0, len);
	    }
	});
    }

    return rc;
}

/*
 * Clear the next n characters on the cursor's line, including the cursor's
 * position.
 */
void
ClearRight(XtermWidget xw, int n)
{
    TScreen *screen = &(xw->screen);
    unsigned len = (MaxCols(screen) - screen->cur_col);

    assert(screen->max_col >= 0);
    assert(screen->max_col >= screen->cur_col);

    if (n < 0)			/* the remainder of the line */
	n = MaxCols(screen);
    if (n == 0)			/* default for 'ECH' */
	n = 1;

    if (len > (unsigned) n)
	len = n;

    (void) ClearInLine(xw, screen->cur_row, screen->cur_col, len);

    /* with the right part cleared, we can't be wrapping */
    ScrnClrWrapped(screen, screen->cur_row);
}

/*
 * Clear first part of cursor's line, inclusive.
 */
static void
ClearLeft(XtermWidget xw)
{
    TScreen *screen = &(xw->screen);
    unsigned len = screen->cur_col + 1;
    assert(screen->cur_col >= 0);

    (void) ClearInLine(xw, screen->cur_row, 0, len);
}

/*
 * Erase the cursor's line.
 */
static void
ClearLine(XtermWidget xw)
{
    TScreen *screen = &(xw->screen);
    unsigned len = MaxCols(screen);

    assert(screen->max_col >= 0);
    (void) ClearInLine(xw, screen->cur_row, 0, len);
}

void
ClearScreen(XtermWidget xw)
{
    TScreen *screen = &(xw->screen);
    int top;

    if (screen->cursor_state)
	HideCursor();

    ScrnDisownSelection(xw);
    screen->do_wrap = 0;
    if ((top = INX2ROW(screen, 0)) <= screen->max_row) {
	if (screen->scroll_amt)
	    FlushScroll(xw);
	ClearCurBackground(xw,
			   top * FontHeight(screen) + screen->border,
			   OriginX(screen),
			   (unsigned) ((screen->max_row - top + 1)
				       * FontHeight(screen)),
			   (unsigned) Width(screen));
    }
    ClearBufRows(xw, 0, screen->max_row);
}

/*
 * If we've written protected text DEC-style, and are issuing a non-DEC
 * erase, temporarily reset the protected_mode flag so that the erase will
 * ignore the protected flags.
 */
void
do_erase_line(XtermWidget xw, int param, int mode)
{
    TScreen *screen = &(xw->screen);
    int saved_mode = screen->protected_mode;

    if (saved_mode == DEC_PROTECT
	&& saved_mode != mode)
	screen->protected_mode = OFF_PROTECT;

    switch (param) {
    case -1:			/* DEFAULT */
    case 0:
	ClearRight(xw, -1);
	break;
    case 1:
	ClearLeft(xw);
	break;
    case 2:
	ClearLine(xw);
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
do_erase_display(XtermWidget xw, int param, int mode)
{
    TScreen *screen = &(xw->screen);
    int saved_mode = screen->protected_mode;

    if (saved_mode == DEC_PROTECT
	&& saved_mode != mode)
	screen->protected_mode = OFF_PROTECT;

    switch (param) {
    case -1:			/* DEFAULT */
    case 0:
	if (screen->cur_row == 0
	    && screen->cur_col == 0) {
	    screen->protected_mode = saved_mode;
	    do_erase_display(xw, 2, mode);
	    saved_mode = screen->protected_mode;
	} else
	    ClearBelow(xw);
	break;

    case 1:
	if (screen->cur_row == screen->max_row
	    && screen->cur_col == screen->max_col) {
	    screen->protected_mode = saved_mode;
	    do_erase_display(xw, 2, mode);
	    saved_mode = screen->protected_mode;
	} else
	    ClearAbove(xw);
	break;

    case 2:
	/*
	 * We use 'ClearScreen()' throughout the remainder of the
	 * program for places where we don't care if the characters are
	 * protected or not.  So we modify the logic around this call
	 * on 'ClearScreen()' to handle protected characters.
	 */
	if (screen->protected_mode != OFF_PROTECT) {
	    int row;
	    int rc = 1;
	    unsigned len = MaxCols(screen);

	    assert(screen->max_col >= 0);
	    for (row = 0; row <= screen->max_row; row++)
		rc &= ClearInLine(xw, row, 0, len);
	    if (rc != 0)
		saved_mode = OFF_PROTECT;
	} else {
	    ClearScreen(xw);
	}
	break;

    case 3:
	/* xterm addition - erase saved lines. */
	screen->savedlines = 0;
	ScrollBarDrawThumb(screen->scrollWidget);
	break;
    }
    screen->protected_mode = saved_mode;
}

static void
CopyWait(XtermWidget xw)
{
    TScreen *screen = &(xw->screen);
    XEvent reply;
    XEvent *rep = &reply;

    while (1) {
	XWindowEvent(screen->display, VWindow(screen),
		     ExposureMask, &reply);
	switch (reply.type) {
	case Expose:
	    HandleExposure(xw, &reply);
	    break;
	case NoExpose:
	case GraphicsExpose:
	    if (screen->incopy <= 0) {
		screen->incopy = 1;
		if (screen->scrolls > 0)
		    screen->scrolls--;
	    }
	    if (reply.type == GraphicsExpose)
		HandleExposure(xw, &reply);

	    if ((reply.type == NoExpose) ||
		((XExposeEvent *) rep)->count == 0) {
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
copy_area(XtermWidget xw,
	  int src_x,
	  int src_y,
	  unsigned width,
	  unsigned height,
	  int dest_x,
	  int dest_y)
{
    TScreen *screen = &(xw->screen);

    if (width != 0 && height != 0) {
	/* wait for previous CopyArea to complete unless
	   multiscroll is enabled and active */
	if (screen->incopy && screen->scrolls == 0)
	    CopyWait(xw);
	screen->incopy = -1;

	/* save for translating Expose events */
	screen->copy_src_x = src_x;
	screen->copy_src_y = src_y;
	screen->copy_width = width;
	screen->copy_height = height;
	screen->copy_dest_x = dest_x;
	screen->copy_dest_y = dest_y;

	XCopyArea(screen->display,
		  VWindow(screen), VWindow(screen),
		  NormalGC(screen),
		  src_x, src_y, width, height, dest_x, dest_y);
    }
}

/*
 * use when inserting or deleting characters on the current line
 */
static void
horizontal_copy_area(XtermWidget xw,
		     int firstchar,	/* char pos on screen to start copying at */
		     int nchars,
		     int amount)	/* number of characters to move right */
{
    TScreen *screen = &(xw->screen);
    int src_x = CurCursorX(screen, screen->cur_row, firstchar);
    int src_y = CursorY(screen, screen->cur_row);

    copy_area(xw, src_x, src_y,
	      (unsigned) nchars * CurFontWidth(screen, screen->cur_row),
	      (unsigned) FontHeight(screen),
	      src_x + amount * CurFontWidth(screen, screen->cur_row), src_y);
}

/*
 * use when inserting or deleting lines from the screen
 */
static void
vertical_copy_area(XtermWidget xw,
		   int firstline,	/* line on screen to start copying at */
		   int nlines,
		   int amount)	/* number of lines to move up (neg=down) */
{
    TScreen *screen = &(xw->screen);

    if (nlines > 0) {
	int src_x = OriginX(screen);
	int src_y = firstline * FontHeight(screen) + screen->border;

	copy_area(xw, src_x, src_y,
		  (unsigned) Width(screen),
		  (unsigned) (nlines * FontHeight(screen)),
		  src_x, src_y - amount * FontHeight(screen));
    }
}

/*
 * use when scrolling the entire screen
 */
void
scrolling_copy_area(XtermWidget xw,
		    int firstline,	/* line on screen to start copying at */
		    int nlines,
		    int amount)	/* number of lines to move up (neg=down) */
{

    if (nlines > 0) {
	vertical_copy_area(xw, firstline, nlines, amount);
    }
}

/*
 * Handler for Expose events on the VT widget.
 * Returns 1 iff the area where the cursor was got refreshed.
 */
int
HandleExposure(XtermWidget xw, XEvent * event)
{
    TScreen *screen = &(xw->screen);
    XExposeEvent *reply = (XExposeEvent *) event;

#ifndef NO_ACTIVE_ICON
    if (reply->window == screen->iconVwin.window) {
	WhichVWin(screen) = &screen->iconVwin;
	TRACE(("HandleExposure - icon"));
    } else {
	WhichVWin(screen) = &screen->fullVwin;
	TRACE(("HandleExposure - normal"));
    }
    TRACE((" event %d,%d %dx%d\n",
	   reply->y,
	   reply->x,
	   reply->height,
	   reply->width));
#endif /* NO_ACTIVE_ICON */

    /* if not doing CopyArea or if this is a GraphicsExpose, don't translate */
    if (!screen->incopy || event->type != Expose)
	return handle_translated_exposure(xw, reply->x, reply->y,
					  reply->width,
					  reply->height);
    else {
	/* compute intersection of area being copied with
	   area being exposed. */
	int both_x1 = Max(screen->copy_src_x, reply->x);
	int both_y1 = Max(screen->copy_src_y, reply->y);
	int both_x2 = Min(screen->copy_src_x + screen->copy_width,
			  (unsigned) (reply->x + reply->width));
	int both_y2 = Min(screen->copy_src_y + screen->copy_height,
			  (unsigned) (reply->y + reply->height));
	int value = 0;

	/* was anything copied affected? */
	if (both_x2 > both_x1 && both_y2 > both_y1) {
	    /* do the copied area */
	    value = handle_translated_exposure
		(xw, reply->x + screen->copy_dest_x - screen->copy_src_x,
		 reply->y + screen->copy_dest_y - screen->copy_src_y,
		 reply->width, reply->height);
	}
	/* was anything not copied affected? */
	if (reply->x < both_x1 || reply->y < both_y1
	    || reply->x + reply->width > both_x2
	    || reply->y + reply->height > both_y2)
	    value = handle_translated_exposure(xw, reply->x, reply->y,
					       reply->width, reply->height);

	return value;
    }
}

static void
set_background(XtermWidget xw, int color)
{
    TScreen *screen = &(xw->screen);
    Pixel c = getXtermBackground(xw, xw->flags, color);

    XSetWindowBackground(screen->display, VShellWindow, c);
    XSetWindowBackground(screen->display, VWindow(screen), c);
}

/*
 * Called by the ExposeHandler to do the actual repaint after the coordinates
 * have been translated to allow for any CopyArea in progress.
 * The rectangle passed in is pixel coordinates.
 */
static int
handle_translated_exposure(XtermWidget xw,
			   int rect_x,
			   int rect_y,
			   int rect_width,
			   int rect_height)
{
    TScreen *screen = &(xw->screen);
    int toprow, leftcol, nrows, ncols;
    int x0, x1;
    int y0, y1;
    int result = 0;

    TRACE(("handle_translated_exposure at %d,%d size %dx%d\n",
	   rect_y, rect_x, rect_height, rect_width));

    x0 = (rect_x - OriginX(screen));
    x1 = (x0 + rect_width);

    y0 = (rect_y - OriginY(screen));
    y1 = (y0 + rect_height);

    if (getXtermBackground(xw, xw->flags, xw->cur_background) !=
	xw->core.background_pixel &&
	(x0 < 0 ||
	 y0 < 0 ||
	 x1 > Width(screen) ||
	 y1 > Height(screen))) {
	set_background(xw, -1);
	XClearArea(screen->display, VWindow(screen),
		   rect_x,
		   rect_y,
		   rect_width,
		   rect_height, False);
    }
    toprow = y0 / FontHeight(screen);
    if (toprow < 0)
	toprow = 0;

    leftcol = x0 / CurFontWidth(screen, screen->cur_row);
    if (leftcol < 0)
	leftcol = 0;

    nrows = (y1 - 1) / FontHeight(screen) - toprow + 1;
    ncols = (x1 - 1) / FontWidth(screen) - leftcol + 1;
    toprow -= screen->scrolls;
    if (toprow < 0) {
	nrows += toprow;
	toprow = 0;
    }
    if (toprow + nrows > MaxRows(screen))
	nrows = MaxRows(screen) - toprow;
    if (leftcol + ncols > MaxCols(screen))
	ncols = MaxCols(screen) - leftcol;

    if (nrows > 0 && ncols > 0) {
	ScrnRefresh(xw, toprow, leftcol, nrows, ncols, True);
	if (waiting_for_initial_map) {
	    first_map_occurred();
	}
	if (screen->cur_row >= toprow &&
	    screen->cur_row < toprow + nrows &&
	    screen->cur_col >= leftcol &&
	    screen->cur_col < leftcol + ncols) {
	    result = 1;
	}

    }
    TRACE(("...handle_translated_exposure %d\n", result));
    return (result);
}

/***====================================================================***/

void
GetColors(XtermWidget xw, ScrnColors * pColors)
{
    TScreen *screen = &xw->screen;
    int n;

    pColors->which = 0;
    for (n = 0; n < NCOLORS; ++n) {
	SET_COLOR_VALUE(pColors, n, T_COLOR(screen, n));
    }
}

void
ChangeColors(XtermWidget xw, ScrnColors * pNew)
{
    Bool repaint = False;
    TScreen *screen = &xw->screen;

    TRACE(("ChangeColors\n"));

    if (COLOR_DEFINED(pNew, TEXT_CURSOR)) {
	T_COLOR(screen, TEXT_CURSOR) = COLOR_VALUE(pNew, TEXT_CURSOR);
	TRACE(("... TEXT_CURSOR: %#lx\n", T_COLOR(screen, TEXT_CURSOR)));
	/* no repaint needed */
    } else if ((T_COLOR(screen, TEXT_CURSOR) == T_COLOR(screen, TEXT_FG)) &&
	       (COLOR_DEFINED(pNew, TEXT_FG))) {
	T_COLOR(screen, TEXT_CURSOR) = COLOR_VALUE(pNew, TEXT_FG);
	TRACE(("... TEXT_CURSOR: %#lx\n", T_COLOR(screen, TEXT_CURSOR)));
	repaint = screen->Vshow;
    }

    if (COLOR_DEFINED(pNew, TEXT_FG)) {
	Pixel fg = COLOR_VALUE(pNew, TEXT_FG);
	T_COLOR(screen, TEXT_FG) = fg;
	TRACE(("... TEXT_FG: %#lx\n", T_COLOR(screen, TEXT_FG)));
	if (screen->Vshow) {
	    XSetForeground(screen->display, NormalGC(screen), fg);
	    XSetBackground(screen->display, ReverseGC(screen), fg);
	    XSetForeground(screen->display, NormalBoldGC(screen), fg);
	    XSetBackground(screen->display, ReverseBoldGC(screen), fg);
	    repaint = True;
	}
    }

    if (COLOR_DEFINED(pNew, TEXT_BG)) {
	Pixel bg = COLOR_VALUE(pNew, TEXT_BG);
	T_COLOR(screen, TEXT_BG) = bg;
	TRACE(("... TEXT_BG: %#lx\n", T_COLOR(screen, TEXT_BG)));
	if (screen->Vshow) {
	    XSetBackground(screen->display, NormalGC(screen), bg);
	    XSetForeground(screen->display, ReverseGC(screen), bg);
	    XSetBackground(screen->display, NormalBoldGC(screen), bg);
	    XSetForeground(screen->display, ReverseBoldGC(screen), bg);
	    set_background(xw, -1);
	    repaint = True;
	}
    }
#if OPT_HIGHLIGHT_COLOR
    if (COLOR_DEFINED(pNew, HIGHLIGHT_BG)) {
	T_COLOR(screen, HIGHLIGHT_BG) = COLOR_VALUE(pNew, HIGHLIGHT_BG);
	TRACE(("... HIGHLIGHT_BG: %#lx\n", T_COLOR(screen, HIGHLIGHT_BG)));
	repaint = screen->Vshow;
    }
#endif

    if (COLOR_DEFINED(pNew, MOUSE_FG) || (COLOR_DEFINED(pNew, MOUSE_BG))) {
	if (COLOR_DEFINED(pNew, MOUSE_FG)) {
	    T_COLOR(screen, MOUSE_FG) = COLOR_VALUE(pNew, MOUSE_FG);
	    TRACE(("... MOUSE_FG: %#lx\n", T_COLOR(screen, MOUSE_FG)));
	}
	if (COLOR_DEFINED(pNew, MOUSE_BG)) {
	    T_COLOR(screen, MOUSE_BG) = COLOR_VALUE(pNew, MOUSE_BG);
	    TRACE(("... MOUSE_BG: %#lx\n", T_COLOR(screen, MOUSE_BG)));
	}

	if (screen->Vshow) {
	    recolor_cursor(screen,
			   screen->pointer_cursor,
			   T_COLOR(screen, MOUSE_FG),
			   T_COLOR(screen, MOUSE_BG));
	    XDefineCursor(screen->display, VWindow(screen),
			  screen->pointer_cursor);
	}
#if OPT_TEK4014
	if (TEK4014_SHOWN(xw)) {
	    TekScreen *tekscr = &(tekWidget->screen);
	    Window tekwin = TWindow(tekscr);
	    if (tekwin) {
		recolor_cursor(screen,
			       tekscr->arrow,
			       T_COLOR(screen, MOUSE_FG),
			       T_COLOR(screen, MOUSE_BG));
		XDefineCursor(screen->display, tekwin, tekscr->arrow);
	    }
	}
#endif
	/* no repaint needed */
    }

    if (COLOR_DEFINED(pNew, TEXT_FG) ||
	COLOR_DEFINED(pNew, TEXT_BG) ||
	COLOR_DEFINED(pNew, TEXT_CURSOR)) {
	set_cursor_gcs(xw);
    }
#if OPT_TEK4014
    if (COLOR_DEFINED(pNew, TEK_FG) ||
	COLOR_DEFINED(pNew, TEK_BG)) {
	ChangeTekColors(tekWidget, screen, pNew);
	if (TEK4014_SHOWN(xw)) {
	    TekRepaint(tekWidget);
	}
    } else if (COLOR_DEFINED(pNew, TEK_CURSOR)) {
	ChangeTekColors(tekWidget, screen, pNew);
    }
#endif
    if (repaint)
	xtermRepaint(xw);
}

void
xtermClear(XtermWidget xw)
{
    TScreen *screen = &xw->screen;

    TRACE(("xtermClear\n"));
    XClearWindow(screen->display, VWindow(screen));
}

void
xtermRepaint(XtermWidget xw)
{
    TScreen *screen = &xw->screen;

    TRACE(("xtermRepaint\n"));
    xtermClear(xw);
    ScrnRefresh(xw, 0, 0, MaxRows(screen), MaxCols(screen), True);
}

/***====================================================================***/

void
ReverseVideo(XtermWidget xw)
{
    TScreen *screen = &xw->screen;
    GC tmpGC;
    Pixel tmp;

    TRACE(("ReverseVideo\n"));

    /*
     * Swap SGR foreground and background colors.  By convention, these are
     * the colors assigned to "black" (SGR #0) and "white" (SGR #7).  Also,
     * SGR #8 and SGR #15 are the bold (or bright) versions of SGR #0 and
     * #7, respectively.
     *
     * We don't swap colors that happen to match the screen's foreground
     * and background because that tends to produce bizarre effects.
     */
    if_OPT_ISO_COLORS(screen, {
	ColorRes tmp2;
	EXCHANGE(screen->Acolors[0], screen->Acolors[7], tmp2);
	EXCHANGE(screen->Acolors[8], screen->Acolors[15], tmp2);
    });

    tmp = T_COLOR(screen, TEXT_BG);
    if (T_COLOR(screen, TEXT_CURSOR) == T_COLOR(screen, TEXT_FG))
	T_COLOR(screen, TEXT_CURSOR) = tmp;
    T_COLOR(screen, TEXT_BG) = T_COLOR(screen, TEXT_FG);
    T_COLOR(screen, TEXT_FG) = tmp;

    EXCHANGE(T_COLOR(screen, MOUSE_FG), T_COLOR(screen, MOUSE_BG), tmp);
    EXCHANGE(NormalGC(screen), ReverseGC(screen), tmpGC);
    EXCHANGE(NormalBoldGC(screen), ReverseBoldGC(screen), tmpGC);
#ifndef NO_ACTIVE_ICON
    tmpGC = screen->iconVwin.normalGC;
    screen->iconVwin.normalGC = screen->iconVwin.reverseGC;
    screen->iconVwin.reverseGC = tmpGC;

    tmpGC = screen->iconVwin.normalboldGC;
    screen->iconVwin.normalboldGC = screen->iconVwin.reverseboldGC;
    screen->iconVwin.reverseboldGC = tmpGC;
#endif /* NO_ACTIVE_ICON */

    xw->misc.re_verse = !xw->misc.re_verse;

    if (XtIsRealized((Widget) xw)) {
	if (screen->Vshow) {
	    recolor_cursor(screen,
			   screen->pointer_cursor,
			   T_COLOR(screen, MOUSE_FG),
			   T_COLOR(screen, MOUSE_BG));
	    XDefineCursor(screen->display, VWindow(screen), screen->pointer_cursor);
	}
    }
#if OPT_TEK4014
    if (TEK4014_SHOWN(xw)) {
	TekScreen *tekscr = &(tekWidget->screen);
	Window tekwin = TWindow(tekscr);
	recolor_cursor(screen,
		       tekscr->arrow,
		       T_COLOR(screen, MOUSE_FG),
		       T_COLOR(screen, MOUSE_BG));
	XDefineCursor(screen->display, tekwin, tekscr->arrow);
    }
#endif

    if (screen->scrollWidget)
	ScrollBarReverseVideo(screen->scrollWidget);

    if (XtIsRealized((Widget) xw)) {
	set_background(xw, -1);
    }
#if OPT_TEK4014
    TekReverseVideo(tekWidget);
#endif
    if (XtIsRealized((Widget) xw)) {
	xtermRepaint(xw);
    }
#if OPT_TEK4014
    if (TEK4014_SHOWN(xw)) {
	TekRepaint(tekWidget);
    }
#endif
    ReverseOldColors();
    set_cursor_gcs(xw);
    update_reversevideo();
    TRACE(("...ReverseVideo\n"));
}

void
recolor_cursor(TScreen * screen,
	       Cursor cursor,	/* X cursor ID to set */
	       unsigned long fg,	/* pixel indexes to look up */
	       unsigned long bg)	/* pixel indexes to look up */
{
    Display *dpy = screen->display;
    XColor colordefs[2];	/* 0 is foreground, 1 is background */

    colordefs[0].pixel = fg;
    colordefs[1].pixel = bg;
    XQueryColors(dpy, DefaultColormap(dpy, DefaultScreen(dpy)),
		 colordefs, 2);
    XRecolorCursor(dpy, cursor, colordefs, colordefs + 1);
    return;
}

#if OPT_RENDERFONT
static XftColor *
getXftColor(XtermWidget xw, Pixel pixel)
{
#define CACHE_SIZE  4
    static struct {
	XftColor color;
	int use;
    } cache[CACHE_SIZE];
    static int use;
    int i;
    int oldest, oldestuse;
    XColor color;

    oldestuse = 0x7fffffff;
    oldest = 0;
    for (i = 0; i < CACHE_SIZE; i++) {
	if (cache[i].use) {
	    if (cache[i].color.pixel == pixel) {
		cache[i].use = ++use;
		return &cache[i].color;
	    }
	}
	if (cache[i].use < oldestuse) {
	    oldestuse = cache[i].use;
	    oldest = i;
	}
    }
    i = oldest;
    color.pixel = pixel;
    XQueryColor(xw->screen.display, xw->core.colormap, &color);
    cache[i].color.color.red = color.red;
    cache[i].color.color.green = color.green;
    cache[i].color.color.blue = color.blue;
    cache[i].color.color.alpha = 0xffff;
    cache[i].color.pixel = pixel;
    cache[i].use = ++use;
    return &cache[i].color;
}

/*
 * The cell-width is related to, but not the same as the wide-character width.
 * We will only get useful values from wcwidth() for codes above 255.
 * Otherwise, interpret according to internal data.
 */
static int
xtermCellWidth(XtermWidget xw, wchar_t ch)
{
    int result = 0;

    (void) xw;
    if (ch == 0 || ch == 127) {
	result = 0;
    } else if (ch < 256) {
#if OPT_C1_PRINT
	if (ch >= 128 && ch < 160) {
	    result = (xw->screen.c1_printable ? 1 : 0);
	} else
#endif

	    result = 1;		/* 1..31 are line-drawing characters */
    } else {
	result = my_wcwidth(ch);
    }
    return result;
}

/*
 * fontconfig/Xft combination prior to 2.2 has a problem with
 * CJK truetype 'double-width' (bi-width/monospace) fonts leading
 * to the 's p a c e d o u t' rendering. Consequently, we can't
 * rely on XftDrawString8/16  when one of  those fonts is used.
 * Instead, we need to roll out our own using XftDrawCharSpec.
 * A patch in the same spirit (but in a rather different form)
 * was applied to gnome vte and gtk2 port of vim.
 * See http://bugzilla.mozilla.org/show_bug.cgi?id=196312
 */
static int
xtermXftDrawString(XtermWidget xw,
		   unsigned flags GCC_UNUSED,
		   XftColor * color,
		   XftFont * font,
		   int x,
		   int y,
		   PAIRED_CHARS(Char * text, Char * text2),
		   int len,
		   Bool really)
{
    TScreen *screen = &(xw->screen);
    int ncells = 0;

    if (len != 0) {
#if OPT_RENDERWIDE
	static XftCharSpec *sbuf;
	static int slen = 0;

	XftFont *wfont;
	int n;
	XftFont *lastFont = 0;
	XftFont *currFont = 0;
	int start = 0;
	int charWidth;
	int fontnum = screen->menu_font_number;
	int fwidth = FontWidth(screen);

#if OPT_ISO_COLORS
	if ((flags & UNDERLINE)
	    && screen->italicULMode
	    && screen->renderWideItal[fontnum]) {
	    wfont = screen->renderWideItal[fontnum];
	} else
#endif
	    if ((flags & BOLDATTR(screen))
		&& screen->renderWideBold[fontnum]) {
	    wfont = screen->renderWideBold[fontnum];
	} else {
	    wfont = screen->renderWideNorm[fontnum];
	}

	if ((int) slen < len) {
	    slen = (len + 1) * 2;
	    sbuf = (XftCharSpec *) XtRealloc((char *) sbuf,
					     slen * sizeof(XftCharSpec));
	}

	for (n = 0; n < len; n++) {
	    FcChar32 wc = *text++;

	    if (text2)
		wc |= (*text2++ << 8);

	    sbuf[n].ucs4 = wc;
	    sbuf[n].x = x + fwidth * ncells;
	    sbuf[n].y = y;

	    charWidth = xtermCellWidth(xw, wc);
	    currFont = (charWidth == 2 && wfont != 0) ? wfont : font;
	    ncells += charWidth;

	    if (lastFont != currFont) {
		if ((lastFont != 0) && really) {
		    XftDrawCharSpec(screen->renderDraw,
				    color,
				    lastFont,
				    sbuf + start,
				    n - start);
		}
		start = n;
		lastFont = currFont;
	    }
	}
	if ((n != start) && really) {
	    XftDrawCharSpec(screen->renderDraw,
			    color,
			    lastFont,
			    sbuf + start,
			    n - start);
	}
#else /* !OPT_RENDERWIDE */
	PAIRED_CHARS((void) text, (void) text2);
	if (really) {
	    XftDrawString8(screen->renderDraw,
			   color,
			   font,
			   x, y, (unsigned char *) text, len);
	}
	ncells = len;
#endif
    }
    return ncells;
}
#endif /* OPT_RENDERFONT */

#define DrawX(col) x + (col * (font_width))
#define DrawSegment(first,last) (void)drawXtermText(xw, flags|NOTRANSLATION, gc, DrawX(first), y, chrset, PAIRED_CHARS(text+first, text2+first), (unsigned)(last - first), on_wide)

#if OPT_WIDE_CHARS
/*
 * Actually this should be called "groff_workaround()" - for the places where
 * groff stomps on compatibility.  Still, if enough people get used to it,
 * this might someday become a quasi-standard.
 */
static int
ucs_workaround(XtermWidget xw,
	       unsigned ch,
	       unsigned flags,
	       GC gc,
	       int x,
	       int y,
	       int chrset,
	       int on_wide)
{
    TScreen *screen = &(xw->screen);
    int fixed = False;

    if (screen->wide_chars && screen->utf8_mode && ch > 256) {
	switch (ch) {
	case 0x2010:		/* groff "-" */
	case 0x2011:
	case 0x2012:
	case 0x2013:
	case 0x2014:
	case 0x2015:
	case 0x2212:		/* groff "\-" */
	    ch = '-';
	    fixed = True;
	    break;
	case 0x2018:		/* groff "`" */
	    ch = '`';
	    fixed = True;
	    break;
	case 0x2019:		/* groff ' */
	    ch = '\'';
	    fixed = True;
	    break;
	case 0x201C:		/* groff lq */
	case 0x201D:		/* groff rq */
	    ch = '"';
	    fixed = True;
	    break;
	}
	if (fixed) {
	    Char text[2];
	    Char text2[2];

	    text[0] = ch;
	    text2[0] = 0;
	    drawXtermText(xw,
			  flags,
			  gc,
			  x,
			  y,
			  chrset,
			  PAIRED_CHARS(text, text2),
			  1,
			  on_wide);
	}
    }
    return fixed;
}
#endif

#if OPT_CLIP_BOLD
/*
 * This special case is a couple of percent slower, but avoids a lot of pixel
 * trash in rxcurses' hanoi.cmd demo (e.g., 10x20 font).
 */
#define beginClipping(screen,gc,pwidth,plength) \
	    if (pwidth > 2) { \
		XRectangle clip; \
		int clip_x = x; \
		int clip_y = y - FontHeight(screen) + FontDescent(screen); \
		clip.x = 0; \
		clip.y = 0; \
		clip.height = FontHeight(screen); \
		clip.width = pwidth * plength; \
		XSetClipRectangles(screen->display, gc, \
				   clip_x, clip_y, \
				   &clip, 1, Unsorted); \
	    }
#define endClipping(screen,gc) \
	    XSetClipMask(screen->display, gc, None)
#else
#define beginClipping(screen,gc,pwidth,plength)		/* nothing */
#define endClipping(screen,gc)	/* nothing */
#endif /* OPT_CLIP_BOLD */

/*
 * Draws text with the specified combination of bold/underline.  The return
 * value is the updated x position.
 */
int
drawXtermText(XtermWidget xw,
	      unsigned flags,
	      GC gc,
	      int x,
	      int y,
	      int chrset,
	      PAIRED_CHARS(Char * text, Char * text2),
	      Cardinal len,
	      int on_wide)
{
    TScreen *screen = &(xw->screen);
    int real_length = len;
    int underline_len;
    /* Intended width of the font to draw (as opposed to the actual width of
       the X font, and the width of the default font) */
    int font_width = ((flags & DOUBLEWFONT) ? 2 : 1) * screen->fnt_wide;
    Bool did_ul = False;

#if OPT_WIDE_CHARS
    if (text == 0)
	return 0;
    /*
     * It's simpler to pass in a null pointer for text2 in places where
     * we only use codes through 255.  Fix text2 here so we can increment
     * it, etc.
     */
    if (text2 == 0) {
	static Char *dbuf;
	static unsigned dlen;
	if (dlen <= len) {
	    dlen = (len + 1) * 2;
	    dbuf = (Char *) XtRealloc((char *) dbuf, dlen);
	    memset(dbuf, 0, dlen);
	}
	text2 = dbuf;
    }
#endif
#if OPT_DEC_CHRSET
    if (CSET_DOUBLE(chrset)) {
	/* We could try drawing double-size characters in the icon, but
	 * given that the icon font is usually nil or nil2, there
	 * doesn't seem to be much point.
	 */
	GC gc2 = ((!IsIcon(screen) && screen->font_doublesize)
		  ? xterm_DoubleGC((unsigned) chrset, flags, gc)
		  : 0);

	TRACE(("DRAWTEXT%c[%4d,%4d] (%d) %d:%.*s\n",
	       screen->cursor_state == OFF ? ' ' : '*',
	       y, x, chrset, len, (int) len, text));

	if (gc2 != 0) {		/* draw actual double-sized characters */
	    /* Update the last-used cache of double-sized fonts */
	    int inx = xterm_Double_index((unsigned) chrset, flags);
	    XFontStruct *fs = screen->double_fonts[inx].fs;
	    XRectangle rect, *rp = &rect;
	    int nr = 1;
	    int adjust;

	    font_width *= 2;
	    flags |= DOUBLEWFONT;

	    rect.x = 0;
	    rect.y = 0;
	    rect.width = len * font_width;
	    rect.height = FontHeight(screen);

	    switch (chrset) {
	    case CSET_DHL_TOP:
		rect.y = -(rect.height / 2);
		y -= rect.y;
		flags |= DOUBLEHFONT;
		break;
	    case CSET_DHL_BOT:
		rect.y = (rect.height / 2);
		y -= rect.y;
		flags |= DOUBLEHFONT;
		break;
	    default:
		nr = 0;
		break;
	    }

	    /*
	     * Though it is the right "size", a given bold font may
	     * be shifted up by a pixel or two.  Shift it back into
	     * the clipping rectangle.
	     */
	    if (nr != 0) {
		adjust = fs->ascent
		    + fs->descent
		    - (2 * FontHeight(screen));
		rect.y -= adjust;
		y += adjust;
	    }

	    if (nr)
		XSetClipRectangles(screen->display, gc2,
				   x, y, rp, nr, YXBanded);
	    else
		XSetClipMask(screen->display, gc2, None);

	    /* Call ourselves recursively with the new gc */

	    /*
	     * If we're trying to use proportional font, or if the
	     * font server didn't give us what we asked for wrt
	     * width, position each character independently.
	     */
	    if (screen->fnt_prop
		|| (fs->min_bounds.width != fs->max_bounds.width)
		|| (fs->min_bounds.width != 2 * FontWidth(screen))) {
		/* It is hard to fall-through to the main
		   branch: in a lot of places the check
		   for the cached font info is for
		   normal/bold fonts only. */
		while (len--) {
		    x = drawXtermText(xw, flags, gc2,
				      x, y, 0,
				      PAIRED_CHARS(text++, text2++),
				      1, on_wide);
		    x += FontWidth(screen);
		}
	    } else {
		x = drawXtermText(xw, flags, gc2,
				  x, y, 0,
				  PAIRED_CHARS(text, text2),
				  len, on_wide);
		x += len * FontWidth(screen);
	    }

	    TRACE(("drawtext [%4d,%4d]\n", y, x));
	} else {		/* simulate double-sized characters */
#if OPT_WIDE_CHARS
	    Char *wide = 0;
#endif
	    unsigned need = 2 * len;
	    Char *temp = TypeMallocN(Char, need);
	    unsigned n = 0;
	    if_OPT_WIDE_CHARS(screen, {
		wide = TypeMallocN(Char, need);
	    });
	    while (len--) {
		if_OPT_WIDE_CHARS(screen, {
		    wide[n] = *text2++;
		    wide[n + 1] = 0;
		});
		temp[n++] = *text++;
		temp[n++] = ' ';
	    }
	    x = drawXtermText(xw,
			      flags,
			      gc,
			      x, y,
			      0,
			      PAIRED_CHARS(temp, wide),
			      n,
			      on_wide);
	    free(temp);
	    if_OPT_WIDE_CHARS(screen, {
		free(wide);
	    });
	}
	return x;
    }
#endif
#if OPT_RENDERFONT
    if (UsingRenderFont(xw)) {
	Display *dpy = screen->display;
	XftFont *font;
	XGCValues values;
	int fontnum = screen->menu_font_number;
	int ncells;

	if (!screen->renderDraw) {
	    int scr;
	    Drawable draw = VWindow(screen);
	    Visual *visual;

	    scr = DefaultScreen(dpy);
	    visual = DefaultVisual(dpy, scr);
	    screen->renderDraw = XftDrawCreate(dpy, draw, visual,
					       DefaultColormap(dpy, scr));
	}
#if OPT_ISO_COLORS
	if ((flags & UNDERLINE)
	    && screen->italicULMode
	    && screen->renderFontItal[fontnum]) {
	    font = screen->renderFontItal[fontnum];
	    did_ul = True;
	} else
#endif
	    if ((flags & BOLDATTR(screen))
		&& screen->renderFontBold[fontnum]) {
	    font = screen->renderFontBold[fontnum];
	} else {
	    font = screen->renderFontNorm[fontnum];
	}
	XGetGCValues(dpy, gc, GCForeground | GCBackground, &values);

	if (!(flags & NOBACKGROUND)) {
	    XftColor *bg_color = getXftColor(xw, values.background);
	    ncells = xtermXftDrawString(xw, flags,
					bg_color,
					font, x, y,
					PAIRED_CHARS(text, text2),
					len,
					False);
	    XftDrawRect(screen->renderDraw,
			bg_color,
			x, y,
			ncells * FontWidth(screen),
			(unsigned) FontHeight(screen));
	}

	y += font->ascent;
#if OPT_BOX_CHARS
	if (!screen->force_box_chars) {
	    /* adding code to substitute simulated line-drawing characters */
	    int last, first = 0;
	    Dimension old_wide, old_high = 0;
	    int curX = x;

	    for (last = 0; last < (int) len; last++) {
		unsigned ch = text[last];

		/*
		 * If we're reading UTF-8 from the client, we may have a
		 * line-drawing character.  Translate it back to our box-code
		 * if it is really a line-drawing character (since the
		 * fonts used by Xft generally do not have correct glyphs),
		 * or if Xft can tell us that the glyph is really missing.
		 */
		if_OPT_WIDE_CHARS(screen, {
		    unsigned full = (ch | (text2[last] << 8));
		    unsigned part = ucs2dec(full);
		    if (xtermIsDecGraphic(part) &&
			(xtermIsLineDrawing(part)
			 || xtermXftMissing(xw, font, full)))
			ch = part;
		    else
			ch = full;
		});
		/*
		 * If we have one of our box-codes, draw it directly.
		 */
		if (xtermIsDecGraphic(ch)) {
		    /* line drawing character time */
		    if (last > first) {
			ncells = xtermXftDrawString(xw, flags,
						    getXftColor(xw, values.foreground),
						    font, curX, y,
						    PAIRED_CHARS(text + first,
								 text2 + first),
						    last - first,
						    True);
			curX += ncells * FontWidth(screen);
		    }
		    old_wide = screen->fnt_wide;
		    old_high = screen->fnt_high;
		    screen->fnt_wide = FontWidth(screen);
		    screen->fnt_high = FontHeight(screen);
		    xtermDrawBoxChar(xw, ch, flags, gc,
				     curX, y - FontAscent(screen));
		    curX += FontWidth(screen);
		    screen->fnt_wide = old_wide;
		    screen->fnt_high = old_high;
		    first = last + 1;
		}
	    }
	    if (last > first) {
		xtermXftDrawString(xw, flags,
				   getXftColor(xw, values.foreground),
				   font, curX, y,
				   PAIRED_CHARS(text + first, text2 + first),
				   last - first,
				   True);
	    }
	} else
#endif /* OPT_BOX_CHARS */
	{
	    xtermXftDrawString(xw, flags,
			       getXftColor(xw, values.foreground),
			       font, x, y,
			       PAIRED_CHARS(text, text2),
			       (int) len,
			       True);
	}

	if ((flags & UNDERLINE) && screen->underline && !did_ul) {
	    if (FontDescent(screen) > 1)
		y++;
	    XDrawLine(screen->display, VWindow(screen), gc,
		      x, y,
		      x + (int) len * FontWidth(screen) - 1,
		      y);
	}
	return x + len * FontWidth(screen);
    }
#endif /* OPT_RENDERFONT */
    /*
     * If we're asked to display a proportional font, do this with a fixed
     * pitch.  Yes, it's ugly.  But we cannot distinguish the use of xterm
     * as a dumb terminal vs its use as in fullscreen programs such as vi.
     * Hint: do not try to use a proportional font in the icon.
     */
    if (!IsIcon(screen) && !(flags & CHARBYCHAR) && screen->fnt_prop) {
	int adj, width;
	GC fillGC = gc;		/* might be cursorGC */
	XFontStruct *fs = ((flags & BOLDATTR(screen))
			   ? BoldFont(screen)
			   : NormalFont(screen));

#define GC_PAIRS(a,b) \
	if (gc == a) fillGC = b; \
	if (gc == b) fillGC = a

	/*
	 * Fill the area where we'll write the characters, otherwise
	 * we'll get gaps between them.  The cursor is a special case,
	 * because the XFillRectangle call only uses the foreground,
	 * while we've set the cursor color in the background.  So we
	 * need a special GC for that.
	 */
	if (gc == screen->cursorGC
	    || gc == screen->reversecursorGC)
	    fillGC = screen->fillCursorGC;
	GC_PAIRS(NormalGC(screen), ReverseGC(screen));
	GC_PAIRS(NormalBoldGC(screen), ReverseBoldGC(screen));

	if (!(flags & NOBACKGROUND))
	    XFillRectangle(screen->display, VWindow(screen), fillGC,
			   x, y,
			   len * FontWidth(screen),
			   (unsigned) FontHeight(screen));

	while (len--) {
	    width = XTextWidth(fs, (char *) text, 1);
	    adj = (FontWidth(screen) - width) / 2;
	    (void) drawXtermText(xw, flags | NOBACKGROUND | CHARBYCHAR,
				 gc, x + adj, y, chrset,
				 PAIRED_CHARS(text++, text2++), 1, on_wide);
	    x += FontWidth(screen);
	}
	return x;
    }
#if OPT_BOX_CHARS
    /* If the font is incomplete, draw some substitutions */
    if (!IsIcon(screen)
	&& !(flags & NOTRANSLATION)
	&& (!screen->fnt_boxes || screen->force_box_chars)) {
	/* Fill in missing box-characters.
	   Find regions without missing characters, and draw
	   them calling ourselves recursively.  Draw missing
	   characters via xtermDrawBoxChar(). */
	XFontStruct *font = ((flags & BOLD)
			     ? BoldFont(screen)
			     : NormalFont(screen));
	int last, first = 0;
	for (last = 0; last < (int) len; last++) {
	    unsigned ch = text[last];
	    Bool isMissing;
#if OPT_WIDE_CHARS
	    if (text2 != 0)
		ch |= (text2[last] << 8);
	    isMissing = (ch != HIDDEN_CHAR)
		&& (xtermMissingChar(xw, ch,
				     ((on_wide || iswide((int) ch))
				      && screen->fnts[fWide])
				     ? screen->fnts[fWide]
				     : font));
#else
	    isMissing = xtermMissingChar(xw, ch, font);
#endif
	    if (isMissing) {
		if (last > first)
		    DrawSegment(first, last);
#if OPT_WIDE_CHARS
		if (!ucs_workaround(xw, ch, flags, gc, DrawX(last), y,
				    chrset, on_wide))
#endif
		    xtermDrawBoxChar(xw, ch, flags, gc, DrawX(last), y);
		first = last + 1;
	    }
	}
	if (last <= first) {
	    return x + real_length * FontWidth(screen);
	}
	text += first;
#if OPT_WIDE_CHARS
	text2 += first;
#endif
	len = last - first;
	flags |= NOTRANSLATION;
	if (DrawX(first) != x) {
	    return drawXtermText(xw,
				 flags,
				 gc,
				 DrawX(first),
				 y,
				 chrset,
				 PAIRED_CHARS(text, text2),
				 len,
				 on_wide);
	}
    }
#endif /* OPT_BOX_CHARS */
    /*
     * Behave as if the font has (maybe Unicode-replacements for) drawing
     * characters in the range 1-31 (either we were not asked to ignore them,
     * or the caller made sure that there is none).  The only translation we do
     * in this branch is the removal of HIDDEN_CHAR (for the wide-char case).
     */
    TRACE(("drawtext%c[%4d,%4d] (%d) %d:%s\n",
	   screen->cursor_state == OFF ? ' ' : '*',
	   y, x, chrset, len,
	   visibleChars(PAIRED_CHARS(text, text2), len)));
    y += FontAscent(screen);

#if OPT_WIDE_CHARS
    if (screen->wide_chars || screen->unicode_font) {
	int ascent_adjust = 0;
	int n;
	unsigned ch = text[0] | (text2[0] << 8);
	int wideness = (!IsIcon(screen)
			&& ((on_wide || iswide((int) ch) != 0)
			    && (screen->fnts[fWide] != NULL)));
	unsigned char *endtext = text + len;
	if (screen->draw_len < len) {
	    screen->draw_len = (len + 1) * 2;
	    screen->draw_buf = (XChar2b *) XtRealloc((char *) screen->draw_buf,
						     screen->draw_len *
						     sizeof(*screen->draw_buf));
	}
	for (n = 0; n < (int) len; n++) {
	    screen->draw_buf[n].byte2 = *text;
	    screen->draw_buf[n].byte1 = *text2;
#if OPT_MINI_LUIT
#define UCS2SBUF(n,value)	screen->draw_buf[n].byte2 = (value & 0xff);\
	    			screen->draw_buf[n].byte1 = (value >> 8)
#define Map2Sbuf(n,from,to) (*text == from) { UCS2SBUF(n,to); }
	    if (screen->latin9_mode && !screen->utf8_mode && *text2 == 0) {

		/* see http://www.cs.tut.fi/~jkorpela/latin9.html */
		/* *INDENT-OFF* */
		if Map2Sbuf(n, 0xa4, 0x20ac)
		else if Map2Sbuf(n, 0xa6, 0x0160)
		else if Map2Sbuf(n, 0xa8, 0x0161)
		else if Map2Sbuf(n, 0xb4, 0x017d)
		else if Map2Sbuf(n, 0xb8, 0x017e)
		else if Map2Sbuf(n, 0xbc, 0x0152)
		else if Map2Sbuf(n, 0xbd, 0x0153)
		else if Map2Sbuf(n, 0xbe, 0x0178)
		/* *INDENT-ON* */

	    }
	    if (screen->unicode_font
		&& *text2 == 0
		&& (*text == DEL || *text < 0x20)) {
		int ni = dec2ucs((unsigned) ((*text == DEL) ? 0 : *text));
		UCS2SBUF(n, ni);
	    }
#endif /* OPT_MINI_LUIT */
	    text++;
	    text2++;
	    if (wideness) {
		/* filter out those pesky fake characters. */
		while (text < endtext
		       && *text == HIDDEN_HI
		       && *text2 == HIDDEN_LO) {
		    text++;
		    text2++;
		    len--;
		}
	    }
	}
	/* This is probably wrong. But it works. */
	underline_len = len;

	/* Set the drawing font */
	if (flags & (DOUBLEHFONT | DOUBLEWFONT)) {
	    ;			/* Do nothing: font is already set */
	} else if (wideness
		   && (screen->fnts[fWide]->fid || screen->fnts[fWBold]->fid)) {
	    underline_len = real_length = len * 2;
	    if ((flags & BOLDATTR(screen)) != 0
		&& screen->fnts[fWBold]->fid) {
		XSetFont(screen->display, gc, screen->fnts[fWBold]->fid);
		ascent_adjust = (screen->fnts[fWBold]->ascent
				 - NormalFont(screen)->ascent);
	    } else {
		XSetFont(screen->display, gc, screen->fnts[fWide]->fid);
		ascent_adjust = (screen->fnts[fWide]->ascent
				 - NormalFont(screen)->ascent);
	    }
	    /* fix ascent */
	} else if ((flags & BOLDATTR(screen)) != 0
		   && BoldFont(screen)->fid) {
	    XSetFont(screen->display, gc, BoldFont(screen)->fid);
	} else {
	    XSetFont(screen->display, gc, NormalFont(screen)->fid);
	}

	if (flags & NOBACKGROUND)
	    XDrawString16(screen->display,
			  VWindow(screen), gc,
			  x, y + ascent_adjust,
			  screen->draw_buf, n);
	else
	    XDrawImageString16(screen->display,
			       VWindow(screen), gc,
			       x, y + ascent_adjust,
			       screen->draw_buf, n);

	if ((flags & BOLDATTR(screen)) && screen->enbolden) {
	    beginClipping(screen, gc, font_width, len);
	    XDrawString16(screen->display, VWindow(screen), gc,
			  x + 1,
			  y + ascent_adjust,
			  screen->draw_buf, n);
	    endClipping(screen, gc);
	}

    } else
#endif /* OPT_WIDE_CHARS */
    {
	int length = len;	/* X should have used unsigned */

	if (flags & NOBACKGROUND)
	    XDrawString(screen->display, VWindow(screen), gc,
			x, y, (char *) text, length);
	else
	    XDrawImageString(screen->display, VWindow(screen), gc,
			     x, y, (char *) text, length);
	underline_len = length;
	if ((flags & BOLDATTR(screen)) && screen->enbolden) {
	    beginClipping(screen, gc, font_width, length);
	    XDrawString(screen->display, VWindow(screen), gc,
			x + 1, y, (char *) text, length);
	    endClipping(screen, gc);
	}
    }

    if ((flags & UNDERLINE) && screen->underline && !did_ul) {
	if (FontDescent(screen) > 1)
	    y++;
	XDrawLine(screen->display, VWindow(screen), gc,
		  x, y, x + underline_len * font_width - 1, y);
    }

    return x + real_length * FontWidth(screen);
}

/* set up size hints for window manager; min 1 char by 1 char */
void
xtermSizeHints(XtermWidget xw, int scrollbarWidth)
{
    TScreen *screen = &xw->screen;

    TRACE(("xtermSizeHints\n"));
    TRACE(("   border    %d\n", xw->core.border_width));
    TRACE(("   scrollbar %d\n", scrollbarWidth));

    xw->hints.base_width = 2 * screen->border + scrollbarWidth;
    xw->hints.base_height = 2 * screen->border;

#if OPT_TOOLBAR
    TRACE(("   toolbar   %d\n", ToolbarHeight(xw)));

    xw->hints.base_height += ToolbarHeight(xw);
    xw->hints.base_height += BorderWidth(xw) * 2;
    xw->hints.base_width += BorderWidth(xw) * 2;
#endif

    xw->hints.width_inc = FontWidth(screen);
    xw->hints.height_inc = FontHeight(screen);
    xw->hints.min_width = xw->hints.base_width + xw->hints.width_inc;
    xw->hints.min_height = xw->hints.base_height + xw->hints.height_inc;

    xw->hints.width = MaxCols(screen) * FontWidth(screen) + xw->hints.min_width;
    xw->hints.height = MaxRows(screen) * FontHeight(screen) + xw->hints.min_height;

    xw->hints.flags |= (PSize | PBaseSize | PMinSize | PResizeInc);

    TRACE_HINTS(&(xw->hints));
}

void
getXtermSizeHints(XtermWidget xw)
{
    TScreen *screen = &xw->screen;
    long supp;

    if (!XGetWMNormalHints(screen->display, XtWindow(SHELL_OF(xw)),
			   &xw->hints, &supp))
	bzero(&xw->hints, sizeof(xw->hints));
    TRACE_HINTS(&(xw->hints));
}

/*
 * Returns a GC, selected according to the font (reverse/bold/normal) that is
 * required for the current position (implied).  The GC is updated with the
 * current screen foreground and background colors.
 */
GC
updatedXtermGC(XtermWidget xw, unsigned flags, unsigned fg_bg, Bool hilite)
{
    TScreen *screen = &(xw->screen);
    int my_fg = extract_fg(xw, fg_bg, flags);
    int my_bg = extract_bg(xw, fg_bg, flags);
    Pixel fg_pix = getXtermForeground(xw, flags, my_fg);
    Pixel bg_pix = getXtermBackground(xw, flags, my_bg);
    Pixel xx_pix;
#if OPT_HIGHLIGHT_COLOR
    Pixel hi_pix = T_COLOR(screen, HIGHLIGHT_BG);
#endif
    GC gc;

    (void) fg_bg;
    (void) my_bg;
    (void) my_fg;

    checkVeryBoldColors(flags, my_fg);

    if (ReverseOrHilite(screen, flags, hilite)) {
	if (flags & BOLDATTR(screen))
	    gc = ReverseBoldGC(screen);
	else
	    gc = ReverseGC(screen);

#if OPT_HIGHLIGHT_COLOR
	if (hi_pix != T_COLOR(screen, TEXT_FG)
	    && hi_pix != fg_pix
	    && hi_pix != bg_pix
	    && hi_pix != xw->dft_foreground) {
	    bg_pix = fg_pix;
	    fg_pix = hi_pix;
	}
#endif
	xx_pix = bg_pix;
	bg_pix = fg_pix;
	fg_pix = xx_pix;
    } else {
	if (flags & BOLDATTR(screen))
	    gc = NormalBoldGC(screen);
	else
	    gc = NormalGC(screen);

    }

#if OPT_BLINK_TEXT
    if ((screen->blink_state == ON) && (!screen->blink_as_bold) && (flags & BLINK)) {
	fg_pix = bg_pix;
    }
#endif

    XSetForeground(screen->display, gc, fg_pix);
    XSetBackground(screen->display, gc, bg_pix);
    return gc;
}

/*
 * Resets the foreground/background of the GC returned by 'updatedXtermGC()'
 * to the values that would be set in SGR_Foreground and SGR_Background. This
 * duplicates some logic, but only modifies 1/4 as many GC's.
 */
void
resetXtermGC(XtermWidget xw, unsigned flags, Bool hilite)
{
    TScreen *screen = &(xw->screen);
    Pixel fg_pix = getXtermForeground(xw, flags, xw->cur_foreground);
    Pixel bg_pix = getXtermBackground(xw, flags, xw->cur_background);
    GC gc;

    checkVeryBoldColors(flags, xw->cur_foreground);

    if (ReverseOrHilite(screen, flags, hilite)) {
	if (flags & BOLDATTR(screen))
	    gc = ReverseBoldGC(screen);
	else
	    gc = ReverseGC(screen);

	XSetForeground(screen->display, gc, bg_pix);
	XSetBackground(screen->display, gc, fg_pix);

    } else {
	if (flags & BOLDATTR(screen))
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
 * BOLD or UNDERLINE color-mode active, those will be used.
 */
unsigned
extract_fg(XtermWidget xw, unsigned color, unsigned flags)
{
    unsigned fg = ExtractForeground(color);

    if (xw->screen.colorAttrMode
	|| (fg == ExtractBackground(color))) {
	if (xw->screen.colorULMode && (flags & UNDERLINE))
	    fg = COLOR_UL;
	if (xw->screen.colorBDMode && (flags & BOLD))
	    fg = COLOR_BD;
	if (xw->screen.colorBLMode && (flags & BLINK))
	    fg = COLOR_BL;
    }
    return fg;
}

/*
 * Extract the background-color index from a one-byte color pair.
 * If we've got INVERSE color-mode active, that will be used.
 */
unsigned
extract_bg(XtermWidget xw, unsigned color, unsigned flags)
{
    unsigned bg = ExtractBackground(color);

    if (xw->screen.colorAttrMode
	|| (bg == ExtractForeground(color))) {
	if (xw->screen.colorRVMode && (flags & INVERSE))
	    bg = COLOR_RV;
    }
    return bg;
}

/*
 * Combine the current foreground and background into a single 8-bit number.
 * Note that we're storing the SGR foreground, since cur_foreground may be set
 * to COLOR_UL, COLOR_BD or COLOR_BL, which would make the code larger than 8
 * bits.
 *
 * This assumes that fg/bg are equal when we override with one of the special
 * attribute colors.
 */
unsigned
makeColorPair(int fg, int bg)
{
    unsigned my_bg = (bg >= 0) && (bg < NUM_ANSI_COLORS) ? (unsigned) bg : 0;
    unsigned my_fg = (fg >= 0) && (fg < NUM_ANSI_COLORS) ? (unsigned) fg : my_bg;
#if OPT_EXT_COLORS
    return (my_fg << 8) | my_bg;
#else
    return (my_fg << 4) | my_bg;
#endif
}

/*
 * Using the "current" SGR background, clear a rectangle.
 */
void
ClearCurBackground(XtermWidget xw,
		   int top,
		   int left,
		   unsigned height,
		   unsigned width)
{
    TScreen *screen = &(xw->screen);

    TRACE(("ClearCurBackground(%d,%d,%d,%d) %d\n",
	   top, left, height, width, xw->cur_background));

    if (VWindow(screen)) {
	set_background(xw, xw->cur_background);

	XClearArea(screen->display, VWindow(screen),
		   left, top, width, height, False);
    }
}
#endif /* OPT_ISO_COLORS */

/*
 * Returns a single 8/16-bit number for the given cell
 */
unsigned
getXtermCell(TScreen * screen, int row, int col)
{
    unsigned ch = SCRN_BUF_CHARS(screen, row)[col];
    if_OPT_WIDE_CHARS(screen, {
	ch |= (SCRN_BUF_WIDEC(screen, row)[col] << 8);
    });
    return ch;
}

/*
 * Sets a single 8/16-bit number for the given cell
 */
void
putXtermCell(TScreen * screen, int row, int col, int ch)
{
    SCRN_BUF_CHARS(screen, row)[col] = ch;
    if_OPT_WIDE_CHARS(screen, {
	int off;
	SCRN_BUF_WIDEC(screen, row)[col] = (ch >> 8);
	for (off = OFF_WIDEC + 1; off < MAX_PTRS; ++off) {
	    SCREEN_PTR(screen, row, off)[col] = 0;
	}
    });
}

#if OPT_WIDE_CHARS
unsigned
getXtermCellComb(TScreen * screen, int row, int col, int off)
{
    unsigned ch = SCREEN_PTR(screen, row, off)[col];
    ch |= (SCREEN_PTR(screen, row, off + 1)[col] << 8);
    return ch;
}

/*
 * Add a combining character for the given cell
 */
void
addXtermCombining(TScreen * screen, int row, int col, unsigned ch)
{
    if (ch != 0) {
	int off;
	for (off = OFF_FINAL; off < MAX_PTRS; off += 2) {
	    if (!SCREEN_PTR(screen, row, off + 0)[col]
		&& !SCREEN_PTR(screen, row, off + 1)[col]) {
		SCREEN_PTR(screen, row, off + 0)[col] = ch & 0xff;
		SCREEN_PTR(screen, row, off + 1)[col] = ch >> 8;
		break;
	    }
	}
    }
}
#endif

#ifdef HAVE_CONFIG_H
#ifdef USE_MY_MEMMOVE
char *
my_memmove(char *s1, char *s2, size_t n)
{
    if (n != 0) {
	if ((s1 + n > s2) && (s2 + n > s1)) {
	    static char *bfr;
	    static size_t length;
	    size_t j;
	    if (length < n) {
		length = (n * 3) / 2;
		bfr = ((bfr != 0)
		       ? TypeRealloc(char, length, bfr)
		       : TypeMallocN(char, length));
		if (bfr == NULL)
		    SysError(ERROR_MMALLOC);
	    }
	    for (j = 0; j < n; j++)
		bfr[j] = s2[j];
	    s2 = bfr;
	}
	while (n-- != 0)
	    s1[n] = s2[n];
    }
    return s1;
}
#endif /* USE_MY_MEMMOVE */

#ifndef HAVE_STRERROR
char *
my_strerror(int n)
{
    extern char *sys_errlist[];
    extern int sys_nerr;
    if (n > 0 && n < sys_nerr)
	return sys_errlist[n];
    return "?";
}
#endif
#endif

int
char2lower(int ch)
{
    if (isascii(ch) && isupper(ch)) {	/* lowercasify */
#ifdef _tolower
	ch = _tolower(ch);
#else
	ch = tolower(ch);
#endif
    }
    return ch;
}

void
update_keyboard_type(void)
{
    update_delete_del();
    update_old_fkeys();
    update_hp_fkeys();
    update_sco_fkeys();
    update_sun_fkeys();
    update_sun_kbd();
}

void
set_keyboard_type(XtermWidget xw, xtermKeyboardType type, Bool set)
{
    xtermKeyboardType save = xw->keyboard.type;

    TRACE(("set_keyboard_type(%s, %s) currently %s\n",
	   visibleKeyboardType(type),
	   BtoS(set),
	   visibleKeyboardType(xw->keyboard.type)));
    if (set) {
	xw->keyboard.type = type;
    } else {
	xw->keyboard.type = keyboardIsDefault;
    }

    if (save != xw->keyboard.type) {
	update_keyboard_type();
    }
}

void
toggle_keyboard_type(XtermWidget xw, xtermKeyboardType type)
{
    xtermKeyboardType save = xw->keyboard.type;

    TRACE(("toggle_keyboard_type(%s) currently %s\n",
	   visibleKeyboardType(type),
	   visibleKeyboardType(xw->keyboard.type)));
    if (xw->keyboard.type == type) {
	xw->keyboard.type = keyboardIsDefault;
    } else {
	xw->keyboard.type = type;
    }

    if (save != xw->keyboard.type) {
	update_keyboard_type();
    }
}

void
init_keyboard_type(XtermWidget xw, xtermKeyboardType type, Bool set)
{
    static Bool wasSet = False;

    TRACE(("init_keyboard_type(%s, %s) currently %s\n",
	   visibleKeyboardType(type),
	   BtoS(set),
	   visibleKeyboardType(xw->keyboard.type)));
    if (set) {
	if (wasSet) {
	    fprintf(stderr, "Conflicting keyboard type option (%u/%u)\n",
		    xw->keyboard.type, type);
	}
	xw->keyboard.type = type;
	wasSet = True;
	update_keyboard_type();
    }
}

/*
 * If the keyboardType resource is set, use that, overriding the individual
 * boolean resources for different keyboard types.
 */
void
decode_keyboard_type(XtermWidget xw, XTERM_RESOURCE * rp)
{
#define DATA(n, t, f) { n, t, XtOffsetOf(XTERM_RESOURCE, f) }
#define FLAG(n) *(Boolean *)(((char *)rp) + table[n].offset)
    static struct {
	const char *name;
	xtermKeyboardType type;
	unsigned offset;
    } table[] = {
#if OPT_HP_FUNC_KEYS
	DATA(NAME_HP_KT, keyboardIsHP, hpFunctionKeys),
#endif
#if OPT_SCO_FUNC_KEYS
	    DATA(NAME_SCO_KT, keyboardIsSCO, scoFunctionKeys),
#endif
#if OPT_SUN_FUNC_KEYS
	    DATA(NAME_SUN_KT, keyboardIsSun, sunFunctionKeys),
#endif
#if OPT_SUNPC_KBD
	    DATA(NAME_VT220_KT, keyboardIsVT220, sunKeyboard),
#endif
    };
    Cardinal n;

    TRACE(("decode_keyboard_type(%s)\n", rp->keyboardType));
    if (!x_strcasecmp(rp->keyboardType, "unknown")) {
	/*
	 * Let the individual resources comprise the keyboard-type.
	 */
	for (n = 0; n < XtNumber(table); ++n)
	    init_keyboard_type(xw, table[n].type, FLAG(n));
    } else if (!x_strcasecmp(rp->keyboardType, "default")) {
	/*
	 * Set the keyboard-type to the Sun/PC type, allowing modified
	 * function keys, etc.
	 */
	for (n = 0; n < XtNumber(table); ++n)
	    init_keyboard_type(xw, table[n].type, False);
    } else {
	Bool found = False;

	/*
	 * Choose an individual keyboard type.
	 */
	for (n = 0; n < XtNumber(table); ++n) {
	    if (!x_strcasecmp(rp->keyboardType, table[n].name + 1)) {
		FLAG(n) = True;
		found = True;
	    } else {
		FLAG(n) = False;
	    }
	    init_keyboard_type(xw, table[n].type, FLAG(n));
	}
	if (!found) {
	    fprintf(stderr,
		    "KeyboardType resource \"%s\" not found\n",
		    rp->keyboardType);
	}
    }
#undef DATA
#undef FLAG
}

#if OPT_WIDE_CHARS
#if defined(HAVE_WCHAR_H) && defined(HAVE_WCWIDTH)
/*
 * If xterm is running in a UTF-8 locale, it is still possible to encounter
 * old runtime configurations which yield incomplete or inaccurate data.
 */
static Bool
systemWcwidthOk(void)
{
    wchar_t n;
    int oops = 0;
    int last = 1024;

    for (n = 0; n < last; ++n) {
	int system_code = wcwidth(n);
	int intern_code = mk_wcwidth(n);

	/*
	 * Since mk_wcwidth() is designed to check for nonspacing characters,
	 * and has rough range-checks for double-width characters, it will
	 * generally not detect cases where a code has not been assigned.
	 *
	 * Some experimentation with GNU libc suggests that up to 1/4 of the
	 * codes would differ, simply because the runtime library would have a
	 * table listing the unassigned codes, and return -1 for those.  If
	 * mk_wcwidth() has no information about a code, it returns 1.  On the
	 * other hand, if the runtime returns a positive number, the two should
	 * agree.
	 *
	 * The "up to" is measured for 4k, 8k, 16k of data.  With only 1k, the
	 * number of differences was only 77.  However, that is only one
	 * system, and this is only a sanity check to avoid using broken
	 * libraries.
	 */
	if ((system_code < 0 && intern_code >= 1)
	    || (system_code >= 0 && intern_code != system_code)) {
	    ++oops;
	}
    }
    TRACE(("systemWcwidthOk: %d/%d mismatches\n", oops, last));
    return (oops < (last / 4));
}
#endif /* HAVE_WCWIDTH */

void
decode_wcwidth(int mode)
{
    switch (mode) {
    default:
#if defined(HAVE_WCHAR_H) && defined(HAVE_WCWIDTH)
	if (xtermEnvUTF8() && systemWcwidthOk()) {
	    my_wcwidth = wcwidth;
	    TRACE(("using system wcwidth() function\n"));
	    break;
	}
	/* FALLTHRU */
    case 2:
#endif
	my_wcwidth = &mk_wcwidth;
	TRACE(("using MK wcwidth() function\n"));
	break;
    case 3:
    case 4:
	my_wcwidth = &mk_wcwidth_cjk;
	TRACE(("using MK-CJK wcwidth() function\n"));
	break;
    }
}
#endif
