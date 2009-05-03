/* $XTermId: linedata.c,v 1.18 2009/05/02 23:00:52 tom Exp $ */

/************************************************************

Copyright 2009 by Thomas E. Dickey

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
#include <data.h>		/* FIXME - needed for 'term' */

#include <assert.h>

/*
 * Given a row-number, find the corresponding data for the line in the VT100
 * widget.  Row numbers can be positive or negative.
 *
 * TODO: if the data comes from the scrollback, defer that to getScrollback().
 * TODO: the calculation can be cached, e.g., if we save a pointer to the data.
 */
LineData *
getLineData(TScreen * screen,
	    int row,
	    LineData * work)
{
    if (okScrnRow(screen, row)) {
	void *check = 0;
	Bool update = False;
#if OPT_WIDE_CHARS
	size_t ptr;
#endif

	/*
	 * If caller does not pass a workspace address, use the widget's
	 * cached workspace.
	 */
	if (work == 0) {
	    if ((work = screen->lineData) != 0) {
		check = &SCREEN_PTR(screen, row, 0);
		if (check != screen->lineCache) {
		    update = True;
		    screen->lineCache = check;
		}
	    }
	} else {
	    /* just update - caller's struct is on the stack */
	    update = True;
	}

	if (update) {
	    TRACE2(("getLineData %d:%p\n", row, check));

	    /* FIXME: sizeof() + extraSize for visbuf */

	    work->bufHead = (RowFlags *) & (SCRN_BUF_FLAGS(screen, row));
	    work->attribs = SCRN_BUF_ATTRS(screen, row);
#if OPT_ISO_COLORS
#if OPT_256_COLORS || OPT_88_COLORS
	    work->fgrnd = SCRN_BUF_FGRND(screen, row);
	    work->bgrnd = SCRN_BUF_BGRND(screen, row);
#else
	    work->color = SCRN_BUF_COLOR(screen, row);
#endif
#endif
#if OPT_DEC_CHRSET
	    work->charSets = SCRN_BUF_CSETS(screen, row);
#endif
	    work->charData = SCRN_BUF_CHARS(screen, row);
#if OPT_WIDE_CHARS
	    work->wideData = SCRN_BUF_WIDEC(screen, row);

	    /*
	     * Construct an array of pointers to combining character data. 
	     * This is a flexible array on the end of LineData.
	     *
	     * The scrollback should only store combining characters for rows
	     * that have that data.  The visbuf should store this for all rows
	     * since they can be updated until moved to the scrollback.
	     */
	    work->combSize = (size_t) (screen->max_combining * 2);
	    for (ptr = 0; ptr < work->combSize; ++ptr) {
		work->combData[ptr] = SCREEN_PTR(screen, row, (int) ptr + OFF_FINAL);
	    }
#endif
	}
    } else {
	work = 0;
    }

    checkLineData(screen, row, work);
    return work;
}

/*
 * Allocate a new LineData struct, for working memory used in getLineData
 * calls.  The size of the struct depends on compile-time configure options for
 * normal/wide characters and the maximum number of colors, as well as the
 * runtime maximum number of combining characters.
 */
void
initLineData(XtermWidget xw)
{
    TScreen *screen = &(xw->screen);

    screen->lineCache = 0;
    screen->lineData = newLineData(xw);
}

/*
 * Add a new row of data to the VT100 widget, scrolling the current window
 * off by one line into the scrollback area.
 *
 * TODO: implement addScrollback().
 */
void
addLineData(XtermWidget xw GCC_UNUSED)
{
}

/*
 * Allocate a new LineData struct, which includes an array on the end to
 * address combining characters.
 */
LineData *
newLineData(XtermWidget xw)
{
    LineData *result;
    TScreen *screen = &(xw->screen);

#if OPT_WIDE_CHARS
    screen->lineExtra = ((size_t) (screen->max_combining * 2)
			 * sizeof(IChar *));
#else
    screen->lineExtra = 0;
#endif

    result = CastMallocN(LineData, screen->lineExtra);

    return result;
}

/*
 * For debugging, verify that the pointers in a LineData struct match the
 * expected values for the given row.
 */
#define TRACE_ASSERT(name, expression) \
 	TRACE2(("checkLineData " #name " %p vs %p\n", \
		work->name, expression)); \
	assert(work->name == expression)

void
checkLineData(TScreen * screen GCC_UNUSED,
	      int row GCC_UNUSED,
	      LineData * work GCC_UNUSED)
{
    TRACE2(("checkLineData %p\n", work));
    assert(work != 0);
    TRACE_ASSERT(charData, SCRN_BUF_CHARS(screen, row));
    TRACE_ASSERT(attribs, SCRN_BUF_ATTRS(screen, row));
}

#undef TRACE_ASSERT
