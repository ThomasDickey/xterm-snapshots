/* $XTermId: graphics.c,v 1.1 2013/05/28 16:53:32 Ross.Combs Exp $ */

/*
 * Copyright 1999-2011,2012 by Thomas E. Dickey
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

#include <xterm.h>

#include <stdio.h>
#include <math.h>
#include <ctype.h>

#include <data.h>
#include <VTparse.h>
#include <ptyx.h>

#include <assert.h>
#include <graphics.h>


/* TODO:
 * ReGIS:
 * - everything
 * sixel:
 * - erase graphic when erasing screen
 * - erase text under graphics if bg not transparent
 * - delete graphic if scrolled past end of scrollback
 * - delete graphic if all pixels are transparent/erased
 * - dynamic memory allocation (and limits)
 * - fix drawing over window border areas
 * - base initial palette size and colors on the terminal id
 * - auto convert color graphics in VT330 mode
 * VT55/VT105 waveform graphics
 * - everything
 */


/* font sizes:
 * VT510:
 *   80 Columns 132 Columns Maximum Number of Lines
 *   10 x 16   6 x 16  26 lines + keyboard indicator line
 *   10 x 10   6 x 10  42 lines + keyboard indicator line
 *   10 x 8    6 x 8   53 lines + keyboard indicator line
 *   10 x 13   6 x 13  26 lines + keyboard indicator line
*/

/***====================================================================***/
/*
 * Parse numeric parameters which have the operator as a prefix rather than a
 * suffix as in ANSI format.
 *
 *  #             0
 *  #1            1
 *  #1;           1
 *  "1;2;640;480  4
 *  #1;2;0;0;0    5
 */
static void
parse_prefixedtype_params(ANSI *params, const char **string)
{
    const char *cp = *string;
    ParmType nparam = 0;
    int last_empty = 1;

    memset(params, 0, sizeof(*params));
    params->a_final = CharOf(*cp);
    if (*cp != '\0')
	cp++;

    while (*cp != '\0') {
	Char ch = CharOf(*cp);

	if (isdigit(ch)) {
	    last_empty = 0;
	    if (nparam < NPARAM) {
		params->a_param[nparam] =
		    (ParmType) ((params->a_param[nparam] * 10)
				+ (ch - '0'));
	    }
	} else if (ch == ';') {
	    last_empty = 1;
	    nparam++;
	} else if (ch == ' ' || ch == '\r' || ch == '\n') {
	    /* EMPTY */ ;
	} else {
	    break;
	}
	cp++;
    }

    *string = cp;
    if (!last_empty)
	nparam++;
    if (nparam > NPARAM)
	params->a_nparam = NPARAM;
    else
	params->a_nparam = nparam;
}


typedef struct {
    Pixel pix;
    short r, g, b;
    short allocated;
} ColorRegister;

typedef unsigned short SixelColor;

#define MAX_COLOR_REGISTERS 256U
#define COLOR_HOLE ((unsigned short)MAX_COLOR_REGISTERS)
#define BUFFER_WIDTH 1000
#define BUFFER_HEIGHT 1000
typedef struct {
    SixelColor pixels[BUFFER_HEIGHT*BUFFER_WIDTH];
    ColorRegister color_registers[MAX_COLOR_REGISTERS];
    int max_width; /* largest image which can be stored */
    int max_height; /* largest image which can be stored */
    SixelColor current_register;
    int valid_registers; /* for wrap-around behavior */
    int preserve_background; /* 0: set to color 0, 1: unchanged */
    int aspect_numerator;
    int aspect_denominator;
    int declared_width; /* size as reported by the application */
    int declared_height; /* size as reported by the application */
    int actual_width; /* size measured during parsing */
    int actual_height; /* size measured during parsing */
    int charrow; /* upper left starting point in characters */
    int charcol; /* upper left starting point in characters */
    int pixw; /* width of graphic pixels in screen pixels */
    int pixh; /* height of graphic pixels in screen pixels */
    int row; /* context used during parsing */
    int col; /* context used during parsing */
    int bufferid; /* which screen buffer the graphic is associated with */
    int valid; /* if the graphic has been initialized */
} SixelGraphic;

#define MAX_SIXEL_GRAPHICS 8U
static SixelGraphic sixel_graphics[MAX_SIXEL_GRAPHICS];

/* sixel scrolling:
 * VK100/GIGI ? (did it even support Sixel?)
 * VT125      unsupported
 * VT240      unsupported
 * VT241      unsupported
 * VT330      setting
 * VT340      setting
 * dxterm     ?
 */

