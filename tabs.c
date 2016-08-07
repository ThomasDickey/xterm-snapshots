/*
 *	$XConsortium: tabs.c,v 1.4 91/05/06 17:12:18 gildea Exp $
 *	$XFree86: xc/programs/xterm/tabs.c,v 3.2.4.1 1997/05/23 09:24:43 dawes Exp $
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

/* tabs.c */

#ifdef HAVE_CONFIG_H
#include <xtermcfg.h>
#endif

#include "ptyx.h"

#include "xterm.h"

extern XtermWidget term;

/*
 * This file presumes 32bits/word.  This is somewhat of a crock, and should
 * be fixed sometime.
 */
#define TAB_INDEX(n) ((n) >> 5)
#define TAB_MASK(n)  (1 << ((n) & 31))

#define SET_TAB(tabs,n) tabs[TAB_INDEX(n)] |=  TAB_MASK(n)
#define CLR_TAB(tabs,n) tabs[TAB_INDEX(n)] &= ~TAB_MASK(n)
#define TST_TAB(tabs,n) tabs[TAB_INDEX(n)] &   TAB_MASK(n)

/*
 * places tabstops at only every 8 columns
 */
void
TabReset(tabs)
Tabs	tabs;
{
	register int i;

	for (i=0; i<TAB_ARRAY_SIZE; ++i)
		tabs[i] = 0;

	for (i=0; i<MAX_TABS; i+=8)
		TabSet(tabs, i);
}	


/*
 * places a tabstop at col
 */
void
TabSet(tabs, col)
    Tabs	tabs;
    int		col;
{
	SET_TAB(tabs,col);
}

/*
 * clears a tabstop at col
 */
void
TabClear(tabs, col)
    Tabs	tabs;
    int		col;
{
	CLR_TAB(tabs,col);
}

/*
 * returns the column of the next tabstop
 * (or MAX_TABS - 1 if there are no more).
 * A tabstop at col is ignored.
 */
int
TabNext (tabs, col)
    Tabs	tabs;
    int		col;
{
	register TScreen *screen = &term->screen;

	if(screen->curses && screen->do_wrap && (term->flags & WRAPAROUND)) {
		Index(screen, 1);
		col = screen->cur_col = screen->do_wrap = 0;
	}
	for (++col; col<MAX_TABS; ++col)
		if (TST_TAB(tabs,col))
			return (col);

	return (MAX_TABS - 1);
}

/*
 * returns the column of the previous tabstop
 * (or 0 if there are no more).
 * A tabstop at col is ignored.
 */
int
TabPrev (tabs, col)
    Tabs	tabs;
    int		col;
{
	for (--col; col >= 0; --col)
		if (TST_TAB(tabs,col))
			return (col);

	return (0);
}

/*
 * Tab to the next stop, returning true if the cursor moved
 */
Boolean
TabToNextStop()
{
	register TScreen *screen = &term->screen;
	int saved_column = screen->cur_col;

	screen->cur_col = TabNext(term->tabs, screen->cur_col);
	if (screen->cur_col > screen->max_col)
		screen->cur_col = screen->max_col;

	return (screen->cur_col > saved_column);
}

/*
 * Tab to the previous stop, returning true if the cursor moved
 */
Boolean
TabToPrevStop()
{
	register TScreen *screen = &term->screen;
	int saved_column = screen->cur_col;

	screen->cur_col = TabPrev(term->tabs, screen->cur_col);

	return (screen->cur_col < saved_column);
}

/*
 * clears all tabs
 */
void
TabZonk (tabs)
Tabs	tabs;
{
	register int i;

	for (i=0; i<TAB_ARRAY_SIZE; ++i)
		tabs[i] = 0;
}
