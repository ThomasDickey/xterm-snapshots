/* $XTermId: cachedGCs.c,v 1.2 2007/01/19 00:37:50 tom Exp $ */

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

#include <xterm.h>

/*
 * hide calls to
 *	XSetBackground()
 *	XSetForeground()
 *	XCreateGC()
 *	XtGetGC()
 *	XFreeGC()
 * by associating an integer with each GC, maintaining a cache which
 * reflects frequency of use rather than most recent usage.
 *
 * FIXME: move the cache into XtermWidget
 * FIXME: do I need more params, e.g., for mask?
 * FIXME: I need something to reset or cache the font state
 */
typedef struct {
    GC gc;
    XFontStruct *font;
    Pixel fg;
    Pixel bg;
} CgsCache;

static CgsCache *
myCache(XtermWidget xw GCC_UNUSED, int cgsId)
{
    static CgsCache my_cache[gcMAX];
    CgsCache *result = 0;

    if (cgsId >= 0 && cgsId < gcMAX)
	result = my_cache + cgsId;

    return result;
}

void
setCgsForeground(XtermWidget xw, int cgsId, Pixel fg)
{
    CgsCache *me;
    if ((me = myCache(xw, cgsId)) != 0) {
	me->fg = fg;
    }
}

void
setCgsBackground(XtermWidget xw, int cgsId, Pixel bg)
{
    CgsCache *me;
    if ((me = myCache(xw, cgsId)) != 0) {
	me->bg = bg;
    }
}

void
setCgsFont(XtermWidget xw, int cgsId, XFontStruct * font)
{
    CgsCache *me;
    if ((me = myCache(xw, cgsId)) != 0) {
	me->font = font;
    }
}

GC
getCgs(XtermWidget xw, int cgsId)
{
    CgsCache *me;
    GC result = 0;

    if ((me = myCache(xw, cgsId)) != 0) {
	if (me->gc == 0) {
	    TScreen *screen = &(xw->screen);
	    XGCValues xgcv;
	    XtGCMask mask;

	    xgcv.font = me->font->fid;
	    mask = (GCForeground | GCBackground | GCFont);
	    xgcv.foreground = me->fg;
	    xgcv.background = me->bg;
	    me->gc = XCreateGC(screen->display, VWindow(screen), mask, &xgcv);
	}
	result = me->gc;
    }
    return result;
}

void
freeCgs(XtermWidget xw, int cgsId)
{
    CgsCache *me;

    if ((me = myCache(xw, cgsId)) != 0) {
	if (me->gc == 0) {
	    XFreeGC(xw->screen.display, me->gc);
	    me->gc = 0;
	}
    }
}