static void
init_sixel_background(SixelGraphic *graphic)
{
    SixelColor bgcolor;
    int r, c;

    if (graphic->preserve_background)
	bgcolor = COLOR_HOLE;
    else
	bgcolor = 0; /* FIXME: what is the default background register (per-model?) */

    TRACE(("initializing sixel background to size=%dx%d bgcolor=%hu\n", graphic->declared_width, graphic->declared_height, bgcolor));
    for (r=0; r < graphic->max_height; r++)
	for (c=0; c < graphic->max_width; c++)
	    if (c < graphic->declared_width && r < graphic->declared_height)
		graphic->pixels[r * graphic->max_width + c] = bgcolor;
	    else
		graphic->pixels[r * graphic->max_width + c] = COLOR_HOLE;
}


static void
set_sixel(SixelGraphic *graphic, int sixel)
{
    SixelColor color;
    int pix;

    color = graphic->current_register;
    TRACE(("drawing sixel at pos=%d,%d color=%hu (hole=%d, [%d,%d,%d])\n", graphic->col, graphic->row, color,
	   color == COLOR_HOLE,
	   color != COLOR_HOLE ? (unsigned int)graphic->color_registers[color].r : 0U,
	   color != COLOR_HOLE ? (unsigned int)graphic->color_registers[color].g : 0U,
	   color != COLOR_HOLE ? (unsigned int)graphic->color_registers[color].b : 0U));
    for (pix=0; pix<6; pix++)
	if (graphic->col < graphic->max_width &&
	    graphic->row + pix < graphic->max_height) {
	    if (sixel & (1 << pix)) {
		if (graphic->col + 1 > graphic->actual_width) {
		    graphic->actual_width = graphic->col + 1;
		}
		if (graphic->row + pix + 1 > graphic->actual_height) {
		    graphic->actual_height = graphic->row + pix + 1;
		}
		graphic->pixels[(graphic->row + pix) * graphic->max_width + graphic->col] = color;
	    }
	} else
	    TRACE(("sixel pixel %d out of bounds\n", pix));
}


static void
make_sixel_color(SixelGraphic *graphic, SixelColor c, int r, int g, int b)
{
    graphic->color_registers[c].r = r;
    graphic->color_registers[c].g = g;
    graphic->color_registers[c].b = b;
}


