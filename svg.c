/* $XTermId: svg.c,v 1.17 2020/06/02 23:24:31 tom Exp $ */

/*
 * Copyright 2017-2019,2020	Thomas E. Dickey
 * Copyright 2015-2016,2017	Jens Schweikhardt
 *
 * All Rights Reserved
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name(s) of the above copyright
 * holders shall not be used in advertising or otherwise to promote the sale,
 * use or other dealings in this Software without prior written
 * authorization.
 */

#include <xterm.h>
#include <version.h>

#define MakeDim(color) \
	color = (unsigned short) ((2 * (unsigned) color) / 3)

#define RGBPCT(c) \
 	((double)c.red   / 655.35), \
	((double)c.green / 655.35), \
	((double)c.blue  / 655.35)

#define CELLW 10
#define CELLH 20

static void dumpSvgHeader(XtermWidget xw, FILE *fp);
static void dumpSvgScreen(XtermWidget xw, FILE *fp);
static void dumpSvgLine(XtermWidget xw, int row, FILE *fp);
static void dumpSvgFooter(XtermWidget, FILE *fp);

static int rows = 0;
static int cols = 0;
static Dimension bw = 0;	/* borderWidth */
static int ib = 0;		/* internalBorder */

void
xtermDumpSvg(XtermWidget xw)
{
    char *saveLocale;
    FILE *fp;

    TRACE(("xtermDumpSvg...\n"));
    saveLocale = xtermSetLocale(LC_NUMERIC, "C");
    fp = create_printfile(xw, ".svg");
    if (fp != 0) {
	dumpSvgHeader(xw, fp);
	dumpSvgScreen(xw, fp);
	dumpSvgFooter(xw, fp);
	fclose(fp);
    }
    xtermResetLocale(LC_NUMERIC, saveLocale);
    TRACE(("...xtermDumpSvg done\n"));
}

static void
dumpSvgHeader(XtermWidget xw, FILE *fp)
{
    TScreen *s = TScreenOf(xw);

    rows = s->bot_marg - s->top_marg + 1;
    cols = MaxCols(s);
    bw = BorderWidth(xw);
    ib = s->border;

    fputs("<?xml version='1.0' encoding='UTF-8'?>\n", fp);
    fputs("<!DOCTYPE svg PUBLIC '-//W3C//DTD SVG 1.1//EN'\n", fp);
    fputs("  'http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd'>\n", fp);
    fputs("<svg xmlns='http://www.w3.org/2000/svg'\n", fp);
    fputs(" version='1.1' baseProfile='full'\n", fp);
    fprintf(fp, " viewBox='0 0 %d %d'>\n", 2 * (bw + ib) + cols * CELLW, 2 *
	    (bw + ib) +
	    rows * CELLH);
    fprintf(fp, " <desc>%s Screen Dump</desc>\n", xtermVersion());
    fprintf(fp,
	    " <g font-size='%.2f' font-family='monospace, monospace'>\n",
	    0.80 * CELLH);
    xevents(xw);
}

static void
dumpSvgScreen(XtermWidget xw, FILE *fp)
{
    TScreen *s = TScreenOf(xw);
    int row;

    fprintf(fp, "  <rect x='0' y='0' width='%u' height='%u' fill='%s'/>\n",
	    cols * CELLW + 2 * (bw + ib), rows * CELLH + 2 * (bw + ib),
	    PixelToCSSColor(xw, xw->core.border_pixel));
    fprintf(fp, "  <rect x='%u' y='%u' width='%u' height='%u' fill='%s'/>\n",
	    bw, bw,
	    MaxCols(s) * CELLW + 2 * ib,
	    (unsigned) (rows * CELLH + 2 * ib),
	    PixelToCSSColor(xw, xw->old_background));

    for (row = s->top_marg; row <= s->bot_marg; ++row) {
	fprintf(fp, "  <!-- Row %d -->\n", row);
	dumpSvgLine(xw, row, fp);
    }
}

