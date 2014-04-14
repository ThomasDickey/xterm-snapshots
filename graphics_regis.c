/* $XTermId: graphics_regis.c,v 1.5 2014/04/14 00:29:19 tom Exp $ */

/*
 * Copyright 2014 by Ross Combs
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

#include <xterm.h>

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include <data.h>
#include <VTparse.h>
#include <ptyx.h>

#include <assert.h>
#include <graphics.h>
#include <graphics_regis.h>

typedef struct RegisWriteControls {
    unsigned int pv_multiplier;
    unsigned int pattern;
    unsigned int pattern_multiplier;
    unsigned int invert_pattern;
    RegisterNum foreground;
    unsigned int plane_mask;
    unsigned int write_style;
    unsigned int shading_enabled;
    char shading_character;
    int shading_reference;
    unsigned int shading_reference_dim;
} RegisWriteControls;

typedef struct RegisDataFragment {
    char const *start;
    unsigned int pos;
    unsigned int len;
} RegisDataFragment;

typedef enum RegisParseLevel {
    INPUT,
    OPTIONSET,
} RegisParseLevel;

typedef struct RegisParseState {
    RegisDataFragment input;
    RegisDataFragment optionset;
    char *temp;
    unsigned int templen;
    RegisParseLevel level;
    char command;
    char option;
} RegisParseState;

typedef struct RegisGraphicsContext {
    Graphic *graphic;
    int terminal_id;
    RegisterNum background;
    RegisWriteControls persistent_write_controls;
    RegisWriteControls temporary_write_controls;
    int graphics_output_cursor_x;
    int graphics_output_cursor_y;
} RegisGraphicsContext;

#define MAX_PATTERN_BITS 8U

#define WRITE_STYLE_OVERLAY 1U
#define WRITE_STYLE_REPLACE 2U
#define WRITE_STYLE_COMPLEMENT 3U
#define WRITE_STYLE_ERASE 4U

#define WRITE_SHADING_REF_Y 0U
#define WRITE_SHADING_REF_X 1U

#define ROT_LEFT(V) ( (((V) << 1U) & 255U) | ((V) >> 7U) )

static void
draw_patterned_line(RegisGraphicsContext *context, int x1, int y1, int x2, int y2)
{
    RegisterNum fg, bg, color;
    unsigned int pattern;
    unsigned int mult;
    unsigned int count;
    unsigned int bit;
    int x, y;
    int dx, dy;
    int dir, diff;

    dx = abs(x1 - x2);
    dy = abs(y1 - y2);

    if (context->temporary_write_controls.invert_pattern) {
	fg = context->background;
	bg = context->temporary_write_controls.foreground;
    } else {
	fg = context->temporary_write_controls.foreground;
	bg = context->background;
    }
    pattern = context->temporary_write_controls.pattern;
    mult = context->temporary_write_controls.pattern_multiplier;
    count = 0U;
    bit = 1U;

    if (dx > dy) {
	if (x1 > x2) {
	    int tmp;
	    tmp = x1;
	    x1 = x2;
	    x2 = tmp;
	    tmp = y1;
	    y1 = y2;
	    y2 = tmp;
	}
	if (y1 < y2)
	    dir = 1;
	else if (y1 > y2)
	    dir = -1;
	else
	    dir = 0;

	diff = 0;
	y = y1;
	for (x = x1; x <= x2; x++) {
	    if (diff >= dx) {
		diff -= dx;
		y += dir;
	    }
	    diff += dy;
	    if (count >= mult) {
		count = 0U;
		bit = ROT_LEFT(bit);
	    }
	    count++;
	    color = (pattern & bit) ? fg : bg;
	    draw_solid_pixel(context->graphic, x, y, color);
	}
    } else {
	if (y1 > y2) {
	    int tmp;
	    tmp = y1;
	    y1 = y2;
	    y2 = tmp;
	    tmp = x1;
	    x1 = x2;
	    x2 = tmp;
	}
	if (x1 < x2)
	    dir = 1;
	else if (x1 > x2)
	    dir = -1;
	else
	    dir = 0;

	diff = 0;
	x = x1;
	for (y = y1; y <= y2; y++) {
	    if (diff >= dy) {
		diff -= dy;
		x += dir;
	    }
	    diff += dx;
	    if (count >= mult) {
		count = 0U;
		bit = ROT_LEFT(bit);
	    }
	    count++;
	    color = (pattern & bit) ? fg : bg;
	    draw_solid_pixel(context->graphic, x, y, color);
	}
    }
}

static void
init_fragment(RegisDataFragment *fragment, char const *str)
{
    fragment->start = str;
    fragment->len = strlen(str);
    fragment->pos = 0U;
}

static void
copy_fragment(RegisDataFragment *dst, RegisDataFragment const *src)
{
    dst->start = src->start;
    dst->len = src->len;
    dst->pos = src->pos;
}

static char
peek_fragment(RegisDataFragment const *fragment)
{
    if (fragment->pos < fragment->len) {
	return fragment->start[fragment->pos];
    }
    return '\0';
}

static char
pop_fragment(RegisDataFragment *fragment)
{
    if (fragment->pos < fragment->len) {
	return fragment->start[fragment->pos++];
    }
    return '\0';
}

#if 0
static void
skip_fragment_chars(RegisDataFragment *fragment, unsigned int count)
{
    fragment->pos += count;
    if (fragment->pos > fragment->len) {
	fragment->pos = fragment->len;
    }
}
#endif

static void
fragment_to_string(RegisDataFragment const *fragment, char *out, unsigned int outlen)
{
    unsigned int minlen;

    if (fragment->len < outlen) {
	minlen = fragment->len;
    } else {
	minlen = outlen;
    }
    strncpy(out, &fragment->start[fragment->pos], minlen);
    out[minlen] = '\0';
}

static char const *
fragment_to_tempstr(RegisDataFragment const *fragment)
{
    static char tempstr[1024];
    fragment_to_string(fragment, tempstr, 1024);
    return tempstr;
}

static unsigned int
fragment_len(RegisDataFragment const *fragment)
{
    return fragment->len - fragment->pos;
}

static int
skip_regis_whitespace(RegisDataFragment *input)
{
    int skipped = 0;
    char ch;

    assert(input);

    for (; input->pos < input->len; input->pos++) {
	/* FIXME: the semicolon isn't whitespace -- it also terminates the current command even if inside of an optionset or extent */
	ch = input->start[input->pos];
	if (ch != ',' && ch != ';' &&
	    ch != ' ' && ch != '\b' && ch != '\t' && ch != '\r' && ch != '\n') {
	    break;
	}
	if (ch == '\n') {
	    TRACE(("end of input line\n\n"));
	}
	skipped = 1;
    }

    if (skipped)
	return 1;
    return 0;
}

static int
extract_regis_extent(RegisDataFragment *input, RegisDataFragment *output)
{
    char ch;

    assert(input);
    assert(output);

    output->start = &input->start[input->pos];
    output->len = 0U;
    output->pos = 0U;

    if (input->pos >= input->len)
	return 0;

    ch = input->start[input->pos];
    if (ch != '[')
	return 0;
    input->pos++;
    output->start++;

    /* FIXME: truncate to 16 bit signed integers */
    for (; input->pos < input->len; input->pos++, output->len++) {
	ch = input->start[input->pos];
	if (ch == ';') {
	    TRACE(("DATA_ERROR: end of input before closing bracket\n"));
	    break;
	}
	if (ch == ']')
	    break;
    }
    if (ch == ']')
	input->pos++;

    return 1;
}

static int
extract_regis_num(RegisDataFragment *input, RegisDataFragment *output)
{
    char ch = 0;

    assert(input);
    assert(output);

    output->start = &input->start[input->pos];
    output->len = 0U;
    output->pos = 0U;

    for (; input->pos < input->len; input->pos++, output->len++) {
	ch = input->start[input->pos];
	if (ch != '0' && ch != '1' && ch != '2' && ch != '3' &&
	    ch != '4' && ch != '5' && ch != '6' && ch != '7' &&
	    ch != '8' && ch != '9') {
	    break;
	}
    }

    /* FIXME: what degenerate forms should be accepted ("E10" "1E" "1e" "1." "1ee10")? */
    /* FIXME: the terminal is said to support "floating point values", truncating to int... what do these look like? */
    if (output->len > 0U && ch == 'E') {
	input->pos++;
	output->len++;
	for (; input->pos < input->len; input->pos++, output->len++) {
	    ch = input->start[input->pos];
	    if (ch != '0' && ch != '1' && ch != '2' && ch != '3' &&
		ch != '4' && ch != '5' && ch != '6' && ch != '7' &&
		ch != '8' && ch != '9') {
		break;
	    }
	}
    }

    if (output->len < 1U)
	return 0;

    return 1;
}

static int
extract_regis_pixelvector(RegisDataFragment *input, RegisDataFragment *output)
{
    char ch;

    assert(input);
    assert(output);

    output->start = &input->start[input->pos];
    output->len = 0U;
    output->pos = 0U;

    for (; input->pos < input->len; input->pos++, output->len++) {
	ch = input->start[input->pos];
	if (ch != '0' && ch != '1' && ch != '2' && ch != '3' &&
	    ch != '4' && ch != '5' && ch != '6' && ch != '7') {
	    break;
	}
    }

    if (output->len < 1U)
	return 0;

    return 1;
}

