/*
 * $XFree86: xc/programs/xterm/fontutils.c,v 1.4 1998/11/22 10:37:46 dawes Exp $
 */

/************************************************************

Copyright 1998 by Thomas E. Dickey <dickey@clark.net>

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

/*
 * A portion of this module (for FontNameProperties) is adapted from EMU 1.3;
 * it constructs font names with specific properties changed, e.g., for bold
 * and double-size characters.
 */

#include <fontutils.h>

#include <data.h>
#include <menu.h>
#include <xterm.h>

#include <stdio.h>

#define MAX_FONTNAME 200

/*
 * A structure to hold the relevant properties from a font
 * we need to make a well formed font name for it.
 */
typedef struct {
     /* registry, foundry, family */
     char *beginning;
     char *width;
     /* slant, width, add_style */
     char *middle;
     int pixel_size;
     char *point_size;
     int res_x;
     int res_y;
     char *spacing;
     /* charset registry, charset encoding */
     char *end;
} FontNameProperties;

/*
 * Returns the fields from start to stop in a dash- separated string.  This
 * function will modify the source, putting '\0's in the appropiate place and
 * moving the beginning forward to after the '\0'
 *
 * This will NOT work for the last field (but we won't need it).
 */
static char *
n_fields(char **source, int start, int stop)
{
	int i;
	char *str , *str1;

	/*
	* find the start-1th dash
	*/
	for (i = start-1, str = *source; i; i--, str++)
		if ((str = strchr(str, '-')) == 0)
			return 0;

	/*
	* find the stopth dash
	*/
	for (i = stop - start + 1, str1 = str; i; i--, str1++)
		if ((str1 = strchr(str1, '-')) == 0)
			return 0;

	/*
	* put a \0 at the end of the fields
	*/
	*(str1 - 1) = '\0';

	/*
	* move source forward
	*/
	*source = str1;

	return str;
}


/*
 * Gets the font properties from a given font structure.  We use the FONT name
 * to find them out, since that seems easier.
 *
 * Returns a pointer to a static FontNameProperties structure
 * or NULL on error.
 */
static FontNameProperties *
get_font_name_props(Display *dpy, XFontStruct *fs)
{
	static FontNameProperties props;
	static char *last_name;

	register XFontProp *fp;
	register int i;
	Atom fontatom = XInternAtom (dpy, "FONT", False);
	char *name;
	char *str;

	/*
	* first get the full font name
	*/
	for (name = 0, i = 0, fp = fs->properties;
		i < fs->n_properties;
			i++, fp++)
		if (fp->name == fontatom)
			name = XGetAtomName (dpy, fp->card32);

	if (name == 0)
		return 0;

	/*
	 * XGetAtomName allocates memory - don't leak
	 */
	if (last_name != 0)
		XFree(last_name);
	last_name = name;

	/*
	* Now split it up into parts and put them in
	* their places. Since we are using parts of
	* the original string, we must not free the Atom Name
	*/

	/* registry, foundry, family */
	if ((props.beginning = n_fields(&name, 1, 3)) == 0)
		return 0;

	/* weight is the next */
	if ((props.width = n_fields(&name, 1, 1)) == 0)
		return 0;

	/* slant, width, add style */
	if ((props.middle = n_fields(&name, 1, 3)) == 0)
		return 0;

	/* pixel size */
	if ((str = n_fields(&name, 1, 1)) == 0)
		return 0;
	if ((props.pixel_size = atoi(str)) == 0)
		return 0;

	/* point size */
	if ((props.point_size = n_fields(&name, 1, 1)) == 0)
		return 0;

	/* res_x */
	if ((str = n_fields(&name, 1, 1)) == 0)
		return 0;
	if ((props.res_x = atoi(str)) == 0)
		return 0;

	/* res_y */
	if ((str = n_fields(&name, 1, 1)) == 0)
		return 0;
	if ((props.res_y = atoi(str)) == 0)
		return 0;

	/* spacing */
	if ((props.spacing = n_fields(&name, 1, 1)) == 0)
		return 0;

	/* skip the average width */
	if ((str = n_fields(&name, 1, 1)) == 0)
		return 0;

	/* the rest: charset registry and charset encoding */
	props.end = name;

	return &props;
}

/*
 * Take the given font props and try to make a well formed font name specifying
 * the same base font and size and everything, but in bold.  The return value
 * comes from a static variable, so be careful if you reuse it.
 */
