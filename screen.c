/* $XTermId: screen.c,v 1.660 2025/04/10 23:48:40 tom Exp $ */

/*
 * Copyright 1999-2024,2025 by Thomas E. Dickey
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

/* screen.c */

#include <stdio.h>
#include <xterm.h>
#include <error.h>
#include <data.h>
#include <xterm_io.h>

#include <X11/Xatom.h>

#if OPT_WIDE_ATTRS || OPT_WIDE_CHARS
#include <fontutils.h>
#endif

#include <menu.h>

#include <assert.h>
#include <signal.h>

#include <graphics.h>

#define inSaveBuf(screen, buf, inx) \
	((buf) == (screen)->saveBuf_index && \
	 ((inx) < (screen)->savelines || (screen)->savelines == 0))

#define getMinRow(screen) ((xw->flags & ORIGIN) ? (screen)->top_marg : 0)
#define getMaxRow(screen) ((xw->flags & ORIGIN) ? (screen)->bot_marg : (screen)->max_row)
#define getMinCol(screen) ((xw->flags & ORIGIN) ? (screen)->lft_marg : 0)
#define getMaxCol(screen) ((xw->flags & ORIGIN) ? (screen)->rgt_marg : (screen)->max_col)

#define MoveLineData(base, dst, src, len) \
	memmove(scrnHeadAddr(screen, base, (unsigned) (dst)), \
		scrnHeadAddr(screen, base, (unsigned) (src)), \
		(size_t) scrnHeadSize(screen, (unsigned) (len)))

#define SaveLineData(base, src, len) \
	(void) ScrnPointers(screen, len); \
	memcpy (screen->save_ptr, \
		scrnHeadAddr(screen, base, src), \
		(size_t) scrnHeadSize(screen, (unsigned) (len)))

#define RestoreLineData(base, dst, len) \
	memcpy (scrnHeadAddr(screen, base, dst), \
		screen->save_ptr, \
		(size_t) scrnHeadSize(screen, (unsigned) (len)))

#define VisBuf(screen) screen->editBuf_index[screen->whichBuf]

/*
 * ScrnPtr's can point to different types of data.
 */
#define SizeofScrnPtr(name) \
	(unsigned) sizeof(*((LineData *)0)->name)

/*
 * The pointers in LineData point into a block of text allocated as a single
 * chunk for the given number of rows.  Ensure that these pointers are aligned
 * at least to int-boundaries.
 */
#define AlignMask()      (sizeof(int) - 1)
#define IsAligned(value) (((unsigned long) (value) & AlignMask()) == 0)

#define AlignValue(value) \
		if (!IsAligned(value)) \
		    value = (value | (unsigned) AlignMask()) + 1

#define SetupScrnPtr(dst,src,type) \
		dst = (type *) (void *) src; \
		assert(IsAligned(dst)); \
		src += skipNcol##type

#define ScrnBufAddr(ptrs, offset)  (ScrnBuf)    ((void *) ((char *) (ptrs) + (offset)))
#define LineDataAddr(ptrs, offset) (LineData *) ((void *) ((char *) (ptrs) + (offset)))

#if OPT_TRACE > 1
static void
traceScrnBuf(const char *tag, TScreen *screen, ScrnBuf sb, unsigned len)
{
    unsigned j;

    TRACE(("traceScrnBuf %s\n", tag));
    for (j = 0; j < len; ++j) {
	LineData *src = (LineData *) scrnHeadAddr(screen, sb, j);
	TRACE(("%p %s%3d:%s\n",
	       src, ((int) j >= screen->savelines) ? "*" : " ",
	       j, visibleIChars(src->charData, src->lineSize)));
    }
    TRACE(("...traceScrnBuf %s\n", tag));
}

#define TRACE_SCRNBUF(tag, screen, sb, len) traceScrnBuf(tag, screen, sb, len)
#else
#define TRACE_SCRNBUF(tag, screen, sb, len)	/*nothing */
#endif

#if OPT_WIDE_CHARS
#define scrnHeadSize(screen, count) \
	(unsigned) ((count) * \
		    (SizeOfLineData + \
		     ((screen)->wide_chars \
		      ? (unsigned) (screen)->lineExtra \
		      : 0)))
#else
#define scrnHeadSize(screen, count) \
	(unsigned) ((count) * \
		    SizeOfLineData)
#endif

ScrnBuf
scrnHeadAddr(TScreen *screen, ScrnBuf base, unsigned offset)
{
    unsigned size = scrnHeadSize(screen, offset);
    ScrnBuf result = ScrnBufAddr(base, size);

    (void) screen;
    assert((int) offset >= 0);

    return result;
}

/*
 * Given a block of data, build index to it in the 'base' parameter.
 */
void
setupLineData(TScreen *screen,
	      ScrnBuf base,
	      Char *data,
	      unsigned nrow,
	      unsigned ncol,
	      Bool bottom)
{
    unsigned i;
    unsigned offset = 0;
    unsigned jump = scrnHeadSize(screen, 1);
    LineData *ptr;
#if OPT_WIDE_CHARS
    unsigned j;
#endif
    /* these names are based on types */
    unsigned skipNcolIAttr;
    unsigned skipNcolCharData;
#if OPT_DEC_RECTOPS
    unsigned skipNcolChar;
#endif
#if OPT_ISO_COLORS
    unsigned skipNcolCellColor;
#endif

    (void) screen;
    AlignValue(ncol);

    (void) bottom;
#if OPT_STATUS_LINE
    if (bottom) {
	AddStatusLineRows(nrow);
    }
#endif

    /* *INDENT-EQLS* */
    skipNcolIAttr     = (ncol * SizeofScrnPtr(attribs));
    skipNcolCharData  = (ncol * SizeofScrnPtr(charData));
#if OPT_DEC_RECTOPS
    skipNcolChar      = (ncol * SizeofScrnPtr(charSeen));	/* = charSets */
#endif
#if OPT_ISO_COLORS
    skipNcolCellColor = (ncol * SizeofScrnPtr(color));
#endif

    for (i = 0; i < nrow; i++, offset += jump) {
	ptr = LineDataAddr(base, offset);

	ptr->lineSize = (Dimension) ncol;
	ptr->bufHead = 0;
#if OPT_DEC_CHRSET
	SetLineDblCS(ptr, 0);
#endif
	SetupScrnPtr(ptr->attribs, data, IAttr);
#if OPT_ISO_COLORS
	SetupScrnPtr(ptr->color, data, CellColor);
#endif
	SetupScrnPtr(ptr->charData, data, CharData);
#if OPT_DEC_RECTOPS
	SetupScrnPtr(ptr->charSeen, data, Char);
	SetupScrnPtr(ptr->charSets, data, Char);
#endif
#if OPT_WIDE_CHARS
	if (screen->wide_chars) {
	    unsigned extra = (unsigned) screen->max_combining;

	    ptr->combSize = (Char) extra;
	    for (j = 0; j < extra; ++j) {
		SetupScrnPtr(ptr->combData[j], data, CharData);
	    }
	}
#endif
    }
}

#define ExtractScrnData(name) \
		memcpy(dstPtrs->name, \
		       ((LineData *) srcPtrs)->name,\
		       dstCols * sizeof(dstPtrs->name[0]))

/*
 * As part of reallocating the screen buffer when resizing, extract from
 * the old copy of the screen buffer the data which will be used in the
 * new copy of the screen buffer.
 */
static void
extractScrnData(TScreen *screen,
		ScrnBuf dstPtrs,
		ScrnBuf srcPtrs,
		unsigned nrows,
		unsigned move_down)
{
    unsigned j;

    TRACE(("extractScrnData(nrows %d)\n", nrows));

    TRACE_SCRNBUF("extract from", screen, srcPtrs, nrows);
    for (j = 0; j < nrows; j++) {
	LineData *dst = (LineData *) scrnHeadAddr(screen,
						  dstPtrs, j + move_down);
	LineData *src = (LineData *) scrnHeadAddr(screen,
						  srcPtrs, j);
	copyLineData(dst, src);
    }
}

static ScrnPtr *
allocScrnHead(TScreen *screen, unsigned nrow)
{
    ScrnPtr *result;
    unsigned size = scrnHeadSize(screen, 1);

    (void) screen;
    AddStatusLineRows(nrow);
    result = (ScrnPtr *) calloc((size_t) nrow, (size_t) size);
    if (result == NULL)
	SysError(ERROR_SCALLOC);

    TRACE(("allocScrnHead %d -> %d -> %p..%p\n", nrow, nrow * size,
	   (void *) result,
	   (char *) result + (nrow * size) - 1));
    return result;
}

/*
 * Return the size of a line's data.
 */
static unsigned
sizeofScrnRow(TScreen *screen, unsigned ncol)
{
    unsigned result;
    unsigned sizeAttribs;
#if OPT_ISO_COLORS
    unsigned sizeColors;
#endif

    (void) screen;

    result = (ncol * (unsigned) sizeof(CharData));	/* ->charData */
    AlignValue(result);

#if OPT_DEC_RECTOPS
    result += (ncol * (unsigned) sizeof(Char));		/* ->charSeen */
    AlignValue(result);
    result += (ncol * (unsigned) sizeof(Char));		/* ->charSets */
    AlignValue(result);
#endif

#if OPT_WIDE_CHARS
    if (screen->wide_chars) {
	result *= (unsigned) (1 + screen->max_combining);
    }
#endif

    sizeAttribs = (ncol * SizeofScrnPtr(attribs));
    AlignValue(sizeAttribs);
    result += sizeAttribs;

#if OPT_ISO_COLORS
    sizeColors = (ncol * SizeofScrnPtr(color));
    AlignValue(sizeColors);
    result += sizeColors;
#endif

    return result;
}

Char *
allocScrnData(TScreen *screen, unsigned nrow, unsigned ncol, Bool bottom)
{
    Char *result = NULL;
    size_t length;

    AlignValue(ncol);
    if (bottom) {
	AddStatusLineRows(nrow);
    }
    length = (nrow * sizeofScrnRow(screen, ncol));
    if (length == 0
	|| (result = (Char *) calloc(length, sizeof(Char))) == NULL)
	  SysError(ERROR_SCALLOC2);

    TRACE(("allocScrnData %ux%u -> %lu -> %p..%p\n",
	   nrow, ncol, (unsigned long) length, result, result + length - 1));
    return result;
}

/*
 * Allocates memory for a 2-dimensional array of chars and returns a pointer
 * thereto.  Each line is formed from a set of char arrays, with an index
 * (i.e., the ScrnBuf type).  The first pointer in the index is reserved for
 * per-line flags, and does not point to data.
 *
 * After the per-line flags, we have a series of pointers to char arrays:  The
 * first one is the actual character array, the second one is the attributes,
 * the third is the foreground and background colors, and the fourth denotes
 * the character set.
 *
 * We store it all as pointers, because of alignment considerations.
 */
ScrnBuf
allocScrnBuf(XtermWidget xw, unsigned nrow, unsigned ncol, Char **addr)
{
    TScreen *screen = TScreenOf(xw);
    ScrnBuf base = NULL;

    if (nrow != 0) {
	base = allocScrnHead(screen, nrow);
	*addr = allocScrnData(screen, nrow, ncol, True);

	setupLineData(screen, base, *addr, nrow, ncol, True);
    }

    TRACE(("allocScrnBuf %dx%d ->%p\n", nrow, ncol, (void *) base));
    return (base);
}

/*
 * Copy line-data from the visible (edit) buffer to the save-lines buffer.
 */
static void
saveEditBufLines(TScreen *screen, unsigned n)
{
    unsigned j;

    TRACE(("...copying %d lines from editBuf to saveBuf\n", n));

    for (j = 0; j < n; ++j) {

	LineData *dst = addScrollback(screen);

	LineData *src = getLineData(screen, (int) j);
	copyLineData(dst, src);
    }
}

/*
 * Copy line-data from the save-lines buffer to the visible (edit) buffer.
 */
static void
unsaveEditBufLines(TScreen *screen, ScrnBuf sb, unsigned n)
{
    unsigned j;

    TRACE(("...copying %d lines from saveBuf to editBuf\n", n));
    for (j = 0; j < n; ++j) {
	int extra = (int) (n - j);
	LineData *dst = (LineData *) scrnHeadAddr(screen, sb, j);

	CLineData *src;

	if (extra > screen->saved_fifo || extra > screen->savelines) {
	    TRACE(("...FIXME: must clear text!\n"));
	    continue;
	}
	src = getScrollback(screen, -extra);

	copyLineData(dst, src);
    }
}

/*
 *  This is called when the screen is resized.
 *  Returns the number of lines the text was moved down (neg for up).
 *  (Return value only necessary with SouthWestGravity.)
 */