static int
extract_regis_command(RegisDataFragment *input, char *command)
{
    char ch;

    assert(input);
    assert(command);

    if (input->pos >= input->len)
	return 0;

    ch = input->start[input->pos];
    if (ch == '\0' || ch == ';') {
	return 0;
    }
    if (!islower(ch) && !isupper(ch)) {
	return 0;
    }
    *command = ch;
    input->pos++;

    return 1;
}

static int
extract_regis_string(RegisDataFragment *input, char *out, unsigned int maxlen)
{
    char first_ch;
    char ch;
    char prev_ch;
    unsigned int outlen = 0U;

    assert(input);
    assert(out);

    if (input->pos >= input->len)
	return 0;

    ch = input->start[input->pos];
    if (ch != '\'' && ch != '"')
	return 0;
    first_ch = ch;
    input->pos++;

    ch = '\0';
    for (; input->pos < input->len; input->pos++) {
	prev_ch = ch;
	ch = input->start[input->pos];
	/* ';' (resync) is not recognized in strings */
	if (prev_ch == first_ch) {
	    if (ch == first_ch) {
		if (outlen < maxlen) {
		    out[outlen] = ch;
		}
		outlen++;
		ch = '\0';
		continue;
	    }
	    if (outlen < maxlen)
		out[outlen] = '\0';
	    else
		out[maxlen] = '\0';
	    return 1;
	}
	if (ch == '\0')
	    break;
	if (ch != first_ch) {
	    if (outlen < maxlen) {
		out[outlen] = ch;
	    }
	    outlen++;
	}
    }
    if (ch == first_ch) {
	if (outlen < maxlen)
	    out[outlen] = '\0';
	else
	    out[maxlen] = '\0';
	return 1;
    }
    /* FIXME: handle multiple strings concatenated with commas */

    TRACE(("DATA_ERROR: end of input during before closing quote\n"));
    return 0;
}

static int
extract_regis_optionset(RegisDataFragment *input, RegisDataFragment *output)
{
    char ch;
    int nesting;

    assert(input);
    assert(output);

    output->start = &input->start[input->pos];
    output->len = 0U;
    output->pos = 0U;

    if (input->pos >= input->len)
	return 0;

    ch = input->start[input->pos];
    if (ch != '(')
	return 0;
    input->pos++;
    output->start++;
    nesting = 1;

    /* FIXME: handle strings with parens */
    for (; input->pos < input->len; input->pos++, output->len++) {
	ch = input->start[input->pos];
	if (ch == ';')
	    break;
	if (ch == '(')
	    nesting++;
	if (ch == ')') {
	    nesting--;
	    if (nesting == 0) {
		input->pos++;
		return 1;
	    }
	}
    }

    TRACE(("DATA_ERROR: end of input before closing paren (%d levels deep)\n", nesting));
    return 0;
}

static int
extract_regis_option(RegisDataFragment *input,
		     char *option,
		     RegisDataFragment *output)
{
    char ch;
    int nesting;

    assert(input);
    assert(option);
    assert(output);

    /* LETTER suboptions* value? */
    /* FIXME: can there be whitespace or commas inside of an option? */
    /* FIXME: what are the rules for using separate parens vs. sharing between options? */

    output->start = &input->start[input->pos];
    output->len = 0U;
    output->pos = 0U;

    if (input->pos >= input->len) {
	return 0;
    }

    ch = input->start[input->pos];
    if (ch == ';' || ch == ',' || ch == '(' || ch == ')' || isdigit(ch)) {
	return 0;
    }
    *option = ch;
    input->pos++;
    output->start++;
    nesting = 0;

    /* FIXME: handle strings with parens, nested parens, etc. */
    for (; input->pos < input->len; input->pos++, output->len++) {
	ch = input->start[input->pos];
	/* FIXME: any special rules for commas?  any need to track parens? */
	if (ch == '(')
	    nesting++;
	if (ch == ')') {
	    nesting--;
	    if (nesting < 0) {
		TRACE(("DATA_ERROR: found ReGIS option has value with too many close parens \"%c\"\n", *option));
		return 0;
	    }
	}
	/* top-level commas indicate the end of this option and the start of another */
	if (nesting == 0 && ch == ',')
	    break;
	if (ch == ';')
	    break;
    }
    if (nesting != 0) {
	TRACE(("DATA_ERROR: mismatched parens in argument to ReGIS option \"%c\"\n", *option));
	return 0;
    }

    TRACE(("found ReGIS option and value \"%c\" \"%s\"\n",
	   *option,
	   fragment_to_tempstr(output)));
    return 1;
}

static int
regis_num_to_int(RegisDataFragment const *input, int *out)
{
    char ch;

    /* FIXME: handle exponential notation */
    /* FIXME: check for junk after the number */
    ch = peek_fragment(input);
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
	ch != '+' &&
	ch != '-') {
	return 0;
    }

    *out = atoi(fragment_to_tempstr(input));
    return 1;
}

static int
load_regis_colorspec(Graphic const *graphic, RegisDataFragment const *input, RegisterNum *out)
{
    int val;
    RegisDataFragment colorspec;
    RegisDataFragment coloroption;

    copy_fragment(&colorspec, input);
    TRACE(("looking at colorspec pattern: \"%s\"\n", fragment_to_tempstr(&colorspec)));

    if (regis_num_to_int(&colorspec, &val)) {
	if (val < 0 || val >= graphic->valid_registers) {	/* FIXME: wrap? */
	    TRACE(("DATA_ERROR: erase writing mode %d\n", val));
	    return 0;
	}
	TRACE(("colorspec contains index for register %u\n", val));
	*out = val;
	return 1;
    }

    if (extract_regis_optionset(&colorspec, &coloroption)) {
	short r, g, b;
	TRACE(("option: \"%s\"\n", fragment_to_tempstr(&coloroption)));

	if (fragment_len(&coloroption) == 1) {
	    char ch = pop_fragment(&coloroption);

	    TRACE(("got regis RGB colorspec pattern: \"%s\"\n",
		   fragment_to_tempstr(&coloroption)));
	    switch (ch) {
	    case 'D':
	    case 'd':
		r = 0;
		g = 0;
		b = 0;
		break;
	    case 'R':
	    case 'r':
		r = 100;
		g = 0;
		b = 0;
		break;
	    case 'G':
	    case 'g':
		r = 0;
		g = 100;
		b = 0;
		break;
	    case 'B':
	    case 'b':
		r = 0;
		g = 0;
		b = 100;
		break;
	    case 'C':
	    case 'c':
		r = 0;
		g = 100;
		b = 100;
		break;
	    case 'Y':
	    case 'y':
		r = 100;
		g = 100;
		b = 0;
		break;
	    case 'M':
	    case 'm':
		r = 100;
		g = 0;
		b = 100;
		break;
	    case 'W':
	    case 'w':
		r = 100;
		g = 100;
		b = 100;
		break;
	    default:
		TRACE(("unknown RGB color name: \"%c\"\n", ch));
		return 0;
	    }
	} else {
	    short h, l, s;

	    if (sscanf(fragment_to_tempstr(&coloroption),
		       "%*1[Hh]%hd%*1[Ll]%hd%*1[Ss]%hd",
		       &h, &l, &s) != 3) {
		TRACE(("unrecognized colorspec format: \"%s\"\n",
		       fragment_to_tempstr(&coloroption)));
		return 0;
	    }
	    hls2rgb(h, l, s, &r, &g, &b);
	}
	/* FIXME: check for trailing junk? */
	*out = find_color_register(graphic->color_registers, r, g, b);
	TRACE(("colorspec maps to closest register %u\n", *out));
	return 1;
    }

    TRACE(("unrecognized colorspec format: \"%s\"\n", fragment_to_tempstr(&colorspec)));
    return 0;
}

static int
load_regis_extent(char const *extent, int origx, int origy, int *xloc, int *yloc)
{
    int xsign, ysign;
    char const *xpart;
    char const *ypart;

    xpart = extent;
    if ((ypart = strchr(extent, ','))) {
	ypart++;
    } else {
	ypart = "";
    }

    if (xpart[0] == '-') {
	xsign = -1;
	xpart++;
    } else if (xpart[0] == '+') {
	xsign = +1;
	xpart++;
    } else {
	xsign = 0;
    }
    if (ypart[0] == '-') {
	ysign = -1;
	ypart++;
    } else if (ypart[0] == '+') {
	ysign = +1;
	ypart++;
    } else {
	ysign = 0;
    }

    if (xpart[0] == '\0' || xpart[0] == ',') {
	*xloc = origx;
    } else if (xsign == 0) {
	*xloc = atoi(xpart);
    } else {
	*xloc = origx + xsign * atoi(xpart);
    }
    if (ypart[0] == '\0') {
	*yloc = origy;
    } else if (ysign == 0) {
	*yloc = atoi(ypart);
    } else {
	*yloc = origy + ysign * atoi(ypart);
    }

    return 1;
}

static int
load_regis_pixelvector(char const *pixelvector,
		       unsigned int mul,
		       int origx, int origy,
		       int *xloc, int *yloc)
{
    int dx = 0, dy = 0;
    int i;

    for (i = 0; pixelvector[i] != '\0'; i++) {
	switch (pixelvector[i]) {
	case '0':
	    dx += 1;
	    break;
	case '1':
	    dx += 1;
	    dy -= 1;
	    break;
	case '2':
	    dy -= 1;
	    break;
	case '3':
	    dx -= 1;
	    dy -= 1;
	    break;
	case '4':
	    dx -= 1;
	    break;
	case '5':
	    dx -= 1;
	    dy += 1;
	    break;
	case '6':
	    dy += 1;
	    break;
	case '7':
	    dx += 1;
	    dy += 1;
	    break;
	default:
	    break;
	}
    }

    *xloc = origx + dx * (int) mul;
    *yloc = origy + dy * (int) mul;

    return 1;
}