static char *
bold_font_name(FontNameProperties *props)
{
     static char ret[MAX_FONTNAME];

     /*
      * Put together something in the form
      * "<beginning>-bold-<middle>-<pixel_size>-<point_size>-<res_x>-<res_y>"\
      * "-<spacing>-*-<end>"
      */
     sprintf(ret, "%s-bold-%s-%d-%s-%d-%d-%s-*-%s",
	     props->beginning,
	     props->middle,
	     props->pixel_size,
	     props->point_size,
	     props->res_x,
	     props->res_y,
	     props->spacing,
	     props->end);

     return ret;
}

#ifdef OPT_DEC_CHRSET
/*
 * Take the given font props and try to make a well formed font name specifying
 * the same base font but changed depending on the given attributes and chrset.
 *
 * For double width fonts, we just double the X-resolution, for double height
 * fonts we double the pixel-size and Y-resolution
 */
char *
xtermSpecialFont(unsigned atts, unsigned chrset)
{
#if OPT_TRACE
	static char old_spacing[80];
	static FontNameProperties old_props;
#endif
	TScreen *screen = &term->screen;
	FontNameProperties *props;
	char tmp[MAX_FONTNAME];
	char *ret;
	char *width;
	int pixel_size;
	int res_x;
	int res_y;

	props = get_font_name_props(screen->display, screen->fnt_norm);
	if (props == 0)
		return 0;

	pixel_size = props->pixel_size;
	res_x = props->res_x;
	res_y = props->res_y;
	if (atts & BOLD)
		width = "bold";
	else
		width = props->width;

	if (CSET_DOUBLE(chrset))
		res_x *= 2;

	if (chrset == CSET_DHL_TOP 
	 || chrset == CSET_DHL_BOT) {
		res_y *= 2;
		pixel_size *= 2;
	}

#if OPT_TRACE
	if (old_props.res_x      != res_x
	 || old_props.res_x      != res_y
	 || old_props.pixel_size != pixel_size
	 || strcmp(old_props.spacing, props->spacing)) {
		TRACE(("xtermSpecialFont(atts = %#x, chrset = %#x)\n", atts, chrset))
		TRACE(("res_x      = %d\n", res_x))
		TRACE(("res_y      = %d\n", res_y))
		TRACE(("point_size = %s\n", props->point_size))
		TRACE(("pixel_size = %d\n", pixel_size))
		TRACE(("spacing    = %s\n", props->spacing))
		old_props.res_x      = res_x;
		old_props.res_x      = res_y;
		old_props.pixel_size = pixel_size;
		old_props.spacing    = strcpy(old_spacing, props->spacing);
	}
#endif

	sprintf(tmp, "%s-%s-%s-%d-%s-%d-%d-%s-*-%s",
		props->beginning,
		width,
		props->middle,
		pixel_size,
		props->point_size,
		res_x,
		res_y,
		props->spacing,
		props->end);

	ret = XtMalloc(strlen(tmp) + 1);
	strcpy(ret, tmp);

	return ret;
}
#endif /* OPT_DEC_CHRSET */

/*
 * Double-check the fontname that we asked for versus what the font server
 * actually gave us.  The larger fixed fonts do not always have a matching bold
 * font, and the font server may try to scale another font or otherwise
 * substitute a mismatched font.
 *
 * If we cannot get what we requested, we will fallback to the original
 * behavior, which simulates bold by overstriking each character at one pixel
 * offset.
 */
static int
got_bold_font(Display *dpy, XFontStruct *fs, char *fontname)
{
	FontNameProperties *fp;
	char oldname[MAX_FONTNAME], *p = oldname;
	strcpy(p, fontname);
	if ((fp = get_font_name_props(dpy, fs)) == 0)
		return 0;
	fontname = bold_font_name(fp);
	while (*p && *fontname) {
		if (char2lower(*p++) != char2lower(*fontname++))
			return 0;
	}
	return (*p == *fontname);	/* both should be NUL */
}

/*
 * If the font server tries to adjust another font, it may not adjust it
 * properly.  Check that the bounding boxes are compatible.  Otherwise we'll
 * leave trash on the display when we mix normal and bold fonts.
 */