static void
init_sixel_graphic(SixelGraphic *graphic)
{
    TRACE(("initializing sixel graphic\n"));

    memset(graphic->pixels, 0, sizeof(graphic->pixels));
    memset(graphic->color_registers, 0, sizeof(graphic->color_registers));

    /*
     * dimensions (REGIS logical, physical):
     * VK100/GIGI  768x4??  768x2??
     * VT125       768x460  768x230(+10status) (1:2 aspect ratio, REGIS halves vertical addresses)
     * VT240       800x460  800x230(+10status) (1:2 aspect ratio, REGIS halves vertical addresses)
     * VT241       800x460  800x230(+10status) (1:2 aspect ratio, REGIS halves vertical addresses)
     * VT330       800x460  800x480(+?status)
     * VT340       800x?    800x480(+?status)
     * dxterm      ?x?      variable?
     */
    graphic->max_width = BUFFER_WIDTH;
    graphic->max_height = BUFFER_HEIGHT;

    /* default isn't white on the VT240, but not sure what it is */
    graphic->current_register = 3; /* FIXME: using green, but not sure what it should be */

/*
When an application selects the monochrome map: the terminal sets the 16 entries of the color map to the default monochrome gray level. Therefore, the original colors are lost when changing from the color map to the monochrome map.

If you change the color value (green, red, blue) using the Color Set-Up screen or a ReGIS command, the VT340 sets the gray scale by using the formula (2G + R)/3.

When an application selects the color map: the terminal sets the 16 entries of the color map to the default (color) color map.
*/

    /*
     * color capabilities:
     * VK100/GIGI  1 plane (12x1 pixel attribute blocks) colorspace is 8 fixed colors (black, white, red, green, blue, cyan, yellow, magenta)
     * VT125       2 planes (4 registers) colorspace is (64?) (color), ? (grayscale)
     * VT240       2 planes (4 registers) colorspace is ? shades (grayscale)
     * VT241       2 planes (4 registers) colorspace is ? (color), ? shades (grayscale)
     * VT330       4 planes (16 registers) colorspace is 4 (64?) shades (grayscale)
     * VT340       4 planes (16 registers) colorspace is r16g16b16 (color), 16 shades (grayscale)
     * dxterm      ?
     */
    graphic->valid_registers = 16;

    /*
     * text and graphics interactions:
     * VK100/GIGI                text writes on top of graphics buffer, color attribute shared with text
     * VT240,VT241,VT330,VT340   text writes on top of graphics buffer
     * VT125                     graphics buffer overlaid on top of text in B&W display, text not present in color display
     */

    /*
     * default color registers:
     * VK100/GIGI:
     *  fixed
     * VT125:
     *  0: 0%      0%
     *  1: 33%     blue
     *  2: 66%     red
     *  3: 100%    green
     * VT240:
     *  0: 0%      0%
     *  1: 33%     blue
     *  2: 66%     red
     *  3: 100%    green
     * VT241:
     *  0: 0%      0%
     *  1: 33%     blue
     *  2: 66%     red
     *  3: 100%    green
     * VT330:
     *  0: 0%      0%      (bg for light on dark mode)
     *  1: 33%     red
     *  2: 66%     green   (fg for light on dark mode)
     *  3: 100%    yellow
     * VT340:
     *  0: 0%      0%
     *  1: 13%     blue
     *  2: 26%     red
     *  3: 40%     green
     *  4: 6%      magenta
     *  5: 20%     cyan
     *  6: 33%     yellow
     *  7: 46%     50%
     *  8: 0%      25%
     *  9: 13%     gray-blue
     * 10: 26%     gray-red
     * 11: 40%     gray-green
     * 12: 6%%     gray-magenta
     * 13: 20%     gray-cyan
     * 14: 33%     gray-yellow
     * 15: 46%     75%
     * dxterm:
     * ?
     */
    make_sixel_color(graphic,  0,  0,  0,  0);
    make_sixel_color(graphic,  1, 20, 20, 80);
    make_sixel_color(graphic,  2, 80, 13, 13);
    make_sixel_color(graphic,  3, 20, 80, 20);
    make_sixel_color(graphic,  4, 80, 20, 80);
    make_sixel_color(graphic,  5, 20, 80, 80);
    make_sixel_color(graphic,  6, 80, 80, 20);
    make_sixel_color(graphic,  7, 53, 53, 53);
    make_sixel_color(graphic,  8, 26, 26, 26);
    make_sixel_color(graphic,  9, 33, 33, 60);
    make_sixel_color(graphic, 10, 60, 26, 26);
    make_sixel_color(graphic, 11, 33, 60, 33);
    make_sixel_color(graphic, 12, 60, 33, 60);
    make_sixel_color(graphic, 13, 33, 60, 60);
    make_sixel_color(graphic, 14, 60, 60, 33);
    make_sixel_color(graphic, 15, 80, 80, 80);

    graphic->preserve_background = 0; /* overwrite with current background */

    /* pixel sizes seem to have differed by model and options */
    /* VT240 and VT340 defaulted to 2:1 ratio */
    graphic->aspect_numerator = 2;
    graphic->aspect_denominator = 1;

    graphic->declared_width = 0;
    graphic->declared_height = 0;

    graphic->actual_width = 0;
    graphic->actual_height = 0;

    graphic->charrow = 0;
    graphic->charcol = 0;

    graphic->row = 0;
    graphic->col = 0;

    graphic->valid = 0;
}


static SixelGraphic *
get_sixel_graphic(int bufferid)
{
    SixelGraphic *graphic;
    unsigned int ii;

    for (ii = 0U; ii < MAX_SIXEL_GRAPHICS; ii++) {
	graphic = &sixel_graphics[ii];
	if (!graphic->valid)
	    break;
    }

    if (ii >= MAX_SIXEL_GRAPHICS) {
	int min_charrow = 0;
	SixelGraphic *min_graphic = NULL;

	for (ii = 0U; ii < MAX_SIXEL_GRAPHICS; ii++) {
	    graphic = &sixel_graphics[ii];
	    if (!min_graphic || graphic->charrow < min_charrow) {
		min_charrow = graphic->charrow;
		min_graphic = graphic;
	    }
	}
	graphic = min_graphic;
    }

    graphic->bufferid = bufferid;
    init_sixel_graphic(graphic);
    return graphic;
}


static void
dump_sixel(SixelGraphic const *graphic)
{
    int r, c;
    SixelColor color;
    ColorRegister const *reg;

    TRACE(("sixel stats: charrow=%d charcol=%d actual_width=%d actual_height=%d pixw=%d pixh=%d\n", graphic->charrow, graphic->charcol, graphic->actual_width, graphic->actual_height, graphic->pixw, graphic->pixh));
#if 0
    TRACE(("sixel dump:\n"));
    for (r=0; r < graphic->max_height; r++) {
	for (c=0; c < graphic->max_width; c++) {
	    color = graphic->pixels[r * graphic->max_width + c];
	    if (color == COLOR_HOLE) {
		TRACE(("?"));
	    } else {
		reg = &graphic->color_registers[color];
		if (reg->r + reg->g + reg->b > 200) {
		    TRACE(("#"));
		} else if (reg->r + reg->g + reg->b > 150) {
		    TRACE(("%%"));
		} else if (reg->r + reg->g + reg->b > 100) {
		    TRACE((":"));
		} else if (reg->r + reg->g + reg->b > 80) {
		    TRACE(("."));
		} else {
		    TRACE((" "));
		}
	    }
	}
	TRACE(("\n"));
    }
#endif
    TRACE(("\n"));
}