static int
load_regis_write_control(RegisParseState *state,
			 Graphic const *graphic,
			 int cur_x, int cur_y,
			 char option,
			 RegisDataFragment *arg,
			 RegisWriteControls *out)
{
    switch (option) {
    case 'E':
    case 'e':
	TRACE(("write control erase writing mode \"%s\"\n",
	       fragment_to_tempstr(arg)));
	out->write_style = WRITE_STYLE_ERASE;
	break;
    case 'F':
    case 'f':
	TRACE(("write control plane write mask \"%s\"\n",
	       fragment_to_tempstr(arg)));
	{
	    int val;
	    if (!regis_num_to_int(arg, &val) || val < 0 || val >= graphic->valid_registers) {
		TRACE(("interpreting out of range value as 0 FIXME\n"));
		out->plane_mask = 0U;
	    } else {
		out->plane_mask = (unsigned int) val;
	    }
	}
	break;
    case 'I':
    case 'i':
	TRACE(("write control foreground color \"%s\"\n",
	       fragment_to_tempstr(arg)));
	if (!load_regis_colorspec(graphic, arg, &out->foreground)) {
	    TRACE(("DATA_ERROR: write control foreground color specifier not recognized: \"%s\"\n",
		   fragment_to_tempstr(arg)));
	    return 0;
	}
	break;
    case 'M':
    case 'm':
	TRACE(("write control found pixel multiplication factor \"%s\"\n",
	       fragment_to_tempstr(arg)));
	{
	    int val;
	    if (!regis_num_to_int(arg, &val) || val <= 0) {
		TRACE(("interpreting out of range value as 1 FIXME\n"));
		out->pv_multiplier = 1U;
	    } else {
		out->pv_multiplier = (unsigned int) val;
	    }
	}
	break;
    case 'N':
    case 'n':
	TRACE(("write control negative pattern control \"%s\"\n",
	       fragment_to_tempstr(arg)));
	{
	    int val;
	    if (!regis_num_to_int(arg, &val)) {
		val = -1;
	    }
	    switch (val) {
	    default:
		TRACE(("interpreting out of range value as 0 FIXME\n"));
		out->invert_pattern = 0U;
		break;
	    case 0:
		out->invert_pattern = 0U;
		break;
	    case 1:
		out->invert_pattern = 1U;
		break;
	    }
	}
	break;
    case 'P':
    case 'p':
	TRACE(("write control found pattern control \"%s\"\n",
	       fragment_to_tempstr(arg)));
	{
	    RegisDataFragment suboptionset;
	    RegisDataFragment suboptionarg;
	    RegisDataFragment item;
	    char suboption;

	    while (arg->pos < arg->len) {
		skip_regis_whitespace(arg);

		TRACE(("looking for option in \"%s\"\n", fragment_to_tempstr(arg)));
		if (extract_regis_optionset(arg, &suboptionset)) {
		    TRACE(("got regis write pattern suboptionset: \"%s\"\n",
			   fragment_to_tempstr(&suboptionset)));
		    while (suboptionset.pos < suboptionset.len) {
			skip_regis_whitespace(&suboptionset);
			if (peek_fragment(&suboptionset) == ',') {
			    pop_fragment(&suboptionset);
			    continue;
			}
			if (extract_regis_option(&suboptionset, &suboption, &suboptionarg)) {
			    TRACE(("inspecting write pattern suboption \"%c\" with value \"%s\"\n",
				   suboption, fragment_to_tempstr(&suboptionarg)));
			    switch (suboption) {
			    case 'M':
			    case 'm':
				TRACE(("found pattern multiplier \"%s\"\n",
				       fragment_to_tempstr(&suboptionarg)));
				{
				    int val;

				    skip_regis_whitespace(&suboptionarg);
				    if (!regis_num_to_int(&suboptionarg, &val)
					|| val < 1) {
					TRACE(("interpreting out of range pattern multiplier as 2 FIXME\n"));
					out->pattern_multiplier = 2U;
				    } else {
					out->pattern_multiplier = val;
				    }
				    skip_regis_whitespace(&suboptionarg);
				    if (fragment_len(&suboptionarg)) {
					TRACE(("DATA_ERROR: unknown content after patern multiplier \"%s\"\n",
					       fragment_to_tempstr(&suboptionarg)));
					return 0;
				    }
				}
				break;
			    default:
				TRACE(("DATA_ERROR: unknown ReGIS write pattern suboption '%c' arg \"%s\"\n",
				       suboption, fragment_to_tempstr(&suboptionarg)));
				return 0;
			    }
			    continue;
			}

			TRACE(("DATA_ERROR: skipping unknown token in pattern control suboptionset (expecting option): \"%s\"\n",
			       fragment_to_tempstr(&suboptionset)));
			pop_fragment(&suboptionset);
		    }
		    continue;
		}

		TRACE(("looking for int in \"%s\"\n", fragment_to_tempstr(arg)));
		if (extract_regis_num(arg, &item)) {
		    if (peek_fragment(&item) == '0' ||
			peek_fragment(&item) == '1') {
			unsigned int pattern = 0U;
			unsigned int bitcount;
			char ch;

			TRACE(("converting pattern bits \"%s\"\n",
			       fragment_to_tempstr(&item)));
			for (bitcount = 0;; bitcount++) {
			    ch = pop_fragment(&item);
			    if (ch == '\0')
				break;
			    switch (ch) {
			    case '0':
				if (bitcount < MAX_PATTERN_BITS) {
				    pattern <<= 1U;
				}
				break;
			    case '1':
				if (bitcount < MAX_PATTERN_BITS) {
				    pattern <<= 1U;
				    pattern |= 1U;
				}
				break;
			    default:
				TRACE(("DATA_ERROR: unknown ReGIS write pattern bit value \"%c\"\n",
				       ch));
				return 0;
			    }
			}

			if (bitcount > 0U) {
			    int extrabits;

			    for (extrabits = 0;
				 bitcount + extrabits < MAX_PATTERN_BITS;
				 extrabits++) {
				if (pattern & (1U << (bitcount - 1U))) {
				    pattern <<= 1U;
				    pattern |= 1U;
				} else {
				    pattern <<= 1U;
				}
			    }
			}

			out->pattern = pattern;
		    } else {
			int val;

			TRACE(("converting pattern id \"%s\"\n",
			       fragment_to_tempstr(&item)));
			regis_num_to_int(&item, &val);
			switch (val) {	/* FIXME: exponential allowed? */
			case 0:
			    out->pattern = 0x00;	/* solid bg */
			    break;
			case 1:
			    out->pattern = 0xff;	/* solid fg */
			    break;
			case 2:
			    out->pattern = 0xf0;	/* dash */
			    break;
			case 3:
			    out->pattern = 0xe4;	/* dash dot */
			    break;
			case 4:
			    out->pattern = 0xaa;	/* dot */
			    break;
			case 5:
			    out->pattern = 0xea;	/* dash dot dot */
			    break;
			case 6:
			    out->pattern = 0x88;	/* sparse dot */
			    break;
			case 7:
			    out->pattern = 0x84;	/* asymmetric sparse dot */
			    break;
			case 8:
			    out->pattern = 0xc8;	/* sparse dash dot */
			    break;
			case 9:
			    out->pattern = 0x86;	/* sparse dot dash */
			    break;
			default:
			    TRACE(("DATA_ERROR: unknown ReGIS standard write pattern \"%d\"\n", val));
			    return 0;
			}
		    }

		    TRACE(("final pattern is %02x\n", out->pattern));
		    continue;
		}

		TRACE(("DATA_ERROR: skipping unknown token in pattern suboption: \"%s\"\n",
		       fragment_to_tempstr(arg)));
		pop_fragment(arg);
	    }
	}
	break;
    case 'C':
    case 'c':
	TRACE(("write control compliment writing mode \"%s\"\n",
	       fragment_to_tempstr(arg)));
	out->write_style = WRITE_STYLE_COMPLEMENT;
	break;
    case 'R':
    case 'r':
	TRACE(("write control switch to replacement writing mode \"%s\"\n",
	       fragment_to_tempstr(arg)));
	out->write_style = WRITE_STYLE_REPLACE;
	break;
    case 'S':
    case 's':
	TRACE(("write control shading control \"%s\"\n", fragment_to_tempstr(arg)));
	{
	    RegisDataFragment suboptionset;
	    RegisDataFragment suboptionarg;
	    RegisDataFragment item;
	    char suboption;
	    char shading_character = '\0';
	    int reference_dim = WRITE_SHADING_REF_Y;
	    int ref_x = cur_x, ref_y = cur_y;
	    int shading_enabled = 0;

	    while (arg->pos < arg->len) {
		skip_regis_whitespace(arg);

		if (extract_regis_string(arg, state->temp, state->templen)) {
		    TRACE(("found fill char \"%s\"\n", state->temp));
		    if (strlen(state->temp) != 1) {	/* FIXME: allow strings?  should extra chars be ignored? */
			TRACE(("DATA_ERROR: expected exactly one char in fill string FIXME\n"));
			return 0;
		    }
		    shading_character = state->temp[0];
		    TRACE(("shading character is: %d\n", (int) shading_character));
		    continue;
		}

		if (extract_regis_optionset(arg, &suboptionset)) {
		    TRACE(("got regis shading control suboptionset: \"%s\"\n",
			   fragment_to_tempstr(&suboptionset)));
		    while (suboptionset.pos < suboptionset.len) {
			skip_regis_whitespace(&suboptionset);
			if (peek_fragment(&suboptionset) == ',') {
			    pop_fragment(&suboptionset);
			    continue;
			}
			if (extract_regis_option(&suboptionset, &suboption, &suboptionarg)) {
			    TRACE(("inspecting write shading suboption \"%c\" with value \"%s\"\n",
				   suboption, fragment_to_tempstr(&suboptionarg)));
			    switch (suboption) {
			    case 'X':
			    case 'x':
				TRACE(("found vertical shading suboption \"%s\"\n",
				       fragment_to_tempstr(&suboptionarg)));
				if (fragment_len(&suboptionarg) > 0U) {
				    TRACE(("DATA_ERROR: unexpected value to vertical shading suboption FIXME\n"));
				    return 0;
				}
				reference_dim = WRITE_SHADING_REF_X;
				break;
			    default:
				TRACE(("DATA_ERROR: unknown ReGIS write pattern suboption '%c' arg \"%s\"\n",
				       suboption, fragment_to_tempstr(&suboptionarg)));
				return 0;
			    }
			    continue;
			}

			TRACE(("DATA_ERROR: skipping unknown token in shading control suboptionset (expecting option): \"%s\"\n",
			       fragment_to_tempstr(&suboptionset)));
			pop_fragment(&suboptionset);
		    }
		    continue;
		}

		if (extract_regis_extent(arg, &item)) {
		    if (!load_regis_extent(fragment_to_tempstr(&item),
					   ref_x, ref_y,
					   &ref_x, &ref_y)) {
			TRACE(("DATA_ERROR: unable to parse extent in write shading option '%c': \"%s\"\n",
			       option, fragment_to_tempstr(&item)));
			return 0;
		    }
		    TRACE(("shading reference = %d,%d (%s)\n", ref_x, ref_y,
			   reference_dim == WRITE_SHADING_REF_X ? "X" : "Y"));
		    continue;
		}

		if (extract_regis_num(arg, &item)) {
		    if (!regis_num_to_int(&item, &shading_enabled)) {
			TRACE(("DATA_ERROR: unable to parse int in write shading option '%c': \"%s\"\n",
			       option, fragment_to_tempstr(&item)));
			return 0;
		    }
		    if (shading_enabled < 0 || shading_enabled > 1) {
			TRACE(("interpreting out of range value %d as 0 FIXME\n", shading_enabled));
			shading_enabled = 0;
		    }
		    TRACE(("shading enabled = %d\n", shading_enabled));
		    continue;
		}

		TRACE(("DATA_ERROR: skipping unknown token in shade suboption: \"%s\"\n",
		       fragment_to_tempstr(arg)));
		pop_fragment(arg);
	    }

	    if (shading_enabled) {
		out->shading_enabled = 1U;
		out->shading_reference_dim = reference_dim;
		out->shading_reference = ((reference_dim == WRITE_SHADING_REF_X)
					  ? ref_x
					  : ref_y);
		out->shading_character = shading_character;
	    } else {
		/* FIXME: confirm there is no effect if shading isn't enabled in the same command */
		out->shading_enabled = 0U;
	    }
	}
	break;
    case 'V':
    case 'v':
	TRACE(("write control switch to overlay writing mode \"%s\"\n",
	       fragment_to_tempstr(arg)));
	out->write_style = WRITE_STYLE_OVERLAY;
	break;
    default:
	TRACE(("DATA_ERROR: ignoring unknown ReGIS write option \"%c\" arg \"%s\"\n",
	       option, fragment_to_tempstr(arg)));
	return 0;
    }

    return 1;
}