static int
same_font_size(XFontStruct *nfs, XFontStruct *bfs)
{
	return (
		nfs->ascent           == bfs->ascent
	 &&	nfs->descent          == bfs->descent
	 &&	nfs->min_bounds.width == bfs->min_bounds.width
	 &&	nfs->min_bounds.width == bfs->min_bounds.width
	 &&	nfs->max_bounds.width == bfs->max_bounds.width
	 &&	nfs->max_bounds.width == bfs->max_bounds.width);
}

#define EmptyFont(fs) ((fs)->ascent + (fs)->descent == 0 \
		   ||  (fs)->max_bounds.width == 0)

int
xtermLoadFont (
	TScreen *screen,
	char *nfontname,
	char *bfontname,
	Bool doresize,
	int fontnum)
{
	/* FIXME: use XFreeFontInfo */
	FontNameProperties *fp;
	XFontStruct *nfs = NULL;
	XFontStruct *bfs = NULL;
	XGCValues xgcv;
	unsigned long mask;
	GC new_normalGC      = NULL;
	GC new_normalboldGC  = NULL;
	GC new_reverseGC     = NULL;
	GC new_reverseboldGC = NULL;
	Pixel new_normal;
	Pixel new_revers;
	char *tmpname = NULL;
	Boolean proportional = False;

	if (!nfontname) return 0;

	if (fontnum == fontMenu_fontescape
	 && nfontname != screen->menu_font_names[fontnum]) {
		tmpname = (char *) malloc (strlen(nfontname) + 1);
		if (!tmpname)
			return 0;
		strcpy (tmpname, nfontname);
	}

	TRACE(("xtermLoadFont normal %s\n", nfontname))

	if (!(nfs = XLoadQueryFont (screen->display, nfontname))) goto bad;
	if (EmptyFont(nfs))
		goto bad;		/* can't use a 0-sized font */

	if (bfontname == 0) {
		fp = get_font_name_props(screen->display, nfs);
		if (fp != 0) {
			bfontname = bold_font_name(fp);
			TRACE(("...derived bold %s\n", bfontname))
		}
		if (bfontname == 0
		 || (bfs = XLoadQueryFont (screen->display, bfontname)) == 0) {
			bfs = nfs;
			TRACE(("...cannot load a matching bold font\n"))
		} else if (!same_font_size(nfs, bfs)
		 || !got_bold_font(screen->display, bfs, bfontname)) {
			XFreeFont(screen->display, bfs);
			bfs = nfs;
			TRACE(("...did not get a matching bold font\n"))
		}
	} else if ((bfs = XLoadQueryFont (screen->display, bfontname)) == 0) {
		bfs = nfs;
		TRACE(("...cannot load bold font %s\n", bfontname))
	}

	if (EmptyFont(bfs))
		goto bad;		/* can't use a 0-sized font */

	if (nfs->min_bounds.width != nfs->max_bounds.width
	 || bfs->min_bounds.width != bfs->max_bounds.width
	 || nfs->min_bounds.width != bfs->min_bounds.width
	 || nfs->max_bounds.width != bfs->max_bounds.width) {
		TRACE(("Proportional font!\n"))
		proportional = True;
	}

	mask = (GCFont | GCForeground | GCBackground | GCGraphicsExposures |
	GCFunction);

	new_normal = getXtermForeground(term->flags, term->cur_foreground);
	new_revers = getXtermBackground(term->flags, term->cur_background);

	xgcv.font = nfs->fid;
	xgcv.foreground = new_normal;
	xgcv.background = new_revers;
	xgcv.graphics_exposures = TRUE;	/* default */
	xgcv.function = GXcopy;

	new_normalGC = XtGetGC((Widget)term, mask, &xgcv);
	if (!new_normalGC) goto bad;

	if (nfs == bfs) {			/* there is no bold font */
		new_normalboldGC = new_normalGC;
	} else {
		xgcv.font = bfs->fid;
		new_normalboldGC = XtGetGC((Widget)term, mask, &xgcv);
		if (!new_normalboldGC) goto bad;
	}

	xgcv.font = nfs->fid;
	xgcv.foreground = new_revers;
	xgcv.background = new_normal;
	new_reverseGC   = XtGetGC((Widget)term, mask, &xgcv);
	if (!new_reverseGC) goto bad;

	if (nfs == bfs) {			/* there is no bold font */
		new_reverseboldGC = new_reverseGC;
	} else {
		xgcv.font = bfs->fid;
		new_reverseboldGC = XtGetGC((Widget)term, mask, &xgcv);
		if (!new_reverseboldGC) goto bad;
	}

	if (NormalGC(screen) != NormalBoldGC(screen))
		XtReleaseGC ((Widget) term, NormalBoldGC(screen));
	XtReleaseGC ((Widget) term, NormalGC(screen));

	if (ReverseGC(screen) != ReverseBoldGC(screen))
		XtReleaseGC ((Widget) term, ReverseBoldGC(screen));
	XtReleaseGC ((Widget) term, ReverseGC(screen));

	NormalGC(screen)      = new_normalGC;
	NormalBoldGC(screen)  = new_normalboldGC;
	ReverseGC(screen)     = new_reverseGC;
	ReverseBoldGC(screen) = new_reverseboldGC;

	/*
	 * If we're switching fonts, free the old ones.  Otherwise we'll leak
	 * the memory that is associated with the old fonts.  The
	 * XLoadQueryFont call allocates a new XFontStruct.
	 */
	if (screen->fnt_bold != 0
	 && screen->fnt_bold != screen->fnt_norm)
		XFreeFont(screen->display, screen->fnt_bold);
	if (screen->fnt_norm != 0)
		XFreeFont(screen->display, screen->fnt_norm);

	screen->fnt_norm = nfs;
	screen->fnt_bold = bfs;
	screen->fnt_prop = proportional;

	screen->enbolden = (nfs == bfs);
	set_menu_font (False);
	screen->menu_font_number = fontnum;
	set_menu_font (True);
	if (tmpname) {			/* if setting escape or sel */
		if (screen->menu_font_names[fontnum])
			free (screen->menu_font_names[fontnum]);
		screen->menu_font_names[fontnum] = tmpname;
		if (fontnum == fontMenu_fontescape) {
			set_sensitivity (term->screen.fontMenu,
				fontMenuEntries[fontMenu_fontescape].widget,
				TRUE);
		}
	}
	set_cursor_gcs (screen);
	xtermUpdateFontInfo (screen, doresize);
	return 1;

bad:
	if (tmpname)
		free (tmpname);
	if (new_normalGC)
		XtReleaseGC ((Widget) term, screen->fullVwin.normalGC);
	if (new_normalGC && new_normalGC != new_normalboldGC)
		XtReleaseGC ((Widget) term, new_normalboldGC);
	if (new_reverseGC)
		XtReleaseGC ((Widget) term, new_reverseGC);
	if (new_reverseGC && new_reverseGC != new_reverseboldGC)
		XtReleaseGC ((Widget) term, new_reverseboldGC);
	if (nfs)
		XFreeFont (screen->display, nfs);
	if (bfs && nfs != bfs)
		XFreeFont (screen->display, bfs);
	return 0;
}

