/*
 * $XFree86: xc/programs/xterm/charsets.c,v 1.3 1999/01/23 09:56:18 dawes Exp $
 */

/************************************************************

Copyright 1998, 1999 by Thomas E. Dickey <dickey@clark.net>

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
#include <data.h>

#include <X11/keysym.h>

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

/*
 * Translate an input keysym to the corresponding NRC keysym.
 */
unsigned xtermCharSetIn(unsigned code, int charset)
{
	if (code >= 128 && code < 256) {
		switch (charset) {
		case 'A':	/* United Kingdom set (or Latin 1)	*/
			if (code == XK_sterling)
				code = '#';
			code &= 0x7f;
			break;

#if OPT_XMC_GLITCH
		case '?':
#endif
		case '1':	/* Alternate Character ROM standard characters */
		case '2':	/* Alternate Character ROM special graphics */
		case 'B':	/* ASCII set				*/
			break;

		case '0':	/* special graphics (line drawing)	*/
			break;

		case '4':	/* Dutch */
			switch (code) {
			case XK_sterling:	code = '#';	break;
			case XK_threequarters:	code = '@';	break;
			case XK_ydiaeresis:	code = 0x5b;	break;
			case XK_onehalf:	code = 0x5c;	break;
	/* N/A		case '|':		code = 0x5d;	break; */
			case XK_diaeresis:	code = 0x7b;	break;
	/* N/A		case 'f':		code = 0x7c;	break; */
			case XK_onequarter:	code = 0x7d;	break;
			case XK_acute:		code = 0x7e;	break;
			}
			break;

		case 'C':
		case '5':	/* Finnish */
			switch (code) {
			case XK_Adiaeresis:	code = 0x5b;	break;
			case XK_Odiaeresis:	code = 0x5c;	break;
			case XK_Aring:		code = 0x5d;	break;
			case XK_Udiaeresis:	code = 0x5e;	break;
			case XK_eacute:		code = 0x60;	break;
			case XK_adiaeresis:	code = 0x7b;	break;
			case XK_odiaeresis:	code = 0x7c;	break;
			case XK_aring:		code = 0x7d;	break;
			case XK_udiaeresis:	code = 0x7e;	break;
			}
			break;

		case 'R':	/* French */
			switch (code) {
			case XK_sterling:	code = '#';	break;
			case XK_agrave:		code = '@';	break;
			case XK_degree:		code = 0x5b;	break;
			case XK_ccedilla:	code = 0x5c;	break;
			case XK_section:	code = 0x5d;	break;
			case XK_eacute:		code = 0x7b;	break;
			case XK_ugrave:		code = 0x7c;	break;
			case XK_egrave:		code = 0x7d;	break;
			case XK_diaeresis:	code = 0x7e;	break;
			}
			break;

		case 'Q':	/* French Canadian */
			switch (code) {
			case XK_agrave:		code = '@';	break;
			case XK_acircumflex:	code = 0x5b;	break;
			case XK_ccedilla:	code = 0x5c;	break;
			case XK_ecircumflex:	code = 0x5d;	break;
			case XK_icircumflex:	code = 0x5e;	break;
			case XK_ocircumflex:	code = 0x60;	break;
			case XK_eacute:		code = 0x7b;	break;
			case XK_ugrave:		code = 0x7c;	break;
			case XK_egrave:		code = 0x7d;	break;
			case XK_ucircumflex:	code = 0x7e;	break;
			}
			break;

		case 'K':	/* German */
			switch (code) {
			case XK_section:	code = '@';	break;
			case XK_Adiaeresis:	code = 0x5b;	break;
			case XK_Odiaeresis:	code = 0x5c;	break;
			case XK_Udiaeresis:	code = 0x5d;	break;
			case XK_adiaeresis:	code = 0x7b;	break;
			case XK_odiaeresis:	code = 0x7c;	break;
			case XK_udiaeresis:	code = 0x7d;	break;
			case XK_ssharp:		code = 0x7e;	break;
			}
			break;

		case 'Y':	/* Italian */
			switch (code) {
			case XK_sterling:	code = '#';	break;
			case XK_section:	code = '@';	break;
			case XK_degree:		code = 0x5b;	break;
			case XK_ccedilla:	code = 0x5c;	break;
			case XK_eacute:		code = 0x5d;	break;
			case XK_ugrave:		code = 0x60;	break;
			case XK_agrave:		code = 0x7b;	break;
			case XK_ograve:		code = 0x7c;	break;
			case XK_egrave:		code = 0x7d;	break;
			case XK_igrave:		code = 0x7e;	break;
			}
			break;

		case 'E':
		case '6':	/* Norwegian/Danish */
			switch (code) {
			case XK_Adiaeresis:	code = '@';	break;
			case XK_AE:		code = 0x5b;	break;
			case XK_Ooblique:	code = 0x5c;	break;
			case XK_Aring:		code = 0x5d;	break;
			case XK_Udiaeresis:	code = 0x5e;	break;
			case XK_adiaeresis:	code = 0x60;	break;
			case XK_ae:		code = 0x7b;	break;
			case XK_oslash:		code = 0x7c;	break;
			case XK_aring:		code = 0x7d;	break;
			case XK_udiaeresis:	code = 0x7e;	break;
			}
			break;

		case 'Z':	/* Spanish */
			switch (code) {
			case XK_sterling:	code = '#';	break;
			case XK_section:	code = '@';	break;
			case XK_exclamdown:	code = 0x5b;	break;
			case XK_Ntilde:		code = 0x5c;	break;
			case XK_questiondown:	code = 0x5d;	break;
			case XK_degree:		code = 0x7b;	break;
			case XK_ntilde:		code = 0x7c;	break;
			case XK_ccedilla:	code = 0x7d;	break;
			}
			break;

		case 'H':
		case '7':	/* Swedish */
			switch (code) {
			case XK_Eacute:		code = '@';	break;
			case XK_Adiaeresis:	code = 0x5b;	break;
			case XK_Odiaeresis:	code = 0x5c;	break;
			case XK_Aring:		code = 0x5d;	break;
			case XK_Udiaeresis:	code = 0x5e;	break;
			case XK_eacute:		code = 0x60;	break;
			case XK_adiaeresis:	code = 0x7b;	break;
			case XK_odiaeresis:	code = 0x7c;	break;
			case XK_aring:		code = 0x7d;	break;
			case XK_udiaeresis:	code = 0x7e;	break;
			}
			break;

		case '=':	/* Swiss */
			switch (code) {
			case XK_ugrave:		code = '#';	break;
			case XK_agrave:		code = '@';	break;
			case XK_eacute:		code = 0x5b;	break;
			case XK_ccedilla:	code = 0x5c;	break;
			case XK_ecircumflex:	code = 0x5d;	break;
			case XK_icircumflex:	code = 0x5e;	break;
			case XK_egrave:		code = 0x5f;	break;
			case XK_ocircumflex:	code = 0x60;	break;
			case XK_adiaeresis:	code = 0x7b;	break;
			case XK_odiaeresis:	code = 0x7c;	break;
			case XK_udiaeresis:	code = 0x7d;	break;
			case XK_ucircumflex:	code = 0x7e;	break;
			}
			break;

		default:	/* any character sets we don't recognize*/
			break;
		}
		code &= 0x7f;	/* NRC in any case is 7-bit */
	}
	return code;
}