static int
load_regis_write_control_set(RegisParseState *state,
			     Graphic const *graphic,
			     int cur_x, int cur_y,
			     RegisDataFragment *controls,
			     RegisWriteControls *out)
{
    RegisDataFragment optionset;
    RegisDataFragment arg;
    char option;

    while (controls->pos < controls->len) {
	skip_regis_whitespace(controls);

	if (extract_regis_optionset(controls, &optionset)) {
	    TRACE(("got regis write control optionset: \"%s\"\n",
		   fragment_to_tempstr(&optionset)));
	    while (optionset.pos < optionset.len) {
		skip_regis_whitespace(&optionset);
		if (peek_fragment(&optionset) == ',') {
		    pop_fragment(&optionset);
		    continue;
		}
		if (extract_regis_option(&optionset, &option, &arg)) {
		    TRACE(("got regis write control option and value: \"%c\" \"%s\"\n",
			   option, fragment_to_tempstr(&arg)));
		    if (!load_regis_write_control(state, graphic,
						  cur_x, cur_y,
						  option, &arg, out)) {
			return 0;
		    }
		    continue;
		}

		TRACE(("DATA_ERROR: skipping unknown token in write control optionset (expecting option): \"%s\"\n",
		       fragment_to_tempstr(&optionset)));
		pop_fragment(&optionset);
	    }
	    continue;
	}

	TRACE(("DATA_ERROR: skipping unknown token in write controls (expecting optionset): \"%s\"\n",
	       fragment_to_tempstr(controls)));
	pop_fragment(controls);
    }

    return 1;
}

static void
init_regis_write_controls(int terminal_id, RegisWriteControls *controls)
{
    controls->pv_multiplier = 1U;
    controls->pattern = 0xff;	/* solid */
    controls->pattern_multiplier = 2U;
    controls->invert_pattern = 0U;
    switch (terminal_id) {
    case 125:			/* FIXME */
    case 240:			/* FIXME */
    case 241:			/* FIXME */
    case 330:
	controls->foreground = 3U;
	controls->plane_mask = 3U;
	break;
    case 340:
	controls->foreground = 7U;
	controls->plane_mask = 15U;
	break;
    default:			/* FIXME */
	controls->foreground = 64U;
	controls->plane_mask = 63U;
	break;
    }
    controls->write_style = WRITE_STYLE_OVERLAY;
    controls->shading_enabled = 0U;
    controls->shading_character = '\0';
    controls->shading_reference = 0;	/* no meaning if shading is disabled */
    controls->shading_reference_dim = WRITE_SHADING_REF_Y;
    /* FIXME: add the rest */
}

static void
copy_regis_write_controls(RegisWriteControls const *src,
			  RegisWriteControls *dst)
{
    dst->pv_multiplier = src->pv_multiplier;
    dst->pattern = src->pattern;
    dst->pattern_multiplier = src->pattern_multiplier;
    dst->invert_pattern = src->invert_pattern;
    dst->foreground = src->foreground;
    dst->plane_mask = src->plane_mask;
    dst->write_style = src->write_style;
    dst->shading_enabled = src->shading_enabled;
    dst->shading_character = src->shading_character;
    dst->shading_reference = src->shading_reference;
    dst->shading_reference_dim = src->shading_reference_dim;
}

static void
init_regis_graphics_context(int terminal_id, RegisGraphicsContext *context)
{
    context->terminal_id = terminal_id;
    init_regis_write_controls(terminal_id, &context->persistent_write_controls);
    copy_regis_write_controls(&context->persistent_write_controls, &context->temporary_write_controls);
    /* FIXME: coordinates */
    /* FIXME: scrolling */
    /* FIXME: output maps */
    context->background = 0U;
    /* FIXME: input cursor location */
    /* FIXME: input cursor style */
    context->graphics_output_cursor_x = 0;
    context->graphics_output_cursor_y = 0;
    /* FIXME: output cursor style */
    /* FIXME: text settings */
}

