/*
 * $XFree86: xc/programs/xterm/doublechr.c,v 3.6 1999/03/14 03:22:36 dawes Exp $
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

#include <xterm.h>
#include <data.h>
#include <fontutils.h>

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
	int currow = screen->cur_row;
	int len = screen->max_col + 1;
	int width = len;
	unsigned oldChrSet = SCRN_BUF_CSETS(screen, currow)[0];

	/*
	 * Ignore repetition.
	 */
	if (oldChrSet == newChrSet)
		return;

	TRACE(("repaint_line(%2d,%2d) (%d)\n", currow, screen->cur_col, newChrSet))
	HideCursor();

	/* If switching from single-width, keep the cursor in the visible part
	 * of the line.
	 */
	if (CSET_DOUBLE(newChrSet)) {
		width /= 2;
		if (curcol > width)
			curcol = width;
	}

	/*
	 * ScrnRefresh won't paint blanks for us if we're switching between a
	 * single-size and double-size font.
	 */
	if (CSET_DOUBLE(oldChrSet) != CSET_DOUBLE(newChrSet)) {
		ClearCurBackground(
			screen,
			CursorY (screen, currow),
			CurCursorX (screen, currow, 0),
			FontHeight(screen),
			len * CurFontWidth(screen,currow));
	}

	/* FIXME: do VT220 softchars allow double-sizes? */
	memset(SCRN_BUF_CSETS(screen, currow), newChrSet, len);

	screen->cur_col = 0;
	ScrnRefresh (screen, currow, 0, 1, len, True);
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


#if OPT_DEC_CHRSET
int
xterm_Double_index(unsigned chrset, unsigned flags)
{
	int n = (chrset % 4);
#if NUM_CHRSET == 8
	if (flags & BOLD)
		n |= 4;
#endif
	return n;
}

/*
 * Lookup/cache a GC for the double-size character display.  We save up to
 * NUM_CHRSET values.
 */
GC
xterm_DoubleGC(unsigned chrset, unsigned flags, GC old_gc)
{
	XGCValues gcv;
	register TScreen *screen = &term->screen;
	unsigned long mask = (GCForeground | GCBackground | GCFont);
	int n = xterm_Double_index(chrset, flags);
	char *name = xtermSpecialFont(flags, chrset);

	if (name == 0)
		return 0;

	if (screen->double_fn[n] != 0) {
		if (!strcmp(screen->double_fn[n], name)) {
			if (screen->double_fs[n] != 0) {
				XCopyGC(screen->display, old_gc, ~GCFont, screen->double_gc[n]);
				return screen->double_gc[n];
			}
		}
	}
	screen->double_fn[n] = name;

	if (screen->double_fs[n] != 0) {
		XFreeFont(screen->display, screen->double_fs[n]);
		screen->double_fs[n] = 0;
	}

	TRACE(("xterm_DoubleGC %s %d: %s\n", flags&BOLD ? "BOLD" : "NORM", n, name))

	if ((screen->double_fs[n] = XLoadQueryFont (screen->display, name)) == 0)
		return 0;
	TRACE(("-> OK\n"))

	gcv.graphics_exposures = TRUE;	/* default */
	gcv.font       = screen->double_fs[n]->fid;
	gcv.foreground = screen->foreground;
	gcv.background = term->core.background_pixel;

	screen->double_gc[n] = XCreateGC (screen->display, VWindow(screen), mask, &gcv);
	XCopyGC(screen->display, old_gc, ~GCFont, screen->double_gc[n]);
	return screen->double_gc[n];
}
#endif