static Pixel
sixel_register_to_xpixel(ColorRegister *reg, XtermWidget xw)
{
    if (!reg->allocated) {
	XColor def;

	def.red = (short)(reg->r * 65535.0 / 100.0);
	def.green = (short)(reg->g * 65535.0 / 100.0);
	def.blue = (short)(reg->b * 65535.0 / 100.0);
	def.flags = DoRed | DoGreen | DoBlue;
	if (!allocateBestRGB(xw, &def)) {
	    return 0UL;
	}
	reg->pix = def.pixel;
	reg->allocated = 1;
    }

    return reg->pix;
}

static void
refresh_sixel_graphic(SixelGraphic *graphic, XtermWidget xw, int xbase, int ybase, int x, int y, int w, int h)
{
    TScreen const *screen = TScreenOf(xw);
    Display *display = screen->display;
    Window vwindow = WhichVWin(screen)->window;
    GC graphics_gc;
    int r, c;
    int wx, wy;
    int pw, ph;
    SixelColor color;
    SixelColor old_fg;
    XGCValues xgcv;
    XtGCMask mask;
    int holes, total;

    TRACE(("refreshing sixel graphic from %d,%d %dx%d (valid=%d, bg=%dx%d size=%dx%d, max=%dx%d) at base=%d,%d\n", x, y, w, h, graphic->valid, graphic->declared_width, graphic->declared_height, graphic->actual_width, graphic->actual_height, graphic->max_width, graphic->max_height, xbase, ybase));

    memset(&xgcv, 0, sizeof(xgcv));
    xgcv.foreground = 0UL;
    xgcv.graphics_exposures = False;
    mask = GCForeground|GCGraphicsExposures;
    graphics_gc = XCreateGC(display, vwindow, mask, &xgcv);

    pw = graphic->pixw;
    ph = graphic->pixh;

    TRACE(("refreshed graphic covers 0,0 to %d,%d\n", (graphic->actual_width-1) * pw + pw - 1, (graphic->actual_height-1) * ph + ph - 1));
    TRACE(("refreshed area covers %d,%d to %d,%d\n", x, y, x + w - 1, y + h - 1));
    old_fg = COLOR_HOLE;
    holes = total = 0;
    for (r = 0; r < graphic->actual_height; r++)
	for (c = 0; c < graphic->actual_width; c++) {
	    if (r * ph + ph - 1 < y ||
		r * ph > y + h - 1 ||
		c * pw + pw - 1 < x ||
		c * pw > x + w - 1)
		continue;

	    wy = ybase + r * ph;
	    wx = xbase + c * pw;

	    total++;
	    color = graphic->pixels[r * graphic->max_width + c];
	    if (color == COLOR_HOLE) {
		holes++;
		continue;
	    }

	    if (color != old_fg) {
		xgcv.foreground = sixel_register_to_xpixel(&graphic->color_registers[color], xw);
		XChangeGC(display, graphics_gc, mask, &xgcv);
		old_fg = color;
	    }

	    XFillRectangle(display, vwindow, graphics_gc, wx, wy, pw, ph);
	}
    XFlush(display);
    TRACE(("done refreshing sixel graphic: %d of %d refreshed pixels were holes\n", holes, total));

    XFreeGC(display, graphics_gc);
}

/*
 * Primary color hues:
 *  blue:    0 degrees
 *  red:   120 degrees
 *  green: 240 degrees
 */
static void
hls2rgb(short h, short l, short s, short *r, short *g, short *b)
{
    double hs = (h + 240) % 360;
    double hv = hs / 360.0;
    double lv = l / 100.0;
    double sv = s / 100.0;
    double c, x, m;
    double r1, g1, b1;
    int hpi;

    if (s == 0) {
	*r = *g = *b = l;
	return;
    }

    c = (1.0 - fabs(2.0 * lv - 1.0)) * sv;
    hpi = (int)(hv * 6.0);
    x = (hpi & 1) ? c : 0.0;
    m = lv - 0.5 * c;

    switch (hpi) {
    case 0:
	r1 = c; g1 = x; b1 = 0.0;
	break;
    case 1:
	r1 = x; g1 = c; b1 = 0.0;
	break;
    case 2:
	r1 = 0.0; g1 = c; b1 = x;
	break;
    case 3:
	r1 = 0.0; g1 = x; b1 = c;
	break;
    case 4:
	r1 = x; g1 = 0.0; b1 = c;
	break;
    case 5:
	r1 = c; g1 = 0.0; b1 = x;
	break;
    default:
        printf("BAD\n");
	*r = (short)100;
	*g = (short)100;
	*b = (short)100;
        return;
    }

    *r = (short)((r1 + m) * 100.0 + 0.5);
    *g = (short)((g1 + m) * 100.0 + 0.5);
    *b = (short)((b1 + m) * 100.0 + 0.5);

    if (*r < 0)
	*r = 0;
    else if (*r > 100)
	*r = 100;
    if (*g < 0)
	*g = 0;
    else if (*g > 100)
	*g = 100;
    if (*b < 0)
	*b = 0;
    else if (*b > 100)
	*b = 100;
}