static int
parse_regis_command(RegisParseState *state)
{
    char ch = peek_fragment(&state->input);
    if (ch == '\0')
	return 0;

    if (!extract_regis_command(&state->input, &ch))
	return 0;

    switch (ch) {
    case 'C':
    case 'c':
	/* Curve

	 * C
	 * (A)  # set the arc length in degrees (+ or nothing for
	 *      # counter-clockwise, - for clockwise, rounded to the
	 *      # closest integer degree)
	 * (B)  # begin closed curve sequence (must have at least two
	 *      # values; this option can not be nested)
	 * (C)  # position is the center, current location is the
	 *      # circumference (stays in effect until next command)
	 * (E)  # end closed curve sequence (drawing is performed here)
	 * (S)  # begin open curve sequence (FIXME: finish doc)
	 * (W)  # temporary write options (see write command)
	 * [<center, circumference position>]  # center if (C), otherwise point on circumference
	 * [<point in closed curve sequence>]...  # if between (B) and (E)
	 * <pv>...  # if between (B) and (E)
	 */
	TRACE(("found ReGIS command \"%c\" (curve)\n", ch));
	state->command = 'c';
	break;
    case 'F':
    case 'f':
	/* Fill

	 * F
	 * (V)  # polygon (see vector command)
	 * (C)  # curve (see curve command)
	 * (W)  # temporary write options (see write command)
	 */
	TRACE(("found ReGIS command \"%c\" (filled polygon)\n", ch));
	state->command = 'f';
	break;
    case 'L':
    case 'l':
	/* Load

	 * L
	 * (A)  # set character set number and name
	 * "ascii"xx,xx,xx,xx,xx,xx,xx,xx  # pixel values
	 */
	TRACE(("found ReGIS command \"%c\" (load charset)\n", ch));
	state->command = 'l';
	break;
    case 'P':
    case 'p':
	/* Position

	 * P
	 * (B)  # begin bounded position stack (last point returns to first)
	 * (E)  # end position stack
	 * (S)  # begin unbounded position stack
	 * (W)  # temporary write options (see write command)
	 * <pv>  # move: 0 == right, 1 == upper right, ..., 7 == lower right
	 * [<position>]  # move to position (X, Y, or both)
	 *
	 * Note the stack does not need to be ended before the next command
	 * Note: maximum depth is 16 levels
	 */
	TRACE(("found ReGIS command \"%c\" (position)\n", ch));
	state->command = 'p';
	break;
    case 'R':
    case 'r':
	/* Report

	 * R
	 * (E)  # parse error
	 * (I<val>)  # set input mode (0 == oneshot, 1 == multiple) (always returns CR)
	 * (L)  # character set
	 * (M(<name>)  # macrograph contents
	 * (M(=)  # macrograph storage
	 * (P)  # cursor position
	 * (P(I))  # interactive cursor position
	 */
	TRACE(("found ReGIS command \"%c\" (report status)\n", ch));
	state->command = 'r';
	break;
    case 'S':
    case 's':
	/* Screen

	 * S
	 * (A[<upper left>][<lower right>])
	 * (C<setting>  # 0 (cursor output off), 1 (cursor output on)
	 * (E  # erase to background color, resets shades, curves, and stacks
	 * (H(P<printer offset>)[<print area cornet>][<print area corner>)
	 * (I<color register>)  # set the background to a specific register
	 * (I(<rgb>))  # set the background to the register closest to an RGB value
	 * (I(<hls>))  # set the background to the register closest to an HLS color
	 * (M<color index to set>(L<mono level>)...)  # level is 0 ... 100 (sets grayscale registers only)
	 * (M<color index to set>(<RGB code>)...)  # codes are D (black), R (red), G (green), B (blue), C (cyan), Y (yellow), M (magenta), W (white) (sets color and grayscale registers)
	 * (M<color index to set>(A<RGB code>)...)  # codes are D (black), R (red), G (green), B (blue), C (cyan), Y (yellow), M (magenta), W (white) (sets color registers only)
	 * (M<color index to set>(H<hue>L<lightness>S<saturation>)...)  # 0..360, 0..100, 0..100 (sets color and grayscale registers)
	 * (M<color index to set>(AH<hue>L<lightness>S<saturation>)...)  # 0..360, 0..100, 0..100 (sets color registers only)
	 * (P<graphics page number>)  # 0 (default) or 1
	 * (T(<time delay ticks>)  # 60 ticks per second, up to 32767 ticks
	 * (W(M<factor>)  # PV value
	 * [scroll offset]  # optional
	 */
	TRACE(("found ReGIS command \"%c\" (screen)\n", ch));
	state->command = 's';
	break;
    case 'T':
    case 't':
	/* Text

	 * T
	 * (A0L"<designator>"))  # specify a built-in set for GL via two-char designator
	 * (A0R"<designator>"))  # specify a built-in set for GR via two-char or three-char designator
	 * (A<num>R"<designator>"))  # specify a user-loaded (1-3) set for GR via two-char or three-char designator
	 * (B)  # begin temporary text control
	 * (D<angle>)  # specify a string tilt
	 * (E)  # end temporary text control
	 * (H<factor>)  # select a height multiplier (1-256)
	 * (I<angle>)  # italics: no slant (0), lean back (-1 though -45), lean forward (+1 through +45)
	 * (M[width factor,height factor])  # select size multipliers (width 1-16) (height 1-256)
	 * (S<size id>)  # select one of the 17 standard cell sizes
	 * (S[dimensions])  # set a custom display cell size (char with border)
	 * (U[dimensions])  # set a custom unit cell size (char size)
	 * (W<write command>)  # temporary write options (see write command)
	 * [<char offset>]  # optional offset between characters
	 * <PV spacing>  # for subscripts and superscripts
	 * '<text>'  # optional
	 * "<text>"  # optional
	 */
	TRACE(("found ReGIS command \"%c\" (text)\n", ch));
	state->command = 't';
	break;
    case 'V':
    case 'v':
	/* Vector

	 * V
	 * (B)  # begin bounded position stack (last point returns to first)
	 * (E)  # end position stack
	 * (S)  # begin unbounded position stack
	 * (W)  # temporary write options (see write command)
	 * <pv>  # draw a line to the pixel vector
	 * []  # draw a dot at the current location
	 * [<position>]  # draw a line to position
	 */
	TRACE(("found ReGIS command \"%c\" (vector)\n", ch));
	state->command = 'v';
	break;
    case 'W':
    case 'w':
	/* Write

	 * W
	 * (C)  # complement writing mode
	 * (E)  # erase writing mode
	 * (F<plane>)  # set the foreground intensity to a specific register
	 * (I<color register>)  # set the foreground to a specific register
	 * (I(<rgb>))  # set the foreground to the register closest to an RGB value
	 * (I(<hls>))  # set the foreground to the register closest to an HLS color
	 * (M<pixel vector multiplier>)  # set the multiplication factor
	 * (N<setting>)  # 0 == negative patterns disabled, 1 == negative patterns enabled
	 * (P<pattern number>)  # 0..9: 0 == none, 1 == solid, 2 == 50% dash, 3 == dash-dot
	 * (P<pattern bits>)  # 2 to 8 bits represented as a 0/1 sequence
	 * (P<(M<pattern multiplier>))
	 * (R)  # replacement writing mode
	 * (S'<character>')  # set shading character
	 * (S<setting>)  # 0 == disable shding, 1 == enable shading
	 * (S[reference point])  # set a horizontal reference line including this point
	 * (S(X)[reference point])  # set a vertical reference line including this point
	 * (V)  # overlay writing mode
	 */
	TRACE(("found ReGIS command \"%c\" (write parameters)\n", ch));
	state->command = 'w';
	break;
    case '@':
	/* Macrograph */
	TRACE(("found ReGIS macrograph command\n"));
	ch = pop_fragment(&state->input);
	TRACE(("inspecting macrograph character \"%c\"\n", ch));
	switch (ch) {
	case '.':
	    TRACE(("clearing all macrographs FIXME\n"));
	    /* FIXME: handle */
	    break;
	case ':':
	    TRACE(("defining macrograph FIXME\n"));
	    /* FIXME: parse, handle  :<name> */
	    break;
	case ';':
	    TRACE(("DATA_ERROR: found extraneous terminator for macrograph definition\n"));
	    break;
	default:
	    if ((ch > 'A' && ch < 'Z') || (ch > 'a' && ch < 'z')) {
		TRACE(("expanding macrograph \"%c\" FIXME\n", ch));
		/* FIXME: handle */
	    } else {
		TRACE(("DATA_ERROR: unknown macrograph subcommand \"\%c\"\n", ch));
	    }
	    /* FIXME: parse, handle */
	    break;
	}
	break;
    default:
	TRACE(("DATA_ERROR: unknown ReGIS command %04x (%c)\n",
	       (int) ch, ch));
	state->command = '_';
	state->option = '_';
	return 0;
    }
    state->option = '_';

    return 1;
}

static int
parse_regis_optionset(RegisParseState *state)
{
    if (!extract_regis_optionset(&state->input, &state->optionset))
	return 0;

    TRACE(("found ReGIS optionset \"%s\"\n", fragment_to_tempstr(&state->optionset)));
    state->option = '_';

    return 1;
}