static int
Reallocate(XtermWidget xw,
	   ScrnBuf *sbuf,
	   Char **sbufaddr,
	   unsigned nrow,
	   unsigned ncol,
	   unsigned oldrow)
{
    TScreen *screen = TScreenOf(xw);
    ScrnBuf oldBufHead;
    ScrnBuf newBufHead;
    Char *newBufData;
    unsigned minrows;
    Char *oldBufData;
    int move_down = 0, move_up = 0;

    if (sbuf == NULL || *sbuf == NULL) {
	return 0;
    }

    oldBufData = *sbufaddr;

    TRACE(("Reallocate %dx%d -> %dx%d\n", oldrow, MaxCols(screen), nrow, ncol));

    /*
     * realloc sbuf, the pointers to all the lines.
     * If the screen shrinks, remove lines off the top of the buffer
     * if resizeGravity resource says to do so.
     */
    TRACE(("Check move_up, nrow %d vs oldrow %d (resizeGravity %s)\n",
	   nrow, oldrow,
	   BtoS(GravityIsSouthWest(xw))));
    if (GravityIsSouthWest(xw)) {
	if (nrow < oldrow) {
	    /* Remove lines off the top of the buffer if necessary. */
	    move_up = (int) (oldrow - nrow)
		- (TScreenOf(xw)->max_row - TScreenOf(xw)->cur_row);
	    if (move_up < 0)
		move_up = 0;
	    /* Overlapping move here! */
	    TRACE(("move_up %d\n", move_up));
	    if (move_up) {
		ScrnBuf dst = *sbuf;
		unsigned len = (unsigned) ((int) oldrow - move_up);

		TRACE_SCRNBUF("before move_up", screen, dst, oldrow);
		SaveLineData(dst, 0, (size_t) move_up);
		MoveLineData(dst, 0, (size_t) move_up, len);
		RestoreLineData(dst, len, (size_t) move_up);
		TRACE_SCRNBUF("after move_up", screen, dst, oldrow);
	    }
	}
    }
    oldBufHead = *sbuf;
    *sbuf = allocScrnHead(screen, (unsigned) nrow);
    newBufHead = *sbuf;

    /*
     * Create the new buffer space and copy old buffer contents there, line by
     * line.
     */
    newBufData = allocScrnData(screen, nrow, ncol, True);
    *sbufaddr = newBufData;

    minrows = (oldrow < nrow) ? oldrow : nrow;
    if (GravityIsSouthWest(xw)) {
	if (nrow > oldrow) {
	    /* move data down to bottom of expanded screen */
	    move_down = Min((int) (nrow - oldrow), TScreenOf(xw)->savedlines);
	}
    }

    setupLineData(screen, newBufHead, *sbufaddr, nrow, ncol, True);
    extractScrnData(screen, newBufHead, oldBufHead, minrows, 0);

    /* Now free the old data */
    free(oldBufData);
    free(oldBufHead);

    TRACE(("...Reallocate %dx%d ->%p\n", nrow, ncol, (void *) newBufHead));
    return move_down ? move_down : -move_up;	/* convert to rows */
}

#if OPT_WIDE_CHARS
/*
 * This function reallocates memory if changing the number of Buf offsets.
 * The code is based on Reallocate().
 */
static void
ReallocateBufOffsets(XtermWidget xw,
		     ScrnBuf *sbuf,
		     Char **sbufaddr,
		     unsigned nrow,
		     unsigned ncol)
{
    TScreen *screen = TScreenOf(xw);
    unsigned i;
    ScrnBuf newBufHead;
    Char *oldBufData;
    ScrnBuf oldBufHead;

    unsigned old_jump = scrnHeadSize(screen, 1);
    unsigned new_jump;
    unsigned dstCols = ncol;
    LineData *dstPtrs;
    LineData *srcPtrs;

    assert(nrow != 0);
    assert(ncol != 0);

    oldBufData = *sbufaddr;
    oldBufHead = *sbuf;

    /*
     * Allocate a new LineData array, retain the old one until we've copied
     * the data that it points to, as well as non-pointer data, e.g., bufHead.
     *
     * Turn on wide-chars temporarily when constructing pointers, since that is
     * used to decide whether to address the combData[] array, which affects
     * the length of the LineData structure.
     */
    screen->wide_chars = True;

    new_jump = scrnHeadSize(screen, 1);
    newBufHead = allocScrnHead(screen, nrow);
    *sbufaddr = allocScrnData(screen, nrow, ncol, True);
    setupLineData(screen, newBufHead, *sbufaddr, nrow, ncol, True);

    screen->wide_chars = False;

    srcPtrs = (LineData *) oldBufHead;
    dstPtrs = (LineData *) newBufHead;
    for (i = 0; i < nrow; i++) {
	dstPtrs->bufHead = srcPtrs->bufHead;
	ExtractScrnData(attribs);
#if OPT_ISO_COLORS
	ExtractScrnData(color);
#endif
	ExtractScrnData(charData);

	srcPtrs = LineDataAddr(srcPtrs, old_jump);
	dstPtrs = LineDataAddr(dstPtrs, new_jump);
    }

    /* Now free the old data */
    free(oldBufData);
    free(oldBufHead);

    *sbuf = newBufHead;

    TRACE(("ReallocateBufOffsets %dx%d ->%p\n", nrow, ncol, *sbufaddr));
}

/*
 * Allocate a new FIFO index.
 */
static void
ReallocateFifoIndex(XtermWidget xw)
{
    TScreen *screen = TScreenOf(xw);

    if (screen->savelines > 0 && screen->saveBuf_index != NULL) {
	ScrnBuf newBufHead;
	LineData *dstPtrs;
	LineData *srcPtrs;
	unsigned i;
	unsigned old_jump = scrnHeadSize(screen, 1);
	unsigned new_jump;

	screen->wide_chars = True;
	newBufHead = allocScrnHead(screen, (unsigned) screen->savelines);
	new_jump = scrnHeadSize(screen, 1);

	srcPtrs = (LineData *) screen->saveBuf_index;
	dstPtrs = (LineData *) newBufHead;

	for (i = 0; i < (unsigned) screen->savelines; ++i) {
	    memcpy(dstPtrs, srcPtrs, SizeOfLineData);
	    srcPtrs = LineDataAddr(srcPtrs, old_jump);
	    dstPtrs = LineDataAddr(dstPtrs, new_jump);
	}

	screen->wide_chars = False;
	free(screen->saveBuf_index);
	screen->saveBuf_index = newBufHead;
    }
}

/*
 * This function dynamically adds support for wide-characters.
 */
void
ChangeToWide(XtermWidget xw)
{
    TScreen *screen = TScreenOf(xw);

    if (screen->wide_chars)
	return;

    TRACE(("ChangeToWide\n"));
    if (xtermLoadWideFonts(xw, True)) {
	int whichBuf = screen->whichBuf;

	/*
	 * If we're displaying the alternate screen, switch the pointers back
	 * temporarily so ReallocateBufOffsets() will operate on the proper
	 * data in the alternate buffer.
	 */
	if (screen->whichBuf)
	    SwitchBufPtrs(xw, 0);

	ReallocateFifoIndex(xw);

	if (screen->editBuf_index[0]) {
	    ReallocateBufOffsets(xw,
				 &screen->editBuf_index[0],
				 &screen->editBuf_data[0],
				 (unsigned) MaxRows(screen),
				 (unsigned) MaxCols(screen));
	}

	if (screen->editBuf_index[1]) {
	    ReallocateBufOffsets(xw,
				 &screen->editBuf_index[1],
				 &screen->editBuf_data[1],
				 (unsigned) MaxRows(screen),
				 (unsigned) MaxCols(screen));
	}

	screen->wide_chars = True;
	screen->visbuf = VisBuf(screen);

	/*
	 * Switch the pointers back before we start painting on the screen.
	 */
	if (whichBuf)
	    SwitchBufPtrs(xw, whichBuf);

	update_font_utf8_mode();
	SetVTFont(xw, screen->menu_font_number, True, NULL);
    }
    TRACE(("...ChangeToWide\n"));
}
#endif

/*
 * Copy cells, no side-effects.
 */
void
CopyCells(TScreen *screen, LineData *src, LineData *dst, int col, int len, Bool down)
{
    (void) screen;
    (void) down;

    if (len > 0) {
	int n;
	int last = col + len;
#if OPT_WIDE_CHARS
	int fix_l = -1;
	int fix_r = -1;
#endif

	/*
	 * If the copy overwrites a double-width character which has one half
	 * outside the margin, then we will replace both cells with blanks.
	 */
	if_OPT_WIDE_CHARS(screen, {
	    if (col > 0) {
		if (dst->charData[col] == HIDDEN_CHAR) {
		    if (down) {
			Clear2Cell(dst, src, col - 1);
			Clear2Cell(dst, src, col);
		    } else {
			if (src->charData[col] != HIDDEN_CHAR) {
			    Clear2Cell(dst, src, col - 1);
			    Clear2Cell(dst, src, col);
			} else {
			    fix_l = col - 1;
			}
		    }
		} else if (src->charData[col] == HIDDEN_CHAR) {
		    Clear2Cell(dst, src, col - 1);
		    Clear2Cell(dst, src, col);
		    ++col;
		}
	    }
	    if (last < (int) src->lineSize) {
		if (dst->charData[last] == HIDDEN_CHAR) {
		    if (down) {
			Clear2Cell(dst, src, last - 1);
			Clear2Cell(dst, src, last);
		    } else {
			if (src->charData[last] != HIDDEN_CHAR) {
			    Clear2Cell(dst, src, last);
			} else {
			    fix_r = last - 1;
			}
		    }
		} else if (src->charData[last] == HIDDEN_CHAR) {
		    last--;
		    Clear2Cell(dst, src, last);
		}
	    }
	});

	for (n = col; n < last; ++n) {
	    dst->charData[n] = src->charData[n];
	    dst->attribs[n] = src->attribs[n];
	}

	if_OPT_ISO_COLORS(screen, {
	    for (n = col; n < last; ++n) {
		dst->color[n] = src->color[n];
	    }
	});

	if_OPT_WIDE_CHARS(screen, {
	    size_t off;
	    for (n = col; n < last; ++n) {
		for_each_combData(off, src) {
		    dst->combData[off][n] = src->combData[off][n];
		}
	    }
	});

	if_OPT_WIDE_CHARS(screen, {
	    if (fix_l >= 0) {
		Clear2Cell(dst, src, fix_l);
		Clear2Cell(dst, src, fix_l + 1);
	    }
	    if (fix_r >= 0) {
		Clear2Cell(dst, src, fix_r);
		Clear2Cell(dst, src, fix_r + 1);
	    }
	});
    }
}

static void
FillIAttr(IAttr * target, unsigned source, size_t count)
{
    while (count-- != 0) {
	*target++ = (IAttr) source;
    }
}

/*
 * Clear cells, no side-effects.
 */
void
ClearCells(XtermWidget xw, int flags, unsigned len, int row, int col)
{
    if (len != 0) {
	TScreen *screen = TScreenOf(xw);
	LineData *ld;
	unsigned n;

	ld = getLineData(screen, row);

	if (((unsigned) col + len) > ld->lineSize)
	    len = (unsigned) (ld->lineSize - col);

	if_OPT_WIDE_CHARS(screen, {
	    if (((unsigned) col + len) < ld->lineSize &&
		ld->charData[col + (int) len] == HIDDEN_CHAR) {
		len++;
	    }
	    if (col > 0 &&
		ld->charData[col] == HIDDEN_CHAR) {
		len++;
		col--;
	    }
	});

	flags = (int) ((unsigned) flags | TERM_COLOR_FLAGS(xw));

	for (n = 0; n < len; ++n) {
	    if_OPT_DEC_RECTOPS(ld->charSeen[(unsigned) col + n] = ' ');
	    ld->charData[(unsigned) col + n] = (CharData) ' ';
	}

	FillIAttr(ld->attribs + col, (unsigned) flags, (size_t) len);

	if_OPT_ISO_COLORS(screen, {
	    CellColor p = xtermColorPair(xw);
	    for (n = 0; n < len; ++n) {
		ld->color[(unsigned) col + n] = p;
	    }
	});
	if_OPT_WIDE_CHARS(screen, {
	    size_t off;
	    for_each_combData(off, ld) {
		memset(ld->combData[off] + col, 0, (size_t) len * sizeof(CharData));
	    }
	});
    }
}

/*
 * Clear data in the screen-structure (no I/O).
 * Check for wide-character damage as well, clearing the damaged cells.
 */
void
ScrnClearCells(XtermWidget xw, int row, int col, unsigned len)
{
#if OPT_WIDE_CHARS
    TScreen *screen = TScreenOf(xw);
#endif
    int flags = 0;

    if_OPT_WIDE_CHARS(screen, {
	int kl;
	int kr;

	if (DamagedCells(screen, len, &kl, &kr, row, col)
	    && kr >= kl) {
	    ClearCells(xw, flags, (unsigned) (kr - kl + 1), row, kl);
	}
    });
    ClearCells(xw, flags, len, row, col);
}

/*
 * Disown the selection and repaint the area that is highlighted so it is no
 * longer highlighted.
 */
void
ScrnDisownSelection(XtermWidget xw)
{
    if (ScrnHaveSelection(TScreenOf(xw))) {
	TRACE(("ScrnDisownSelection\n"));
	if (TScreenOf(xw)->keepSelection) {
	    UnhiliteSelection(xw);
	} else {
	    DisownSelection(xw);
	}
    }
}

/*
 * Writes str into buf at screen's current row and column.  Characters are set
 * to match flags.
 */