/*
 * Interpret sixel graphics sequences.
 *
 * Resources:
 *  http://en.wikipedia.org/wiki/Sixel
 *  http://vt100.net/docs/vt3xx-gp/chapter14.html
 *  ftp://ftp.cs.utk.edu/pub/shuford/terminal/sixel_graphics_news.txt
 *  ftp://ftp.cs.utk.edu/pub/shuford/terminal/all_about_sixels.txt
 *
 */
extern void
parse_sixel(XtermWidget xw, ANSI *params, char const *string)
{
    TScreen *screen = TScreenOf(xw);
    SixelGraphic *graphic;
    Char ch;

    graphic = get_sixel_graphic(screen->whichBuf);

    {
	int Pmacro = params->a_param[0];
	int Pbgmode = params->a_param[1];
	int Phgrid = params->a_param[2];
	int Pan = params->a_param[3];
	int Pad = params->a_param[4];
	int Ph = params->a_param[5];
	int Pv = params->a_param[6];

	TRACE(("sixel bitmap graphics sequence: params=%d (Pmacro=%d Pbgmode=%d Phgrid=%d) scroll_amt=%d\n", params->a_nparam, Pmacro, Pbgmode, Phgrid, screen->scroll_amt));

	switch (params->a_nparam) {
	case 7:
	    if (Pan == 0 || Pad == 0) {
		TRACE(("DATA_ERROR: invalid raster ratio %d/%d\n", Pan, Pad));
		return;
	    }
	    graphic->aspect_numerator = Pan;
	    graphic->aspect_denominator = Pad;

	    if (Ph == 0 || Pv == 0) {
		TRACE(("DATA_ERROR: raster image dimensions are invalid %dx%d\n", Ph, Pv));
		return;
	    }
	    if (Ph > graphic->max_width || Pv > graphic->max_height) {
		TRACE(("DATA_ERROR: raster image dimensions are too large %dx%d\n", Ph, Pv));
		return;
	    }
	    graphic->declared_width = Ph;
	    graphic->declared_height = Pv;
	    if (graphic->declared_width > graphic->actual_width) {
		graphic->actual_width = graphic->declared_width;
	    }
	    if (graphic->declared_height > graphic->actual_height) {
		graphic->actual_height = graphic->declared_height;
	    }
	    break;
	case 3:
	case 2:
	    switch (Pmacro) {
	    case 0:
	    case 1:
		graphic->aspect_numerator = 2;
		graphic->aspect_denominator = 1;
		break;
	    case 2:
		graphic->aspect_numerator = 2;
		graphic->aspect_denominator = 1;
		break;
	    case 3:
	    case 4:
		graphic->aspect_numerator = 2;
		graphic->aspect_denominator = 1;
		break;
	    case 5:
	    case 6:
		graphic->aspect_numerator = 2;
		graphic->aspect_denominator = 1;
		break;
	    case 7:
	    case 8:
	    case 9:
		graphic->aspect_numerator = 2;
		graphic->aspect_denominator = 1;
		break;
	    default:
		TRACE(("unknown sixel macro mode parameter\n"));
		return;
	    }
	    break;
	case 0:
	    break;
	default:
	    TRACE(("DATA_ERROR: unexpected parameter count (found %d)\n", params->a_nparam));
	    return;
	}

	graphic->preserve_background = (Pbgmode == 1);

	/* ignore the grid parameter because it isn't clear what did real terminals did with it */

	/* "Pan;Pad;Ph;Pv\n
	 *  Pan and Pad define the pixel aspect ratio for the following sixel data string.
	 *  The pixel aspect ratio defines the shape of the pixels the terminal uses to draw the sixel image.
	 *  Ph and Pv define the horizontal and vertical size of the image (in pixels), respectively.
	 */
    }

    /* We want to keep the ratio accurate but would like every pixel to have
     * the same size so keep these as whole numbers.
     * FIXME: DEC terminals had pixels about twice as tall as they were wide,
     * but it is not clear if this is exposed to the application.
     */
    if (graphic->aspect_numerator > graphic->aspect_denominator) {
	graphic->pixw = 1;
	graphic->pixh = (graphic->aspect_numerator * 1 + graphic->aspect_denominator - 1) / graphic->aspect_denominator;
    } else {
	graphic->pixw = (graphic->aspect_denominator + graphic->aspect_denominator - 1) / (graphic->aspect_numerator * 1);
	graphic->pixh = 1;
    }

    if (xw->keyboard.flags & MODE_DECSDM) {
	TRACE(("sixel scrolling enabled: inline positioning for graphic\n"));
        graphic->charrow = screen->cur_row;
        graphic->charcol = screen->cur_col;
    }

    init_sixel_background(graphic);
    graphic->valid = 1;

    for (;;) {
	ch = CharOf(*string);
	if (ch == '\0')
	    break;

	if (ch >= 0x3f && ch <= 0x7e) {
	    int sixel = ch - 0x3f;
	    TRACE(("sixel=%x (%c)\n", sixel, (char)ch));
	    set_sixel(graphic, sixel);
	    graphic->col++;
	} else if (ch == '$') /* DECGCR */ {
	    /* ignore DECCRNLM in sixel mode */
	    TRACE(("sixel CR\n"));
	    graphic->col = 0;
	} else if (ch == '-') /* DECGNL */ {
	    int scroll_lines;
	    TRACE(("sixel NL\n"));
	    scroll_lines = 0;
	    while (graphic->charrow - scroll_lines +
		((graphic->row + 6) * graphic->pixh + FontHeight(screen) - 1) /
		FontHeight(screen) > screen->bot_marg) {
		scroll_lines++;
	    }
	    graphic->col = 0;
	    graphic->row += 6;
	    /* If we hit the bottom margin on the graphics page (well, we just use the text margin for now),
	     * the behavior is to either scroll or to discard the remainder of the graphic depending on this
	     * setting.
	     */
	    if (scroll_lines > 0) {
		if (xw->keyboard.flags & MODE_DECSDM) {
		    Display *display = screen->display;
		    xtermScroll(xw, scroll_lines);
		    XSync(display, False);
		    TRACE(("graphic scrolled the screen %d lines. screen->scroll_amt=%d screen->topline=%d, now starting row is %d\n", scroll_lines, screen->scroll_amt, screen->topline, graphic->charrow));
		} else {
		    break;
		}
	    }
	} else if (ch == '!') /* DECGRI */ {
	    int Pcount;
	    const char *start;
	    int sixel;
	    int i;

	    start = ++string;
	    for (;;) {
		ch = CharOf(*string);
		if (ch != '0' &&
		    ch != '1' &&
		    ch != '2' &&
		    ch != '3' &&
		    ch != '4' &&
		    ch != '5' &&
		    ch != '6' &&
		    ch != '7' &&
		    ch != '8' &&
		    ch != '9' &&
		    ch != ' ' &&
		    ch != '\r' &&
		    ch != '\n')
		    break;
		string++;
	    }
	    if (ch == '\0') {
		TRACE(("DATA_ERROR: sixel data string terminated in the middle of a repeat operator\n"));
		return;
	    }
	    if (string == start) {
		TRACE(("DATA_ERROR: sixel data string contains a repeat operator with empty count\n"));
		return;
	    }
	    Pcount = atoi(start);
	    sixel = ch - 0x3f;
	    TRACE(("sixel repeat operator: sixel=%d (%c), count=%d\n", sixel, (char)ch, Pcount));
	    for (i = 0; i < Pcount; i++) {
		set_sixel(graphic, sixel);
		graphic->col++;
	    }
	} else if (ch == '#') /* DECGCI */ {
	    ANSI color_params;
	    int Pregister;

	    parse_prefixedtype_params(&color_params, &string);
	    Pregister = color_params.a_param[0];
	    if (Pregister >= graphic->valid_registers) {
		TRACE(("DATA_ERROR: sixel color operator uses out-of-range register %d\n", Pregister));
		/* FIXME: supposedly the DEC terminals wrapped register indicies */
		while (Pregister >= graphic->valid_registers)
		    Pregister -= graphic->valid_registers;
	    }

	    if (color_params.a_nparam > 2 && color_params.a_nparam <= 5) {
		int Pspace = color_params.a_param[1];
		int Pc1 = color_params.a_param[2];
		int Pc2 = color_params.a_param[3];
		int Pc3 = color_params.a_param[4];

		TRACE(("sixel set color register=%d space=%d color=[%d,%d,%d] (nparams=%d)\n", Pregister, Pspace, Pc1, Pc2, Pc3, color_params.a_nparam));

		switch (Pspace) {
		case 1: /* HLS */
		    if (Pc1 > 360 || Pc2 > 100 || Pc3 > 100) {
			TRACE(("DATA_ERROR: sixel set color operator uses out-of-range color coordinates\n"));
			return;
		    }
		    hls2rgb(Pc1, Pc2, Pc3,
			    &graphic->color_registers[Pregister].r,
			    &graphic->color_registers[Pregister].g,
			    &graphic->color_registers[Pregister].b);
		    break;
		case 2: /* RGB */
		    if (Pc1 > 100 || Pc2 > 100 || Pc3 > 100) {
			TRACE(("DATA_ERROR: sixel set color operator uses out-of-range color coordinates\n"));
			return;
		    }
		    graphic->color_registers[Pregister].r = Pc1;
		    graphic->color_registers[Pregister].g = Pc2;
		    graphic->color_registers[Pregister].b = Pc3;
		    break;
		default: /* unknown */
		    TRACE(("DATA_ERROR: sixel set color operator uses unknown color space %d\n", Pspace));
		    return;
		}
	    } else if (color_params.a_nparam == 1) {
		TRACE(("sixel switch to color register=%d (nparams=%d)\n", Pregister, color_params.a_nparam));
		graphic->current_register = Pregister;
	    } else {
		TRACE(("DATA_ERROR: sixel switch color operator with unexpected parameter count (nparams=%d)\n", color_params.a_nparam));
		return;
	    }
	    continue;
	} else if (ch == '"') /* DECGRA */ {
	    ANSI raster_params;

	    parse_prefixedtype_params(&raster_params, &string);
	    if (raster_params.a_nparam < 2) {
		TRACE(("DATA_ERROR: sixel raster attribute operator with incomplete parameters (found %d, expected 2 or 4)\n", raster_params.a_nparam));
		return;
	    }

	    {
		int Pan = raster_params.a_param[0];
		int Pad = raster_params.a_param[1];
		TRACE(("sixel raster attribute with h:w=%d:%d\n", Pan, Pad));
		if (Pan == 0 || Pad == 0) {
		    TRACE(("DATA_ERROR: invalid raster ratio %d/%d\n", Pan, Pad));
		    return;
		}
		graphic->aspect_numerator = Pan;
		graphic->aspect_denominator = Pad;
	    }

	    if (raster_params.a_nparam >= 4) {
		int Ph = raster_params.a_param[2];
		int Pv = raster_params.a_param[3];

		TRACE(("sixel raster attribute with h=%d v=%d\n", Ph, Pv));
		if (Ph == 0 || Pv == 0) {
		    TRACE(("DATA_ERROR: raster image dimensions are invalid %dx%d\n", Ph, Pv));
		    return;
		}
		if (Ph > graphic->max_width || Pv > graphic->max_height) {
		    TRACE(("DATA_ERROR: raster image dimensions are too large %dx%d\n", Ph, Pv));
		    return;
		}
		graphic->declared_width = Ph;
		graphic->declared_height = Pv;
		if (graphic->declared_width > graphic->actual_width) {
		    graphic->actual_width = graphic->declared_width;
		}
		if (graphic->declared_height > graphic->actual_height) {
		    graphic->actual_height = graphic->declared_height;
		}
	    }

	    continue;
	} else if (ch == ' ' || ch == '\r' || ch == '\n') {
	    /* EMPTY */ ;
	} else {
	    TRACE(("DATA_ERROR: unknown sixel command %04x (%c)\n", (int)ch, ch));
	}

	string++;
    }

    /* update the screen */
    if (screen->scroll_amt)
	FlushScroll(xw);

    if (xw->keyboard.flags & MODE_DECSDM) {
	int new_row = graphic->charrow + ((graphic->actual_height * graphic->pixh)) / FontHeight(screen);
	int new_col = graphic->charcol + ((graphic->actual_width * graphic->pixw) + FontWidth(screen) - 1) / FontWidth(screen);

	TRACE(("setting text position after %dx%d graphic starting on row=%d col=%d: cursor new_row=%d new_col=%d\n", graphic->actual_width * graphic->pixw, graphic->actual_height * graphic->pixh, graphic->charrow, graphic->charcol, new_row, new_col));

	if (new_col > screen->rgt_marg) {
	    new_col = screen->lft_marg;
	    new_row++;
	    TRACE(("column past left margin, overriding to row=%d col=%d\n", new_row, new_col));
	}

	while (new_row > screen->bot_marg) {
	    xtermScroll(xw, 1);
	    new_row--;
	    TRACE(("bottom row was past screen.  new start row=%d, cursor row=%d\n", graphic->charrow, new_row));
	}

	set_cur_row(screen, new_row);
	set_cur_col(screen, new_col <= screen->rgt_marg ? new_col : screen->rgt_marg);
    }

    {
	int nrows = ((graphic->actual_height * graphic->pixh)) / FontHeight(screen);
	int ncols = ((graphic->actual_width * graphic->pixw) + FontWidth(screen) - 1) / FontWidth(screen);
	int w = ncols * FontWidth(screen);
	int h = nrows * FontHeight(screen);

	int xbase = screen->border + graphic->charcol * FontWidth(screen);
	int ybase = screen->border + (graphic->charrow - screen->topline) * FontHeight(screen);

	TRACE(("parse refresh: screen->topline=%d nrows=%d ncols=%d w=%d h=%d xbase=%d ybase=%d\n", screen->topline, nrows, ncols, w, h, xbase, ybase));
	refresh_sixel_graphic(graphic, xw, xbase, ybase, 0, 0, w, h);
    }

    TRACE(("done parsing sixel data\n"));
    dump_sixel(graphic);
}


