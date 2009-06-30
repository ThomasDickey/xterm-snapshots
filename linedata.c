/* $XTermId: linedata.c,v 1.58 2009/06/30 00:26:46 tom Exp $ */

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
 */
LineData *
getLineData(TScreen * screen, int row)
{
    LineData *result = 0;
    ScrnBuf buffer;

    if (row >= 0) {
	buffer = screen->visbuf;
    } else {
	buffer = screen->allbuf;
	row += screen->savelines;
    }
    if (row >= 0) {
	/*
	 * FIXME - the references to allbuf should be in scrollback.c
	 */
	result = (LineData *) scrnHeadAddr(screen, buffer, (unsigned) row);
	if (result != 0) {
#if 1				/* FIXME - these should be done in setupLineData, etc. */
	    result->lineSize = (Dimension) MaxCols(screen);
#if OPT_WIDE_CHARS
	    if (screen->wide_chars) {
		result->combSize = (Char) screen->max_combining;
	    } else {
		result->combSize = 0;
	    }
#endif
#endif /* FIXME */
	}
    }

    return result;
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

void
initLineData(XtermWidget xw)
{
    TScreen *screen = &(xw->screen);

    initLineExtra(screen);
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
	item->attribs = ld->attribs[column];
#if OPT_ISO_COLORS
	item->color = ld->color[column];
#endif
	item->charData = ld->charData[column];
	if_OPT_WIDE_CHARS(screen, {
	    size_t off;
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
	ld->attribs[column] = item->attribs;
#if OPT_ISO_COLORS
	ld->color[column] = item->color;
#endif
	ld->charData[column] = item->charData;
	if_OPT_WIDE_CHARS(screen, {
	    size_t off;
	    ld->combSize = item->combSize;
	    for_each_combData(off, ld) {
		ld->combData[off][column] = item->combData[off];
	    }
	})
    }
}