void
ScrnWriteText(XtermWidget xw,
	      Cardinal offset,
	      Cardinal length,
	      unsigned flags,
	      CellColor cur_fg_bg)
{
    IChar *str = xw->work.write_text + offset;
    TScreen *screen = TScreenOf(xw);
    LineData *ld;
    IAttr *attrs;
    int avail = MaxCols(screen) - screen->cur_col;
    IChar *chars;
#if OPT_DEC_RECTOPS
    Char *seens;
#endif
#if OPT_WIDE_CHARS
    IChar starcol1;
#endif
    unsigned n;
    unsigned real_width = visual_width(str, length);

    (void) cur_fg_bg;		/* quiet compiler warnings when unused */

    if (real_width + (unsigned) screen->cur_col > (unsigned) MaxCols(screen)) {
	real_width = (unsigned) (MaxCols(screen) - screen->cur_col);
    }

    if (avail <= 0)
	return;
    if (length > (unsigned) avail)
	length = (unsigned) avail;
    if (length == 0 || real_width == 0)
	return;

    ld = getLineData(screen, screen->cur_row);

    if_OPT_DEC_RECTOPS(seens = ld->charSeen + screen->cur_col);
    chars = ld->charData + screen->cur_col;
    attrs = ld->attribs + screen->cur_col;

#if OPT_WIDE_CHARS
    starcol1 = *chars;
#endif

    /*
     * Copy the string onto the line,
     * writing blanks if we're writing invisible text.
     */
    for (n = 0; n < length; ++n) {
#if OPT_DEC_RECTOPS
	if (xw->work.write_sums != NULL) {
	    ld->charSeen[screen->cur_col + (int) n] = xw->work.buffer_sums[n];
	    ld->charSets[screen->cur_col + (int) n] = xw->work.buffer_sets[n];
	} else {
	    seens[n] = (str[n] < 32
#if OPT_WIDE_CHARS
			|| str[n] > 255
#endif
		)
		? ANSI_ESC
		: (Char) str[n];
	}
#endif /* OPT_DEC_RECTOPS */
	chars[n] = str[n];
    }

#if OPT_BLINK_TEXT
    if ((flags & BLINK) && !(screen->blink_as_bold)) {
	LineSetBlinked(ld);
    }
#endif

    if_OPT_WIDE_CHARS(screen, {

	if (real_width != length) {
	    IChar *char1 = chars;
	    if (screen->cur_col
		&& starcol1 == HIDDEN_CHAR
		&& isWide((int) char1[-1])) {
		char1[-1] = (CharData) ' ';
	    }
	    /* if we are overwriting the right hand half of a
	       wide character, make the other half vanish */
	    while (length) {
		int ch = (int) str[0];

		*char1++ = *str++;
		length--;

		if (isWide(ch)) {
		    *char1++ = (CharData) HIDDEN_CHAR;
		}
	    }

	    if (*char1 == HIDDEN_CHAR
		&& char1[-1] == HIDDEN_CHAR) {
		*char1 = (CharData) ' ';
	    }
	    /* if we are overwriting the left hand half of a
	       wide character, make the other half vanish */
	} else {
	    if (screen->cur_col
		&& starcol1 == HIDDEN_CHAR
		&& isWide((int) chars[-1])) {
		chars[-1] = (CharData) ' ';
	    }
	    /* if we are overwriting the right hand half of a
	       wide character, make the other half vanish */
	    if (chars[length] == HIDDEN_CHAR
		&& isWide((int) chars[length - 1])) {
		chars[length] = (CharData) ' ';
	    }
	}
    });

    flags &= ATTRIBUTES;
    flags |= CHARDRAWN;
    FillIAttr(attrs, flags, (size_t) real_width);

    if_OPT_WIDE_CHARS(screen, {
	size_t off;
	for_each_combData(off, ld) {
	    memset(ld->combData[off] + screen->cur_col,
		   0,
		   real_width * sizeof(CharData));
	}
    });
    if_OPT_ISO_COLORS(screen, {
	unsigned j;
	for (j = 0; j < real_width; ++j)
	    ld->color[screen->cur_col + (int) j] = cur_fg_bg;
    });

#if OPT_WIDE_CHARS
    screen->last_written_col = screen->cur_col + (int) real_width - 1;
    screen->last_written_row = screen->cur_row;
#endif

    chararea_clear_displayed_graphics(screen,
				      screen->cur_col,
				      screen->cur_row,
				      (int) real_width, 1);

    if_OPT_XMC_GLITCH(screen, {
	Resolve_XMC(xw);
    });

    return;
}

/*
 * Saves pointers to the n lines beginning at sb + where, and clears the lines
 */
static void
ScrnClearLines(XtermWidget xw, ScrnBuf sb, int where, unsigned n, unsigned size)
{
    TScreen *screen = TScreenOf(xw);
    ScrnPtr *base;
    unsigned jump = scrnHeadSize(screen, 1);
    unsigned i;
    LineData *work;
    unsigned flags = TERM_COLOR_FLAGS(xw);
#if OPT_ISO_COLORS
    unsigned j;
#endif

    TRACE(("ScrnClearLines(%s:where %d, n %d, size %d)\n",
	   (sb == screen->saveBuf_index) ? "save" : "edit",
	   where, n, size));

    assert((int) n > 0);
    assert(size != 0);

    /* save n lines at where */
    SaveLineData(sb, (unsigned) where, (size_t) n);

    /* clear contents of old rows */
    base = screen->save_ptr;
    for (i = 0; i < n; ++i) {
	work = (LineData *) base;
	work->bufHead = 0;
#if OPT_DEC_CHRSET
	SetLineDblCS(work, 0);
#endif

	memset(work->charData, 0, size * sizeof(CharData));
	if (TERM_COLOR_FLAGS(xw)) {
	    FillIAttr(work->attribs, flags, (size_t) size);
#if OPT_ISO_COLORS
	    {
		CellColor p = xtermColorPair(xw);
		for (j = 0; j < size; ++j) {
		    work->color[j] = p;
		}
	    }
#endif
	} else {
	    FillIAttr(work->attribs, 0, (size_t) size);
#if OPT_ISO_COLORS
	    memset(work->color, 0, size * sizeof(work->color[0]));
#endif
	}
	if_OPT_DEC_RECTOPS({
	    memset(work->charSeen, 0, size * sizeof(Char));
	    memset(work->charSets, 0, size * sizeof(work->charSets[0]));
	});
#if OPT_WIDE_CHARS
	if (screen->wide_chars) {
	    size_t off;

	    for (off = 0; off < work->combSize; ++off) {
		memset(work->combData[off], 0, size * sizeof(CharData));
	    }
	}
#endif
	base = ScrnBufAddr(base, jump);
    }

    /* FIXME: this looks wrong -- rcombs */
    chararea_clear_displayed_graphics(screen,
				      where + screen->savelines,
				      0,
				      screen->max_col + 1,
				      (int) n);
}

/*
 * We're always ensured of having a visible buffer, but may not have saved
 * lines.  Check the pointer that's sure to work.
 */

#define OkAllocBuf(screen) (screen->editBuf_index[0] != NULL)

void
ScrnAllocBuf(XtermWidget xw)
{
    TScreen *screen = TScreenOf(xw);

    if (!OkAllocBuf(screen)) {
	int nrows = MaxRows(screen);

	TRACE(("ScrnAllocBuf %dx%d (%d)\n",
	       nrows, MaxCols(screen), screen->savelines));

	if (screen->savelines != 0) {
	    /* for FIFO, we only need space for the index - addScrollback inits */
	    screen->saveBuf_index = allocScrnHead(screen,
						  (unsigned) (screen->savelines));
	} else {
	    screen->saveBuf_index = NULL;
	}
	screen->editBuf_index[0] = allocScrnBuf(xw,
						(unsigned) nrows,
						(unsigned) MaxCols(screen),
						&screen->editBuf_data[0]);
	screen->visbuf = VisBuf(screen);
    }
    return;
}

size_t
ScrnPointers(TScreen *screen, size_t len)
{
    size_t result = scrnHeadSize(screen, (unsigned) len);

    if (result > screen->save_len) {
	if (screen->save_len)
	    screen->save_ptr = (ScrnPtr *) realloc(screen->save_ptr, result);
	else
	    screen->save_ptr = (ScrnPtr *) malloc(result);
	screen->save_len = len;
	if (screen->save_ptr == NULL)
	    SysError(ERROR_SAVE_PTR);
    }
    TRACE2(("ScrnPointers %ld ->%p\n", (long) len, screen->save_ptr));
    return result;
}

/*
 * Inserts n blank lines at sb + where, treating last as a bottom margin.
 */
void
ScrnInsertLine(XtermWidget xw, ScrnBuf sb, int last, int where, unsigned n)
{
    TScreen *screen = TScreenOf(xw);
    unsigned size = (unsigned) MaxCols(screen);

    TRACE(("ScrnInsertLine(last %d, where %d, n %d, size %d)\n",
	   last, where, n, size));

    assert(where >= 0);
    assert(last >= where);

    assert((int) n > 0);
    assert(size != 0);

    /* save n lines at bottom */
    ScrnClearLines(xw, sb, (last -= (int) n - 1), n, size);
    if (last < 0) {
	TRACE(("...remainder of screen is blank\n"));
	return;
    }

    /*
     * WARNING, overlapping copy operation.  Move down lines (pointers).
     *
     *   +----|---------|--------+
     *
     * is copied in the array to:
     *
     *   +--------|---------|----+
     */
    assert(last >= where);
    /*
     * This will never shift from the saveBuf to editBuf, so there is no need
     * to handle that case.
     */
    MoveLineData(sb,
		 (unsigned) (where + (int) n),
		 (unsigned) where,
		 (unsigned) (last - where));

    /* reuse storage for new lines at where */
    RestoreLineData(sb, (unsigned) where, n);
}

/*
 * Deletes n lines at sb + where, treating last as a bottom margin.
 */
void
ScrnDeleteLine(XtermWidget xw, ScrnBuf sb, int last, int where, unsigned n)
{
    TScreen *screen = TScreenOf(xw);
    unsigned size = (unsigned) MaxCols(screen);

    TRACE(("ScrnDeleteLine(%s:last %d, where %d, n %d, size %d)\n",
	   (sb == screen->saveBuf_index) ? "save" : "edit",
	   last, where, n, size));

    assert(where >= 0);
    assert(last >= where + (int) n - 1);

    assert((int) n > 0);
    assert(size != 0);

    /* move up lines */
    last -= ((int) n - 1);

    if (inSaveBuf(screen, sb, where)) {

	/* we shouldn't be editing the saveBuf, only scroll into it */
	assert(last >= screen->savelines);

	if (sb != NULL) {
	    /* copy lines from editBuf to saveBuf (allocating as we go...) */
	    saveEditBufLines(screen, n);
	}

	/* adjust variables to fall-thru into changes only to editBuf */
	TRACE(("...adjusting variables, to work on editBuf alone\n"));
	last -= screen->savelines;
	where = 0;
	sb = screen->visbuf;
    }

    /*
     * Scroll the visible buffer (editBuf).
     */
    ScrnClearLines(xw, sb, where, n, size);

    MoveLineData(sb,
		 (unsigned) where,
		 (unsigned) (where + (int) n),
		 (size_t) (last - where));

    /* reuse storage for new bottom lines */
    RestoreLineData(sb, (unsigned) last, n);
}

/*
 * Inserts n blanks in screen at current row, col.  Size is the size of each
 * row.
 */
void
ScrnInsertChar(XtermWidget xw, unsigned n)
{
#define MemMove(data) \
    	for (j = last; j >= (col + (int) n); --j) \
	    data[j] = data[j - (int) n]

    TScreen *screen = TScreenOf(xw);
    int first = ScrnLeftMargin(xw);
    int last = ScrnRightMargin(xw);
    int row = screen->cur_row;
    int col = screen->cur_col;
    LineData *ld;

    if (col < first || col > last) {
	TRACE(("ScrnInsertChar - col %d outside [%d..%d]\n", col, first, last));
	return;
    } else if (last < (col + (int) n)) {
	n = (unsigned) (last + 1 - col);
    }

    assert(screen->cur_col >= 0);
    assert(screen->cur_row >= 0);
    assert((int) n >= 0);
    assert((last + 1) >= (int) n);

    if_OPT_WIDE_CHARS(screen, {
	int xx = screen->cur_row;
	int kl;
	int kr = screen->cur_col;
	if (DamagedCells(screen, n, &kl, (int *) 0, xx, kr) && kr > kl) {
	    ClearCells(xw, 0, (unsigned) (kr - kl + 1), row, kl);
	}
	kr = last - (int) n + 1;
	if (DamagedCells(screen, n, &kl, (int *) 0, xx, kr) && kr > kl) {
	    ClearCells(xw, 0, (unsigned) (kr - kl + 1), row, kl);
	}
    });

    if ((ld = getLineData(screen, row)) != NULL) {
	int j;

	MemMove(ld->charData);
	MemMove(ld->attribs);

	if_OPT_ISO_COLORS(screen, {
	    MemMove(ld->color);
	});
	if_OPT_WIDE_CHARS(screen, {
	    size_t off;
	    for_each_combData(off, ld) {
		MemMove(ld->combData[off]);
	    }
	});
    }
    ClearCells(xw, CHARDRAWN, n, row, col);

#undef MemMove
}

/*
 * Deletes n characters at current row, col.
 */