extern void
parse_regis(XtermWidget xw, ANSI *params, char const *string)
{
    TRACE(("ReGIS vector graphics mode, params=%d\n", params->a_nparam));
}


/* Erase the portion of any displayed graphic overlapping with a rectangle
 * of the given size and location in pixels.
 * This is used to allow text to "erase" graphics underneath it.
 */
static void
erase_sixel_graphic(SixelGraphic *graphic, int x, int y, int w, int h)
{
    SixelColor hole;
    int pw, ph;
    int r, c;

    pw = graphic->pixw;
    ph = graphic->pixh;

    hole = COLOR_HOLE;

    TRACE(("erasing sixel bitmap %d,%d %dx%d\n", x, y, w, h));

    for (r = 0; r < graphic->actual_height; r++)
	for (c = 0; c < graphic->actual_width; c++) {
	    if (r * ph + ph - 1 < y ||
		r * ph > y + h - 1 ||
		c * pw + pw - 1 < x ||
		c * pw > x + w - 1)
		continue;

	    graphic->pixels[r * graphic->max_width + c] = hole;
	}
}


extern void
refresh_displayed_graphics(XtermWidget xw, int leftcol, int toprow, int ncols, int nrows)
{
    TScreen const *screen = TScreenOf(xw);
    SixelGraphic *graphic;
    unsigned int ii;
    int x, y, w, h;
    int xbase, ybase;

    for (ii = 0U; ii < MAX_SIXEL_GRAPHICS; ii++) {
	graphic = &sixel_graphics[ii];
	if (!graphic->valid || graphic->bufferid != screen->whichBuf)
	    continue;

	x = (leftcol - graphic->charcol) * FontWidth(screen);
	y = (toprow - graphic->charrow) * FontHeight(screen);
	w = ncols * FontWidth(screen);
	h = nrows * FontHeight(screen);

	xbase = screen->border + graphic->charcol * FontWidth(screen);
	ybase = screen->border + (graphic->charrow - screen->topline) * FontHeight(screen);

	TRACE(("graphics refresh: screen->topline=%d leftcol=%d toprow=%d nrows=%d ncols=%d x=%d y=%d w=%d h=%d xbase=%d ybase=%d\n", screen->topline, leftcol, toprow, nrows, ncols, x, y, w, h, xbase, ybase));
	refresh_sixel_graphic(graphic, xw, xbase, ybase, x, y, w, h);
    }
}


