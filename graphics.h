/* $XTermId: graphics.h,v 1.11 2014/04/14 00:38:27 tom Exp $ */

/*
 * Copyright 2013,2014 by Ross Combs
 * Copyright 2013,2014 by Thomas E. Dickey
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
 */

#ifndef included_graphics_h
#define included_graphics_h
/* *INDENT-OFF* */

#include <ptyx.h>

#if OPT_GRAPHICS

typedef struct {
    Pixel pix;
    short r, g, b;
    short allocated;
} ColorRegister;

typedef unsigned short RegisterNum;

#define MAX_COLOR_REGISTERS 256U
#define COLOR_HOLE ((RegisterNum)MAX_COLOR_REGISTERS)

#define MAX_GRAPHICS 16U

#define BUFFER_WIDTH 1000
#define BUFFER_HEIGHT 800
#define MAX_PIXELS (BUFFER_HEIGHT * BUFFER_WIDTH)

typedef struct {
    RegisterNum *pixels;
    ColorRegister *private_color_registers;
    ColorRegister *color_registers;
    char color_registers_used[MAX_COLOR_REGISTERS];
    XtermWidget xw;
    int max_width;              /* largest image which can be stored */
    int max_height;             /* largest image which can be stored */
    int valid_registers;        /* for wrap-around behavior */
    int actual_width;           /* size of image before scaling */
    int actual_height;          /* size of image before scaling */
    int private_colors;         /* if not using the shared color registers */
    int charrow;                /* upper left starting point in characters */
    int charcol;                /* upper left starting point in characters */
    int pixw;                   /* width of graphic pixels in screen pixels */
    int pixh;                   /* height of graphic pixels in screen pixels */
    int bufferid;               /* which screen buffer the graphic is associated with */
    unsigned int type;          /* type of graphic 0==sixel, 1...NUM_REGIS_PAGES==ReGIS page */
    unsigned int id;            /* sequential id used for preserving layering */
    int valid;                  /* if the graphic has been initialized */
    int dirty;                  /* if the graphic needs to be redrawn */
} Graphic;

extern Graphic *get_new_graphic(XtermWidget xw, int charrow, int charcol, unsigned int type);
extern Graphic *get_new_or_matching_graphic(XtermWidget xw, int charrow, int charcol, int actual_width, int actual_height, unsigned int type);
extern void draw_solid_pixel(Graphic *graphic, int x, int y, RegisterNum color);
extern void draw_solid_rectangle(Graphic *graphic, int x1, int y1, int x2, int y2, RegisterNum color);
extern void draw_solid_line(Graphic *graphic, int x1, int y1, int x2, int y2, RegisterNum color);
extern void hls2rgb(int h, int l, int s, short *r, short *g, short *b);
extern void dump_graphic(Graphic const *graphic);
extern void update_color_register(Graphic *graphic, RegisterNum color, short r, short g, short b);
extern RegisterNum find_color_register(ColorRegister const *color_registers, short r, short g, short b);
extern void chararea_clear_displayed_graphics(TScreen const *screen, int leftcol, int toprow, int ncols, int nrows);
extern void pixelarea_clear_displayed_graphics(TScreen const *screen, int winx, int winy, int w, int h);
extern void refresh_displayed_graphics(TScreen const *screen, int leftcol, int toprow, int ncols, int nrows);
extern void refresh_modified_displayed_graphics(TScreen const *screen);
extern void reset_displayed_graphics(TScreen const *screen);
extern void scroll_displayed_graphics(int rows);
extern void update_displayed_graphics_color_registers(TScreen const *screen, RegisterNum color, short r, short g, short b);

#if NO_LEAKS
extern void noleaks_graphics(void);
#endif

#else

#define get_new_graphic(xw, charrow, charcol, type) /* nothing */
#define get_new_or_matching_graphic(xw, charrow, charcol, actual_width, actual_height, type) /* nothing */
#define draw_solid_pixel(graphic, x, y, color) /* nothing */
#define draw_solid_rectangle(graphic, x1, y1, x2, y2, color) /* nothing */
#define draw_solid_line(graphic, x1, y1, x2, y2, color) /* nothing */
#define hls2rgb(h, l, s, r, g, b) /* nothing */
#define dump_graphic(graphic) /* nothing */
#define update_color_register(graphic, color, r, g, b) /* nothing */
#define find_color_register(color_registers, r, g, b) /* nothing */
#define chararea_clear_displayed_graphics(screen, leftcol, toprow, ncols, nrows) /* nothing */
#define pixelarea_clear_displayed_graphics(screen, winx, winy, w, h) /* nothing */
#define refresh_displayed_graphics(screen, leftcol, toprow, ncols, nrows) /* nothing */
#define refresh_modified_displayed_graphics(screen) /* nothing */
#define reset_displayed_graphics(screen) /* nothing */
#define scroll_displayed_graphics(rows) /* nothing */
#define update_displayed_graphics_color_registers(screen, color, r, g, b) /* nothing */

#endif

/* *INDENT-ON* */

#endif /* included_graphics_h */