void
ScrnDeleteChar(XtermWidget xw, unsigned n)
{
#define MemMove(data) \
    	for (j = col; j < last - (int) n; ++j) \
	    data[j] = data[j + (int) n]

    TScreen *screen = TScreenOf(xw);
    int first = ScrnLeftMargin(xw);
    int last = ScrnRightMargin(xw) + 1;
    int row = screen->cur_row;
    int col = screen->cur_col;
    LineData *ld;

    if (col < first || col > last) {
	TRACE(("ScrnDeleteChar - col %d outside [%d..%d]\n", col, first, last));
	return;
    } else if (last <= (col + (int) n)) {
	n = (unsigned) (last - col);
    }

    assert(screen->cur_col >= 0);
    assert(screen->cur_row >= 0);
    assert((int) n >= 0);
    assert(last >= (int) n);

    if_OPT_WIDE_CHARS(screen, {
	int kl;
	int kr;
	if (DamagedCells(screen, n, &kl, &kr,
			 screen->cur_row,
			 screen->cur_col))
	    ClearCells(xw, 0, (unsigned) (kr - kl + 1), row, kl);
    });

    if ((ld = getLineData(screen, row)) != NULL) {
	int j;

	MemMove(ld->charData);
	MemMove(ld->attribs);

	if_OPT_ISO_COLORS(screen, {
	    MemMove(ld->color);
	});
	if_OPT_WIDE_CHARS(screen, {
	    size_t off;
	    for_each_combData(off, ld) {
		MemMove(ld->combData[off]);
	    }
	});
	LineClrWrapped(ld);
	ShowWrapMarks(xw, row, ld);
    }
    ClearCells(xw, 0, n, row, (last - (int) n));

#undef MemMove
}

#define WhichMarkGC(set) (set ? 1 : 0)
#define WhichMarkColor(set) T_COLOR(screen, (set ? TEXT_CURSOR : TEXT_BG))

void
FreeMarkGCs(XtermWidget xw)
{
    TScreen *const screen = TScreenOf(xw);
    Display *const display = screen->display;
    VTwin *vwin = WhichVWin(screen);
    int which;

    for (which = 0; which < 2; ++which) {
	if (vwin->marker_gc[which] != NULL) {
	    XFreeGC(display, vwin->marker_gc[which]);
	    vwin->marker_gc[which] = NULL;
	}
    }
}

static GC
MakeMarkGC(XtermWidget xw, Bool set)
{
    TScreen *const screen = TScreenOf(xw);
    VTwin *vwin = WhichVWin(screen);
    int which = WhichMarkGC(set);

    if (vwin->marker_gc[which] == NULL) {
	Display *const display = screen->display;
	Window const drawable = VDrawable(screen);
	XGCValues xgcv;
	XtGCMask mask = GCForeground;

	memset(&xgcv, 0, sizeof(xgcv));
	xgcv.foreground = WhichMarkColor(set);
	vwin->marker_gc[which] = XCreateGC(display,
					   drawable,
					   mask,
					   &xgcv);
    }
    return vwin->marker_gc[which];
}

/*
 * This is useful for debugging both xterm and applications that may manipulate
 * its line-wrapping state.
 */
void
ShowWrapMarks(XtermWidget xw, int row, CLineData *ld)
{
    TScreen *screen = TScreenOf(xw);
    if (screen->show_wrap_marks && row >= 0 && row <= screen->max_row) {
	Bool set = (Bool) LineTstWrapped(ld);
	int y = row * FontHeight(screen) + screen->border;
	int x = LineCursorX(screen, ld, screen->max_col + 1);

	TRACE2(("ShowWrapMarks %d:%s\n", row, BtoS(set)));

	XFillRectangle(screen->display,
		       VDrawable(screen),
		       MakeMarkGC(xw, set),
		       x, y,
		       (unsigned) screen->border,
		       (unsigned) FontHeight(screen));
    }
}

#if OPT_BLOCK_SELECT
/*
 * Return the start and end cols of a block selection
 */
static void
blockSelectBounds(TScreen *screen,
		  int *start,
		  int *end)
{
    assert(screen->blockSelecting);
    if (screen->startH.col < screen->endH.col) {
	*start = screen->startH.col;
	*end = screen->endH.col;
    } else {
	*start = screen->endH.col;
	*end = screen->startH.col;
    }
}

/*
 * Return 1 if any part of [col, maxcol] intersects with the selection.
 */
static int
intersectsSelection(TScreen *screen,
		    int row,
		    int col,
		    int maxcol)
{
    if (screen->blockSelecting) {
	int start, end;
	blockSelectBounds(screen, &start, &end);
	return start != end
	    && (row >= screen->startH.row && row <= screen->endH.row)
	    && ((start >= col && start <= maxcol)
		|| (end > col && end < maxcol)) ? 1 : 0;
    }
    return !(row < screen->startH.row || row > screen->endH.row
	     || (row == screen->startH.row && maxcol < screen->startH.col)
	     || (row == screen->endH.row && col >= screen->endH.col)) ? 1 : 0;
}

/*
 * If there are any parts of [col, maxcol] not in the selection,
 * invoke ScrnRefresh on them, then adjust [col, maxcol] to be fully
 * inside the selection. The intent is to optimize the loop at the
 * end of ScrnRefresh, so that we are painting either all highlighted
 * or all unhighlighted cells.
 */
static void
recurseForNotSelectedAndAdjust(XtermWidget xw,
			       int row,
			       int *col,
			       int *maxcol,
			       int force)
{
    TScreen *screen = TScreenOf(xw);
    if (screen->blockSelecting) {
	int start, end;
	blockSelectBounds(screen, &start, &end);
	if (*col < start) {
	    ScrnRefresh(xw, row, *col, 1, start - *col, force);
	    *col = start;
	}
	if (*maxcol >= end) {
	    ScrnRefresh(xw, row, end, 1, *maxcol - end + 1, force);
	    *maxcol = end - 1;
	}
    } else {
	if (row == screen->startH.row && *col < screen->startH.col) {
	    ScrnRefresh(xw, row, *col, 1, screen->startH.col - *col,
			force);
	    *col = screen->startH.col;
	}
	if (row == screen->endH.row && *maxcol >= screen->endH.col) {
	    ScrnRefresh(xw, row, screen->endH.col, 1,
			*maxcol - screen->endH.col + 1, force);
	    *maxcol = screen->endH.col - 1;
	}
    }
}
#else
#define intersectsSelection(screen, row, col, maxcol) \
	((row >= screen->startH.row && row <= screen->endH.row) \
	 && (row != screen->startH.row || maxcol >= screen->startH.col) \
	 && (row != screen->endH.row || col < screen->endH.col))
#endif /* OPT_BLOCK_SELECT */

/*
 * Repaints the area enclosed by the parameters.
 * Requires: (toprow, leftcol), (toprow + nrows, leftcol + ncols) are
 *	     coordinates of characters in screen;
 *	     nrows and ncols positive.
 *	     all dimensions are based on single-characters.
 */