extern void
scroll_displayed_graphics(int rows)
{
    SixelGraphic *graphic;
    unsigned int ii;

    TRACE(("graphics scroll: moving all up %d rows\n", rows));
    for (ii = 0U; ii < MAX_SIXEL_GRAPHICS; ii++) {
	graphic = &sixel_graphics[ii];
	if (!graphic->valid)
	    continue;

	graphic->charrow -= rows;
    }
}


extern void
erase_displayed_graphics(XtermWidget xw, int leftcol, int toprow, int ncols, int nrows)
{
    TScreen const *screen = TScreenOf(xw);
    SixelGraphic *graphic;
    unsigned int ii;
    int x, y, w, h;

    for (ii = 0U; ii < MAX_SIXEL_GRAPHICS; ii++) {
	graphic = &sixel_graphics[ii];
	if (!graphic->valid)
	    continue;

	x = (leftcol - graphic->charcol) * FontWidth(screen);
	y = (toprow - graphic->charrow) * FontHeight(screen);
	w = ncols * FontWidth(screen);
	h = nrows * FontHeight(screen);

	TRACE(("graphics erase: screen->topline=%d leftcol=%d toprow=%d nrows=%d ncols=%d x=%d y=%d w=%d h=%d\n", screen->topline, leftcol, toprow, nrows, ncols, x, y, w, h));
	erase_sixel_graphic(graphic, x, y, w, h);
    }
}


extern void
reset_displayed_graphics(void)
{
    SixelGraphic *graphic;
    unsigned int ii;

    TRACE(("resetting all sixel graphics\n"));
    for (ii = 0U; ii < MAX_SIXEL_GRAPHICS; ii++) {
	graphic = &sixel_graphics[ii];
	graphic->valid = 0;
    }
}
