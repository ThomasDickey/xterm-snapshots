/*
 * $XFree86: xc/programs/xterm/charsets.c,v 1.1 1998/04/05 00:49:04 robin Exp $
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

#ifdef HAVE_CONFIG_H
#include <xtermcfg.h>
#endif

#include <X11/Xos.h>

#include "ptyx.h"
#include "data.h"
#include "xterm.h"

/*
 * This module performs translation as needed to support the DEC VT220 national
 * replacement character sets.  We assume that xterm's font is based on the ISO
 * 8859-1 (Latin 1) character set, which is equivalent to the DEC multinational
 * character set.  (Glyph positions 0-31 have to be the DEC graphic characters,
 * though).
 *
 * References:
 *	"VT220 Programmer Pocket Guide" EK-VT220-HR-002 (2nd ed., 1984), which
 *		contains character charts for the national character sets.
 *	"VT330/VT340 Programmer Reference Manual Volume 1: Text Programming"
 *		EK-VT3XX-TP-001 (1st ed, 1987), which contains a table (2-1)
 *		listing the glyphs which are mapped from the multinational
 *		character set to the national character set.
 *
 * The latter reference, though easier to read, has a few errors and omissions.
 */
int xtermCharSets(Char *buf, Char *ptr, char leftset)
{
	Char *s;
	register TScreen *screen = &term->screen;
	int count = 0;
	int rightset = screen->gsets[(int)(screen->curgr)];

	TRACE(("CHARSET %c/%c %.*s\n", leftset, rightset, ptr-buf, buf))

	for (s=buf; s<ptr; ++s) {
		int cs = (*s >= 128) ? rightset : leftset;

		count++;
		switch (cs) {
		case 'A':	/* United Kingdom set (or Latin 1)	*/
			if ((term->flags & NATIONAL)
			 || (screen->ansi_level <= 1)) {
				*s &= 0xff;
				if (*s == '#')
					*s = '\036';	/* UK pound sign*/
			} else {
				*s |= 0x80;
			}
			break;

#if OPT_XMC_GLITCH
		case '?':
#endif
		case '1':	/* Alternate Character ROM standard characters */
		case '2':	/* Alternate Character ROM special graphics */
		case 'B':	/* ASCII set				*/
			break;

		case '0':	/* special graphics (line drawing)	*/
			if (*s>=0x5f && *s<=0x7e)
				*s = *s == 0x5f ? 0x7f : *s - 0x5f;
			break;

		case '4':	/* Dutch */
			switch (*s &= 0x7f) {
			case '#':	*s = 0x1e;		break;
			case '@':	*s = 0xbe;		break;
			case 0x5b:	*s = 0xff;		break;
			case 0x5c:	*s = 0xbd;		break;
			case 0x5d:	*s = '|';		break;
			case 0x7b:	*s = 0xa8;		break;
			case 0x7c:	*s = 'f';		break;
			case 0x7d:	*s = 0xbc;		break;
			case 0x7e:	*s = 0xb4;		break;
			}
			break;

		case 'C':
		case '5':	/* Finnish */
			switch (*s &= 0x7f) {
			case 0x5b:	*s = 0xc4;		break;
			case 0x5c:	*s = 0xd6;		break;
			case 0x5d:	*s = 0xc5;		break;
			case 0x5e:	*s = 0xdc;		break;
			case 0x60:	*s = 0xe9;		break;
			case 0x7b:	*s = 0xe4;		break;
			case 0x7c:	*s = 0xf6;		break;
			case 0x7d:	*s = 0xe5;		break;
			case 0x7e:	*s = 0xfc;		break;
			}
			break;

		case 'R':	/* French */
			switch (*s &= 0x7f) {
			case '#':	*s = 0x1e;		break;
			case '@':	*s = 0xe0;		break;
			case 0x5b:	*s = 0xb0;		break;
			case 0x5c:	*s = 0xe7;		break;
			case 0x5d:	*s = 0xa7;		break;
			case 0x7b:	*s = 0xe9;		break;
			case 0x7c:	*s = 0xf9;		break;
			case 0x7d:	*s = 0xe8;		break;
			case 0x7e:	*s = 0xa8;		break;
			}
		break;

		case 'Q':	/* French Canadian */
			switch (*s &= 0x7f) {
			case '@':	*s = 0xe0;		break;
			case 0x5b:	*s = 0xe2;		break;
			case 0x5c:	*s = 0xe7;		break;
			case 0x5d:	*s = 0xea;		break;
			case 0x5e:	*s = 0xee;		break;
			case 0x60:	*s = 0xf4;		break;
			case 0x7b:	*s = 0xe9;		break;
			case 0x7c:	*s = 0xf9;		break;
			case 0x7d:	*s = 0xe8;		break;
			case 0x7e:	*s = 0xfb;		break;
			}
			break;

		case 'K':	/* German */
			switch (*s &= 0x7f) {
			case '@':	*s = 0xa7;		break;
			case 0x5b:	*s = 0xc4;		break;
			case 0x5c:	*s = 0xd6;		break;
			case 0x5d:	*s = 0xdc;		break;
			case 0x7b:	*s = 0xe4;		break;
			case 0x7c:	*s = 0xf6;		break;
			case 0x7d:	*s = 0xfc;		break;
			case 0x7e:	*s = 0xdf;		break;
			}
			break;

		case 'Y':	/* Italian */
			switch (*s &= 0x7f) {
			case '#':	*s = 0x1e;		break;
			case '@':	*s = 0xa7;		break;
			case 0x5b:	*s = 0xb0;		break;
			case 0x5c:	*s = 0xe7;		break;
			case 0x5d:	*s = 0xe9;		break;
			case 0x60:	*s = 0xf9;		break;
			case 0x7b:	*s = 0xe0;		break;
			case 0x7c:	*s = 0xf2;		break;
			case 0x7d:	*s = 0xe8;		break;
			case 0x7e:	*s = 0xec;		break;
			}
			break;

		case 'E':
		case '6':	/* Norwegian/Danish */
			switch (*s &= 0x7f) {
			case '@':	*s = 0xc4;		break;
			case 0x5b:	*s = 0xc6;		break;
			case 0x5c:	*s = 0xd8;		break;
			case 0x5d:	*s = 0xc5;		break;
			case 0x5e:	*s = 0xdc;		break;
			case 0x60:	*s = 0xe4;		break;
			case 0x7b:	*s = 0xe6;		break;
			case 0x7c:	*s = 0xf8;		break;
			case 0x7d:	*s = 0xe5;		break;
			case 0x7e:	*s = 0xfc;		break;
			}
			break;

		case 'Z':	/* Spanish */
			switch (*s &= 0x7f) {
			case '#':	*s = 0x1e;		break;
			case '@':	*s = 0xa7;		break;
			case 0x5b:	*s = 0xa1;		break;
			case 0x5c:	*s = 0xd1;		break;
			case 0x5d:	*s = 0xbf;		break;
			case 0x7b:	*s = 0xb0;		break;
			case 0x7c:	*s = 0xf1;		break;
			case 0x7d:	*s = 0xe7;		break;
			}
			break;

		case 'H':
		case '7':	/* Swedish */
			switch (*s &= 0x7f) {
			case '@':	*s = 0xc9;		break;
			case 0x5b:	*s = 0xc4;		break;
			case 0x5c:	*s = 0xd6;		break;
			case 0x5d:	*s = 0xc5;		break;
			case 0x5e:	*s = 0xdc;		break;
			case 0x60:	*s = 0xe9;		break;
			case 0x7b:	*s = 0xe4;		break;
			case 0x7c:	*s = 0xf6;		break;
			case 0x7d:	*s = 0xe5;		break;
			case 0x7e:	*s = 0xfc;		break;
			}
			break;

		case '=':	/* Swiss */
			switch (*s &= 0x7f) {
			case '#':	*s = 0xf9;		break;
			case '@':	*s = 0xe0;		break;
			case 0x5b:	*s = 0xe9;		break;
			case 0x5c:	*s = 0xe7;		break;
			case 0x5d:	*s = 0xea;		break;
			case 0x5e:	*s = 0xee;		break;
			case 0x5f:	*s = 0xe8;		break;
			case 0x60:	*s = 0xf4;		break;
			case 0x7b:	*s = 0xe4;		break;
			case 0x7c:	*s = 0xf6;		break;
			case 0x7d:	*s = 0xfc;		break;
			case 0x7e:	*s = 0xfb;		break;
			}
			break;

		default:	/* any character sets we don't recognize*/
			count --;
			break;
		}
	}
	return count;
}