static int
parse_regis_option(RegisParseState *state, RegisGraphicsContext *context)
{
    RegisDataFragment optionarg;

    if (!extract_regis_option(&state->optionset, &state->option, &optionarg))
	return 0;

    TRACE(("found ReGIS option \"%c\": \"%s\"\n",
	   state->option, fragment_to_tempstr(&optionarg)));

    switch (state->command) {
    case 'c':
	TRACE(("inspecting curve option \"%c\" with value \"%s\"\n",
	       state->option, fragment_to_tempstr(&optionarg)));
	switch (state->option) {
	case 'A':
	case 'a':
	    TRACE(("found arc length \"%s\" FIXME\n",
		   fragment_to_tempstr(&optionarg)));
	    /* FIXME: handle */
	    break;
	case 'B':
	case 'b':
	    TRACE(("begin closed curve \"%s\" FIXME\n",
		   fragment_to_tempstr(&optionarg)));
	    /* FIXME: handle */
	    break;
	case 'C':
	case 'c':
	    TRACE(("found center position \"%s\" FIXME\n",
		   fragment_to_tempstr(&optionarg)));
	    /* FIXME: handle */
	    break;
	case 'E':
	case 'e':
	    TRACE(("end closed curve \"%s\" FIXME\n",
		   fragment_to_tempstr(&optionarg)));
	    /* FIXME: handle */
	    break;
	case 'S':
	case 's':
	    TRACE(("begin open curve \"%s\" FIXME\n",
		   fragment_to_tempstr(&optionarg)));
	    /* FIXME: handle */
	    break;
	case 'W':
	case 'w':
	    TRACE(("found temporary write options \"%s\"\n",
		   fragment_to_tempstr(&optionarg)));
	    if (!load_regis_write_control_set(state, context->graphic,
					      context->graphics_output_cursor_x, context->graphics_output_cursor_y,
					      &optionarg, &context->temporary_write_controls)) {
		TRACE(("DATA_ERROR: invalid temporary write options \"%s\"\n",
		       fragment_to_tempstr(&optionarg)));
	    }
	    break;
	default:
	    TRACE(("DATA_ERROR: ignoring unknown ReGIS curve command option '%c' arg \"%s\"\n",
		   state->option, fragment_to_tempstr(&optionarg)));
	    break;
	}
	break;
    case 'f':
	TRACE(("inspecting fill option \"%c\" with value \"%s\"\n",
	       state->option, fragment_to_tempstr(&optionarg)));
	switch (state->option) {
	case 'C':
	case 'c':
	    state->command = 'c';
	    state->option = '_';
	    break;
	case 'V':
	case 'v':
	    state->command = 'v';
	    state->option = '_';
	    break;
	case 'W':
	case 'w':
	    TRACE(("found temporary write options \"%s\"\n",
		   fragment_to_tempstr(&optionarg)));
	    if (!load_regis_write_control_set(state, context->graphic,
					      context->graphics_output_cursor_x, context->graphics_output_cursor_y,
					      &optionarg, &context->temporary_write_controls)) {
		TRACE(("DATA_ERROR: invalid temporary write options \"%s\"\n",
		       fragment_to_tempstr(&optionarg)));
	    }
	    break;
	default:
	    TRACE(("DATA_ERROR: ignoring unknown ReGIS fill command option '%c' arg \"%s\"\n",
		   state->option, fragment_to_tempstr(&optionarg)));
	    break;
	}
	break;
    case 'l':
	TRACE(("inspecting load option \"%c\" with value \"%s\"\n",
	       state->option, fragment_to_tempstr(&optionarg)));
	/* FIXME: parse options */
	switch (state->option) {
	case 'A':
	case 'a':
	    TRACE(("found character specifier option \"%s\" FIXME\n",
		   fragment_to_tempstr(&optionarg)));
	    /* FIXME: handle */
	    break;
	default:
	    TRACE(("DATA_ERROR: ignoring unknown ReGIS load command option '%c' arg \"%s\"\n",
		   state->option, fragment_to_tempstr(&optionarg)));
	    break;
	}
	break;
    case 'p':
	TRACE(("inspecting position option \"%c\" with value \"%s\"\n",
	       state->option, fragment_to_tempstr(&optionarg)));
	switch (state->option) {
	case 'B':
	case 'b':
	    TRACE(("found begin bounded position stack \"%s\" FIXME\n",
		   fragment_to_tempstr(&optionarg)));
	    /* FIXME: handle */
	    break;
	case 'E':
	case 'e':
	    TRACE(("found end position stack \"%s\" FIXME\n",
		   fragment_to_tempstr(&optionarg)));
	    /* FIXME: handle */
	    break;
	case 'S':
	case 's':
	    TRACE(("found begin unbounded position stack \"%s\" FIXME\n",
		   fragment_to_tempstr(&optionarg)));
	    /* FIXME: handle */
	    break;
	case 'W':
	case 'w':
	    TRACE(("found temporary write options \"%s\"\n",
		   fragment_to_tempstr(&optionarg)));
	    if (!load_regis_write_control_set(state, context->graphic,
					      context->graphics_output_cursor_x, context->graphics_output_cursor_y,
					      &optionarg, &context->temporary_write_controls)) {
		TRACE(("DATA_ERROR: invalid temporary write options \"%s\"\n",
		       fragment_to_tempstr(&optionarg)));
	    }
	    break;
	default:
	    TRACE(("DATA_ERROR: ignoring unknown ReGIS position command option '%c' arg \"%s\"\n",
		   state->option, fragment_to_tempstr(&optionarg)));
	    break;
	}
	break;
    case 'r':
	TRACE(("inspecting report option \"%c\" with value \"%s\"\n",
	       state->option, fragment_to_tempstr(&optionarg)));
	switch (state->option) {
	case 'E':
	case 'e':
	    TRACE(("found parse error report \"%s\" FIXME\n",
		   fragment_to_tempstr(&optionarg)));
	    /* FIXME: handle */
	    break;
	case 'I':
	case 'i':
	    TRACE(("found set input mode \"%s\" FIXME\n",
		   fragment_to_tempstr(&optionarg)));
	    /* FIXME: handle */
	    break;
	case 'L':
	case 'l':
	    TRACE(("found character set report \"%s\" FIXME\n",
		   fragment_to_tempstr(&optionarg)));
	    /* FIXME: handle */
	    break;
	case 'M':
	case 'm':
	    TRACE(("found macrograph report \"%s\" FIXME\n",
		   fragment_to_tempstr(&optionarg)));
	    /* FIXME: handle */
	    break;
	case 'P':
	case 'p':
	    TRACE(("found cursor position report \"%s\" FIXME\n",
		   fragment_to_tempstr(&optionarg)));
	    /* FIXME: handle */
	    break;
	default:
	    TRACE(("DATA_ERROR: ignoring unknown ReGIS report command option '%c' arg \"%s\"\n",
		   state->option, fragment_to_tempstr(&optionarg)));
	    break;
	}
	break;
    case 's':
	TRACE(("inspecting screen option \"%c\" with value \"%s\"\n",
	       state->option, fragment_to_tempstr(&optionarg)));
	switch (state->option) {
	case 'A':
	case 'a':
	    TRACE(("found address definition \"%s\" FIXME\n",
		   fragment_to_tempstr(&optionarg)));
	    /* FIXME: handle */
	    if (fragment_len(&optionarg) < 1U) {
		TRACE(("DATA_ERROR: ignoring malformed ReGIS screen address definition option value \"%s\"\n",
		       fragment_to_tempstr(&optionarg)));
		return 0;
	    }
	    break;
	case 'C':
	case 'c':
	    TRACE(("found cursor control \"%s\" FIXME\n",
		   fragment_to_tempstr(&optionarg)));
	    /* FIXME: handle */
	    if (fragment_len(&optionarg) < 1U) {
		TRACE(("DATA_ERROR: ignoring malformed ReGIS screen cursor control option value \"%s\"\n",
		       fragment_to_tempstr(&optionarg)));
		return 0;
	    }
	    break;
	case 'E':
	case 'e':
	    TRACE(("found erase request \"%s\"\n", fragment_to_tempstr(&optionarg)));
	    if (fragment_len(&optionarg) > 0U) {
		TRACE(("DATA_ERROR: ignoring unexpected argument to ReGIS screen erase option \"%s\"\n",
		       fragment_to_tempstr(&optionarg)));
		return 0;
	    }
	    draw_solid_rectangle(context->graphic, 0, 0,
				 context->graphic->actual_width - 1,
				 context->graphic->actual_height - 1,
				 context->background);
	    break;
	case 'H':
	case 'h':
	    TRACE(("found hardcopy control \"%s\" FIXME\n",
		   fragment_to_tempstr(&optionarg)));
	    /* FIXME: handle */
	    if (fragment_len(&optionarg) < 1U) {
		TRACE(("DATA_ERROR: ignoring malformed ReGIS screen hardcopy control option value \"%s\"\n",
		       fragment_to_tempstr(&optionarg)));
		return 0;
	    }
	    break;
	case 'I':
	case 'i':
	    TRACE(("found screen background color index \"%s\"\n",
		   fragment_to_tempstr(&optionarg)));
	    if (!load_regis_colorspec(context->graphic, &optionarg, &context->background)) {
		TRACE(("DATA_ERROR: screen background color specifier not recognized: \"%s\"\n",
		       fragment_to_tempstr(&optionarg)));
		return 0;
	    }
	    break;
	case 'M':
	case 'm':
	    TRACE(("found screen color register mapping \"%s\"\n",
		   fragment_to_tempstr(&optionarg)));
	    {
		RegisDataFragment regnum;
		RegisDataFragment colorspec;
		char ch;

		while (fragment_len(&optionarg) > 0U) {
		    if (skip_regis_whitespace(&optionarg))
			continue;
		    if (extract_regis_num(&optionarg, &regnum)) {
			int register_num;
			int color_only;
			short r, g, b;

			if (!regis_num_to_int(&regnum, &register_num)) {
			    TRACE(("DATA_ERROR: unable to parse int in screen color register mapping option: \"%s\"\n",
				   fragment_to_tempstr(&regnum)));
			    return 0;
			}
			if (register_num < 0 || register_num > context->graphic->valid_registers) {
			    TRACE(("interpreting out of range register number %d as 0 FIXME\n", register_num));
			    register_num = 0;
			}
			if (!extract_regis_optionset(&optionarg, &colorspec)) {
			    TRACE(("DATA_ERROR: expected to find optionset after register number: \"%s\"\n",
				   fragment_to_tempstr(&optionarg)));
			    return 0;
			}

			switch (peek_fragment(&colorspec)) {
			case 'A':
			case 'a':
			    pop_fragment(&colorspec);
			    color_only = 1;
			    break;
			default:
			    color_only = 0;
			    break;
			}

			TRACE(("mapping register %d to color spec: \"%s\"\n",
			       register_num, fragment_to_tempstr(&colorspec)));
			if (fragment_len(&colorspec) == 1) {
			    ch = pop_fragment(&colorspec);

			    TRACE(("got regis RGB colorspec pattern: \"%s\"\n",
				   fragment_to_tempstr(&colorspec)));
			    switch (ch) {
			    case 'D':
			    case 'd':
				r = 0;
				g = 0;
				b = 0;
				break;
			    case 'R':
			    case 'r':
				r = 100;
				g = 0;
				b = 0;
				break;
			    case 'G':
			    case 'g':
				r = 0;
				g = 100;
				b = 0;
				break;
			    case 'B':
			    case 'b':
				r = 0;
				g = 0;
				b = 100;
				break;
			    case 'C':
			    case 'c':
				r = 0;
				g = 100;
				b = 100;
				break;
			    case 'Y':
			    case 'y':
				r = 100;
				g = 100;
				b = 0;
				break;
			    case 'M':
			    case 'm':
				r = 100;
				g = 0;
				b = 100;
				break;
			    case 'W':
			    case 'w':
				r = 100;
				g = 100;
				b = 100;
				break;
			    default:
				TRACE(("unknown RGB color name: \"%c\"\n", ch));
				return 0;
			    }
			} else {
			    short h, l, s;

			    if (sscanf(fragment_to_tempstr(&colorspec),
				       "%*1[Hh]%hd%*1[Ll]%hd%*1[Ss]%hd",
				       &h, &l, &s) != 3) {
				h = 0;
				s = 0;
				if (sscanf(fragment_to_tempstr(&colorspec),
					   "%*1[Ll]%hd", &l) != 1) {
				    TRACE(("unrecognized colorspec: \"%s\"\n",
					   fragment_to_tempstr(&colorspec)));
				    return 0;
				}
			    }
			    hls2rgb(h, l, s, &r, &g, &b);
			}

			if (color_only &&
			    (context->terminal_id == 240 ||
			     context->terminal_id == 330))
			    continue;
			/* FIXME: It is not clear how to handle a mixture of
			 * monochrome and color mappings. */
			update_color_register(context->graphic,
					      (RegisterNum) register_num,
					      r, g, b);
			continue;
		    }

		    ch = pop_fragment(&optionarg);
		    TRACE(("DATA_ERROR: ignoring unexpected character in ReGIS screen color register mapping value \"%c\"\n", ch));
		    return 0;
		}
	    }
	    break;
	case 'P':
	case 'p':
	    TRACE(("found graphics page request \"%s\" FIXME\n",
		   fragment_to_tempstr(&optionarg)));
	    /* FIXME: handle */
	    if (fragment_len(&optionarg) < 1U) {
		TRACE(("DATA_ERROR: ignoring malformed ReGIS screen graphics page option value \"%s\"\n",
		       fragment_to_tempstr(&optionarg)));
		return 0;
	    }
	    break;
	case 'T':
	case 't':
	    TRACE(("found time delay \"%s\" FIXME\n", fragment_to_tempstr(&optionarg)));
	    /* FIXME: handle */
	    if (fragment_len(&optionarg) < 1U) {
		TRACE(("DATA_ERROR: ignoring malformed ReGIS screen time delay option value \"%s\"\n",
		       fragment_to_tempstr(&optionarg)));
		return 0;
	    }
	    break;
	case 'W':
	case 'w':
	    TRACE(("found PV \"%s\" FIXME\n", fragment_to_tempstr(&optionarg)));
	    /* FIXME: handle */
	    if (fragment_len(&optionarg) < 1U) {
		TRACE(("DATA_ERROR: ignoring malformed ReGIS screen PV option value \"%s\"\n",
		       fragment_to_tempstr(&optionarg)));
		return 0;
	    }
	    break;
	default:
	    TRACE(("DATA_ERROR: ignoring unknown ReGIS screen command option '%c' arg \"%s\"\n",
		   state->option, fragment_to_tempstr(&optionarg)));
	    break;
	}
	break;
    case 't':
	TRACE(("inspecting text option \"%c\" with value \"%s\"\n",
	       state->option, fragment_to_tempstr(&optionarg)));
	if (fragment_len(&optionarg) < 1U) {
	    TRACE(("DATA_ERROR: ignoring malformed ReGIS text command option value \"%s\"\n",
		   fragment_to_tempstr(&optionarg)));
	    return 0;
	}
	switch (state->option) {
	case 'A':
	case 'a':
	    TRACE(("found character set specifier \"%s\" FIXME\n",
		   fragment_to_tempstr(&optionarg)));
	    /* FIXME: handle */
	    break;
	case 'B':
	case 'b':
	    TRACE(("found beginning of temporary text control \"%s\" FIXME\n",
		   fragment_to_tempstr(&optionarg)));
	    /* FIXME: handle */
	    break;
	case 'D':
	case 'd':
	    TRACE(("found string tilt control \"%s\" FIXME\n",
		   fragment_to_tempstr(&optionarg)));
	    /* FIXME: handle */
	    break;
	case 'E':
	case 'e':
	    TRACE(("found end of temporary text control \"%s\" FIXME\n",
		   fragment_to_tempstr(&optionarg)));
	    /* FIXME: handle */
	    break;
	case 'H':
	case 'h':
	    TRACE(("found height multiplier \"%s\" FIXME\n",
		   fragment_to_tempstr(&optionarg)));
	    /* FIXME: handle */
	    break;
	case 'I':
	case 'i':
	    TRACE(("found italic control \"%s\" FIXME\n",
		   fragment_to_tempstr(&optionarg)));
	    /* FIXME: handle */
	    break;
	case 'M':
	case 'm':
	    TRACE(("found size multiplier \"%s\" FIXME\n",
		   fragment_to_tempstr(&optionarg)));
	    /* FIXME: handle */
	    break;
	case 'S':
	case 's':
	    TRACE(("found custom display cell size \"%s\" FIXME\n",
		   fragment_to_tempstr(&optionarg)));
	    /* FIXME: handle */
	    break;
	case 'U':
	case 'u':
	    TRACE(("found custom display unit size \"%s\" FIXME\n",
		   fragment_to_tempstr(&optionarg)));
	    /* FIXME: handle */
	    break;
	default:
	    TRACE(("DATA_ERROR: ignoring unknown ReGIS text command option '%c' arg \"%s\"\n",
		   state->option, fragment_to_tempstr(&optionarg)));
	    break;
	}
	break;
    case 'v':
	TRACE(("inspecting vector option \"%c\" with value \"%s\"\n",
	       state->option, fragment_to_tempstr(&optionarg)));
	switch (state->option) {
	case 'B':
	case 'b':
	    TRACE(("found begin bounded position stack \"%s\" FIXME\n",
		   fragment_to_tempstr(&optionarg)));
	    /* FIXME: handle */
	    break;
	case 'E':
	case 'e':
	    TRACE(("found end position stack \"%s\" FIXME\n",
		   fragment_to_tempstr(&optionarg)));
	    /* FIXME: handle */
	    break;
	case 'S':
	case 's':
	    TRACE(("found begin unbounded position stack \"%s\" FIXME\n",
		   fragment_to_tempstr(&optionarg)));
	    /* FIXME: handle */
	    break;
	case 'W':
	case 'w':
	    TRACE(("found temporary write options \"%s\"\n",
		   fragment_to_tempstr(&optionarg)));
	    if (!load_regis_write_control_set(state, context->graphic,
					      context->graphics_output_cursor_x, context->graphics_output_cursor_y,
					      &optionarg, &context->temporary_write_controls)) {
		TRACE(("DATA_ERROR: invalid temporary write options \"%s\"\n",
		       fragment_to_tempstr(&optionarg)));
	    }
	    break;
	default:
	    TRACE(("DATA_ERROR: ignoring unknown ReGIS vector command option '%c' arg \"%s\"\n",
		   state->option, fragment_to_tempstr(&optionarg)));
	    break;
	}
	break;
    case 'w':
	TRACE(("inspecting write option \"%c\" with value \"%s\"\n",
	       state->option, fragment_to_tempstr(&optionarg)));
	if (!load_regis_write_control(state, context->graphic,
				      context->graphics_output_cursor_x, context->graphics_output_cursor_y,
				      state->option, &optionarg, &context->temporary_write_controls)) {
	    TRACE(("DATA_ERROR: invalid write options \"%s\"\n",
		   fragment_to_tempstr(&optionarg)));
	}
	break;
    default:
	TRACE(("DATA_ERROR: unexpected option in \"%c\" command: \"%s\"\n",
	       state->command, fragment_to_tempstr(&optionarg)));
	return 0;
    }

    return 1;
}