void
ScrnRefresh(XtermWidget xw,
	    int toprow,
	    int leftcol,
	    int nrows,
	    int ncols,
	    Bool force)		/* ... leading/trailing spaces */
{
    TScreen *screen = TScreenOf(xw);
    XTermDraw params;
    CLineData *ld;
    int y = toprow * FontHeight(screen) + screen->border;
    int row;
    int maxrow = toprow + nrows - 1;
    int scrollamt = screen->scroll_amt;
    unsigned gc_changes = 0;
#ifdef __CYGWIN__
    static char first_time = 1;
#endif
    static int recurse = 0;
#if OPT_WIDE_ATTRS
    unsigned old_attrs = xw->flags;
#endif

    TRACE(("ScrnRefresh top %d (%d,%d) - (%d,%d)%s " TRACE_L "\n",
	   screen->topline, toprow, leftcol,
	   nrows, ncols,
	   force ? " force" : ""));

#if OPT_STATUS_LINE
    if (!recurse && (maxrow == screen->max_row) && IsStatusShown(screen)) {
	TRACE(("...allow a row for status-line\n"));
	nrows += StatusLineRows;
	maxrow += StatusLineRows;
    }
#endif
    (void) recurse;
    ++recurse;

    if (screen->cursorp.col >= leftcol
	&& screen->cursorp.col <= (leftcol + ncols - 1)
	&& screen->cursorp.row >= ROW2INX(screen, toprow)
	&& screen->cursorp.row <= ROW2INX(screen, maxrow))
	screen->cursor_state = OFF;

    for (row = toprow; row <= maxrow; y += FontHeight(screen), row++) {
#if OPT_ISO_COLORS
	CellColor *fb = NULL;
#define ColorOf(col) (fb ? fb[col] : initCColor)
#endif
#if OPT_WIDE_CHARS
	int wideness = 0;
#endif
#define BLANK_CEL(cell) (chars[cell] == ' ')
	IChar *chars;
	const IAttr *attrs;
	int col = leftcol;
	int maxcol = leftcol + ncols - 1;
	int hi_col = maxcol;
	int lastind;
	unsigned flags;
	unsigned test;
	CellColor fg_bg = initCColor;
	Pixel fg = 0, bg = 0;
	int x;
	GC gc;
	Bool hilite;

	(void) fg;
	(void) bg;
#if !OPT_ISO_COLORS
	fg_bg = 0;
#endif

	if (row < screen->top_marg || row > screen->bot_marg)
	    lastind = row;
	else
	    lastind = row - scrollamt;

	if (lastind < 0 || lastind > LastRowNumber(screen))
	    continue;

	TRACE2(("ScrnRefresh row=%d lastind=%d ->%d\n",
		row, lastind, ROW2INX(screen, lastind)));

	if ((ld = getLineData(screen, ROW2INX(screen, lastind))) == NULL
	    || ld->charData == NULL
	    || ld->attribs == NULL) {
	    break;
	}

	ShowWrapMarks(xw, lastind, ld);

	if (maxcol >= (int) ld->lineSize) {
	    maxcol = ld->lineSize - 1;
	    hi_col = maxcol;
	}

	chars = ld->charData;
	attrs = ld->attribs;

	if_OPT_WIDE_CHARS(screen, {
	    /* This fixes an infinite recursion bug, that leads
	       to display anomalies. It seems to be related to
	       problems with the selection. */
	    if (recurse < 3) {
		/* adjust to redraw all of a widechar if we just wanted
		   to draw the right hand half */
		if (leftcol > 0 &&
		    chars[leftcol] == HIDDEN_CHAR &&
		    isWide((int) chars[leftcol - 1])) {
		    leftcol--;
		    ncols++;
		    col = leftcol;
		}
	    } else {
		xtermWarning("Unexpected recursion drawing hidden characters.\n");
	    }
	});

	if (!intersectsSelection(screen, row, col, maxcol)) {
#if OPT_DEC_CHRSET
	    /*
	     * Temporarily change dimensions to double-sized characters so
	     * we can reuse the recursion on this function.
	     */
	    if (CSET_DOUBLE(GetLineDblCS(ld))) {
		col /= 2;
		maxcol /= 2;
	    }
#endif
	    /*
	     * If row does not intersect selection; don't hilite blanks
	     * unless block selecting.
	     */
	    if (!force
#if OPT_BLOCK_SELECT
		&& !screen->blockSelecting
#endif
		) {
		while (col <= maxcol && (attrs[col] & ~BOLD) == 0 &&
		       BLANK_CEL(col))
		    col++;

		while (col <= maxcol && (attrs[maxcol] & ~BOLD) == 0 &&
		       BLANK_CEL(maxcol))
		    maxcol--;
	    }
#if OPT_DEC_CHRSET
	    if (CSET_DOUBLE(GetLineDblCS(ld))) {
		col *= 2;
		maxcol *= 2;
	    }
#endif
	    hilite = False;
	} else {
#if OPT_BLOCK_SELECT
	    /* row intersects selection; recurse for the unselected pieces
	     * of col to maxcol, then adjust col and maxcol so that they are
	     * strictly inside the selection.
	     */
	    recurseForNotSelectedAndAdjust(xw, row, &col, &maxcol, force);
#else
	    /* row intersects selection; split into pieces of single type */
	    if (row == screen->startH.row && col < screen->startH.col) {
		ScrnRefresh(xw, row, col, 1, screen->startH.col - col,
			    force);
		col = screen->startH.col;
	    }
	    if (row == screen->endH.row && maxcol >= screen->endH.col) {
		ScrnRefresh(xw, row, screen->endH.col, 1,
			    maxcol - screen->endH.col + 1, force);
		maxcol = screen->endH.col - 1;
	    }
#endif

	    /*
	     * If we're highlighting because the user is doing cut/paste,
	     * trim the trailing blanks from the highlighted region so we're
	     * showing the actual extent of the text that'll be cut.  If
	     * we're selecting a blank line, we'll highlight one column
	     * anyway.
	     *
	     * We don't do this if the mouse-hilite mode is set because that
	     * would be too confusing.  The same applies to block select mode.
	     *
	     * The default if the highlightSelection resource isn't set will
	     * highlight the whole width of the terminal, which is easy to
	     * see, but harder to use (because trailing blanks aren't as
	     * apparent).
	     */
	    if (screen->highlight_selection
#if OPT_BLOCK_SELECT
		&& !screen->blockSelecting
#endif
		&& screen->send_mouse_pos != VT200_HIGHLIGHT_MOUSE) {
		hi_col = screen->max_col;
		while (hi_col > 0 && !(attrs[hi_col] & CHARDRAWN))
		    hi_col--;
	    }

	    /* remaining piece should be hilited */
	    hilite = True;
	}

	if (col > maxcol)
	    continue;

	/*
	 * Go back to double-sized character dimensions if the line has
	 * double-width characters.  Note that 'hi_col' is already in the
	 * right units.
	 */
	if_OPT_DEC_CHRSET({
	    if (CSET_DOUBLE(GetLineDblCS(ld))) {
		col /= 2;
		maxcol /= 2;
	    }
	});

	flags = attrs[col];

	if_OPT_WIDE_CHARS(screen, {
	    wideness = isWide((int) chars[col]);
	});

	if_OPT_ISO_COLORS(screen, {
	    fb = ld->color;
	    fg_bg = ColorOf(col);
	    fg = extract_fg(xw, fg_bg, flags);
	    bg = extract_bg(xw, fg_bg, flags);
	});
#if OPT_WIDE_ATTRS
	old_attrs = xtermUpdateItalics(xw, flags, old_attrs);
#endif
	gc = updatedXtermGC(xw, flags, fg_bg, hilite);
	gc_changes |= (flags & (FG_COLOR | BG_COLOR));

	x = LineCursorX(screen, ld, col);
	lastind = col;

	for (; col <= maxcol; col++) {
	    if (
#if OPT_WIDE_CHARS
		   (chars[col] != HIDDEN_CHAR) &&
#endif
		   ((attrs[col] != flags)
		    || (hilite && (col > hi_col))
#if OPT_ISO_COLORS
		    || ((flags & FG_COLOR)
			&& (extract_fg(xw, ColorOf(col), attrs[col]) != fg))
		    || ((flags & BG_COLOR)
			&& (extract_bg(xw, ColorOf(col), attrs[col]) != bg))
#endif
#if OPT_WIDE_CHARS
		    || (isWide((int) chars[col]) != wideness)
#endif
		   )
		) {
		assert(col >= lastind);
		TRACE(("ScrnRefresh looping drawXtermText %d..%d:%s\n",
		       lastind, col,
		       visibleIChars((&chars[lastind]),
				     (unsigned) (col - lastind))));

		test = flags;
		checkVeryBoldColors(test, fg);

		/* *INDENT-EQLS* */
		params.xw          = xw;
		params.attr_flags  = (test & DRAWX_MASK);
		params.draw_flags  = 0;
		params.this_chrset = GetLineDblCS(ld);
		params.real_chrset = CSET_SWL;
		params.on_wide     = 0;

		x = drawXtermText(&params,
				  gc, x, y,
				  &chars[lastind],
				  (unsigned) (col - lastind));

		if_OPT_WIDE_CHARS(screen, {
		    int i;
		    size_t off;

		    params.draw_flags = NOBACKGROUND;

		    for_each_combData(off, ld) {
			IChar *com_off = ld->combData[off];

			for (i = lastind; i < col; i++) {
			    int my_x = LineCursorX(screen, ld, i);
			    IChar base = chars[i];

			    if ((params.on_wide = isWide((int) base)) != 0)
				my_x = LineCursorX(screen, ld, i - 1);

			    if (com_off[i] != 0)
				drawXtermText(&params,
					      gc, my_x, y,
					      com_off + i,
					      1);
			}
		    }
		});

		resetXtermGC(xw, flags, hilite);

		lastind = col;

		if (hilite && (col > hi_col))
		    hilite = False;

		flags = attrs[col];
		if_OPT_ISO_COLORS(screen, {
		    fg_bg = ColorOf(col);
		    fg = extract_fg(xw, fg_bg, flags);
		    bg = extract_bg(xw, fg_bg, flags);
		});
		if_OPT_WIDE_CHARS(screen, {
		    wideness = isWide((int) chars[col]);
		});

#if OPT_WIDE_ATTRS
		old_attrs = xtermUpdateItalics(xw, flags, old_attrs);
#endif
		gc = updatedXtermGC(xw, flags, fg_bg, hilite);
		gc_changes |= (flags & (FG_COLOR | BG_COLOR));
	    }

	    if (chars[col] == 0) {
		chars[col] = ' ';
	    }
	}

	assert(col >= lastind);
	TRACE(("ScrnRefresh calling drawXtermText %d..%d:%s\n",
	       lastind, col,
	       visibleIChars(&chars[lastind], (unsigned) (col - lastind))));

	test = flags;
	checkVeryBoldColors(test, fg);

	/* *INDENT-EQLS* */
	params.xw          = xw;
	params.attr_flags  = (test & DRAWX_MASK);
	params.draw_flags  = 0;
	params.this_chrset = GetLineDblCS(ld);
	params.real_chrset = CSET_SWL;
	params.on_wide     = 0;

	drawXtermText(&params,
		      gc, x, y,
		      &chars[lastind],
		      (unsigned) (col - lastind));

	if_OPT_WIDE_CHARS(screen, {
	    int i;
	    size_t off;

	    params.draw_flags = NOBACKGROUND;

	    for_each_combData(off, ld) {
		IChar *com_off = ld->combData[off];

		for (i = lastind; i < col; i++) {
		    int my_x = LineCursorX(screen, ld, i);
		    int base = (int) chars[i];

		    if ((params.on_wide = isWide(base)) != 0)
			my_x = LineCursorX(screen, ld, i - 1);

		    if (com_off[i] != 0)
			drawXtermText(&params,
				      gc, my_x, y,
				      com_off + i,
				      1);
		}
	    }
	});

	resetXtermGC(xw, flags, hilite);
    }

    refresh_displayed_graphics(xw, leftcol, toprow, ncols, nrows);

    /*
     * If we're in color mode, reset the various GC's to the current
     * screen foreground and background so that other functions (e.g.,
     * ClearRight) will get the correct colors.
     */
#if OPT_WIDE_ATTRS
    (void) xtermUpdateItalics(xw, xw->flags, old_attrs);
#endif
    if_OPT_ISO_COLORS(screen, {
	if (gc_changes & FG_COLOR)
	    SGR_Foreground(xw, xw->cur_foreground);
	if (gc_changes & BG_COLOR)
	    SGR_Background(xw, xw->cur_background);
    });
    (void) gc_changes;

#if defined(__CYGWIN__) && defined(TIOCSWINSZ)
    if (first_time == 1) {
	first_time = 0;
	update_winsize(screen, nrows, ncols, xw->core.height, xw->core.width);
    }
#endif
    recurse--;

    TRACE((TRACE_R " ScrnRefresh\n"));
    return;
}

/*
 * Call this wrapper to ScrnRefresh() when the data has changed.  If the
 * refresh region overlaps the selection, we will release the primary selection.
 */
void
ScrnUpdate(XtermWidget xw,
	   int toprow,
	   int leftcol,
	   int nrows,
	   int ncols,
	   Bool force)		/* ... leading/trailing spaces */
{
    TScreen *screen = TScreenOf(xw);

    if (ScrnHaveSelection(screen)
	&& (toprow <= screen->endH.row)
	&& (toprow + nrows - 1 >= screen->startH.row)) {
	ScrnDisownSelection(xw);
    }
    ScrnRefresh(xw, toprow, leftcol, nrows, ncols, force);
}

/*
 * Sets the rows first though last of the buffer of screen to spaces.
 * Requires first <= last; first, last are rows of screen->buf.
 */
void
ClearBufRows(XtermWidget xw,
	     int first,
	     int last)
{
    TScreen *screen = TScreenOf(xw);
    unsigned len = (unsigned) MaxCols(screen);
    int row;

    TRACE(("ClearBufRows %d..%d\n", first, last));
    for (row = first; row <= last; row++) {
	LineData *ld = getLineData(screen, row);
	if (ld != NULL) {
	    if_OPT_DEC_CHRSET({
		/* clearing the whole row resets the doublesize characters */
		SetLineDblCS(ld, CSET_SWL);
	    });
	    LineClrWrapped(ld);
	    ShowWrapMarks(xw, row, ld);
	    ClearCells(xw, 0, len, row, 0);
	}
    }
}

#if OPT_STATUS_LINE
static LineData *
freeLineData(TScreen *screen, LineData *source)
{
    (void) screen;
    if (source != NULL) {
	free(source->attribs);
	free(source->charData);
#if OPT_ISO_COLORS
	free(source->color);
#endif
#if OPT_WIDE_CHARS
	if_OPT_WIDE_CHARS(screen, {
	    size_t off;
	    for_each_combData(off, source) {
		free(source->combData[off]);
	    }
	});
#endif
	free(source);
	source = NULL;
    }
    return source;
}

#define ALLOC_IT(field) \
    if (result != NULL) { \
	if ((result->field = calloc((size_t) ncol, sizeof(*result->field))) == NULL) { \
	    result = freeLineData(screen, result); \
	} \
    }

/*
 * Allocate a temporary LineData structure, which is not part of the index.
 */
static LineData *
allocLineData(TScreen *screen, LineData *source)
{
    LineData *result = NULL;
    Dimension ncol = (Dimension) (source->lineSize + 1);
    size_t size = sizeof(*result);
#if OPT_WIDE_CHARS
    size += source->combSize * sizeof(result->combData[0]);
#endif
    if ((result = calloc((size_t) 1, size)) != NULL) {
	result->lineSize = ncol;
	ALLOC_IT(attribs);
#if OPT_ISO_COLORS
	ALLOC_IT(color);
#endif
#if OPT_DEC_RECTOPS
	ALLOC_IT(charSeen);
	ALLOC_IT(charData);
#endif
#if OPT_WIDE_CHARS
	ALLOC_IT(charSets);
	if_OPT_WIDE_CHARS(screen, {
	    size_t off;
	    for_each_combData(off, source) {
		ALLOC_IT(combData[off]);
	    }
	});
#endif
    }
    return result;
}

#undef ALLOC_IT
#endif /* OPT_STATUS_LINE */

/*
  Resizes screen:
  1. If new window would have fractional characters, sets window size so as to
  discard fractional characters and returns -1.
  Minimum screen size is 1 X 1.
  Note that this causes another ExposeWindow event.
  2. Enlarges screen->buf if necessary.  New space is appended to the bottom
  and to the right
  3. Reduces  screen->buf if necessary.  Old space is removed from the bottom
  and from the right
  4. Cursor is positioned as closely to its former position as possible
  5. Sets screen->max_row and screen->max_col to reflect new size
  6. Maintains the inner border (and clears the border on the screen).
  7. Clears origin mode and sets scrolling region to be entire screen.
  */
