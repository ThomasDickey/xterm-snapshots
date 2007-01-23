/* $XTermId: cachedGCs.c,v 1.14 2007/01/22 23:32:04 tom Exp $ */

/************************************************************

Copyright 2007 by Thomas E. Dickey

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

#include <data.h>

/*
 * hide calls to
 *	XCreateGC()
 *	XFreeGC()
 *	XGetGCValues()
 *	XSetBackground()
 *	XSetForeground()
 *	XtGetGC()
 *	XtReleaseGC()
 * by associating an integer with each GC, maintaining a cache which
 * reflects frequency of use rather than most recent usage.
 *
 * FIXME: do I need more params, e.g., for mask?
 * FIXME: I need something to reset or cache the font state
 * FIXME: XtermFonts should hold gc, font, fs.
 */
typedef struct {
    GC gc;
    XFontStruct *font;
    Pixel fg;
    Pixel bg;
    XtGCMask mask;		/* changes since the last getCgs() */
} CgsCache;

/*
 * FIXME: move the cache into XtermWidget
 */
static CgsCache *
myCache(XtermWidget xw GCC_UNUSED, VTwin * cgsWin GCC_UNUSED, CgsEnum cgsId)
{
    static CgsCache main_cache[gcMAX];
    CgsCache *my_cache = main_cache;
    CgsCache *result = 0;

    if ((int) cgsId >= 0 && cgsId < gcMAX) {
#ifndef NO_ACTIVE_ICON
	static CgsCache icon_cache[gcMAX];
	if (cgsWin == &(xw->screen.iconVwin))
	    my_cache = icon_cache;
#endif
	result = my_cache + cgsId;
    }

    return result;
}

static Display *
myDisplay(XtermWidget xw)
{
    return xw->screen.display;
}

static Drawable
myDrawable(XtermWidget xw, VTwin * cgsWin)
{
    Drawable drawable = 0;

    if (cgsWin != 0 && cgsWin->window != 0)
	drawable = cgsWin->window;
    if (drawable == 0)
	drawable = RootWindowOfScreen(XtScreen(xw));
    return drawable;
}

/*
 * Use the "setCgsXXXX()" calls to initialize parameters for a new GC.
 */
void
setCgsFore(XtermWidget xw, VTwin * cgsWin, CgsEnum cgsId, Pixel fg)
{
    CgsCache *me;

    if ((me = myCache(xw, cgsWin, cgsId)) != 0) {
	if (me->fg != fg) {
	    me->fg = fg;
	    me->mask |= GCForeground;
	}
    }
}

void
setCgsBack(XtermWidget xw, VTwin * cgsWin, CgsEnum cgsId, Pixel bg)
{
    CgsCache *me;

    if ((me = myCache(xw, cgsWin, cgsId)) != 0) {
	if (me->bg != bg) {
	    me->bg = bg;
	    me->mask |= GCBackground;
	}
    }
}

void
setCgsFont(XtermWidget xw, VTwin * cgsWin, CgsEnum cgsId, XFontStruct * font)
{
    CgsCache *me;

    if ((me = myCache(xw, cgsWin, cgsId)) != 0) {
	if (font == 0)
	    font = xw->screen.fnts[fNorm];
	if (me->font != font) {
	    me->font = font;
	    me->mask |= GCFont;
	}
    }
}

/*
 * Return a GC associated with the given id, allocating if needed.
 */
GC
getCgs(XtermWidget xw, VTwin * cgsWin, CgsEnum cgsId)
{
    XGCValues xgcv;
    XtGCMask mask;
    CgsCache *me;
    GC result = 0;

    if ((me = myCache(xw, cgsWin, cgsId)) != 0) {
	if (me->gc == 0) {
	    if (me->font == 0) {
		setCgsFont(xw, cgsWin, cgsId, 0);
	    }

	    memset(&xgcv, 0, sizeof(xgcv));
	    xgcv.font = me->font->fid;
	    mask = (GCForeground | GCBackground | GCFont);

	    switch (cgsId) {
	    case gcNorm:
	    case gcBold:
	    case gcNormReverse:
	    case gcBoldReverse:
#if OPT_WIDE_CHARS
	    case gcWide:
	    case gcWBold:
	    case gcWideReverse:
	    case gcWBoldReverse:
#endif
		mask |= (GCGraphicsExposures | GCFunction);
		xgcv.graphics_exposures = True;		/* default */
		xgcv.function = GXcopy;
		break;
	    case gcVTcursNormal:	/* FALLTHRU */
	    case gcVTcursFilled:	/* FALLTHRU */
	    case gcVTcursReverse:	/* FALLTHRU */
	    case gcVTcursOutline:	/* FALLTHRU */
		break;
#if OPT_TEK4014
	    case gcTKcurs:	/* FALLTHRU */
		/* FIXME */
#endif
	    case gcMAX:	/* should not happen */
		return 0;
	    }
	    xgcv.foreground = me->fg;
	    xgcv.background = me->bg;

	    me->gc = XCreateGC(myDisplay(xw), myDrawable(xw, cgsWin), mask, &xgcv);
	    TRACE(("getCgs(%d) created %p\n", cgsId, me->gc));
	} else if (me->mask != 0) {
	    mask = me->mask;
	    memset(&xgcv, 0, sizeof(xgcv));
	    xgcv.font = me->font->fid;
	    xgcv.foreground = me->fg;
	    xgcv.background = me->bg;
	    XChangeGC(myDisplay(xw), me->gc, mask, &xgcv);
	    TRACE(("getCgs(%d) updated %p\n", cgsId, me->gc));
	} else if (0) {
	    TRACE(("getCgs(%d) reused: %p\n", cgsId, me->gc));
	}
	me->mask = 0;
	result = me->gc;
    }
    return result;
}

/*
 * Copy the parameters (except GC of course) from one cache record to another.
 */
void
copyCgs(XtermWidget xw, VTwin * cgsWin, CgsEnum dstCgsId, CgsEnum srcCgsId)
{
    if (dstCgsId != srcCgsId) {
	CgsCache *me;

	if ((me = myCache(xw, cgsWin, srcCgsId)) != 0) {
	    freeCgs(xw, cgsWin, dstCgsId);
	    setCgsFont(xw, cgsWin, dstCgsId, me->font);
	    setCgsFore(xw, cgsWin, dstCgsId, me->fg);
	    setCgsBack(xw, cgsWin, dstCgsId, me->bg);
	}
    }
}

/*
 * Swap the cache records, e.g., when doing reverse-video.
 */
void
swapCgs(XtermWidget xw, VTwin * cgsWin, CgsEnum dstCgsId, CgsEnum srcCgsId)
{
    if (dstCgsId != srcCgsId) {
	CgsCache *dst;
	CgsCache *src;

	if ((src = myCache(xw, cgsWin, srcCgsId)) != 0) {
	    if ((dst = myCache(xw, cgsWin, dstCgsId)) != 0) {
		CgsCache tmp;
		tmp = *dst;
		*dst = *src;
		*src = tmp;
	    }
	}
    }
}

/*
 * Free any GC associated with the given id.
 */
GC
freeCgs(XtermWidget xw, VTwin * cgsWin, CgsEnum cgsId)
{
    CgsCache *me;

    if ((me = myCache(xw, cgsWin, cgsId)) != 0) {
	if (me->gc != 0) {
	    TRACE(("freeCgs(%d) %p\n", cgsId, me->gc));
	    XFreeGC(xw->screen.display, me->gc);
	    me->gc = 0;
	}
    }
    return 0;
}