static int
parse_regis_items(RegisParseState *state, RegisGraphicsContext *context)
{
    RegisDataFragment *input;
    RegisDataFragment item;

    switch (state->level) {
    case INPUT:
	input = &state->input;
	break;
    case OPTIONSET:
	input = &state->optionset;
	break;
    default:
	TRACE(("invalid parse level: %d\n", state->level));
	return 0;
    }

    if (input->pos >= input->len)
	return 0;

    if (extract_regis_extent(input, &item)) {
	TRACE(("found extent \"%s\"\n", fragment_to_tempstr(&item)));
	switch (state->command) {
	case 'c':
	    /* FIXME: parse, handle */
	    TRACE(("extent in curve command FIXME\n"));
	    break;
	case 'p':
	    /* FIXME TRACE(("DATA_ERROR: ignoring pen command with no location\n")); */
	    if (!load_regis_extent(fragment_to_tempstr(&item),
				   context->graphics_output_cursor_x, context->graphics_output_cursor_y,
				   &context->graphics_output_cursor_x, &context->graphics_output_cursor_y)) {
		TRACE(("DATA_ERROR: unable to parse extent in '%c' command: \"%s\"\n",
		       state->command, fragment_to_tempstr(&item)));
		break;
	    }
	    TRACE(("moving pen to location %d,%d\n",
		   context->graphics_output_cursor_x,
		   context->graphics_output_cursor_y));
	    break;
	case 's':
	    /* FIXME: parse, handle */
	    TRACE(("extent in screen command FIXME\n"));
	    break;
	case 't':
	    /* FIXME: parse, handle */
	    TRACE(("extent in text command FIXME\n"));
	    break;
	case 'v':
	    {
		int orig_x, orig_y;

		orig_x = context->graphics_output_cursor_x;
		orig_y = context->graphics_output_cursor_y;
		if (!load_regis_extent(fragment_to_tempstr(&item),
				       orig_x, orig_y,
				       &context->graphics_output_cursor_x, &context->graphics_output_cursor_y)) {
		    TRACE(("DATA_ERROR: unable to parse extent in '%c' command: \"%s\"\n",
			   state->command, fragment_to_tempstr(&item)));
		    break;
		}
		TRACE(("drawing line to location %d,%d\n",
		       context->graphics_output_cursor_x,
		       context->graphics_output_cursor_y));
		draw_patterned_line(context,
				    orig_x, orig_y,
				    context->graphics_output_cursor_x,
				    context->graphics_output_cursor_y);
	    }
	    break;
	default:
	    TRACE(("DATA_ERROR: unexpected extent in \"%c\" command: \"%s\"\n",
		   state->command, fragment_to_tempstr(&item)));
	    break;
	}
	return 1;
    }

    if (extract_regis_pixelvector(input, &item)) {
	TRACE(("found pixel vector \"%s\"\n", fragment_to_tempstr(&item)));
	switch (state->command) {
	case 'c':
	    /* FIXME: parse, handle */
	    TRACE(("pixelvector in curve command FIXME\n"));
	    break;
	    /* FIXME: not sure if 'f' supports pvs */
	case 'p':
	    /* FIXME: error checking */
	    if (!load_regis_pixelvector(fragment_to_tempstr(&item), context->temporary_write_controls.pv_multiplier,
					context->graphics_output_cursor_x, context->graphics_output_cursor_y,
					&context->graphics_output_cursor_x, &context->graphics_output_cursor_y)) {
		TRACE(("DATA_ERROR: unable to parse pixel vector in '%c' command: \"%s\"\n",
		       state->command, fragment_to_tempstr(&item)));
		break;
	    }
	    TRACE(("moving pen to location %d,%d\n",
		   context->graphics_output_cursor_x,
		   context->graphics_output_cursor_y));
	    break;
	case 's':
	    /* FIXME: parse, handle scroll argument */
	    TRACE(("pixelvector in screen command FIXME\n"));
	    break;
	case 't':
	    /* FIXME: parse, handle */
	    TRACE(("pixelvector in text command FIXME\n"));
	    break;
	case 'v':
	    /* FIXME: error checking */
	    {
		int orig_x, orig_y;

		orig_x = context->graphics_output_cursor_x;
		orig_y = context->graphics_output_cursor_y;
		if (!load_regis_pixelvector(fragment_to_tempstr(&item), context->temporary_write_controls.pv_multiplier,
					    orig_x, orig_y,
					    &context->graphics_output_cursor_x, &context->graphics_output_cursor_y)) {
		    TRACE(("DATA_ERROR: unable to parse pixel vector in '%c' command: \"%s\"\n",
			   state->command, fragment_to_tempstr(&item)));
		    break;
		}
		TRACE(("drawing line to location %d,%d\n",
		       context->graphics_output_cursor_x,
		       context->graphics_output_cursor_y));
		draw_patterned_line(context, orig_x, orig_y,
				    context->graphics_output_cursor_x,
				    context->graphics_output_cursor_y);
	    }
	    break;
	default:
	    TRACE(("DATA_ERROR: unexpected pixel vector in \"%c\" command: \"%s\"\n",
		   state->command, fragment_to_tempstr(&item)));
	    break;
	}
	return 1;
    }

    if (extract_regis_string(input, state->temp, state->templen)) {
	switch (state->command) {
	case 'l':
	    TRACE(("found character to load: \"%s\" FIXME\n", state->temp));
	    /* FIXME: handle */
	case 't':
	    TRACE(("found string to draw: \"%s\" FIXME\n", state->temp));
	    /* FIXME: handle */
	    break;
	default:
	    TRACE(("DATA_ERROR: unexpected string in \"%c\" command: \"%s\"\n",
		   state->command, state->temp));
	    break;
	}
	return 1;
    }

    /* hex values */
    if (state->command == 'l') {
	char ch1 = peek_fragment(input);
	char ch2 = peek_fragment(input);
	if ((ch1 == '0' ||
	     ch1 == '1' ||
	     ch1 == '2' ||
	     ch1 == '3' ||
	     ch1 == '4' ||
	     ch1 == '5' ||
	     ch1 == '6' ||
	     ch1 == '7' ||
	     ch1 == '8' ||
	     ch1 == '9' ||
	     ch1 == 'a' ||
	     ch1 == 'b' ||
	     ch1 == 'c' ||
	     ch1 == 'd' ||
	     ch1 == 'e' ||
	     ch1 == 'f' ||
	     ch1 == 'A' ||
	     ch1 == 'B' ||
	     ch1 == 'C' ||
	     ch1 == 'D' ||
	     ch1 == 'E' ||
	     ch1 == 'F') &&
	    (ch2 == '0' ||
	     ch2 == '1' ||
	     ch2 == '2' ||
	     ch2 == '3' ||
	     ch2 == '4' ||
	     ch2 == '5' ||
	     ch2 == '6' ||
	     ch2 == '7' ||
	     ch2 == '8' ||
	     ch2 == '9' ||
	     ch2 == 'a' ||
	     ch2 == 'b' ||
	     ch2 == 'c' ||
	     ch2 == 'd' ||
	     ch2 == 'e' ||
	     ch2 == 'f' ||
	     ch2 == 'A' ||
	     ch2 == 'B' ||
	     ch2 == 'C' ||
	     ch2 == 'D' ||
	     ch2 == 'E' ||
	     ch2 == 'F')) {
	    /* FIXME: handle */
	    TRACE(("found hex number: \"%c%c\" FIXME\n", ch1, ch2));
	    pop_fragment(input);
	    pop_fragment(input);
	    if (peek_fragment(input) == ',')
		pop_fragment(input);
	    return 1;
	}
    }

    return 0;
}