static void
dumpSvgLine(XtermWidget xw, int row, FILE *fp)
{
    TScreen *s = TScreenOf(xw);
    int inx = ROW2INX(s, row);
    LineData *ld = getLineData(s, inx);
    int col, sal, i;		/* sal: same attribute length */

    if (ld == 0)
	return;

    for (col = 0; col < MaxCols(s); col += sal) {
	XColor fgcolor, bgcolor;

	/* Count how many consecutive cells have the same color & attributes. */
	for (sal = 1; col + sal < MaxCols(s); ++sal) {
#if OPT_ISO_COLORS
	    if (!isSameCColor(ld->color[col], ld->color[col + sal]))
		break;
#endif
	    if (ld->attribs[col] != ld->attribs[col + sal])
		break;
	}

	fgcolor.pixel = xw->old_foreground;
	bgcolor.pixel = xw->old_background;
#if OPT_ISO_COLORS
	if (ld->attribs[col] & FG_COLOR) {
	    Pixel fg = extract_fg(xw, ld->color[col], ld->attribs[col]);
#if OPT_DIRECT_COLOR
	    if (ld->attribs[col] & ATR_DIRECT_FG)
		fgcolor.pixel = fg;
	    else
#endif
		fgcolor.pixel = s->Acolors[fg].value;
	}
	if (ld->attribs[col] & BG_COLOR) {
	    Pixel bg = extract_bg(xw, ld->color[col], ld->attribs[col]);
#if OPT_DIRECT_COLOR
	    if (ld->attribs[col] & ATR_DIRECT_BG)
		bgcolor.pixel = bg;
	    else
#endif
		bgcolor.pixel = s->Acolors[bg].value;
	}
#endif

	XQueryColor(xw->screen.display, xw->core.colormap, &fgcolor);
	XQueryColor(xw->screen.display, xw->core.colormap, &bgcolor);
	xevents(xw);

	if (ld->attribs[col] & BLINK) {
	    /* White on red. */
	    fgcolor.red = fgcolor.green = fgcolor.blue = 65535u;
	    bgcolor.red = 65535u;
	    bgcolor.green = bgcolor.blue = 0u;
	}
#if OPT_WIDE_ATTRS
	if (ld->attribs[col] & ATR_FAINT) {
	    MakeDim(fgcolor.red);
	    MakeDim(fgcolor.green);
	    MakeDim(fgcolor.blue);
	}
#endif
	if (ld->attribs[col] & INVERSE) {
	    XColor tmp = fgcolor;
	    fgcolor = bgcolor;
	    bgcolor = tmp;
	}

	/* Draw the background rectangle. */
	fprintf(fp, "  <rect x='%d' y='%d' ", bw + ib + col * CELLW, bw + ib
		+ row * CELLH);
	fprintf(fp, "height='%d' width='%d' ", CELLH, sal * CELLW);
	fprintf(fp, "fill='rgb(%.2f%%, %.2f%%, %.2f%%)'/>\n", RGBPCT(bgcolor));

	/* Now the <text>. */
	/*
	 * SVG: Rendering text strings into a given rectangle is a challenge.
	 * Some renderers accept and do the right thing with the 'textLength'
	 * attribute, while others ignore it. The only predictable way to place
	 * (even monospaced) text properly is to do it character by character.
	 */

	fprintf(fp, "  <g");
	if (ld->attribs[col] & BOLD)
	    fprintf(fp, " font-weight='bold'");
#if OPT_WIDE_ATTRS
	if (ld->attribs[col] & ATR_ITALIC)
	    fprintf(fp, " font-style='italic'");
#endif
	fprintf(fp, " fill='rgb(%.2f%%, %.2f%%, %.2f%%)'>\n", RGBPCT(fgcolor));

	for (i = 0; i < sal; ++i) {
	    IChar chr = ld->charData[col + i];

	    if (chr == ' ')
		continue;
	    fprintf(fp, "   <text x='%d' y='%d'>", bw + ib + (col + i) *
		    CELLW, bw + ib + row * CELLH + (CELLH * 3) / 4);
#if OPT_WIDE_CHARS
	    if (chr > 127) {
		/* Ignore hidden characters. */
		if (chr != HIDDEN_CHAR) {
		    Char temp[10];
		    *convertToUTF8(temp, chr) = 0;
		    fputs((char *) temp, fp);
		}
	    } else
#endif
		switch (chr) {
		case 0:
		    fputc(' ', fp);
		    break;
		case '&':
		    fputs("&amp;", fp);
		    break;
		case '<':
		    fputs("&lt;", fp);
		    break;
		case '>':
		    fputs("&gt;", fp);
		    break;
		default:
		    fputc((int) chr, fp);
		}
	    fprintf(fp, "</text>\n");
	    xevents(xw);
	}
	fprintf(fp, "  </g>\n");
	xevents(xw);

#define HLINE(x) \
  fprintf(fp, "  <line x1='%d' y1='%d' " \
                      "x2='%d' y2='%d' " \
                  "stroke='rgb(%.2f%%, %.2f%%, %.2f%%)'/>\n", \
    bw + ib + col * CELLW,         bw + ib + row * CELLH + CELLH - (x), \
    bw + ib + (col + sal) * CELLW, bw + ib + row * CELLH + CELLH - (x), \
    RGBPCT(fgcolor))

	/* Now the line attributes. */
	if (ld->attribs[col] & UNDERLINE) {
	    HLINE(4);
	}
#if OPT_WIDE_ATTRS
	if (ld->attribs[col] & ATR_STRIKEOUT) {
	    HLINE(9);
	}
	if (ld->attribs[col] & ATR_DBL_UNDER) {
	    HLINE(3);
	    HLINE(1);
	}
#endif
	xevents(xw);
    }
    xevents(xw);
}

static void
dumpSvgFooter(XtermWidget xw, FILE *fp)
{
    fputs(" </g>\n</svg>\n", fp);
    xevents(xw);
}
