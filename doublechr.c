/*
 * $XFree86: xc/programs/xterm/doublechr.c,v 3.3 1998/06/04 16:43:57 hohndel Exp $
 */

/************************************************************

Copyright 1997 by Thomas E. Dickey <dickey@clark.net>

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

#ifdef HAVE_CONFIG_H
#include <xtermcfg.h>
#endif

#include "ptyx.h"
#include "data.h"
#include "xterm.h"

/*
 * The first column is all that matters for double-size characters (since the
 * controls apply to a whole line).  However, it's easier to maintain the
 * information for special fonts by writing to all cells.
 */
#define curChrSet SCRN_BUF_CSETS(screen, screen->cur_row)[0]

#if OPT_DEC_CHRSET
static void
repaint_line(unsigned newChrSet)
{
	register TScreen *screen = &term->screen;
	int curcol = screen->cur_col;
	int len = screen->max_col + 1;
	int width = len;

	TRACE(("repaint_line(%2d,%2d) (%d)\n", screen->cur_row, screen->cur_col, newChrSet))

	/* If switching from single-width, keep the cursor in the visible part
	 * of the line.
	 */
	if (CSET_DOUBLE(newChrSet)) {
		width /= 2;
		if (curcol > width)
			curcol = width;
	}

	/* FIXME: do VT220 softchars allow double-sizes? */
	memset(SCRN_BUF_CSETS(screen, screen->cur_row), newChrSet, len);

	screen->cur_col = 0;
	ScrnRefresh (screen, screen->cur_row, 0, 1, len, True);
	screen->cur_col = curcol;
}
#endif

/*
 * Set the line to double-height characters.  The 'top' flag denotes whether
 * we'll be using it for the top (true) or bottom (false) of the line.
 */
void
xterm_DECDHL(Bool top)
{
#if OPT_DEC_CHRSET
	repaint_line(top ? CSET_DHL_TOP : CSET_DHL_BOT);
#endif
}

/*
 * Set the line to single-width characters (the normal state).
 */
void
xterm_DECSWL(void)
{
#if OPT_DEC_CHRSET
	repaint_line(CSET_SWL);
#endif
}

/*
 * Set the line to double-width characters
 */
void
xterm_DECDWL(void)
{
#if OPT_DEC_CHRSET
	repaint_line(CSET_DWL);
#endif
}