/*
 * context:
 * two pages of 800x480
 * current page #
 * current command
 * persistent write options
 * temporary write options
 * output position stack
 */
void
parse_regis(XtermWidget xw, ANSI *params, char const *string)
{
    TScreen *screen = TScreenOf(xw);
    char ch;
    RegisGraphicsContext context;
    RegisParseState state;

    (void) xw;
    (void) string;
    (void) params;

    TRACE(("ReGIS vector graphics mode, params=%d\n", params->a_nparam));

    init_fragment(&state.input, string);
    init_fragment(&state.optionset, "");
    state.level = INPUT;
    state.templen = (unsigned) strlen(string) + 1U;
    if (!(state.temp = malloc(state.templen))) {
	TRACE(("Unable to allocate temporary buffer of size %u\n", state.templen));
	return;
    }
    state.command = '_';
    state.option = '_';

    init_regis_graphics_context(screen->terminal_id, &context);
    context.graphic = get_new_or_matching_graphic(xw, 0, 0, 800, 480, 1U);	/* FIXME: use page number */
    context.graphic->valid = 1;
    context.graphic->dirty = 1;
    refresh_modified_displayed_graphics(screen);

    for (;;) {
	state.level = INPUT;
	TRACE(("parsing at top level: %d of %d (next char %c)\n",
	       state.input.pos,
	       state.input.len,
	       peek_fragment(&state.input)));
	if (skip_regis_whitespace(&state.input))
	    continue;
	if (parse_regis_command(&state))
	    continue;
	if (parse_regis_optionset(&state)) {
	    state.level = OPTIONSET;
	    TRACE(("parsing at optionset level: %d of %d\n",
		   state.optionset.pos,
		   state.optionset.len));
	    for (;;) {
		if (skip_regis_whitespace(&state.optionset))
		    continue;
		if (parse_regis_option(&state, &context))
		    continue;
		if (parse_regis_items(&state, &context))
		    continue;
		if (state.optionset.pos >= state.optionset.len)
		    break;
		ch = pop_fragment(&state.optionset);
		TRACE(("DATA_ERROR: skipping unknown token in optionset: \"%c\"\n", ch));
		/* FIXME: suboptions */
	    }
	    state.option = '_';
	    continue;
	}
	if (parse_regis_items(&state, &context))
	    continue;
	if (state.optionset.pos >= state.optionset.len)
	    break;
	ch = pop_fragment(&state.input);
	TRACE(("DATA_ERROR: skipping unknown token at top level: \"%c\"\n", ch));
    }

    free(state.temp);

    context.graphic->dirty = 1;
    refresh_modified_displayed_graphics(screen);
    TRACE(("DONE! Successfully parsed ReGIS data.\n"));
}