void
ScreenResize(XtermWidget xw,
	     int width,
	     int height,
	     unsigned *flags)
{
    TScreen *screen = TScreenOf(xw);
    int rows, cols;
    const int border = 2 * screen->border;
    int move_down_by = 0;
    Boolean forced = False;

#if OPT_STATUS_LINE
    LineData *savedStatus = NULL;
#endif

    TRACE(("ScreenResize %dx%d border 2*%d font %dx%d\n",
	   height, width, screen->border,
	   FontHeight(screen), FontWidth(screen)));

    assert(width > 0);
    assert(height > 0);

    TRACE(("...computing rows/cols: %.2f %.2f\n",
	   (double) (height - border) / FontHeight(screen),
	   (double) (width - border - ScrollbarWidth(screen)) / FontWidth(screen)));

    rows = (height - border) / FontHeight(screen);
    cols = (width - border - ScrollbarWidth(screen)) / FontWidth(screen);
    if (rows < 1)
	rows = 1;
    if (cols < 1)
	cols = 1;

#if OPT_STATUS_LINE
    /*
     * The dimensions passed to this function include the status-line.
     * Discount that here (to obtain the actual rows/columns), and save
     * the contents of the status-line, to repaint it after resizing.
     */
    TRACE(("...StatusShown %d/%d\n", IsStatusShown(screen), screen->status_shown));
    if (IsStatusShown(screen)) {
	int oldRow = MaxRows(screen);
	int newRow = rows - StatusLineRows;
	LineData *oldLD;
	TRACE(("...status line is currently on row %d(%d-%d) vs %d\n",
	       oldRow,
	       MaxRows(screen),
	       (screen->status_shown ? 0 : StatusLineRows),
	       rows));
	oldLD = getLineData(screen, oldRow);
	TRACE(("...copying:%s\n",
	       visibleIChars(oldLD->charData,
			     oldLD->lineSize)));
	TRACE(("...will move status-line from row %d to %d\n",
	       oldRow,
	       newRow));
	savedStatus = allocLineData(screen, oldLD);
	copyLineData(savedStatus, oldLD);
	TRACE(("...copied::%s\n",
	       visibleIChars(savedStatus->charData,
			     savedStatus->lineSize)));
	TRACE(("...discount a row for status-line\n"));
	rows = newRow;
	height -= FontHeight(screen) * StatusLineRows;
    }
#endif

    if (screen->is_running) {
	/* clear the right and bottom internal border because of NorthWest
	   gravity might have left junk on the right and bottom edges */
	if (width >= (int) FullWidth(screen)) {
	    xtermClear2(xw,
			FullWidth(screen), 0,	/* right edge */
			0, (unsigned) height);	/* from top to bottom */
	}
	if (height >= (int) FullHeight(screen)) {
	    xtermClear2(xw,
			0, FullHeight(screen),	/* bottom */
			(unsigned) width, 0);	/* all across the bottom */
	}
    }

    /* update buffers if the screen has changed size */
    if (forced) {
	;
    } else if (MaxRows(screen) != rows || MaxCols(screen) != cols) {
	int delta_rows = rows - MaxRows(screen);
#if OPT_TRACE
	int delta_cols = cols - MaxCols(screen);
#endif

	TRACE(("...ScreenResize chars %dx%d delta %dx%d\n",
	       rows, cols, delta_rows, delta_cols));

	if (screen->is_running) {
	    if (screen->cursor_state)
		HideCursor(xw);

	    /*
	     * The non-visible buffer is simple, since we will not copy data
	     * to/from the saved-lines.  Do that first.
	     */
	    if (screen->editBuf_index[!screen->whichBuf]) {
		(void) Reallocate(xw,
				  &screen->editBuf_index[!screen->whichBuf],
				  &screen->editBuf_data[!screen->whichBuf],
				  (unsigned) rows,
				  (unsigned) cols,
				  (unsigned) MaxRows(screen));
	    }

	    /*
	     * The save-lines buffer may change width, but will not change its
	     * height.  Deal with the cases where we copy data to/from the
	     * saved-lines buffer.
	     */
	    if (GravityIsSouthWest(xw)
		&& delta_rows
		&& screen->saveBuf_index != NULL) {

		if (delta_rows < 0) {
		    unsigned move_up = (unsigned) (-delta_rows);
		    int amount = ((MaxRows(screen) - (int) move_up - 1)
				  - screen->cur_row);

		    if (amount < 0) {
			/* move line-data from visible-buffer to save-buffer */
			saveEditBufLines(screen, (unsigned) -amount);
			move_down_by = amount;
		    } else {
			move_down_by = 0;
		    }

		    /* decrease size of visible-buffer */
		    (void) Reallocate(xw,
				      &screen->editBuf_index[screen->whichBuf],
				      &screen->editBuf_data[screen->whichBuf],
				      (unsigned) rows,
				      (unsigned) cols,
				      (unsigned) MaxRows(screen));
		    TRACE_SCRNBUF("reallocEDIT",
				  screen,
				  screen->editBuf_index[screen->whichBuf],
				  rows);
		} else {
		    unsigned move_down = (unsigned) delta_rows;
		    long unsave_fifo;
		    ScrnBuf dst;
		    int amount;

		    if ((int) move_down > screen->savedlines) {
			move_down = (unsigned) screen->savedlines;
		    }
		    move_down_by = (int) move_down;
		    amount = rows - (int) move_down;

		    /* increase size of visible-buffer */
		    (void) Reallocate(xw,
				      &screen->editBuf_index[screen->whichBuf],
				      &screen->editBuf_data[screen->whichBuf],
				      (unsigned) rows,
				      (unsigned) cols,
				      (unsigned) MaxRows(screen));

		    dst = screen->editBuf_index[screen->whichBuf];
		    TRACE_SCRNBUF("reallocEDIT", screen, dst, rows);

		    TRACE(("...%smoving pointers in editBuf (compare %d %d)\n",
			   (amount > 0
			    ? ""
			    : "SKIP "),
			   rows,
			   move_down));
		    if (amount > 0) {
			/* shift lines in visible-buffer to make room */
			SaveLineData(dst, (unsigned) amount, (size_t) move_down);

			MoveLineData(dst,
				     move_down,
				     0,
				     (unsigned) amount);

			TRACE(("...reuse %d lines storage in editBuf\n", move_down));
			RestoreLineData(dst,
					0,
					move_down);

			TRACE_SCRNBUF("shifted", screen, dst, rows);
		    }

		    /* copy line-data from save-buffer to visible-buffer */
		    unsaveEditBufLines(screen, dst, move_down);
		    TRACE_SCRNBUF("copied", screen, dst, rows);

		    unsave_fifo = (long) move_down;
		    if (screen->saved_fifo < (int) unsave_fifo)
			unsave_fifo = screen->saved_fifo;

		    /* free up storage in fifo from the copied lines */
		    while (unsave_fifo-- > 0) {
			deleteScrollback(screen);
		    }

		    /* recover storage in save-buffer */
		}
	    } else {
		(void) Reallocate(xw,
				  &screen->editBuf_index[screen->whichBuf],
				  &screen->editBuf_data[screen->whichBuf],
				  (unsigned) rows,
				  (unsigned) cols,
				  (unsigned) MaxRows(screen));
	    }

	    screen->visbuf = VisBuf(screen);
	}

	AdjustSavedCursor(xw, move_down_by);
	set_max_row(screen, screen->max_row + delta_rows);
	set_max_col(screen, cols - 1);

	if (screen->is_running) {
	    if (GravityIsSouthWest(xw)) {
		screen->savedlines -= move_down_by;
		if (screen->savedlines < 0)
		    screen->savedlines = 0;
		if (screen->savedlines > screen->savelines)
		    screen->savedlines = screen->savelines;
		if (screen->topline < -screen->savedlines)
		    screen->topline = -screen->savedlines;
		set_cur_row(screen, screen->cur_row + move_down_by);
		screen->cursorp.row += move_down_by;
		ScrollSelection(screen, move_down_by, True);
	    }
	}

	/* adjust scrolling region */
	resetMargins(xw);
	UIntClr(*flags, ORIGIN);

	if (screen->cur_row > screen->max_row)
	    set_cur_row(screen, screen->max_row);
	if (screen->cur_col > screen->max_col)
	    set_cur_col(screen, screen->max_col);

	screen->fullVwin.height = height - border;
	screen->fullVwin.width = width - border - screen->fullVwin.sb_info.width;

	scroll_displayed_graphics(xw, -move_down_by);
    } else if (FullHeight(screen) == height && FullWidth(screen) == width) {
#if OPT_STATUS_LINE
	if (savedStatus != NULL) {
	    TRACE(("...status line is currently saved!\n"));
	    freeLineData(screen, savedStatus);
	}
#endif
	return;			/* nothing has changed at all */
    }

    screen->fullVwin.fullheight = (Dimension) height;
    screen->fullVwin.fullwidth = (Dimension) width;

    ResizeScrollBar(xw);
    ResizeSelection(screen, rows, cols);

#ifndef NO_ACTIVE_ICON
    if (screen->iconVwin.window) {
	XWindowChanges changes;
	screen->iconVwin.width =
	    MaxCols(screen) * screen->iconVwin.f_width;

	screen->iconVwin.height =
	    MaxRows(screen) * screen->iconVwin.f_height;

	changes.width = screen->iconVwin.fullwidth =
	    (Dimension) ((unsigned) screen->iconVwin.width
			 + 2 * xw->misc.icon_border_width);

	changes.height = screen->iconVwin.fullheight =
	    (Dimension) ((unsigned) screen->iconVwin.height
			 + 2 * xw->misc.icon_border_width);

	changes.border_width = (int) xw->misc.icon_border_width;

	TRACE(("resizing icon window %dx%d\n", changes.height, changes.width));
	XConfigureWindow(XtDisplay(xw), screen->iconVwin.window,
			 CWWidth | CWHeight | CWBorderWidth, &changes);
    }
#endif /* NO_ACTIVE_ICON */

#if OPT_STATUS_LINE
    if (savedStatus != NULL) {
	int newRow = LastRowNumber(screen);
	LineData *newLD = getLineData(screen, newRow);
	TRACE(("...status line is currently on row %d\n",
	       LastRowNumber(screen)));
	copyLineData(newLD, savedStatus);
	TRACE(("...copied::%s\n",
	       visibleIChars(newLD->charData,
			     newLD->lineSize)));
	freeLineData(screen, savedStatus);
    }
#endif

#ifdef TTYSIZE_STRUCT
    if (update_winsize(screen, rows, cols, height, width) == 0) {
#if defined(SIGWINCH) && defined(TIOCGPGRP)
	if (screen->pid > 1) {
	    int pgrp;

	    TRACE(("getting process-group\n"));
	    if (ioctl(screen->respond, TIOCGPGRP, &pgrp) != -1) {
		TRACE(("sending SIGWINCH to process group %d\n", pgrp));
		kill_process_group(pgrp, SIGWINCH);
	    }
	}
#endif /* SIGWINCH */
    }
#else
    TRACE(("ScreenResize cannot do anything to pty\n"));
#endif /* TTYSIZE_STRUCT */
    return;
}

/*
 * Return true if any character cell starting at [row,col], for len-cells is
 * nonnull.
 */
Bool
non_blank_line(TScreen *screen,
	       int row,
	       int col,
	       int len)
{
    Bool found = False;
    LineData *ld = getLineData(screen, row);

    if (ld != NULL) {
	int i;

	for (i = col; i < len; i++) {
	    if (ld->charData[i]) {
		found = True;
		break;
	    }
	}
    }
    return found;
}

/*
 * Limit/map rectangle parameters.
 */
#define minRectRow(screen) (getMinRow(screen) + 1)
#define minRectCol(screen) (getMinCol(screen) + 1)
#define maxRectRow(screen) (getMaxRow(screen) + 1)
#define maxRectCol(screen) (getMaxCol(screen) + 1)

static int
limitedParseRow(XtermWidget xw, int row)
{
    TScreen *screen = TScreenOf(xw);
    int min_row = minRectRow(screen);
    int max_row = maxRectRow(screen);

    if (xw->flags & ORIGIN)
	row += screen->top_marg;

    if (row < min_row)
	row = min_row;
    else if (row > max_row)
	row = max_row;

    return row;
}

static int
limitedParseCol(XtermWidget xw, int col)
{
    TScreen *screen = TScreenOf(xw);
    int min_col = minRectCol(screen);
    int max_col = maxRectCol(screen);

    if (xw->flags & ORIGIN)
	col += screen->lft_marg;

    if (col < min_col)
	col = min_col;
    else if (col > max_col)
	col = max_col;

    return col;
}

#define LimitedParse(num, func, dft) \
	func(xw, (nparams > num && params[num] > 0) ? params[num] : dft)

/*
 * Copy the rectangle boundaries into a struct, providing default values as
 * needed.
 */
void
xtermParseRect(XtermWidget xw, int nparams, int *params, XTermRect *target)
{
    TScreen *screen = TScreenOf(xw);

    memset(target, 0, sizeof(*target));
    target->top = LimitedParse(0, limitedParseRow, minRectRow(screen));
    target->left = LimitedParse(1, limitedParseCol, minRectCol(screen));
    target->bottom = LimitedParse(2, limitedParseRow, maxRectRow(screen));
    target->right = LimitedParse(3, limitedParseCol, maxRectCol(screen));
    TRACE(("parsed %d params for rectangle %d,%d %d,%d default %d,%d %d,%d\n",
	   nparams,
	   target->top,
	   target->left,
	   target->bottom,
	   target->right,
	   minRectRow(screen),
	   minRectCol(screen),
	   maxRectRow(screen),
	   maxRectCol(screen)));
}

static Bool
validRect(XtermWidget xw, XTermRect *target)
{
    TScreen *screen = TScreenOf(xw);
    Bool result = (target != NULL
		   && target->top >= minRectRow(screen)
		   && target->left >= minRectCol(screen)
		   && target->top <= target->bottom
		   && target->left <= target->right
		   && target->top <= maxRectRow(screen)
		   && target->right <= maxRectCol(screen));

    TRACE(("comparing against screensize %dx%d, is%s valid\n",
	   maxRectRow(screen),
	   maxRectCol(screen),
	   result ? "" : " NOT"));
    return result;
}

/*
 * Fills a rectangle with the given 8-bit character and video-attributes.
 * Colors and double-size attribute are unmodified.
 */