/*
 * Translate a string to the display form.  This assumes the font has the
 * DEC graphic characters in cells 0-31, and otherwise is ISO-8859-1.
 */
int xtermCharSetOut(Char *buf, Char *ptr, char leftset)
{
	Char *s;
	register TScreen *screen = &term->screen;
	int count = 0;
	int rightset = screen->gsets[(int)(screen->curgr)];

	TRACE(("CHARSET GL=%c(G%d) GR=%c(G%d) %.*s\n",
		leftset,  screen->curss ? screen->curss : screen->curgl,
		rightset, screen->curgr,
		ptr-buf, buf))

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
				*s = (*s == 0x5f) ? 0x7f : *s - 0x5f;
			break;

		case '4':	/* Dutch */
			switch (*s &= 0x7f) {
			case '#':	*s = XK_sterling;	break;
			case '@':	*s = XK_threequarters;	break;
			case 0x5b:	*s = XK_ydiaeresis;	break;
			case 0x5c:	*s = XK_onehalf;	break;
			case 0x5d:	*s = '|';		break;
			case 0x7b:	*s = XK_diaeresis;	break;
			case 0x7c:	*s = 'f';		break;
			case 0x7d:	*s = XK_onequarter;	break;
			case 0x7e:	*s = XK_acute;		break;
			}
			break;

		case 'C':
		case '5':	/* Finnish */
			switch (*s &= 0x7f) {
			case 0x5b:	*s = XK_Adiaeresis;	break;
			case 0x5c:	*s = XK_Odiaeresis;	break;
			case 0x5d:	*s = XK_Aring;		break;
			case 0x5e:	*s = XK_Udiaeresis;	break;
			case 0x60:	*s = XK_eacute;		break;
			case 0x7b:	*s = XK_adiaeresis;	break;
			case 0x7c:	*s = XK_odiaeresis;	break;
			case 0x7d:	*s = XK_aring;		break;
			case 0x7e:	*s = XK_udiaeresis;	break;
			}
			break;

		case 'R':	/* French */
			switch (*s &= 0x7f) {
			case '#':	*s = XK_sterling;	break;
			case '@':	*s = XK_agrave;		break;
			case 0x5b:	*s = XK_degree;		break;
			case 0x5c:	*s = XK_ccedilla;	break;
			case 0x5d:	*s = XK_section;	break;
			case 0x7b:	*s = XK_eacute;		break;
			case 0x7c:	*s = XK_ugrave;		break;
			case 0x7d:	*s = XK_egrave;		break;
			case 0x7e:	*s = XK_diaeresis;	break;
			}
			break;

		case 'Q':	/* French Canadian */
			switch (*s &= 0x7f) {
			case '@':	*s = XK_agrave;		break;
			case 0x5b:	*s = XK_acircumflex;	break;
			case 0x5c:	*s = XK_ccedilla;	break;
			case 0x5d:	*s = XK_ecircumflex;	break;
			case 0x5e:	*s = XK_icircumflex;	break;
			case 0x60:	*s = XK_ocircumflex;	break;
			case 0x7b:	*s = XK_eacute;		break;
			case 0x7c:	*s = XK_ugrave;		break;
			case 0x7d:	*s = XK_egrave;		break;
			case 0x7e:	*s = XK_ucircumflex;	break;
			}
			break;

		case 'K':	/* German */
			switch (*s &= 0x7f) {
			case '@':	*s = XK_section;	break;
			case 0x5b:	*s = XK_Adiaeresis;	break;
			case 0x5c:	*s = XK_Odiaeresis;	break;
			case 0x5d:	*s = XK_Udiaeresis;	break;
			case 0x7b:	*s = XK_adiaeresis;	break;
			case 0x7c:	*s = XK_odiaeresis;	break;
			case 0x7d:	*s = XK_udiaeresis;	break;
			case 0x7e:	*s = XK_ssharp;		break;
			}
			break;

		case 'Y':	/* Italian */
			switch (*s &= 0x7f) {
			case '#':	*s = XK_sterling;	break;
			case '@':	*s = XK_section;	break;
			case 0x5b:	*s = XK_degree;		break;
			case 0x5c:	*s = XK_ccedilla;	break;
			case 0x5d:	*s = XK_eacute;		break;
			case 0x60:	*s = XK_ugrave;		break;
			case 0x7b:	*s = XK_agrave;		break;
			case 0x7c:	*s = XK_ograve;		break;
			case 0x7d:	*s = XK_egrave;		break;
			case 0x7e:	*s = XK_igrave;		break;
			}
			break;

		case 'E':
		case '6':	/* Norwegian/Danish */
			switch (*s &= 0x7f) {
			case '@':	*s = XK_Adiaeresis;	break;
			case 0x5b:	*s = XK_AE;		break;
			case 0x5c:	*s = XK_Ooblique;	break;
			case 0x5d:	*s = XK_Aring;		break;
			case 0x5e:	*s = XK_Udiaeresis;	break;
			case 0x60:	*s = XK_adiaeresis;	break;
			case 0x7b:	*s = XK_ae;		break;
			case 0x7c:	*s = XK_oslash;		break;
			case 0x7d:	*s = XK_aring;		break;
			case 0x7e:	*s = XK_udiaeresis;	break;
			}
			break;

		case 'Z':	/* Spanish */
			switch (*s &= 0x7f) {
			case '#':	*s = XK_sterling;	break;
			case '@':	*s = XK_section;	break;
			case 0x5b:	*s = XK_exclamdown;	break;
			case 0x5c:	*s = XK_Ntilde;		break;
			case 0x5d:	*s = XK_questiondown;	break;
			case 0x7b:	*s = XK_degree;		break;
			case 0x7c:	*s = XK_ntilde;		break;
			case 0x7d:	*s = XK_ccedilla;	break;
			}
			break;

		case 'H':
		case '7':	/* Swedish */
			switch (*s &= 0x7f) {
			case '@':	*s = XK_Eacute;		break;
			case 0x5b:	*s = XK_Adiaeresis;	break;
			case 0x5c:	*s = XK_Odiaeresis;	break;
			case 0x5d:	*s = XK_Aring;		break;
			case 0x5e:	*s = XK_Udiaeresis;	break;
			case 0x60:	*s = XK_eacute;		break;
			case 0x7b:	*s = XK_adiaeresis;	break;
			case 0x7c:	*s = XK_odiaeresis;	break;
			case 0x7d:	*s = XK_aring;		break;
			case 0x7e:	*s = XK_udiaeresis;	break;
			}
			break;

		case '=':	/* Swiss */
			switch (*s &= 0x7f) {
			case '#':	*s = XK_ugrave;		break;
			case '@':	*s = XK_agrave;		break;
			case 0x5b:	*s = XK_eacute;		break;
			case 0x5c:	*s = XK_ccedilla;	break;
			case 0x5d:	*s = XK_ecircumflex;	break;
			case 0x5e:	*s = XK_icircumflex;	break;
			case 0x5f:	*s = XK_egrave;		break;
			case 0x60:	*s = XK_ocircumflex;	break;
			case 0x7b:	*s = XK_adiaeresis;	break;
			case 0x7c:	*s = XK_odiaeresis;	break;
			case 0x7d:	*s = XK_udiaeresis;	break;
			case 0x7e:	*s = XK_ucircumflex;	break;
			}
			break;

		default:	/* any character sets we don't recognize*/
			count --;
			break;
		}
	}
	return count;
}
