/* $XTermId: linedata.c,v 1.46 2009/06/15 23:43:39 tom Exp $ */

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

static ScrnPtrs *
getScrnPtrs(TScreen * screen, unsigned row)
{
    return (ScrnPtrs *) scrnHeadAddr(screen, screen->visbuf, row);
}

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

	/*
	 * If caller does not pass a workspace address, use the widget's
	 * cached workspace.
	 */
	if (work == 0) {
	    if ((work = screen->lineData) != 0) {
		ScrnPtrs *ptrs = getScrnPtrs(screen, (unsigned) row);
		check = &(ptrs->bufHead);
		/*
		 * Check if the cached LineData is up to date.
		 * The second part of the comparison is needed since xterm
		 * implements scrolling via memory-moves.
		 */
		if (check != screen->lineCache
		    || ptrs->charData != work->charData) {
		    update = True;
		    screen->lineCache = check;
		    assert(check != 0);
		}
	    }
	} else {
	    /* just update - caller's struct is on the stack */
	    update = True;
	}

	if (update) {
	    TRACE2(("getLineData %d:%p\n", row, check));

	    fillLineData(screen,
			 scrnHeadAddr(screen, screen->visbuf, (unsigned) row),
			 work);
	    work->lineSize = (unsigned) MaxCols(screen);
	}
	checkLineData(screen, row, work);
    } else {
	work = 0;
    }

    return work;
}

void
fillLineData(TScreen * screen, ScrnPtr * ptrs, LineData * work)
{
    ScrnPtrs *realPtrs = (ScrnPtrs *) ptrs;
#if OPT_WIDE_CHARS
    size_t off;
#endif

    work->lineSize = (unsigned) MaxCols(screen);
    work->bufHead = (RowFlags *) & (realPtrs->bufHead);
    work->attribs = realPtrs->attribs;
#if OPT_ISO_COLORS
    work->color = realPtrs->color;
#endif
#if OPT_DEC_CHRSET
    work->charSets = realPtrs->charSets;
#endif
    work->charData = realPtrs->charData;
#if OPT_WIDE_CHARS
    if (screen->wide_chars) {
	work->wideData = realPtrs->wideData;

	/*
	 * Construct an array of pointers to combining character data. 
	 * This is a flexible array on the end of LineData.
	 *
	 * The scrollback should only store combining characters for
	 * rows that have that data.  The visbuf should store this for
	 * all rows since they can be updated until moved to the
	 * scrollback.
	 */
	work->combSize = (Char) screen->max_combining;
	for (off = 0; off < work->combSize; ++off) {
	    work->combData[off] = realPtrs->combData[off];
	}
    } else {
	work->wideData = 0;
	work->combSize = 0;
    }
#endif
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
    screen->lineData = newLineData(screen);
}

#if 0
/*
 * Delete one or more rows of data in the VT100 widget, scrolling the window
 * contents below the line up by the given count.
 *
 * The row(s) to be deleted will be either in the visible VT100 window, or
 * starting at the beginning of the scrolled-back lines (for scrolling).
 *
 * TODO: store the scrolled-back lines in a FIFO, so we do not have to move
 * the whole set of lines for each scrolling operation.
 */
void
deleteLineData(XtermWidget xw, int row, int count)
{
}

/*
 * Insert one or more rows of data in the VT100 widget, scrolling the window
 * contents below the line down by the given count.
 *
 * The row(s) to be inserted will always be in the visible VT100 window.
 */
void
insertLineData(XtermWidget xw, int row, int count)
{
}
#endif

#if OPT_WIDE_CHARS
#define initLineExtra(screen) \
    screen->lineExtra = ((size_t) (screen->max_combining) * sizeof(IChar *))
#else
#define initLineExtra(screen) \
    screen->lineExtra = 0
#endif
/*
 * Allocate a new LineData struct, which includes an array on the end to
 * address combining characters.
 */
LineData *
newLineData(TScreen * screen)
{
    LineData *result;

    initLineExtra(screen);
    result = CastMallocN(LineData, screen->lineExtra);

    assert(result != 0);
    return result;
}

void
destroyLineData(TScreen * screen, LineData * ld)
{
    if (ld != 0) {
	free(ld);
	if (screen->lineData == ld) {
	    screen->lineData = 0;
	    screen->lineCache = 0;
	} else if (screen->lineCache == ld) {
	    screen->lineCache = 0;
	}
    } else if (screen->lineData != 0) {
	destroyLineData(screen, screen->lineData);
    }
}

/*
 * CellData size depends on the "combiningChars" resource.
 * FIXME - revise this to reduce arithmetic...
 */
#define CellDataSize(screen) (sizeof(CellData) + screen->lineExtra)

#define CellDataAddr(screen, data, cell) \
	(CellData *)((char *)data + (cell * CellDataSize(screen)))

CellData *
newCellData(XtermWidget xw, Cardinal count)
{
    CellData *result;
    TScreen *screen = &(xw->screen);

    initLineExtra(screen);
    result = (CellData *) calloc(count, CellDataSize(screen));
    return result;
}

void
saveCellData(TScreen * screen,
	     CellData * data,
	     Cardinal cell,
	     LineData * ld,
	     int column)
{
    CellData *item = CellDataAddr(screen, data, cell);

    if (column < MaxCols(screen)) {
	item->bufHead = *(ld->bufHead);
	item->attribs = ld->attribs[column];
#if OPT_ISO_COLORS
	item->color = ld->color[column];
#endif
#if OPT_DEC_CHRSET
	item->charSets = ld->charSets[column];
#endif
	item->charData = ld->charData[column];
	if_OPT_WIDE_CHARS(screen, {
	    size_t off;
	    item->wideData = ld->wideData[column];
	    item->combSize = ld->combSize;
	    for_each_combData(off, ld) {
		item->combData[off] = ld->combData[off][column];
	    }
	})
    }
}

void
restoreCellData(TScreen * screen,
		CellData * data,
		Cardinal cell,
		LineData * ld,
		int column)
{
    CellData *item = CellDataAddr(screen, data, cell);

    if (column < MaxCols(screen)) {
	/* FIXME - *(ld->bufHead) = item->bufHead; */
	ld->attribs[column] = item->attribs;
#if OPT_ISO_COLORS
	ld->color[column] = item->color;
#endif
#if OPT_DEC_CHRSET
	ld->charSets[column] = item->charSets;
#endif
	ld->charData[column] = item->charData;
	if_OPT_WIDE_CHARS(screen, {
	    size_t off;
	    ld->wideData[column] = item->wideData;
	    ld->combSize = item->combSize;
	    for_each_combData(off, ld) {
		ld->combData[off][column] = item->combData[off];
	    }
	})
    }
}

/*
 * For debugging, verify that the pointers in a LineData struct match the
 * expected values for the given row.
 */
#define TRACE_ASSERT(name, expression) \
 	TRACE2(("checkLineData %10s %p vs %p\n", \
		#name, work->name, expression)); \
	assert(work->name == expression); \
	assert(work->name != 0)

void
checkLineData(TScreen * screen GCC_UNUSED,
	      int row GCC_UNUSED,
	      LineData * work GCC_UNUSED)
{
#if OPT_TRACE
    ScrnPtrs *ptrs = getScrnPtrs(screen, (unsigned) row);
#endif

    TRACE2(("checkLineData %d ->%p\n", row, work));
    assert(work != 0);
    TRACE_ASSERT(charData, ptrs->charData);
    TRACE_ASSERT(attribs, ptrs->attribs);
#if OPT_DEC_CHRSET
    TRACE_ASSERT(charSets, ptrs->charSets);
#endif
}

#undef TRACE_ASSERT