/*
 * Set the limits for the box that outlines the cursor.
 */
void
xtermSetCursorBox (TScreen *screen)
{
	static XPoint VTbox[NBOX];
	XPoint	*vp;

	vp = &VTbox[1];
	(vp++)->x = FontWidth(screen) - 1;
	(vp++)->y = FontHeight(screen) - 1;
	(vp++)->x = -(FontWidth(screen) - 1);
	vp->y = -(FontHeight(screen) - 1);
	screen->box = VTbox;
}

/*
 * After loading a new font, update the structures that use its size.
 */
void
xtermUpdateFontInfo (TScreen *screen, Bool doresize)
{
	int i, j, width, height, scrollbar_width;

	screen->fullVwin.f_width  = (screen->fnt_norm->max_bounds.width);
	screen->fullVwin.f_height = (screen->fnt_norm->ascent +
	screen->fnt_norm->descent);
	scrollbar_width = (term->misc.scrollbar
			? screen->scrollWidget->core.width +
			  screen->scrollWidget->core.border_width
			: 0);
	i = 2 * screen->border + scrollbar_width;
	j = 2 * screen->border;
	width  = (screen->max_col + 1) * screen->fullVwin.f_width + i;
	height = (screen->max_row + 1) * screen->fullVwin.f_height + j;
	screen->fullVwin.fullwidth  = width;
	screen->fullVwin.fullheight = height;
	screen->fullVwin.width  = width - i;
	screen->fullVwin.height = height - j;

	if (doresize) {
		if (VWindow(screen)) {
			XClearWindow (screen->display, VWindow(screen));
		}
		DoResizeScreen (term);	/* set to the new natural size */
		if (screen->scrollWidget)
			ResizeScrollBar (screen);
		Redraw ();
	}
	xtermSetCursorBox (screen);
}