void
ScrnFillRectangle(XtermWidget xw,
		  XTermRect *target,
		  int value,
		  DECNRCM_codes charset,
		  unsigned flags,
		  Bool keepColors)
{
    IChar actual = (IChar) value;
    TScreen *screen = TScreenOf(xw);

    TRACE(("filling rectangle with '%s' %s flags %#x\n",
	   visibleIChars(&actual, 1),
	   visibleScsCode(charset),
	   flags));
    if (validRect(xw, target)) {
	LineData *ld;
	int top = (target->top - 1);
	int left = (target->left - 1);
	int right = (target->right - 1);
	int bottom = (target->bottom - 1);
	int numcols = (right - left) + 1;
	int numrows = (bottom - top) + 1;
	unsigned attrs = flags;
	int row, col;
	int b_left = 0;
	int b_right = 0;

	(void) numcols;

	if (charset != nrc_ASCII) {
	    xw->work.write_text = &actual;
	    if (xtermCharSetOut(xw, 1, charset) == 0)
		actual = ' ';
	}

	attrs &= ATTRIBUTES;
	attrs |= CHARDRAWN;
	for (row = bottom; row >= top; row--) {
	    ld = getLineData(screen, row);

	    TRACE(("filling %d [%d..%d]\n", row, left, left + numcols));

	    if_OPT_WIDE_CHARS(screen, {
		if (left > 0) {
		    if (ld->charData[left] == HIDDEN_CHAR) {
			b_left = 1;
			Clear1Cell(ld, left - 1);
			Clear1Cell(ld, left);
		    }
		}
		if (right + 1 < (int) ld->lineSize) {
		    if (ld->charData[right + 1] == HIDDEN_CHAR) {
			b_right = 1;
			Clear1Cell(ld, right);
			Clear1Cell(ld, right + 1);
		    }
		}
	    });

	    /*
	     * Fill attributes, preserving colors.
	     */
	    for (col = left; col <= right; ++col) {
		unsigned temp = ld->attribs[col];

		if (!keepColors) {
		    UIntClr(temp, (FG_COLOR | BG_COLOR));
		}
		temp = attrs | (temp & (FG_COLOR | BG_COLOR)) | CHARDRAWN;
		ld->attribs[col] = (IAttr) temp;
		if_OPT_ISO_COLORS(screen, {
		    if (attrs & (FG_COLOR | BG_COLOR)) {
			ld->color[col] = xtermColorPair(xw);
		    }
		});
	    }

	    for (col = left; col <= right; ++col) {
		ld->charData[col] = actual;
#if OPT_DEC_RECTOPS
		ld->charSeen[col] = (Char) value;
		ld->charSets[col] = charset;
#endif
	    }

	    if_OPT_WIDE_CHARS(screen, {
		size_t off;
		for_each_combData(off, ld) {
		    memset(ld->combData[off] + left,
			   0,
			   (size_t) numcols * sizeof(CharData));
		}
	    })
	}
	chararea_clear_displayed_graphics(screen,
					  left,
					  top,
					  numcols, numrows);
	ScrnUpdate(xw,
		   top,
		   left - b_left,
		   numrows,
		   numcols + b_left + b_right,
		   False);
    }
}

#if OPT_DEC_RECTOPS
/*
 * Copies the source rectangle to the target location, including video
 * attributes.
 *
 * This implementation ignores page numbers.
 *
 * The reference manual does not indicate if it handles overlapping copy
 * properly - so we make a local copy of the source rectangle first, then apply
 * the target from that.
 */
void
ScrnCopyRectangle(XtermWidget xw, XTermRect *source, int nparam, int *params)
{
    TScreen *screen = TScreenOf(xw);

    TRACE(("copying rectangle\n"));

    if (nparam > 4)
	nparam = 4;

    if (validRect(xw, source)) {
	XTermRect target;
	xtermParseRect(xw,
		       ((nparam > 2) ? 2 : nparam),
		       params,
		       &target);
	if (validRect(xw, &target)) {
	    Cardinal high = (Cardinal) (source->bottom - source->top) + 1;
	    Cardinal wide = (Cardinal) (source->right - source->left) + 1;
	    Cardinal size = (high * wide);
	    int row, col;
	    Cardinal j, k;
	    LineData *ld;
	    int b_left = 0;
	    int b_right = 0;

	    CellData *cells = newCellData(xw, size);

	    if (cells != NULL) {

		TRACE(("OK - make copy %dx%d\n", high, wide));
		target.bottom = target.top + (int) (high - 1);
		target.right = target.left + (int) (wide - 1);

		for (row = source->top - 1; row < source->bottom; ++row) {
		    ld = getLineData(screen, row);
		    if (ld == NULL)
			continue;
		    j = (Cardinal) (row - (source->top - 1));
		    TRACE2(("ROW %d\n", row + 1));
		    for (col = source->left - 1; col < source->right; ++col) {
			k = (Cardinal) (col - (source->left - 1));
			saveCellData(screen, cells,
				     (j * wide) + k,
				     ld, source, col);
		    }
		}
		for (row = target.top - 1; row < target.bottom; ++row) {
		    ld = getLineData(screen, row);
		    if (ld == NULL)
			continue;
		    j = (Cardinal) (row - (target.top - 1));
		    TRACE2(("ROW %d\n", row + 1));
		    for (col = target.left - 1; col < target.right; ++col) {
			k = (Cardinal) (col - (target.left - 1));
			if (row >= getMinRow(screen)
			    && row <= getMaxRow(screen)
			    && col >= getMinCol(screen)
			    && col <= getMaxCol(screen)
			    && (j < high)
			    && (k < wide)) {
			    if_OPT_WIDE_CHARS(screen, {
				if (ld->charData[col] == HIDDEN_CHAR
				    && (col + 1) == target.left) {
				    b_left = 1;
				    Clear1Cell(ld, col - 1);
				}
				if ((col + 1) == target.right
				    && ld->charData[col] == HIDDEN_CHAR) {
				    b_right = 1;
				}
			    });
			    restoreCellData(screen, cells,
					    (j * wide) + k,
					    ld, &target, col);
			}
			ld->attribs[col] |= CHARDRAWN;
		    }
#if OPT_BLINK_TEXT
		    if (LineHasBlinking(screen, ld)) {
			LineSetBlinked(ld);
		    } else {
			LineClrBlinked(ld);
		    }
#endif
		}
		free(cells);

		ScrnUpdate(xw,
			   (target.top - 1),
			   (target.left - (1 + b_left)),
			   (target.bottom - target.top) + 1,
			   ((target.right - target.left) + (1 + b_left + b_right)),
			   False);
	    }
	}
    }
}

/*
 * Modifies the video-attributes only - so selection (not a video attribute) is
 * unaffected.  Colors and double-size flags are unaffected as well.
 *
 * Reference: VSRM - Character Cell Display EL-00070-05
 *
 * Section:
 * -------
 * CHANGE ATTRIBUTES RECTANGULAR AREA -- DECCARA
 * Page 5-173
 *
 * Quote:
 * The character positions affected depend on the current setting of DECSACE
 * (STREAM or RECTANGLE).  See DECSACE for details.
 *
 * Notes:
 * xterm allows 8 (hidden) to be reversed, as an extension.
 *
 * Section:
 * -------
 * REVERSE ATTRIBUTES RECTANGULAR AREA -- DECRARA
 * Page 5-175
 *
 * Quote:
 * The video attribute(s) to be reversed are in the affected area are indicated
 * by one or more subsequent parameters.  These parameters are similar to the
 * parameters of the Set Graphic Rendition control function (SGR):
 *
 * Parameter  Parameter Meaning
 *    0       Reverse all attributes
 *    1       Reverse bold attribute
 *    4       Reverse underscore attribute
 *    5       Reverse blinking attribute
 *    7       Reverse negative (reverse) image attribute
 *
 * All other parameter values shall be ignored unless they are part of a well
 * defined extension to the architecture.  Note if the Color Text Extension is
 * present, the color text SGR values are ignored since the "reverse" of a
 * color is not defined by the extension.
 *
 * Notes:
 * xterm allows 8 (hidden) to be reversed, as an extension.
 *
 * Section:
 * -------
 * SELECT ATTRIBUTE CHANGE EXTENT -- DECSACE
 * Page 5-177
 *
 * Quote:
 * When Ps = 0 or 1, DECCARA and DECRARA affects the stream of character
 * positions beginning with the first character position specified in the
 * command, and ending with the second character position specified.
 *
 * Notes:
 * The description of DECSACE goes on to state that "unoccupied" cells are
 * not affected in STREAM mode, while in RECTANGLE mode they are converted
 * to blanks.
 *
 * While STREAM uses the upper-left and lower-right cell coordinates for a
 * RECTANGLE (which may take into account ORIGIN mode), the characters wrap,
 * in STREAM mode, and DEC 070 does not appear to state that ORIGIN mode
 * affects the wrap-margins.
 */
void
ScrnMarkRectangle(XtermWidget xw,
		  XTermRect *target,
		  Bool reverse,
		  int nparam,
		  int *params)
{
    TScreen *screen = TScreenOf(xw);
    Bool exact = (screen->cur_decsace == 2);

    TRACE(("%s %s\n",
	   reverse ? "reversing" : "marking",
	   (exact
	    ? "rectangle"
	    : "region")));

    if (validRect(xw, target)) {
	LineData *ld;
	int top = target->top - 1;
	int bottom = target->bottom - 1;
	int row, col;
	int n;

	for (row = top; row <= bottom; ++row) {
	    int left = ((exact || (row == top))
			? (target->left - 1)
			: 0);
	    int right = ((exact || (row == bottom))
			 ? (target->right - 1)
			 : screen->max_col);

	    ld = getLineData(screen, row);

	    TRACE(("marking %d [%d..%d]\n", row, left, right));
	    for (col = left; col <= right; ++col) {
		unsigned flags = ld->attribs[col];

		if (!(flags & CHARDRAWN)) {
		    if (exact) {
			flags |= CHARDRAWN;
			Clear1Cell(ld, col);
		    } else {
			continue;
		    }
		}

		for (n = 0; n < nparam; ++n) {
#if OPT_TRACE
		    if (row == top && col == left)
			TRACE(("attr param[%d] %d\n", n + 1, params[n]));
#endif
		    if (reverse) {
			switch (params[n]) {
			case 0:
			    flags ^= SGR_MASK;
			    break;
			case 1:
			    flags ^= BOLD;
			    break;
			case 4:
			    flags ^= UNDERLINE;
			    break;
			case 5:
			    flags ^= BLINK;
			    break;
			case 7:
			    flags ^= INVERSE;
			    break;
			case 8:
			    flags ^= INVISIBLE;
			    break;
			}
		    } else {
			switch (params[n]) {
			case 0:
			    UIntClr(flags, SGR_MASK);
			    break;
			case 1:
			    flags |= BOLD;
			    break;
			case 4:
			    flags |= UNDERLINE;
			    break;
			case 5:
			    flags |= BLINK;
			    break;
			case 7:
			    flags |= INVERSE;
			    break;
			case 8:
			    flags |= INVISIBLE;
			    break;
			case 22:
			    UIntClr(flags, BOLD);
			    break;
			case 24:
			    UIntClr(flags, UNDERLINE);
			    break;
			case 25:
			    UIntClr(flags, BLINK);
			    break;
			case 27:
			    UIntClr(flags, INVERSE);
			    break;
			case 28:
			    UIntClr(flags, INVISIBLE);
			    break;
			}
		    }
		}
#if OPT_TRACE
		if (row == top && col == left)
		    TRACE(("first mask-change is %#x\n",
			   ld->attribs[col] ^ flags));
#endif
		ld->attribs[col] = (IAttr) flags;
	    }
	}
	ScrnRefresh(xw,
		    (target->top - 1),
		    (exact ? (target->left - 1) : getMinCol(screen)),
		    (target->bottom - target->top) + 1,
		    (exact
		     ? ((target->right - target->left) + 1)
		     : (getMaxCol(screen) - getMinCol(screen) + 1)),
		    True);
    }
}

/*
 * Resets characters to space, except where prohibited by DECSCA.  Video
 * attributes (including color) are untouched.
 */
void
ScrnWipeRectangle(XtermWidget xw,
		  XTermRect *target)
{
    TScreen *screen = TScreenOf(xw);

    TRACE(("wiping rectangle\n"));

#define IsProtected(ld, col) \
		((screen->protected_mode == DEC_PROTECT) \
		 && (ld->attribs[col] & PROTECTED))

    if (validRect(xw, target)) {
	int top = target->top - 1;
	int left = target->left - 1;
	int right = target->right - 1;
	int bottom = target->bottom - 1;
	int numcols = (right - left) + 1;
	int numrows = (bottom - top) + 1;
	int row, col;
	int b_left = 0;
	int b_right = 0;

	for (row = top; row <= bottom; ++row) {
	    LineData *ld;

	    TRACE(("wiping %d [%d..%d]\n", row, left, right));

	    ld = getLineData(screen, row);

	    if_OPT_WIDE_CHARS(screen, {
		if (left > 0 && !IsProtected(ld, left)) {
		    if (ld->charData[left] == HIDDEN_CHAR) {
			b_left = 1;
			Clear1Cell(ld, left - 1);
			Clear1Cell(ld, left);
		    }
		}
		if (right + 1 < (int) ld->lineSize && !IsProtected(ld, right)) {
		    if (ld->charData[right + 1] == HIDDEN_CHAR) {
			b_right = 1;
			Clear1Cell(ld, right);
			Clear1Cell(ld, right + 1);
		    }
		}
	    });

	    for (col = left; col <= right; ++col) {
		if (!IsProtected(ld, col)) {
		    ld->attribs[col] |= CHARDRAWN;
		    Clear1Cell(ld, col);
		}
	    }
	}
	chararea_clear_displayed_graphics(screen,
					  left,
					  top,
					  numcols, numrows);
	ScrnUpdate(xw,
		   top,
		   left - b_left,
		   numrows,
		   numcols + b_left + b_right,
		   False);
    }
}

/*
 * Compute a checksum, ignoring the page number (since we have only one page).
 */
