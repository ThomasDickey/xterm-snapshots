/* $XTermId: screen.c,v 1.323 2009/06/15 23:42:16 tom Exp $ */

/*
 * Copyright 1999-2008,2009 by Thomas E. Dickey
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
#include <xcharmouse.h>
#include <xterm_io.h>

#if OPT_WIDE_CHARS
#include <fontutils.h>
#include <menu.h>
#endif

#include <assert.h>
#include <signal.h>

#define getMinRow(screen) ((xw->flags & ORIGIN) ? (screen)->top_marg : 0)
#define getMaxRow(screen) ((xw->flags & ORIGIN) ? (screen)->bot_marg : (screen)->max_row)
#define getMinCol(screen) 0
#define getMaxCol(screen) ((screen)->max_col)

#define MoveScrnPtrs(base, dst, src, len) \
	memmove(scrnHeadAddr(screen, base, dst), \
		scrnHeadAddr(screen, base, src), \
		scrnHeadSize(screen, len))

#define SaveScrnPtrs(base, src, len) \
	memcpy (screen->save_ptr, \
		scrnHeadAddr(screen, base, src), \
		scrnHeadSize(screen, len))

#define RestoreScrnPtrs(base, dst, len) \
	memcpy (scrnHeadAddr(screen, base, dst), \
		screen->save_ptr, \
		scrnHeadSize(screen, len))

#define VisBuf(screen) scrnHeadAddr(screen, screen->allbuf, (unsigned) savelines)

#define SetupScrnPtr(dst,src,type) \
		dst = (type *) src; \
		src += ((unsigned) ncol * sizeof(*dst))

#define SizeofHeadData(n) (sizeof(ScrnPtrs) + ((n) - 1) * sizeof(ScrnPtr))

#define ScrnBufAddr(ptrs, offset)  (ScrnBuf)    ((char *) (ptrs) + (offset))
#define ScrnPtrsAddr(ptrs, offset) (ScrnPtrs *) ((char *) (ptrs) + (offset))

#if OPT_WIDE_CHARS
#define ExtraScrnSize(screen) ((screen)->wide_chars ? (unsigned) (screen)->max_combining : 0)
#else
#define ExtraScrnSize(screen) 0
#endif

static unsigned
scrnHeadSize(TScreen * screen, unsigned count)
{
    unsigned result;
    unsigned extra = ExtraScrnSize(screen);

#if OPT_WIDE_CHARS
    if (screen->wide_chars) {
	result = SizeofHeadData(extra + 1);
    } else
#endif
	result = SizeofHeadData(extra);
    result *= count;

    return result;
}

ScrnBuf
scrnHeadAddr(TScreen * screen, ScrnBuf base, unsigned offset)
{
    ScrnBuf result = ScrnBufAddr(base, scrnHeadSize(screen, offset));

    return result;
}

/*
 * Given a block of data, build index to it in the 'base' parameter.
 */
static void
setupScrnPtrs(XtermWidget xw, ScrnBuf base, Char * data, unsigned nrow, unsigned ncol)
{
    TScreen *screen = TScreenOf(xw);
    unsigned i, j;
    unsigned offset = 0;
    unsigned jump = scrnHeadSize(screen, 1);
    ScrnPtrs *ptr;

    for (i = 0; i < nrow; i++, offset += jump) {
	ptr = ScrnPtrsAddr(base, offset);

	ptr->bufHead = 0;
	SetupScrnPtr(ptr->attribs, data, Char);
#if OPT_ISO_COLORS
	SetupScrnPtr(ptr->color, data, CellColor);
#endif
#if OPT_DEC_CHRSET
	SetupScrnPtr(ptr->charSets, data, Char);
#endif
	SetupScrnPtr(ptr->charData, data, Char);
#if OPT_WIDE_CHARS
	if (screen->wide_chars) {
	    unsigned extra = ExtraScrnSize(screen);

	    SetupScrnPtr(ptr->wideData, data, Char);
	    for (j = 0; j < extra; ++j) {
		SetupScrnPtr(ptr->combData[j], data, IChar);
	    }
	}
#endif
    }
}

#define ExtractScrnData(name) \
		memcpy(dstPtrs->name, \
		       ((ScrnPtrs *) srcPtrs)->name,\
		       dstCols * sizeof(dstPtrs->name[0])); \
		nextPtr += (srcCols * sizeof(dstPtrs->name[0]))

/*
 * As part of reallocating the screen buffer when resizing, extract from
 * the old copy of the screen buffer the data which will be used in the
 * new copy of the screen buffer.
 */
static void
extractScrnData(XtermWidget xw,
		ScrnBuf srcPtrs,
		Char * dstData,
		unsigned nrows,
		unsigned dstCols,
		unsigned srcCols)
{
    TScreen *screen = TScreenOf(xw);
    unsigned i, j;
    unsigned jump = scrnHeadSize(screen, 1);
    Char *nextPtr = dstData;

    ScrnPtrs *dstPtrs = (ScrnPtrs *) malloc(jump);

    for (i = 0; i < nrows; i++) {
	setupScrnPtrs(xw, (ScrnBuf) dstPtrs, dstData, 1, srcCols);

	ExtractScrnData(attribs);
#if OPT_ISO_COLORS
	ExtractScrnData(color);
#endif
#if OPT_DEC_CHRSET
	ExtractScrnData(charSets);
#endif
	ExtractScrnData(charData);
#if OPT_WIDE_CHARS
	if (screen->wide_chars) {
	    unsigned extra = ExtraScrnSize(screen);

	    ExtractScrnData(wideData);
	    for (j = 0; j < extra; ++j) {
		ExtractScrnData(combData[j]);
	    }
	}
#endif
	srcPtrs = ScrnBufAddr(srcPtrs, jump);
	dstData = nextPtr;
    }
    free(dstPtrs);
}

static ScrnPtr *
allocScrnHead(TScreen * screen, unsigned nrow)
{
    ScrnPtr *result;

    result = (ScrnPtr *) calloc(nrow, scrnHeadSize(screen, 1));
    if (result == 0)
	SysError(ERROR_SCALLOC);

    return result;
}

static ScrnPtr *
reallocScrnHead(TScreen * screen, ScrnPtr * oldptr, unsigned nrow)
{
    ScrnPtr *result;

    result = (ScrnPtr *) realloc(oldptr, scrnHeadSize(screen, nrow));
    if (result == 0)
	SysError(ERROR_RESIZE);

    return result;
}

/*
 * ScrnPtr's can point to different types of data.
 */
#define SizeofScrnPtr(name) \
	sizeof(*((ScrnPtrs *)0)->name)

static unsigned
sizeofScrnRow(TScreen * screen, unsigned ncol)
{
    unsigned result = (SizeofScrnPtr(attribs)
#if OPT_ISO_COLORS
		       + SizeofScrnPtr(color)
#endif
#if OPT_DEC_CHRSET
		       + SizeofScrnPtr(charSets)
#endif
		       + SizeofScrnPtr(charData));

#if OPT_WIDE_CHARS
    if (screen->wide_chars) {
	result += (SizeofScrnPtr(wideData)
		   + SizeofScrnPtr(combData[0]) * ExtraScrnSize(screen));
    }
#endif

    return ncol * result;
}

static Char *
allocScrnData(TScreen * screen, unsigned nrow, unsigned ncol)
{
    Char *result;
    size_t length = (nrow * sizeofScrnRow(screen, ncol));

    if ((result = (Char *) calloc(length, sizeof(Char))) == 0)
	SysError(ERROR_SCALLOC2);

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
allocScrnBuf(XtermWidget xw, unsigned nrow, unsigned ncol, Char ** addr)
{
    TScreen *screen = TScreenOf(xw);
    ScrnBuf base;

    base = allocScrnHead(screen, nrow);
    *addr = allocScrnData(screen, nrow, ncol);

    setupScrnPtrs(xw, base, *addr, nrow, ncol);

    TRACE(("allocScrnBuf %dx%d ->%p\n", nrow, ncol, base));
    return (base);
}

/*
 *  This is called when the screen is resized.
 *  Returns the number of lines the text was moved down (neg for up).
 *  (Return value only necessary with SouthWestGravity.)
 */
static int
Reallocate(XtermWidget xw,
	   ScrnBuf * sbuf,
	   Char ** sbufaddr,
	   unsigned nrow,
	   unsigned ncol,
	   unsigned oldrow,
	   unsigned oldcol)
{
    TScreen *screen = TScreenOf(xw);
    ScrnPtrs *ptrs;
    ScrnBuf newBufHead;
    Char *newBufData;
    unsigned minrows;
    unsigned mincols;
    Char *oldBufData;
    int move_down = 0, move_up = 0;

    /* save/restore row-flags */
    RowFlags *saveFlags;
    int saveFlagLo = -1;
    int saveFlagHi = -1;
    unsigned jump = scrnHeadSize(screen, 1);

    if (sbuf == NULL || *sbuf == NULL) {
	return 0;
    }

    oldBufData = *sbufaddr;

    /*
     * Save the row flags, to reapply after calling setupScrnPtrs.
     */
    saveFlags = TypeCallocN(RowFlags, nrow + oldrow);
    if (saveFlags != NULL) {
	int j;

	ptrs = (ScrnPtrs *) (*sbuf);
	for (j = 0; j < (int) oldrow; ++j) {
	    RowFlags thisFlag = (RowFlags) ptrs->bufHead;
	    if (thisFlag != 0) {
		if (saveFlagLo < 0)
		    saveFlagLo = j;
		saveFlagHi = j;
		saveFlags[j] = thisFlag;
	    }
	    ptrs = ScrnPtrsAddr(ptrs, jump);
	}
	if (saveFlagHi < 0) {
	    free(saveFlags);
	    saveFlags = 0;
	}
    }

    /*
     * realloc sbuf, the pointers to all the lines.
     * If the screen shrinks, remove lines off the top of the buffer
     * if resizeGravity resource says to do so.
     */
    TRACE(("Check move_up, nrow %d vs oldrow %d (resizeGravity %s)\n",
	   nrow, oldrow,
	   BtoS(xw->misc.resizeGravity == SouthWestGravity)));
    if (xw->misc.resizeGravity == SouthWestGravity) {
	if (nrow < oldrow) {
	    /* Remove lines off the top of the buffer if necessary. */
	    move_up = (int) (oldrow - nrow)
		- (xw->screen.max_row - xw->screen.cur_row);
	    if (move_up < 0)
		move_up = 0;
	    /* Overlapping move here! */
	    TRACE(("move_up %d\n", move_up));
	    if (move_up) {
		MoveScrnPtrs(*sbuf,
			     0,
			     (unsigned) move_up,
			     (unsigned) ((int) oldrow - move_up));
	    }
	}
    }
    *sbuf = reallocScrnHead(screen, *sbuf, (unsigned) nrow);
    newBufHead = *sbuf;

    /*
     *  create the new buffer space and copy old buffer contents there
     *  line by line.
     */
    newBufData = allocScrnData(screen, nrow, ncol);
    *sbufaddr = newBufData;

    minrows = (oldrow < nrow) ? oldrow : nrow;
    mincols = (oldcol < ncol) ? oldcol : ncol;
    if (xw->misc.resizeGravity == SouthWestGravity) {
	if (nrow > oldrow) {
	    /* move data down to bottom of expanded screen */
	    move_down = Min((int) (nrow - oldrow), xw->screen.savedlines);
	    newBufData += ((unsigned) move_down) * sizeofScrnRow(screen, ncol);
	}
    }

    extractScrnData(xw, newBufHead, newBufData, minrows, mincols, ncol);

    setupScrnPtrs(xw, newBufHead, *sbufaddr, nrow, ncol);

    if (saveFlags != NULL) {
	int j, k;
	int adjust = 0;

	if (move_down) {
	    adjust = move_down;
	} else if (move_up) {
	    adjust = -move_up;
	}

	ptrs = ScrnPtrsAddr(newBufHead, jump * (unsigned) (saveFlagLo + adjust));
	for (j = saveFlagLo; j <= saveFlagHi; ++j) {
	    k = j + adjust;
	    if (k >= 0 && k < (int) ncol) {
		ptrs->bufHead = (ScrnPtr) saveFlags[j];
	    }
	    ptrs = ScrnPtrsAddr(ptrs, jump);
	}
	free(saveFlags);
    }

    /* Now free the old data */
    free(oldBufData);

    TRACE(("Reallocate %dx%d ->%p\n", nrow, ncol, newBufHead));
    return move_down ? move_down : -move_up;	/* convert to rows */
}

#if OPT_WIDE_CHARS
/*
 * This function reallocates memory if changing the number of Buf offsets.
 * The code is based on Reallocate().
 */
static void
ReallocateBufOffsets(XtermWidget xw,
		     ScrnBuf * sbuf,
		     Char ** sbufaddr,
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
    unsigned new_ptrs = 1 + (unsigned) (screen->max_combining);
    unsigned dstCols = ncol;
    unsigned srcCols = ncol;
    ScrnPtrs *dstPtrs;
    ScrnPtrs *srcPtrs;
    Char *nextPtr;

    assert(nrow != 0);
    assert(ncol != 0);

    oldBufData = *sbufaddr;
    oldBufHead = *sbuf;

    /*
     * Allocate a new ScrnPtrs array, retain the old one until we've copied
     * the data that it points to, as well as non-pointer data, e.g., bufHead.
     *
     * Turn on wide-chars temporarily when constructing pointers, since that is
     * used to decide whether to address the combData[] array, which affects
     * the length of the ScrnPtrs structure.
     */
    screen->wide_chars = True;

    new_jump = scrnHeadSize(screen, 1);
    newBufHead = allocScrnHead(screen, nrow);
    *sbufaddr = allocScrnData(screen, nrow, ncol);
    setupScrnPtrs(xw, newBufHead, *sbufaddr, nrow, ncol);

    screen->wide_chars = False;

    nextPtr = *sbufaddr;

    srcPtrs = (ScrnPtrs *) oldBufHead;
    dstPtrs = (ScrnPtrs *) newBufHead;
    for (i = 0; i < nrow; i++) {
	dstPtrs->bufHead = srcPtrs->bufHead;
	ExtractScrnData(attribs);
#if OPT_ISO_COLORS
	ExtractScrnData(color);
#endif
#if OPT_DEC_CHRSET
	ExtractScrnData(charSets);
#endif
	ExtractScrnData(charData);

	nextPtr += ncol * new_ptrs;
	srcPtrs = ScrnPtrsAddr(srcPtrs, old_jump);
	dstPtrs = ScrnPtrsAddr(dstPtrs, new_jump);
    }

    /* Now free the old data */
    free(oldBufData);
    free(oldBufHead);

    *sbuf = newBufHead;

    TRACE(("ReallocateBufOffsets %dx%d ->%p\n", nrow, ncol, *sbufaddr));
}

/*
 * This function dynamically adds support for wide-characters.
 */
void
ChangeToWide(XtermWidget xw)
{
    TScreen *screen = &(xw->screen);
    int savelines = screen->scrollWidget ? screen->savelines : 0;

    if (screen->wide_chars)
	return;

    TRACE(("ChangeToWide\n"));
    if (xtermLoadWideFonts(xw, True)) {
	if (savelines < 0)
	    savelines = 0;

	/*
	 * If we're displaying the alternate screen, switch the pointers back
	 * temporarily so ReallocateBufOffsets() will operate on the proper
	 * data in altbuf.
	 */
	if (screen->alternate)
	    SwitchBufPtrs(screen);

	ReallocateBufOffsets(xw,
			     &screen->allbuf, &screen->sbuf_address,
			     (unsigned) (MaxRows(screen) + savelines),
			     (unsigned) MaxCols(screen));
	if (screen->altbuf) {
	    ReallocateBufOffsets(xw,
				 &screen->altbuf, &screen->abuf_address,
				 (unsigned) MaxRows(screen),
				 (unsigned) MaxCols(screen));
	}

	screen->wide_chars = True;
	screen->visbuf = VisBuf(screen);

	/*
	 * Switch the pointers back before we start painting on the screen.
	 */
	if (screen->alternate)
	    SwitchBufPtrs(screen);

	update_font_utf8_mode();
	SetVTFont(xw, screen->menu_font_number, True, NULL);
    }
    TRACE(("...ChangeToWide\n"));
}
#endif

/*
 * Clear cells, no side-effects.
 */
void
ClearCells(XtermWidget xw, int flags, unsigned len, int row, int col)
{
    if (len != 0) {
	TScreen *screen = &(xw->screen);
	LineData *ld = newLineData(screen);

	(void) getLineData(screen, row, ld);

	flags |= TERM_COLOR_FLAGS(xw);

	memset(ld->charData + col, ' ', len);
	memset(ld->attribs + col, flags, len);

	if_OPT_ISO_COLORS(screen, {
	    unsigned n;
	    CellColor p = xtermColorPair(xw);
	    for (n = 0; n < len; ++n) {
		ld->color[(unsigned) col + n] = p;
	    }
	});
	if_OPT_DEC_CHRSET({
	    memset(ld->charSets + col, LineCharSet(screen, ld), len);
	});
	if_OPT_WIDE_CHARS(screen, {
	    size_t off;
	    memset(ld->wideData + col, 0, len);
	    for_each_combData(off, ld) {
		memset(ld->combData[off] + col, 0, len * sizeof(IChar));
	    }
	});

	destroyLineData(screen, ld);
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
    TScreen *screen = &(xw->screen);
#endif
    int flags = 0;

    if_OPT_WIDE_CHARS(screen, {
	int kl;
	int kr;
	if (DamagedCells(screen, len, &kl, &kr, INX2ROW(screen, row), col)
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
    if (ScrnHaveSelection(&(xw->screen))) {
	if (xw->screen.keepSelection) {
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
	      IChar * str,
	      unsigned flags,
	      CellColor cur_fg_bg,
	      unsigned length)
{
    TScreen *screen = &(xw->screen);
    LineData *ld;
#if OPT_ISO_COLORS
    CellColor *fb = 0;
#endif
#if OPT_DEC_CHRSET
    Char *cb = 0;
#endif
    Char *attrs;
    int avail = MaxCols(screen) - screen->cur_col;
    Char *chars;
#if OPT_WIDE_CHARS
    Char starcol1, starcol2;
#endif
    unsigned n;
    unsigned real_width = visual_width(str, length);

    (void) cur_fg_bg;

    if (avail <= 0)
	return;
    if (length > (unsigned) avail)
	length = (unsigned) avail;
    if (length == 0 || real_width == 0)
	return;

    ld = newLineData(screen);
    (void) getLineData(screen, screen->cur_row, ld);

    chars = ld->charData + screen->cur_col;
    attrs = ld->attribs + screen->cur_col;

    if_OPT_ISO_COLORS(screen, {
	fb = ld->color + screen->cur_col;
    });
    if_OPT_DEC_CHRSET({
	cb = ld->charSets + screen->cur_col;
    });

#if OPT_WIDE_CHARS
    starcol1 = *chars;
    starcol2 = chars[length - 1];
#endif

    /* write blanks if we're writing invisible text */
    for (n = 0; n < length; ++n) {
	if ((flags & INVISIBLE))
	    chars[n] = ' ';
	else
	    chars[n] = LO_BYTE(str[n]);
    }

#if OPT_BLINK_TEXT
    if ((flags & BLINK) && !(screen->blink_as_bold)) {
	LineSetBlinked(ld);
    }
#endif

#define ERROR_1 0x20
#define ERROR_2 0x00
    if_OPT_WIDE_CHARS(screen, {

	Char *char2;

	if (real_width != length) {
	    Char *char1 = chars;
	    char2 = ld->wideData + screen->cur_col;
	    if (screen->cur_col && starcol1 == HIDDEN_LO && *char2 == HIDDEN_HI
		&& isWide(PACK_PAIR(char1, char2, -1))) {
		char1[-1] = ERROR_1;
		char2[-1] = ERROR_2;
	    }
	    /* if we are overwriting the right hand half of a
	       wide character, make the other half vanish */
	    while (length) {
		int ch = (int) str[0];

		*char1++ = LO_BYTE(*str);
		*char2++ = HI_BYTE(*str);
		str++;
		length--;

		if (isWide(ch)) {
		    *char1 = HIDDEN_LO;
		    *char2 = HIDDEN_HI;
		    char1++;
		    char2++;
		}
	    }

	    if (*char1 == HIDDEN_LO
		&& *char2 == HIDDEN_HI
		&& char1[-1] == HIDDEN_LO
		&& char2[-1] == HIDDEN_HI) {
		*char1 = ERROR_1;
		*char2 = ERROR_2;
	    }
	    /* if we are overwriting the left hand half of a
	       wide character, make the other half vanish */
	} else {
	    char2 = ld->wideData;
	    if (char2 != 0) {
		char2 += screen->cur_col;
		if (screen->cur_col && starcol1 == HIDDEN_LO && *char2 == HIDDEN_HI
		    && isWide(PACK_PAIR(chars, char2, -1))) {
		    chars[-1] = ERROR_1;
		    char2[-1] = ERROR_2;
		}
		/* if we are overwriting the right hand half of a
		   wide character, make the other half vanish */
		if (chars[length] == HIDDEN_LO && char2[length] == HIDDEN_HI &&
		    isWide(PACK_PAIR(chars, char2, length - 1))) {
		    chars[length] = ERROR_1;
		    char2[length] = ERROR_2;
		}
		/* if we are overwriting the left hand half of a
		   wide character, make the other half vanish */
		for (n = 0; n < length; ++n) {
		    if ((flags & INVISIBLE))
			char2[n] = 0;
		    else
			char2[n] = HI_BYTE(str[n]);
		}
	    }
	}
    });

    flags &= ATTRIBUTES;
    flags |= CHARDRAWN;
    memset(attrs, (Char) flags, real_width);

    if_OPT_WIDE_CHARS(screen, {
	size_t off;
	for_each_combData(off, ld) {
	    memset(ld->combData[off] + screen->cur_col,
		   0,
		   real_width * sizeof(IChar));
	}
    });
    if_OPT_ISO_COLORS(screen, {
	unsigned j;
	for (j = 0; j < real_width; ++j)
	    fb[j] = cur_fg_bg;
    });
    if_OPT_DEC_CHRSET({
	memset(cb, LineCharSet(screen, ld), real_width);
    });

    if_OPT_WIDE_CHARS(screen, {
	screen->last_written_col = screen->cur_col + (int) real_width - 1;
	screen->last_written_row = screen->cur_row;
    });

    if_OPT_XMC_GLITCH(screen, {
	Resolve_XMC(xw);
    });

    destroyLineData(screen, ld);
    return;
}

/*
 * Saves pointers to the n lines beginning at sb + where, and clears the lines
 */
static void
ScrnClearLines(XtermWidget xw, ScrnBuf sb, int where, unsigned n, unsigned size)
{
    TScreen *screen = &(xw->screen);
    ScrnPtr *base;
    unsigned jump = scrnHeadSize(screen, 1);
    unsigned i, j;
    LineData *work = newLineData(screen);
    unsigned flags = TERM_COLOR_FLAGS(xw);

    TRACE(("ScrnClearLines(where %d, n %d, size %d)\n", where, n, size));

    assert(n != 0);
    assert(size != 0);

    /* save n lines at where */
    (void) ScrnPointers(screen, n);
    SaveScrnPtrs(sb, (unsigned) where, n);

    /* clear contents of old rows */
    base = screen->save_ptr;
    for (i = 0; i < n; ++i) {
	fillLineData(screen, base, work);
	*(work->bufHead) = 0;

	memset(work->charData, 0, size);
	if (TERM_COLOR_FLAGS(xw)) {
	    memset(work->attribs, (int) flags, size);
#if OPT_ISO_COLORS
	    {
		CellColor p = xtermColorPair(xw);
		for (j = 0; j < size; ++j) {
		    work->color[j] = p;
		}
	    }
#endif
	} else {
	    memset(work->attribs, 0, size);
#if OPT_ISO_COLORS
	    memset(work->color, 0, size * sizeof(work->color[0]));
#endif
	}
	if_OPT_DEC_CHRSET({
	    memset(work->charSets, 0, size);
	});
#if OPT_WIDE_CHARS
	if (screen->wide_chars) {
	    size_t off;

	    memset(work->wideData, 0, size);
	    for (off = 0; off < work->combSize; ++off) {
		work->combData[off] = 0;
	    }
	}
#endif
	base = ScrnBufAddr(base, jump);
    }
    destroyLineData(screen, work);
}

void
ScrnAllocBuf(XtermWidget xw)
{
    TScreen *screen = TScreenOf(xw);

    if (screen->allbuf == NULL) {
	int nrows = MaxRows(screen);
	int savelines = screen->scrollWidget ? screen->savelines : 0;

	TRACE(("ScrnAllocBuf %dx%d (%d)\n",
	       nrows, MaxCols(screen), screen->savelines));

	screen->allbuf = allocScrnBuf(xw,
				      (unsigned) (nrows + screen->savelines),
				      (unsigned) (MaxCols(screen)),
				      &screen->sbuf_address);
	screen->visbuf = VisBuf(screen);
    }
    return;
}

size_t
ScrnPointers(TScreen * screen, size_t len)
{
    size_t result = scrnHeadSize(screen, len);

    if (result > screen->save_len) {
	if (screen->save_len)
	    screen->save_ptr = (ScrnPtr *) realloc(screen->save_ptr, result);
	else
	    screen->save_ptr = (ScrnPtr *) malloc(result);
	screen->save_len = len;
	if (screen->save_ptr == 0)
	    SysError(ERROR_SAVE_PTR);
    }
    TRACE2(("ScrnPointers %ld ->%p\n", (long) len, screen->save_ptr));
    return result;
}

/*
 * Inserts n blank lines at sb + where, treating last as a bottom margin.
 * size is the size of each entry in sb.
 */
void
ScrnInsertLine(XtermWidget xw, ScrnBuf sb, int last, int where,
	       unsigned n, unsigned size)
{
    TScreen *screen = &(xw->screen);

    TRACE(("ScrnInsertLine(last %d, where %d, n %d, size %d)\n",
	   last, where, n, size));

    assert(where >= 0);
    assert(last >= (int) n);
    assert(last >= where);

    assert(n != 0);
    assert(size != 0);

    /* save n lines at bottom */
    ScrnClearLines(xw, sb, (last -= (int) n - 1), n, size);

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
    MoveScrnPtrs(sb,
		 (unsigned) (where + (int) n),
		 (unsigned) where,
		 (unsigned) (last - where));

    /* reuse storage for new lines at where */
    RestoreScrnPtrs(sb, (unsigned) where, n);
}

/*
 * Deletes n lines at sb + where, treating last as a bottom margin.
 * size is the size of each entry in sb.
 */
void
ScrnDeleteLine(XtermWidget xw, ScrnBuf sb, int last, int where,
	       unsigned n, unsigned size)
{
    TScreen *screen = &(xw->screen);

    TRACE(("ScrnDeleteLine(last %d, where %d, n %d, size %d)\n",
	   last, where, n, size));

    assert(where >= 0);
    assert(last >= where + (int) n - 1);

    assert(n != 0);
    assert(size != 0);

    ScrnClearLines(xw, sb, where, n, size);

    /* move up lines */
    MoveScrnPtrs(sb,
		 (unsigned) where,
		 (unsigned) (where + (int) n),
		 (unsigned) ((last -= ((int) n - 1)) - where));

    /* reuse storage for new bottom lines */
    RestoreScrnPtrs(sb, (unsigned) last, n);
}

/*
 * Inserts n blanks in screen at current row, col.  Size is the size of each
 * row.
 */
void
ScrnInsertChar(XtermWidget xw, unsigned n)
{
#define MemMove(data) \
    	for (j = last - 1; j >= (col + (int) n); --j) \
	    data[j] = data[j - (int) n]

    TScreen *screen = &(xw->screen);
    int last = MaxCols(screen);
    int row = screen->cur_row;
    int col = screen->cur_col;
    int j, nbytes;
    LineData *ld;

    if (last <= (col + (int) n)) {
	if (last <= col)
	    return;
	n = (unsigned) (last - col);
    }
    nbytes = (last - (col + (int) n));

    assert(screen->cur_col >= 0);
    assert(screen->cur_row >= 0);
    assert(n > 0);
    assert(last > (int) n);

    if_OPT_WIDE_CHARS(screen, {
	int xx = INX2ROW(screen, screen->cur_row);
	int kl;
	int kr = screen->cur_col;
	if (DamagedCells(screen, n, &kl, (int *) 0, xx, kr) && kr > kl) {
	    ClearCells(xw, 0, (unsigned) (kr - kl + 1), row, kl);
	}
	kr = screen->max_col - (int) n + 1;
	if (DamagedCells(screen, n, &kl, (int *) 0, xx, kr) && kr > kl) {
	    ClearCells(xw, 0, (unsigned) (kr - kl + 1), row, kl);
	}
    });

    if ((ld = getLineData(screen, row, NULL)) != 0) {
	MemMove(ld->charData);
	MemMove(ld->attribs);

	if_OPT_ISO_COLORS(screen, {
	    MemMove(ld->color);
	});
	if_OPT_DEC_CHRSET({
	    MemMove(ld->charSets);
	});
	if_OPT_WIDE_CHARS(screen, {
	    size_t off;
	    MemMove(ld->wideData);
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

    TScreen *screen = &(xw->screen);
    int last = MaxCols(screen);
    int row = screen->cur_row;
    int col = screen->cur_col;
    int j, nbytes;
    LineData *ld;

    if (last <= (col + (int) n)) {
	if (last <= col)
	    return;
	n = (unsigned) (last - col);
    }
    nbytes = (last - (col + (int) n));

    assert(screen->cur_col >= 0);
    assert(screen->cur_row >= 0);
    assert(n > 0);
    assert(last > (int) n);

    if_OPT_WIDE_CHARS(screen, {
	int kl;
	int kr;
	if (DamagedCells(screen, n, &kl, &kr,
			 INX2ROW(screen, screen->cur_row),
			 screen->cur_col))
	    ClearCells(xw, 0, (unsigned) (kr - kl + 1), row, kl);
    });

    if ((ld = getLineData(screen, row, NULL)) != 0) {
	MemMove(ld->charData);
	MemMove(ld->attribs);

	if_OPT_ISO_COLORS(screen, {
	    MemMove(ld->color);
	});
	if_OPT_DEC_CHRSET({
	    MemMove(ld->charSets);
	});
	if_OPT_WIDE_CHARS(screen, {
	    size_t off;
	    MemMove(ld->wideData);
	    for_each_combData(off, ld) {
		MemMove(ld->combData[off]);
	    }
	});
	LineClrWrapped(ld);
    }
    ClearCells(xw, 0, n, row, (last - (int) n));

#undef MemMove
}

/*
 * Repaints the area enclosed by the parameters.
 * Requires: (toprow, leftcol), (toprow + nrows, leftcol + ncols) are
 * 	     coordinates of characters in screen;
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
    TScreen *screen = &(xw->screen);
    LineData *ld = newLineData(screen);
    int y = toprow * FontHeight(screen) + screen->border;
    int row;
    int maxrow = toprow + nrows - 1;
    int scrollamt = screen->scroll_amt;
    int max = screen->max_row;
    unsigned gc_changes = 0;
#ifdef __CYGWIN__
    static char first_time = 1;
#endif
    static int recurse = 0;

    TRACE(("ScrnRefresh (%d,%d) - (%d,%d)%s {{\n",
	   toprow, leftcol,
	   nrows, ncols,
	   force ? " force" : ""));

    if (screen->cursorp.col >= leftcol
	&& screen->cursorp.col <= (leftcol + ncols - 1)
	&& screen->cursorp.row >= ROW2INX(screen, toprow)
	&& screen->cursorp.row <= ROW2INX(screen, maxrow))
	screen->cursor_state = OFF;

    for (row = toprow; row <= maxrow; y += FontHeight(screen), row++) {
#if OPT_ISO_COLORS
	CellColor *fb = 0;
#define ColorOf(col) fb[col]
#endif
#if OPT_DEC_CHRSET
	Char *cb = 0;
#endif
#if OPT_WIDE_CHARS
	int wideness = 0;
	Char *widec = 0;
#define WIDEC_PTR(cell) widec ? &widec[cell] : 0
#define BLANK_CEL(cell) ((chars[cell] == ' ') && (widec == 0 || widec[cell] == 0))
#else
#define BLANK_CEL(cell) (chars[cell] == ' ')
#endif
	Char cs = 0;
	Char *chars;
	Char *attrs;
	int col = leftcol;
	int maxcol = leftcol + ncols - 1;
	int hi_col = maxcol;
	int lastind;
	unsigned flags;
	unsigned test;
	CellColor fg_bg;
	unsigned fg = 0, bg = 0;
	int x;
	GC gc;
	Bool hilite;

	(void) fg;
	(void) bg;

	if (row < screen->top_marg || row > screen->bot_marg)
	    lastind = row;
	else
	    lastind = row - scrollamt;

	TRACE2(("ScrnRefresh row=%d lastind=%d/%d\n", row, lastind, max));
	if (lastind < 0 || lastind > max)
	    continue;

	if (getLineData(screen, ROW2INX(screen, lastind), ld) == 0)
	    break;

	chars = ld->charData;
	attrs = ld->attribs;

	if_OPT_DEC_CHRSET({
	    cb = ld->charSets;
	});

	if_OPT_WIDE_CHARS(screen, {
	    widec = ld->wideData;
	});

	if_OPT_WIDE_CHARS(screen, {
	    /* This fixes an infinite recursion bug, that leads
	       to display anomalies. It seems to be related to
	       problems with the selection. */
	    if (recurse < 3) {
		/* adjust to redraw all of a widechar if we just wanted
		   to draw the right hand half */
		if (leftcol > 0 &&
		    (PACK_PAIR(chars, widec, leftcol)) == HIDDEN_CHAR &&
		    isWide(PACK_PAIR(chars, widec, leftcol - 1))) {
		    leftcol--;
		    ncols++;
		    col = leftcol;
		}
	    } else {
		fprintf(stderr, "This should not happen. Why is it so?\n");
	    }
	});

	if (row < screen->startH.row || row > screen->endH.row ||
	    (row == screen->startH.row && maxcol < screen->startH.col) ||
	    (row == screen->endH.row && col >= screen->endH.col)) {
#if OPT_DEC_CHRSET
	    if (cb != 0) {
		/*
		 * Temporarily change dimensions to double-sized characters so
		 * we can reuse the recursion on this function.
		 */
		if (CSET_DOUBLE(*cb)) {
		    col /= 2;
		    maxcol /= 2;
		}
	    }
#endif
	    /*
	     * If row does not intersect selection; don't hilite blanks.
	     */
	    if (!force) {
		while (col <= maxcol && (attrs[col] & ~BOLD) == 0 &&
		       BLANK_CEL(col))
		    col++;

		while (col <= maxcol && (attrs[maxcol] & ~BOLD) == 0 &&
		       BLANK_CEL(maxcol))
		    maxcol--;
	    }
#if OPT_DEC_CHRSET
	    if (cb != 0) {
		if (CSET_DOUBLE(*cb)) {
		    col *= 2;
		    maxcol *= 2;
		}
	    }
#endif
	    hilite = False;
	} else {
	    /* row intersects selection; split into pieces of single type */
	    if (row == screen->startH.row && col < screen->startH.col) {
		recurse++;
		ScrnRefresh(xw, row, col, 1, screen->startH.col - col,
			    force);
		col = screen->startH.col;
	    }
	    if (row == screen->endH.row && maxcol >= screen->endH.col) {
		recurse++;
		ScrnRefresh(xw, row, screen->endH.col, 1,
			    maxcol - screen->endH.col + 1, force);
		maxcol = screen->endH.col - 1;
	    }

	    /*
	     * If we're highlighting because the user is doing cut/paste,
	     * trim the trailing blanks from the highlighted region so we're
	     * showing the actual extent of the text that'll be cut.  If
	     * we're selecting a blank line, we'll highlight one column
	     * anyway.
	     *
	     * We don't do this if the mouse-hilite mode is set because that
	     * would be too confusing.
	     *
	     * The default if the highlightSelection resource isn't set will
	     * highlight the whole width of the terminal, which is easy to
	     * see, but harder to use (because trailing blanks aren't as
	     * apparent).
	     */
	    if (screen->highlight_selection
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
	    if (cb != 0) {
		if (CSET_DOUBLE(*cb)) {
		    col /= 2;
		    maxcol /= 2;
		}
		cs = cb[col];
	    }
	});

	flags = attrs[col];
#if OPT_WIDE_CHARS
	if (widec)
	    wideness = isWide(PACK_PAIR(chars, widec, col));
	else
	    wideness = 0;
#endif
	if_OPT_ISO_COLORS(screen, {
	    fb = ld->color;
	    fg_bg = ColorOf(col);
	    fg = extract_fg(xw, fg_bg, flags);
	    bg = extract_bg(xw, fg_bg, flags);
	});

	gc = updatedXtermGC(xw, flags, fg_bg, hilite);
	gc_changes |= (flags & (FG_COLOR | BG_COLOR));

	x = LineCursorX(screen, ld, col);
	lastind = col;

	for (; col <= maxcol; col++) {
	    if ((attrs[col] != flags)
		|| (hilite && (col > hi_col))
#if OPT_ISO_COLORS
		|| ((flags & FG_COLOR)
		    && (extract_fg(xw, ColorOf(col), attrs[col]) != fg))
		|| ((flags & BG_COLOR)
		    && (extract_bg(xw, ColorOf(col), attrs[col]) != bg))
#endif
#if OPT_WIDE_CHARS
		|| (widec
		    && ((isWide(PACK_PAIR(chars, widec, col))) != wideness)
		    && !((PACK_PAIR(chars, widec, col)) == HIDDEN_CHAR))
#endif
#if OPT_DEC_CHRSET
		|| (cb != 0 && (cb[col] != cs))
#endif
		) {
		assert(col >= lastind);
		TRACE(("ScrnRefresh looping drawXtermText %d..%d:%s\n",
		       lastind, col,
		       visibleChars(PAIRED_CHARS(&chars[lastind],
						 WIDEC_PTR(lastind)),
				    (unsigned) (col - lastind))));

		test = flags;
		checkVeryBoldColors(test, fg);

		x = drawXtermText(xw, test & DRAWX_MASK, gc, x, y,
				  cs,
				  PAIRED_CHARS(&chars[lastind], WIDEC_PTR(lastind)),
				  (unsigned) (col - lastind), 0);

		if_OPT_WIDE_CHARS(screen, {
		    int i;
		    size_t off;

		    for_each_combData(off, ld) {
			IChar *com_off = ld->combData[off];

			for (i = lastind; i < col; i++) {
			    int my_x = LineCursorX(screen, ld, i);
			    int base = PACK_PAIR(chars, widec, i);

			    if (isWide(base))
				my_x = LineCursorX(screen, ld, i - 1);

			    if (com_off[i] != 0)
				drawXtermText(xw,
					      (test & DRAWX_MASK)
					      | NOBACKGROUND,
					      gc, my_x, y, cs,
					      PAIRED_CHARS(
							      loByteIChars(com_off
									   +
									   i, 1),
							      hiByteIChars(com_off
									   +
									   i, 1)),
					      1, isWide(base));
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
		if_OPT_DEC_CHRSET({
		    if (cb != 0) {
			cs = cb[col];
		    }
		});
#if OPT_WIDE_CHARS
		if (widec)
		    wideness = isWide(PACK_PAIR(chars, widec, col));
#endif

		gc = updatedXtermGC(xw, flags, fg_bg, hilite);
		gc_changes |= (flags & (FG_COLOR | BG_COLOR));
	    }

	    if (chars[col] == 0) {
#if OPT_WIDE_CHARS
		if (widec == 0 || widec[col] == 0)
#endif
		    chars[col] = ' ';
	    }
	}

	assert(col >= lastind);
	TRACE(("ScrnRefresh calling drawXtermText %d..%d:%s\n",
	       lastind, col,
	       visibleChars(PAIRED_CHARS(&chars[lastind], WIDEC_PTR(lastind)),
			    (unsigned) (col - lastind))));

	test = flags;
	checkVeryBoldColors(test, fg);

	drawXtermText(xw, test & DRAWX_MASK, gc, x, y,
		      cs,
		      PAIRED_CHARS(&chars[lastind], WIDEC_PTR(lastind)),
		      (unsigned) (col - lastind), 0);

	if_OPT_WIDE_CHARS(screen, {
	    int i;
	    size_t off;

	    for_each_combData(off, ld) {
		IChar *com_off = ld->combData[off];

		for (i = lastind; i < col; i++) {
		    int my_x = LineCursorX(screen, ld, i);
		    int base = PACK_PAIR(chars, widec, i);

		    if (isWide(base))
			my_x = LineCursorX(screen, ld, i - 1);

		    if (com_off[i] != 0)
			drawXtermText(xw,
				      (test & DRAWX_MASK)
				      | NOBACKGROUND,
				      gc, my_x, y, cs,
				      PAIRED_CHARS(
						      loByteIChars(com_off +
								   i, 1),
						      hiByteIChars(com_off +
								   i, 1)),
				      1, isWide(base));
		}
	    }
	});

	resetXtermGC(xw, flags, hilite);
    }

    /*
     * If we're in color mode, reset the various GC's to the current
     * screen foreground and background so that other functions (e.g.,
     * ClearRight) will get the correct colors.
     */
    if_OPT_ISO_COLORS(screen, {
	if (gc_changes & FG_COLOR)
	    SGR_Foreground(xw, xw->cur_foreground);
	if (gc_changes & BG_COLOR)
	    SGR_Background(xw, xw->cur_background);
    });

#if defined(__CYGWIN__) && defined(TIOCSWINSZ)
    if (first_time == 1) {
	TTYSIZE_STRUCT ts;

	first_time = 0;
	TTYSIZE_ROWS(ts) = nrows;
	TTYSIZE_COLS(ts) = ncols;
	ts.ws_xpixel = xw->core.width;
	ts.ws_ypixel = xw->core.height;
	SET_TTYSIZE(screen->respond, ts);
    }
#endif
    recurse--;

    destroyLineData(screen, ld);

    TRACE(("...}} ScrnRefresh\n"));
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
    TScreen *screen = &(xw->screen);

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
    TScreen *screen = &(xw->screen);
    unsigned len = (unsigned) MaxCols(screen);
    int row;

    TRACE(("ClearBufRows %d..%d\n", first, last));
    for (row = first; row <= last; row++) {
	LineData *ld = getLineData(screen, ROW2INX(screen, row), NULL);
	if_OPT_DEC_CHRSET({
	    /* clearing the whole row resets the doublesize characters */
	    LINEDATA_CSET(ld) = CSET_SWL;
	});
	LineClrWrapped(ld);
	ClearCells(xw, 0, len, row, 0);
    }
}

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
  8. Returns 0
  */
int
ScreenResize(XtermWidget xw,
	     int width,
	     int height,
	     unsigned *flags)
{
    TScreen *screen = &(xw->screen);
    int code, rows, cols;
    int border = 2 * screen->border;
    int move_down_by = 0;
#ifdef TTYSIZE_STRUCT
    TTYSIZE_STRUCT ts;
#endif
    Window tw = VWindow(screen);

    TRACE(("ScreenResize %dx%d border %d font %dx%d\n",
	   height, width, border,
	   FontHeight(screen), FontWidth(screen)));

    assert(width > 0);
    assert(height > 0);

    if (screen->is_running) {
	/* clear the right and bottom internal border because of NorthWest
	   gravity might have left junk on the right and bottom edges */
	if (width >= FullWidth(screen)) {
	    XClearArea(screen->display, tw,
		       FullWidth(screen), 0,	/* right edge */
		       0, (unsigned) height,	/* from top to bottom */
		       False);
	}
	if (height >= FullHeight(screen)) {
	    XClearArea(screen->display, tw,
		       0, FullHeight(screen),	/* bottom */
		       (unsigned) width, 0,	/* all across the bottom */
		       False);
	}
    }

    TRACE(("...computing rows/cols: %.2f %.2f\n",
	   (double) (height - border) / FontHeight(screen),
	   (double) (width - border - ScrollbarWidth(screen)) / FontWidth(screen)));

    rows = (height - border) / FontHeight(screen);
    cols = (width - border - ScrollbarWidth(screen)) / FontWidth(screen);
    if (rows < 1)
	rows = 1;
    if (cols < 1)
	cols = 1;

    /* update buffers if the screen has changed size */
    if (MaxRows(screen) != rows || MaxCols(screen) != cols) {
	int savelines = (screen->scrollWidget
			 ? screen->savelines
			 : 0);
	int delta_rows = rows - MaxRows(screen);

	TRACE(("...ScreenResize chars %dx%d\n", rows, cols));

	if (screen->is_running) {
	    if (screen->cursor_state)
		HideCursor();
	    if (screen->alternate
		&& xw->misc.resizeGravity == SouthWestGravity)
		/* swap buffer pointers back to make this work */
		SwitchBufPtrs(screen);
	    if (screen->altbuf)
		(void) Reallocate(xw,
				  &screen->altbuf,
				  &screen->abuf_address,
				  (unsigned) rows,
				  (unsigned) cols,
				  (unsigned) MaxRows(screen),
				  (unsigned) MaxCols(screen));
	    move_down_by = Reallocate(xw,
				      &screen->allbuf,
				      &screen->sbuf_address,
				      (unsigned) (rows + savelines),
				      (unsigned) cols,
				      (unsigned) (MaxRows(screen) + savelines),
				      (unsigned) MaxCols(screen));
	    screen->visbuf = VisBuf(screen);
	}

	AdjustSavedCursor(xw, move_down_by);
	set_max_row(screen, screen->max_row + delta_rows);
	set_max_col(screen, cols - 1);

	if (screen->is_running) {
	    if (xw->misc.resizeGravity == SouthWestGravity) {
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

		if (screen->alternate)
		    SwitchBufPtrs(screen);	/* put the pointers back */
	    }
	}

	/* adjust scrolling region */
	set_tb_margins(screen, 0, screen->max_row);
	*flags &= ~ORIGIN;

	if (screen->cur_row > screen->max_row)
	    set_cur_row(screen, screen->max_row);
	if (screen->cur_col > screen->max_col)
	    set_cur_col(screen, screen->max_col);

	screen->fullVwin.height = height - border;
	screen->fullVwin.width = width - border - screen->fullVwin.sb_info.width;

    } else if (FullHeight(screen) == height && FullWidth(screen) == width)
	return (0);		/* nothing has changed at all */

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
	    (Dimension) (screen->iconVwin.width + 2 * xw->misc.icon_border_width);
	changes.height = screen->iconVwin.fullheight =
	    (Dimension) (screen->iconVwin.height + 2 * xw->misc.icon_border_width);
	changes.border_width = (int) xw->misc.icon_border_width;

	TRACE(("resizing icon window %dx%d\n", changes.height, changes.width));
	XConfigureWindow(XtDisplay(xw), screen->iconVwin.window,
			 CWWidth | CWHeight | CWBorderWidth, &changes);
    }
#endif /* NO_ACTIVE_ICON */

#ifdef TTYSIZE_STRUCT
    /* Set tty's idea of window size */
    TTYSIZE_ROWS(ts) = rows;
    TTYSIZE_COLS(ts) = cols;
#ifdef USE_STRUCT_WINSIZE
    ts.ws_xpixel = width;
    ts.ws_ypixel = height;
#endif
    code = SET_TTYSIZE(screen->respond, ts);
    TRACE(("return %d from SET_TTYSIZE %dx%d\n", code, rows, cols));
    (void) code;

#if defined(SIGWINCH) && defined(USE_STRUCT_TTYSIZE)
    if (screen->pid > 1) {
	int pgrp;

	TRACE(("getting process-group\n"));
	if (ioctl(screen->respond, TIOCGPGRP, &pgrp) != -1) {
	    TRACE(("sending SIGWINCH to process group %d\n", pgrp));
	    kill_process_group(pgrp, SIGWINCH);
	}
    }
#endif /* SIGWINCH */

#else
    TRACE(("ScreenResize cannot do anything to pty\n"));
#endif /* TTYSIZE_STRUCT */
    return (0);
}

/*
 * Return true if any character cell starting at [row,col], for len-cells is
 * nonnull.
 */
Bool
non_blank_line(TScreen * screen,
	       int row,
	       int col,
	       int len)
{
    int i;
    Bool found = False;
    LineData *ld = getLineData(screen, row, NULL);

    if (ld != 0) {
	for (i = col; i < len; i++) {
	    if (ld->charData[i]) {
		found = True;
		break;
	    }
	}

	if_OPT_WIDE_CHARS(screen, {
	    if (!found) {
		if (ld->wideData != 0) {
		    for (i = col; i < len; i++) {
			if (ld->wideData[i]) {
			    found = True;
			    break;
			}
		    }
		}
	    }
	});
    }
    return found;
}

/*
 * Rectangle parameters start from one.
 */
#define minRectRow(screen) (getMinRow(screen) + 1)
#define minRectCol(screen) (getMinCol(screen) + 1)
#define maxRectRow(screen) (getMaxRow(screen) + 1)
#define maxRectCol(screen) (getMaxCol(screen) + 1)

static int
limitedParseRow(XtermWidget xw, TScreen * screen, int row)
{
    int min_row = minRectRow(screen);
    int max_row = maxRectRow(screen);

    if (row < min_row)
	row = min_row;
    else if (row > max_row)
	row = max_row;
    return row;
}

static int
limitedParseCol(XtermWidget xw, TScreen * screen, int col)
{
    int min_col = minRectCol(screen);
    int max_col = maxRectCol(screen);

    (void) xw;
    if (col < min_col)
	col = min_col;
    else if (col > max_col)
	col = max_col;
    return col;
}

#define LimitedParse(num, func, dft) \
	func(xw, screen, (nparams > num) ? params[num] : dft)

/*
 * Copy the rectangle boundaries into a struct, providing default values as
 * needed.
 */
void
xtermParseRect(XtermWidget xw, int nparams, int *params, XTermRect * target)
{
    TScreen *screen = &(xw->screen);

    memset(target, 0, sizeof(*target));
    target->top = LimitedParse(0, limitedParseRow, minRectRow(screen));
    target->left = LimitedParse(1, limitedParseCol, minRectCol(screen));
    target->bottom = LimitedParse(2, limitedParseRow, maxRectRow(screen));
    target->right = LimitedParse(3, limitedParseCol, maxRectCol(screen));
    TRACE(("parsed rectangle %d,%d %d,%d\n",
	   target->top,
	   target->left,
	   target->bottom,
	   target->right));
}

static Bool
validRect(XtermWidget xw, XTermRect * target)
{
    TScreen *screen = &(xw->screen);

    TRACE(("comparing against screensize %dx%d\n",
	   maxRectRow(screen),
	   maxRectCol(screen)));
    return (target != 0
	    && target->top >= minRectRow(screen)
	    && target->left >= minRectCol(screen)
	    && target->top <= target->bottom
	    && target->left <= target->right
	    && target->top <= maxRectRow(screen)
	    && target->right <= maxRectCol(screen));
}

/*
 * Fills a rectangle with the given 8-bit character and video-attributes.
 * Colors and double-size attribute are unmodified.
 */
void
ScrnFillRectangle(XtermWidget xw,
		  XTermRect * target,
		  int value,
		  unsigned flags,
		  Bool keepColors)
{
    TScreen *screen = &(xw->screen);

    TRACE(("filling rectangle with '%c' flags %#x\n", value, flags));
    if (validRect(xw, target)) {
	LineData *ld = newLineData(screen);
	unsigned left = (unsigned) (target->left - 1);
	unsigned size = (unsigned) (target->right - (int) left);
	unsigned attrs = flags;
	int row, col;

	attrs &= ATTRIBUTES;
	attrs |= CHARDRAWN;
	for (row = target->bottom - 1; row >= (target->top - 1); row--) {
	    (void) getLineData(screen, row, ld);

	    TRACE(("filling %d [%d..%d]\n", row, left, left + size));

	    /*
	     * Fill attributes, preserving "protected" flag, as well as
	     * colors if asked.
	     */
	    for (col = (int) left; col < target->right; ++col) {
		unsigned temp = ld->attribs[col];

		if (!keepColors) {
		    temp &= ~(FG_COLOR | BG_COLOR);
		}
		temp = attrs | (temp & (FG_COLOR | BG_COLOR | PROTECTED));
		temp |= CHARDRAWN;
		ld->attribs[col] = (Char) temp;
#if OPT_ISO_COLORS
		if (attrs & (FG_COLOR | BG_COLOR)) {
		    if_OPT_ISO_COLORS(screen, {
			ld->color[col] = xtermColorPair(xw);
		    });
		}
#endif
	    }

	    memset(ld->charData + left, (Char) value, size);
	    if_OPT_WIDE_CHARS(screen, {
		size_t off;
		memset(ld->wideData + left, 0, size);
		for_each_combData(off, ld) {
		    memset(ld->combData[off] + left, 0, size * sizeof(IChar));
		}
	    })
	}
	ScrnUpdate(xw,
		   target->top - 1,
		   target->left - 1,
		   (target->bottom - target->top) + 1,
		   (target->right - target->left) + 1,
		   False);

	destroyLineData(screen, ld);
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
ScrnCopyRectangle(XtermWidget xw, XTermRect * source, int nparam, int *params)
{
    TScreen *screen = &(xw->screen);

    TRACE(("copying rectangle\n"));

    if (validRect(xw, source)) {
	XTermRect target;
	xtermParseRect(xw,
		       ((nparam > 3) ? 2 : (nparam - 1)),
		       params,
		       &target);
	if (validRect(xw, &target)) {
	    Cardinal high = (Cardinal) (source->bottom - source->top) + 1;
	    Cardinal wide = (Cardinal) (source->right - source->left) + 1;
	    Cardinal size = (high * wide);
	    int row, col;
	    Cardinal j, k;

	    LineData *ld = newLineData(screen);
	    if (ld != 0) {
		CellData *cells = newCellData(xw, size);

		if (cells != 0) {

		    TRACE(("OK - make copy %dx%d\n", high, wide));
		    target.bottom = target.top + (int) (high - 1);
		    target.right = target.left + (int) (wide - 1);

		    for (row = source->top - 1; row < source->bottom; ++row) {
			(void) getLineData(screen, row, ld);
			j = (Cardinal) (row - (source->top - 1));
			for (col = source->left - 1; col < source->right; ++col) {
			    k = (Cardinal) (col - (source->left - 1));
			    saveCellData(screen, cells,
					 (j * wide) + k,
					 ld, col);
			}
		    }
		    for (row = target.top - 1; row < target.bottom; ++row) {
			(void) getLineData(screen, row, ld);
			j = (Cardinal) (row - (target.top - 1));
			for (col = target.left - 1; col < target.right; ++col) {
			    k = (Cardinal) (col - (target.left - 1));
			    if (row >= getMinRow(screen)
				&& row <= getMaxRow(screen)
				&& col >= getMinCol(screen)
				&& col <= getMaxCol(screen)) {
				if (j < high && k < wide) {
				    restoreCellData(screen, cells,
						    (j * wide) + k,
						    ld, col);
				} else {
				    /* FIXME - clear the target cell? */
				}
				ld->attribs[col] |= CHARDRAWN;
			    }
			}
		    }
		    free(cells);
		    destroyLineData(screen, ld);

		    ScrnUpdate(xw,
			       (target.top - 1),
			       (target.left - 1),
			       (target.bottom - target.top) + 1,
			       ((target.right - target.left) + 1),
			       False);
		}
	    }
	}
    }
}

/*
 * Modifies the video-attributes only - so selection (not a video attribute) is
 * unaffected.  Colors and double-size flags are unaffected as well.
 *
 * FIXME: our representation for "invisible" does not work with this operation,
 * since the attribute byte is fully-allocated for other flags.  The logic
 * is shown for INVISIBLE because it's harmless, and useful in case the
 * CHARDRAWN or PROTECTED flags are reassigned.
 */
void
ScrnMarkRectangle(XtermWidget xw,
		  XTermRect * target,
		  Bool reverse,
		  int nparam,
		  int *params)
{
    TScreen *screen = &(xw->screen);
    Bool exact = (screen->cur_decsace == 2);

    TRACE(("%s %s\n",
	   reverse ? "reversing" : "marking",
	   (exact
	    ? "rectangle"
	    : "region")));

    if (validRect(xw, target)) {
	LineData *ld = newLineData(screen);
	int top = target->top - 1;
	int bottom = target->bottom - 1;
	int row, col;
	int n;

	for (row = top; row <= bottom; ++row) {
	    int left = ((exact || (row == top))
			? (target->left - 1)
			: getMinCol(screen));
	    int right = ((exact || (row == bottom))
			 ? (target->right - 1)
			 : getMaxCol(screen));

	    (void) getLineData(screen, row, ld);

	    TRACE(("marking %d [%d..%d]\n", row, left, right));
	    for (col = left; col <= right; ++col) {
		unsigned flags = ld->attribs[col];

		for (n = 0; n < nparam; ++n) {
#if OPT_TRACE
		    if (row == top && col == left)
			TRACE(("attr param[%d] %d\n", n + 1, params[n]));
#endif
		    if (reverse) {
			switch (params[n]) {
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
			    flags &= ~SGR_MASK;
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
			    flags &= ~BOLD;
			    break;
			case 24:
			    flags &= ~UNDERLINE;
			    break;
			case 25:
			    flags &= ~BLINK;
			    break;
			case 27:
			    flags &= ~INVERSE;
			    break;
			case 28:
			    flags &= ~INVISIBLE;
			    break;
			}
		    }
		}
#if OPT_TRACE
		if (row == top && col == left)
		    TRACE(("first mask-change is %#x\n",
			   ld->attribs[col] ^ flags));
#endif
		ld->attribs[col] = (Char) flags;
	    }
	}
	ScrnRefresh(xw,
		    (target->top - 1),
		    (exact ? (target->left - 1) : getMinCol(screen)),
		    (target->bottom - target->top) + 1,
		    (exact
		     ? ((target->right - target->left) + 1)
		     : (getMaxCol(screen) - getMinCol(screen) + 1)),
		    False);

	destroyLineData(screen, ld);
    }
}

/*
 * Resets characters to space, except where prohibited by DECSCA.  Video
 * attributes (including color) are untouched.
 */
void
ScrnWipeRectangle(XtermWidget xw,
		  XTermRect * target)
{
    TScreen *screen = &(xw->screen);

    TRACE(("wiping rectangle\n"));

    if (validRect(xw, target)) {
	LineData *ld = newLineData(screen);
	int top = target->top - 1;
	int bottom = target->bottom - 1;
	int row, col;

	for (row = top; row <= bottom; ++row) {
	    int left = (target->left - 1);
	    int right = (target->right - 1);

	    TRACE(("wiping %d [%d..%d]\n", row, left, right));

	    (void) getLineData(screen, row, ld);
	    for (col = left; col <= right; ++col) {
		if (!((screen->protected_mode == DEC_PROTECT)
		      && (ld->attribs[col] & PROTECTED))) {
		    ld->attribs[col] |= CHARDRAWN;
		    ld->charData[col] = ' ';
		    if_OPT_WIDE_CHARS(screen, {
			size_t off;
			ld->wideData[col] = '\0';
			for_each_combData(off, ld) {
			    ld->combData[off][col] = '\0';
			}
		    })
		}
	    }
	}
	ScrnUpdate(xw,
		   (target->top - 1),
		   (target->left - 1),
		   (target->bottom - target->top) + 1,
		   ((target->right - target->left) + 1),
		   False);

	destroyLineData(screen, ld);
    }
}
#endif /* OPT_DEC_RECTOPS */