void
xtermCheckRect(XtermWidget xw,
	       int nparam,
	       int *params,
	       int *result)
{
    TScreen *screen = TScreenOf(xw);
    XTermRect target;
    LineData *ld;
    int total = 0;
    int trimmed = 0;
    int mode = screen->checksum_ext;

    TRACE(("xtermCheckRect: %s%s%s%s%s%s\n",
	   (mode == csDEC) ? "DEC" : "checksumExtension",
	   (mode & csPOSITIVE) ? " !negative" : "",
	   (mode & csATTRIBS) ? " !attribs" : "",
	   (mode & csNOTRIM) ? " !trimmed" : "",
	   (mode & csDRAWN) ? " !drawn" : "",
	   (mode & csBYTE) ? " !byte" : ""));

    if (nparam > 2) {
	nparam -= 2;
	params += 2;
    }
    xtermParseRect(xw, nparam, params, &target);
    if (validRect(xw, &target)) {
	int top = target.top - 1;
	int bottom = target.bottom - 1;
	int row, col;
	Boolean first = True;
	int embedded = 0;

	for (row = top; row <= bottom; ++row) {
	    int left = (target.left - 1);
	    int right = (target.right - 1);
	    int ch;

	    ld = getLineData(screen, row);
	    if (ld == NULL)
		continue;
	    for (col = left; col <= right && col < (int) ld->lineSize; ++col) {
		if (!(ld->attribs[col] & CHARDRAWN)) {
		    if (!(mode & (csNOTRIM | csDRAWN)))
			continue;
		    ch = ' ';
		} else if (!(mode & csBYTE)) {
		    ch = xtermCharSetDec(xw,
					 ld->charSeen[col],
					 ld->charSets[col]);
		} else {
		    ch = (int) ld->charData[col];
		    if_OPT_WIDE_CHARS(screen, {
			if (ld->charSets[col] == nrc_DEC_Spec_Graphic) {
			    ch = (int) dec2ucs(screen, (unsigned) ch);
			}
			if (is_UCS_SPECIAL(ch))
			    continue;
		    });
		}
		if (!(mode & csATTRIBS)) {
#if OPT_ISO_COLORS && OPT_VT525_COLORS
		    if (screen->terminal_id == 525) {
			IAttr flags = ld->attribs[col];
			CellColor fg_bg = ld->color[col];
			int fg = (int) extract_fg(xw, fg_bg, flags);
			int bg = (int) extract_bg(xw, fg_bg, flags);
			Boolean dft_bg = (bg < 0);
			Boolean dft_fg = (fg < 0);

			if (dft_bg)
			    bg = screen->assigned_bg;
			if (bg >= 0 && bg < 16)
			    ch += bg;

			if (dft_fg)
			    fg = screen->assigned_fg;
			if (fg >= 0 && fg < 16)
			    ch += (fg << 4);

			/* special case to match VT525 behavior */
			if (dft_bg && !dft_fg && (ld->attribs[col] & BOLD))
			    ch -= 0x80;
		    }
#endif
		    if (ld->attribs[col] & PROTECTED)
			ch += 0x4;
#if OPT_WIDE_ATTRS
		    if (ld->attribs[col] & INVISIBLE)
			ch += 0x8;
#endif
		    if (ld->attribs[col] & UNDERLINE)
			ch += 0x10;
		    if (ld->attribs[col] & INVERSE)
			ch += 0x20;
		    if (ld->attribs[col] & BLINK)
			ch += 0x40;
		    if (ld->attribs[col] & BOLD)
			ch += 0x80;
		}
		if (first || (ch != ' ') || (ld->attribs[col] & DRAWX_MASK)) {
		    trimmed += ch + embedded;
		    embedded = 0;
		} else if ((mode & csNOTRIM)) {
		    embedded += ch;
		}
		total += ch;
		if_OPT_WIDE_CHARS(screen, {
		    /* FIXME - not counted if trimming blanks */
		    if (!(mode & csBYTE)) {
			size_t off;
			for_each_combData(off, ld) {
			    total += (int) ld->combData[off][col];
			}
		    }
		});
		first = ((mode & csNOTRIM) != 0) ? True : False;
	    }
	    if (!(mode & csNOTRIM)) {
		embedded = 0;
		first = False;
	    }
	}
    }
    if (!(mode & csNOTRIM))
	total = trimmed;
    if (!(mode & csPOSITIVE))
	total = -total;
    *result = total;
}
#endif /* OPT_DEC_RECTOPS */

static void
set_ewmh_hint(XtermWidget xw, int operation, _Xconst char *prop)
{
    TScreen *screen = TScreenOf(xw);
    Display *dpy = screen->display;
    Window window;
    XEvent e;
    Atom atom_fullscreen = CachedInternAtom(dpy, prop);
    Atom atom_state = CachedInternAtom(dpy, "_NET_WM_STATE");

#if OPT_TRACE
    const char *what = "?";
    switch (operation) {
    case _NET_WM_STATE_ADD:
	what = "adding";
	break;
    case _NET_WM_STATE_REMOVE:
	what = "removing";
	break;
    }
    TRACE(("set_ewmh_hint %s %s\n", what, prop));
#endif

#if OPT_TEK4014
    if (TEK4014_ACTIVE(xw)) {
	window = TShellWindow;
    } else
#endif
	window = VShellWindow(xw);

    memset(&e, 0, sizeof(e));
    e.xclient.type = ClientMessage;
    e.xclient.message_type = atom_state;
    e.xclient.display = dpy;
    e.xclient.window = window;
    e.xclient.format = 32;
    e.xclient.data.l[0] = operation;
    e.xclient.data.l[1] = (long) atom_fullscreen;

    XSendEvent(dpy, DefaultRootWindow(dpy), False,
	       SubstructureRedirectMask, &e);
}

void
ResetHiddenHint(XtermWidget xw)
{
    set_ewmh_hint(xw, _NET_WM_STATE_REMOVE, "_NET_WM_STATE_HIDDEN");
}

#if OPT_MAXIMIZE

static _Xconst char *
ewmhProperty(int mode)
{
    _Xconst char *result;
    switch (mode) {
    default:
	result = NULL;
	break;
    case 1:
	result = "_NET_WM_STATE_FULLSCREEN";
	break;
    case 2:
	result = "_NET_WM_STATE_MAXIMIZED_VERT";
	break;
    case 3:
	result = "_NET_WM_STATE_MAXIMIZED_HORZ";
	break;
    }
    return result;
}

static void
set_resize_increments(XtermWidget xw)
{
    TScreen *screen = TScreenOf(xw);
    int min_width = (2 * screen->border) + screen->fullVwin.sb_info.width;
    int min_height = (2 * screen->border);
    XSizeHints sizehints;

    TRACE(("set_resize_increments\n"));
    memset(&sizehints, 0, sizeof(XSizeHints));
    sizehints.width_inc = FontWidth(screen);
    sizehints.height_inc = FontHeight(screen);
    sizehints.flags = PResizeInc;
    TRACE_HINTS(&sizehints);
    XSetWMNormalHints(screen->display, VShellWindow(xw), &sizehints);

    TRACE(("setting values for widget %p:\n", (void *) SHELL_OF(xw)));
    TRACE(("   base width  %d\n", min_width));
    TRACE(("   base height %d\n", min_width));
    TRACE(("   min width   %d\n", min_width + FontWidth(screen)));
    TRACE(("   min height  %d\n", min_width + FontHeight(screen)));
    TRACE(("   width inc   %d\n", FontWidth(screen)));
    TRACE(("   height inc  %d\n", FontHeight(screen)));

    XtVaSetValues(SHELL_OF(xw),
		  XtNbaseWidth, min_width,
		  XtNbaseHeight, min_height,
		  XtNminWidth, min_width + FontWidth(screen),
		  XtNminHeight, min_height + FontHeight(screen),
		  XtNwidthInc, FontWidth(screen),
		  XtNheightInc, FontHeight(screen),
		  (XtPointer) 0);

    XFlush(XtDisplay(xw));
}

static void
unset_resize_increments(XtermWidget xw)
{
    TScreen *screen = TScreenOf(xw);
    XSizeHints sizehints;

    TRACE(("unset_resize_increments\n"));
    memset(&sizehints, 0, sizeof(XSizeHints));
    sizehints.width_inc = 1;
    sizehints.height_inc = 1;
    sizehints.flags = PResizeInc;
    TRACE_HINTS(&sizehints);
    XSetWMNormalHints(screen->display, VShellWindow(xw), &sizehints);

    XtVaSetValues(SHELL_OF(xw),
		  XtNwidthInc, 1,
		  XtNheightInc, 1,
		  (XtPointer) 0);

    XFlush(XtDisplay(xw));
}

/*
 * Check if the given property is supported on the root window.
 *
 * The XGetWindowProperty function returns a list of Atom's which corresponds
 * to the output of xprop.  The actual list (ignore the manpage, which refers
 * to an array of 32-bit values) is constructed by _XRead32, which uses long
 * as a datatype.
 *
 * Alternatively, we could check _NET_WM_ALLOWED_ACTIONS on the application's
 * window.
 */
static Boolean
probe_netwm(Display *dpy, _Xconst char *propname)
{
    Atom atom_fullscreen = CachedInternAtom(dpy, propname);
    Atom atom_supported = CachedInternAtom(dpy, "_NET_SUPPORTED");
    Atom actual_type;
    int actual_format;
    long long_offset = 0;
    long long_length = 128;	/* number of items to ask for at a time */
    unsigned int i;
    unsigned long nitems, bytes_after;
    unsigned char *args;
    long *ldata;
    Boolean has_capability = False;
    Boolean rc;

    while (!has_capability) {
	rc = xtermGetWinProp(dpy,
			     DefaultRootWindow(dpy),
			     atom_supported,
			     long_offset,
			     long_length,
			     AnyPropertyType,	/* req_type */
			     &actual_type,	/* actual_type_return */
			     &actual_format,	/* actual_format_return */
			     &nitems,	/* nitems_return */
			     &bytes_after,	/* bytes_after_return */
			     &args	/* prop_return */
	    );
	if (!rc
	    || actual_type != XA_ATOM) {
	    break;
	}
	ldata = (long *) (void *) args;
	for (i = 0; i < nitems; i++) {
#if OPT_TRACE > 1
	    char *name;
	    if ((name = XGetAtomName(dpy, ldata[i])) != 0) {
		TRACE(("atom[%d] = %s\n", i, name));
		XFree(name);
	    } else {
		TRACE(("atom[%d] = ?\n", i));
	    }
#endif
	    if ((Atom) ldata[i] == atom_fullscreen) {
		has_capability = True;
		break;
	    }
	}
	XFree(ldata);

	if (!has_capability) {
	    if (bytes_after != 0) {
		long remaining = (long) (bytes_after / sizeof(long));
		if (long_length > remaining)
		    long_length = remaining;
		long_offset += (long) nitems;
	    } else {
		break;
	    }
	}
    }

    TRACE(("probe_netwm(%s) ->%d\n", propname, has_capability));
    return has_capability;
}

/*
 * Alter fullscreen mode for the xterm widget, if the window manager supports
 * that feature.
 */
void
FullScreen(XtermWidget xw, int new_ewmh_mode)
{
    TScreen *screen = TScreenOf(xw);
    Display *dpy = screen->display;
    int old_ewmh_mode;
    _Xconst char *oldprop;
    _Xconst char *newprop;

    int which = 0;
#if OPT_TEK4014
    if (TEK4014_ACTIVE(xw))
	which = 1;
#endif

    old_ewmh_mode = xw->work.ewmh[which].mode;
    oldprop = ewmhProperty(old_ewmh_mode);
    newprop = ewmhProperty(new_ewmh_mode);

    TRACE(("FullScreen %d:%s -> %d:%s\n",
	   old_ewmh_mode, NonNull(oldprop),
	   new_ewmh_mode, NonNull(newprop)));

    if (new_ewmh_mode == old_ewmh_mode) {
	TRACE(("...unchanged\n"));
	return;
    } else if (new_ewmh_mode < 0 || new_ewmh_mode > MAX_EWMH_MODE) {
	TRACE(("BUG: FullScreen %d\n", new_ewmh_mode));
	return;
    } else if (new_ewmh_mode == 0) {
	xw->work.ewmh[which].checked[new_ewmh_mode] = True;
	xw->work.ewmh[which].allowed[new_ewmh_mode] = True;
    } else if (resource.fullscreen == esNever) {
	xw->work.ewmh[which].checked[new_ewmh_mode] = True;
	xw->work.ewmh[which].allowed[new_ewmh_mode] = False;
    } else if (!xw->work.ewmh[which].checked[new_ewmh_mode]) {
	xw->work.ewmh[which].checked[new_ewmh_mode] = True;
	xw->work.ewmh[which].allowed[new_ewmh_mode] = probe_netwm(dpy, newprop);
    }

    if (xw->work.ewmh[which].allowed[new_ewmh_mode]) {
	TRACE(("...new EWMH mode is allowed\n"));
	if (new_ewmh_mode && !xw->work.ewmh[which].mode) {
	    unset_resize_increments(xw);
	    set_ewmh_hint(xw, _NET_WM_STATE_ADD, newprop);
	} else if (xw->work.ewmh[which].mode && !new_ewmh_mode) {
	    if (!xw->misc.resizeByPixel) {
		set_resize_increments(xw);
	    }
	    set_ewmh_hint(xw, _NET_WM_STATE_REMOVE, oldprop);
	} else {
	    set_ewmh_hint(xw, _NET_WM_STATE_REMOVE, oldprop);
	    set_ewmh_hint(xw, _NET_WM_STATE_ADD, newprop);
	}
	xw->work.ewmh[which].mode = new_ewmh_mode;
	update_fullscreen();
    } else {
	Bell(xw, XkbBI_MinorError, 100);
    }
}
#endif /* OPT_MAXIMIZE */
