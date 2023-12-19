/*
 * $XTermId: charsets.h,v 1.25 2023/12/19 21:57:10 tom Exp $
 */

/*
 * Copyright 2023 by Thomas E. Dickey
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

#ifndef included_charsets_h
#define included_charsets_h 1

/*
 * According to
 *  Digital ANSI-Compliant Printing Protocol
 *  Level 2 Programming Reference Manual
 *  EK-PPLV2-PM. B01
 *
 * the supplementary character sets Greek, Hebrew, Latin-5 and Latin/Cyrillic
 * are standardized by ISO:
 *  ISO Greek is 8859-7
 *  ISO Hebrew is 8859-8
 *  ISO Latin-5 is 8859-9
 *  ISO Latin/Cyrillic is 8859-5
 *
 * These are derived from the data at
 *  ftp://www.unicode.org/Public/MAPPINGS/ISO8859/
 *
 * Note: the "figure A-xx" comments refer to EK-PPLV2-PM.
 */

#define PUA(n) (0xEEEE + (n))
#define UNDEF  0x2426		/* rendered as a backwards "?" */

/*
 * A "codepage" is treated different from the NRC mode:  it is always enabled.
 * Reuse the UNI() macros by temporarily setting its state.
 */

#if OPT_WIDE_CHARS
#define begin_CODEPAGE(size) \
	if (!(xw->flags & NATIONAL)) { \
	    screen->utf8_nrc_mode++; \
	}
#define end_CODEPAGE() \
	if (!(xw->flags & NATIONAL)) { \
	    screen->utf8_nrc_mode--; \
	}
#else
#define begin_CODEPAGE(size)	/* nothing */
#define end_CODEPAGE()		/* nothing */
#endif

/*
 * xterm's original implementation of NRCS in 1998 was before Unicode became
 * prevalent.  Most of the necessary mappings could be done using definitions
 * from X11/keysymdef.h, using ISO-8859-1 as the default.
 */

#define map_ASCII(code) \
	switch (code) { \
	    XXX(0xA0, UNDEF) \
	    XXX(0xFF, UNDEF) \
	}

#define map_Latin_1(code) \
	switch (code) { \
	}

#define map_NRCS_Dutch(code) \
	switch (code) { \
	    MAP(0x23, XK_sterling)	/* U+00A3 POUND SIGN */ \
	    MAP(0x40, XK_threequarters)	/* U+00BE VULGAR FRACTION THREE QUARTERS */ \
	    UNI(0x5B, 0x0133)	/* LATIN SMALL LIGATURE IJ */ \
	    MAP(0x5C, XK_onehalf)	/* U+00BD VULGAR FRACTION ONE HALF */ \
	    MAP(0x5D, XK_bar)	/* U+007C VERTICAL LINE */ \
	    MAP(0x7B, XK_diaeresis)	/* U+00A8 DIAERESIS */ \
	    UNI(0x7C, 0x0192)	/* LATIN SMALL LETTER F WITH HOOK (florin) */ \
	    MAP(0x7D, XK_onequarter)	/* U+00BC VULGAR FRACTION ONE QUARTER */ \
	    MAP(0x7E, XK_acute)	/* U+00B4 ACUTE ACCENT */ \
	}

#define unmap_NRCS_Dutch(code) \
	switch (code) { \
	    MAP(0x0133,           0x5B) /* LATIN SMALL LIGATURE IJ */ \
	    MAP(0x0192,           0x7C) /* LATIN SMALL LETTER F WITH HOOK (florin) */ \
	    MAP(XK_acute,         0x7E) /* U+00B4 ACUTE ACCENT */ \
	    MAP(XK_bar,           0x5D) /* U+007C VERTICAL LINE */ \
	    MAP(XK_diaeresis,     0x7B) /* U+00A8 DIAERESIS */ \
	    MAP(XK_onehalf,       0x5C) /* U+00BD VULGAR FRACTION ONE HALF */ \
	    MAP(XK_onequarter,    0x7D) /* U+00BC VULGAR FRACTION ONE QUARTER */ \
	    MAP(XK_sterling,      0x23) /* U+00A3 POUND SIGN */ \
	    MAP(XK_threequarters, 0x40) /* U+00BE VULGAR FRACTION THREE QUARTERS */ \
	}

#define map_NRCS_Finnish(code) \
	switch (code) { \
	    MAP(0x5B, XK_Adiaeresis)	/* U+00C4 LATIN CAPITAL LETTER A WITH DIAERESIS */ \
	    MAP(0x5C, XK_Odiaeresis)	/* U+00D6 LATIN CAPITAL LETTER O WITH DIAERESIS */ \
	    MAP(0x5D, XK_Aring)	/* U+00C5 LATIN CAPITAL LETTER A WITH RING ABOVE */ \
	    MAP(0x5E, XK_Udiaeresis)	/* U+00DC LATIN CAPITAL LETTER U WITH DIAERESIS */ \
	    MAP(0x60, XK_eacute)	/* U+00E9 LATIN SMALL LETTER E WITH ACUTE */ \
	    MAP(0x7B, XK_adiaeresis)	/* U+00E4 LATIN SMALL LETTER A WITH DIAERESIS */ \
	    MAP(0x7C, XK_odiaeresis)	/* U+00F6 LATIN SMALL LETTER O WITH DIAERESIS */ \
	    MAP(0x7D, XK_aring)	/* U+00E5 LATIN SMALL LETTER A WITH RING ABOVE */ \
	    MAP(0x7E, XK_udiaeresis)	/* U+00FC LATIN SMALL LETTER U WITH DIAERESIS */ \
	}

#define unmap_NRCS_Finnish(code) \
	switch (code) { \
	    MAP(XK_Adiaeresis,    0x5B) /* U+00C4 LATIN CAPITAL LETTER A WITH DIAERESIS */ \
	    MAP(XK_Aring,         0x5D) /* U+00C5 LATIN CAPITAL LETTER A WITH RING ABOVE */ \
	    MAP(XK_Odiaeresis,    0x5C) /* U+00D6 LATIN CAPITAL LETTER O WITH DIAERESIS */ \
	    MAP(XK_Udiaeresis,    0x5E) /* U+00DC LATIN CAPITAL LETTER U WITH DIAERESIS */ \
	    MAP(XK_adiaeresis,    0x7B) /* U+00E4 LATIN SMALL LETTER A WITH DIAERESIS */ \
	    MAP(XK_aring,         0x7D) /* U+00E5 LATIN SMALL LETTER A WITH RING ABOVE */ \
	    MAP(XK_eacute,        0x60) /* U+00E9 LATIN SMALL LETTER E WITH ACUTE */ \
	    MAP(XK_odiaeresis,    0x7C) /* U+00F6 LATIN SMALL LETTER O WITH DIAERESIS */ \
	    MAP(XK_udiaeresis,    0x7E) /* U+00FC LATIN SMALL LETTER U WITH DIAERESIS */ \
	}

#define map_NRCS_French(code) \
	switch (code) { \
	    MAP(0x23, XK_sterling)	/* U+00A3 POUND SIGN */ \
	    MAP(0x40, XK_agrave)	/* U+00E0 LATIN SMALL LETTER A WITH GRAVE */ \
	    MAP(0x5B, XK_degree)	/* U+00B0 DEGREE SIGN */ \
	    MAP(0x5C, XK_ccedilla)	/* U+00E7 LATIN SMALL LETTER C WITH CEDILLA */ \
	    MAP(0x5D, XK_section)	/* U+00A7 SECTION SIGN */ \
	    MAP(0x7B, XK_eacute)	/* U+00E9 LATIN SMALL LETTER E WITH ACUTE */ \
	    MAP(0x7C, XK_ugrave)	/* U+00F9 LATIN SMALL LETTER U WITH GRAVE */ \
	    MAP(0x7D, XK_egrave)	/* U+00E8 LATIN SMALL LETTER E WITH GRAVE */ \
	    MAP(0x7E, XK_diaeresis)	/* U+00A8 DIAERESIS */ \
	}

#define unmap_NRCS_French(code) \
	switch (code) { \
	    MAP(XK_agrave,        0x40) /* U+00E0 LATIN SMALL LETTER A WITH GRAVE */ \
	    MAP(XK_ccedilla,      0x5C) /* U+00E7 LATIN SMALL LETTER C WITH CEDILLA */ \
	    MAP(XK_degree,        0x5B) /* U+00B0 DEGREE SIGN */ \
	    MAP(XK_diaeresis,     0x7E) /* U+00A8 DIAERESIS */ \
	    MAP(XK_eacute,        0x7B) /* U+00E9 LATIN SMALL LETTER E WITH ACUTE */ \
	    MAP(XK_egrave,        0x7D) /* U+00E8 LATIN SMALL LETTER E WITH GRAVE */ \
	    MAP(XK_section,       0x5D) /* U+00A7 SECTION SIGN */ \
	    MAP(XK_sterling,      0x23) /* U+00A3 POUND SIGN */ \
	    MAP(XK_ugrave,        0x7C) /* U+00F9 LATIN SMALL LETTER U WITH GRAVE */ \
	}

#define map_NRCS_French_Canadian(code) \
	switch (code) { \
	    MAP(0x40, XK_agrave)	/* U+00E0 LATIN SMALL LETTER A WITH GRAVE */ \
	    MAP(0x5B, XK_acircumflex)	/* U+00E2 LATIN SMALL LETTER A WITH CIRCUMFLEX */ \
	    MAP(0x5C, XK_ccedilla)	/* U+00E7 LATIN SMALL LETTER C WITH CEDILLA */ \
	    MAP(0x5D, XK_ecircumflex)	/* U+00EA LATIN SMALL LETTER E WITH CIRCUMFLEX */ \
	    MAP(0x5E, XK_icircumflex)	/* U+00EE LATIN SMALL LETTER I WITH CIRCUMFLEX */ \
	    MAP(0x60, XK_ocircumflex)	/* U+00F4 LATIN SMALL LETTER O WITH CIRCUMFLEX */ \
	    MAP(0x7B, XK_eacute)	/* U+00E9 LATIN SMALL LETTER E WITH ACUTE */ \
	    MAP(0x7C, XK_ugrave)	/* U+00F9 LATIN SMALL LETTER U WITH GRAVE */ \
	    MAP(0x7D, XK_egrave)	/* U+00E8 LATIN SMALL LETTER E WITH GRAVE */ \
	    MAP(0x7E, XK_ucircumflex)	/* U+00FB LATIN SMALL LETTER U WITH CIRCUMFLEX */ \
	}

#define unmap_NRCS_French_Canadian(code) \
	switch (code) { \
	    MAP(XK_acircumflex,   0x5B) /* U+00E2 LATIN SMALL LETTER A WITH CIRCUMFLEX */ \
	    MAP(XK_agrave,        0x40) /* U+00E0 LATIN SMALL LETTER A WITH GRAVE */ \
	    MAP(XK_ccedilla,      0x5C) /* U+00E7 LATIN SMALL LETTER C WITH CEDILLA */ \
	    MAP(XK_eacute,        0x7B) /* U+00E9 LATIN SMALL LETTER E WITH ACUTE */ \
	    MAP(XK_ecircumflex,   0x5D) /* U+00EA LATIN SMALL LETTER E WITH CIRCUMFLEX */ \
	    MAP(XK_egrave,        0x7D) /* U+00E8 LATIN SMALL LETTER E WITH GRAVE */ \
	    MAP(XK_icircumflex,   0x5E) /* U+00EE LATIN SMALL LETTER I WITH CIRCUMFLEX */ \
	    MAP(XK_ocircumflex,   0x60) /* U+00F4 LATIN SMALL LETTER O WITH CIRCUMFLEX */ \
	    MAP(XK_ucircumflex,   0x7E) /* U+00FB LATIN SMALL LETTER U WITH CIRCUMFLEX */ \
	    MAP(XK_ugrave,        0x7C) /* U+00F9 LATIN SMALL LETTER U WITH GRAVE */ \
	}

#define map_NRCS_German(code) \
	switch (code) { \
	    MAP(0x40, XK_section)	/* U+00A7 SECTION SIGN */ \
	    MAP(0x5B, XK_Adiaeresis)	/* U+00C4 LATIN CAPITAL LETTER A WITH DIAERESIS */ \
	    MAP(0x5C, XK_Odiaeresis)	/* U+00D6 LATIN CAPITAL LETTER O WITH DIAERESIS */ \
	    MAP(0x5D, XK_Udiaeresis)	/* U+00DC LATIN CAPITAL LETTER U WITH DIAERESIS */ \
	    MAP(0x7B, XK_adiaeresis)	/* U+00E4 LATIN SMALL LETTER A WITH DIAERESIS */ \
	    MAP(0x7C, XK_odiaeresis)	/* U+00F6 LATIN SMALL LETTER O WITH DIAERESIS */ \
	    MAP(0x7D, XK_udiaeresis)	/* U+00FC LATIN SMALL LETTER U WITH DIAERESIS */ \
	    MAP(0x7E, XK_ssharp)	/* U+00DF LATIN SMALL LETTER SHARP S */ \
	}

#define unmap_NRCS_German(code) \
	switch (code) { \
	    MAP(XK_Adiaeresis,    0x5B) /* U+00C4 LATIN CAPITAL LETTER A WITH DIAERESIS */ \
	    MAP(XK_Odiaeresis,    0x5C) /* U+00D6 LATIN CAPITAL LETTER O WITH DIAERESIS */ \
	    MAP(XK_Udiaeresis,    0x5D) /* U+00DC LATIN CAPITAL LETTER U WITH DIAERESIS */ \
	    MAP(XK_adiaeresis,    0x7B) /* U+00E4 LATIN SMALL LETTER A WITH DIAERESIS */ \
	    MAP(XK_odiaeresis,    0x7C) /* U+00F6 LATIN SMALL LETTER O WITH DIAERESIS */ \
	    MAP(XK_section,       0x40) /* U+00A7 SECTION SIGN */ \
	    MAP(XK_ssharp,        0x7E) /* U+00DF LATIN SMALL LETTER SHARP S */ \
	    MAP(XK_udiaeresis,    0x7D) /* U+00FC LATIN SMALL LETTER U WITH DIAERESIS */ \
	}

#define map_NRCS_Italian(code) \
	switch (code) { \
	    MAP(0x23, XK_sterling)	/* U+00A3 POUND SIGN */ \
	    MAP(0x40, XK_section)	/* U+00A7 SECTION SIGN */ \
	    MAP(0x5B, XK_degree)	/* U+00B0 DEGREE SIGN */ \
	    MAP(0x5C, XK_ccedilla)	/* U+00E7 LATIN SMALL LETTER C WITH CEDILLA */ \
	    MAP(0x5D, XK_eacute)	/* U+00E9 LATIN SMALL LETTER E WITH ACUTE */ \
	    MAP(0x60, XK_ugrave)	/* U+00F9 LATIN SMALL LETTER U WITH GRAVE */ \
	    MAP(0x7B, XK_agrave)	/* U+00E0 LATIN SMALL LETTER A WITH GRAVE */ \
	    MAP(0x7C, XK_ograve)	/* U+00F2 LATIN SMALL LETTER O WITH GRAVE */ \
	    MAP(0x7D, XK_egrave)	/* U+00E8 LATIN SMALL LETTER E WITH GRAVE */ \
	    MAP(0x7E, XK_igrave)	/* U+00EC LATIN SMALL LETTER I WITH GRAVE */ \
	}

#define unmap_NRCS_Italian(code) \
	switch (code) { \
	    MAP(XK_agrave,        0x7B) /* U+00E0 LATIN SMALL LETTER A WITH GRAVE */ \
	    MAP(XK_ccedilla,      0x5C) /* U+00E7 LATIN SMALL LETTER C WITH CEDILLA */ \
	    MAP(XK_degree,        0x5B) /* U+00B0 DEGREE SIGN */ \
	    MAP(XK_eacute,        0x5D) /* U+00E9 LATIN SMALL LETTER E WITH ACUTE */ \
	    MAP(XK_egrave,        0x7D) /* U+00E8 LATIN SMALL LETTER E WITH GRAVE */ \
	    MAP(XK_igrave,        0x7E) /* U+00EC LATIN SMALL LETTER I WITH GRAVE */ \
	    MAP(XK_ograve,        0x7C) /* U+00F2 LATIN SMALL LETTER O WITH GRAVE */ \
	    MAP(XK_section,       0x40) /* U+00A7 SECTION SIGN */ \
	    MAP(XK_sterling,      0x23) /* U+00A3 POUND SIGN */ \
	    MAP(XK_ugrave,        0x60) /* U+00F9 LATIN SMALL LETTER U WITH GRAVE */ \
	}

#define map_NRCS_Norwegian_Danish(code) \
	switch (code) { \
	    MAP(0x40, XK_Adiaeresis)	/* U+00C4 LATIN CAPITAL LETTER A WITH DIAERESIS */ \
	    MAP(0x5B, XK_AE)	/* U+00C6 LATIN CAPITAL LETTER AE */ \
	    MAP(0x5C, XK_Ooblique)	/* U+00D8 LATIN CAPITAL LETTER O WITH STROKE */ \
	    MAP(0x5D, XK_Aring)	/* U+00C5 LATIN CAPITAL LETTER A WITH RING ABOVE */ \
	    MAP(0x5E, XK_Udiaeresis)	/* U+00DC LATIN CAPITAL LETTER U WITH DIAERESIS */ \
	    MAP(0x60, XK_adiaeresis)	/* U+00E4 LATIN SMALL LETTER A WITH DIAERESIS */ \
	    MAP(0x7B, XK_ae)	/* U+00E6 LATIN SMALL LETTER AE */ \
	    MAP(0x7C, XK_oslash)	/* U+00F8 LATIN SMALL LETTER O WITH STROKE */ \
	    MAP(0x7D, XK_aring)	/* U+00E5 LATIN SMALL LETTER A WITH RING ABOVE */ \
	    MAP(0x7E, XK_udiaeresis)	/* U+00FC LATIN SMALL LETTER U WITH DIAERESIS */ \
	}

#define unmap_NRCS_Norwegian_Danish(code) \
	switch (code) { \
	    MAP(XK_AE,            0x5B) /* U+00C6 LATIN CAPITAL LETTER AE */ \
	    MAP(XK_Adiaeresis,    0x40) /* U+00C4 LATIN CAPITAL LETTER A WITH DIAERESIS */ \
	    MAP(XK_Aring,         0x5D) /* U+00C5 LATIN CAPITAL LETTER A WITH RING ABOVE */ \
	    MAP(XK_Ooblique,      0x5C) /* U+00D8 LATIN CAPITAL LETTER O WITH STROKE */ \
	    MAP(XK_Udiaeresis,    0x5E) /* U+00DC LATIN CAPITAL LETTER U WITH DIAERESIS */ \
	    MAP(XK_adiaeresis,    0x60) /* U+00E4 LATIN SMALL LETTER A WITH DIAERESIS */ \
	    MAP(XK_ae,            0x7B) /* U+00E6 LATIN SMALL LETTER AE */ \
	    MAP(XK_aring,         0x7D) /* U+00E5 LATIN SMALL LETTER A WITH RING ABOVE */ \
	    MAP(XK_oslash,        0x7C) /* U+00F8 LATIN SMALL LETTER O WITH STROKE */ \
	    MAP(XK_udiaeresis,    0x7E) /* U+00FC LATIN SMALL LETTER U WITH DIAERESIS */ \
	}

#define map_NRCS_Portuguese(code) \
	switch (code) { \
	    MAP(0x5B, XK_Atilde)	/* U+00C3 LATIN CAPITAL LETTER A WITH TILDE */ \
	    MAP(0x5C, XK_Ccedilla)	/* U+00C7 LATIN CAPITAL LETTER C WITH CEDILLA */ \
	    MAP(0x5D, XK_Otilde)	/* U+00D5 LATIN CAPITAL LETTER O WITH TILDE */ \
	    MAP(0x7B, XK_atilde)	/* U+00E3 LATIN SMALL LETTER A WITH TILDE */ \
	    MAP(0x7C, XK_ccedilla)	/* U+00E7 LATIN SMALL LETTER C WITH CEDILLA */ \
	    MAP(0x7D, XK_otilde)	/* U+00F5 LATIN SMALL LETTER O WITH TILDE */ \
	}

#define unmap_NRCS_Portuguese(code) \
	switch (code) { \
	    MAP(XK_Atilde,        0x5B) /* U+00C3 LATIN CAPITAL LETTER A WITH TILDE */ \
	    MAP(XK_Ccedilla,      0x5C) /* U+00C7 LATIN CAPITAL LETTER C WITH CEDILLA */ \
	    MAP(XK_Otilde,        0x5D) /* U+00D5 LATIN CAPITAL LETTER O WITH TILDE */ \
	    MAP(XK_atilde,        0x7B) /* U+00E3 LATIN SMALL LETTER A WITH TILDE */ \
	    MAP(XK_ccedilla,      0x7C) /* U+00E7 LATIN SMALL LETTER C WITH CEDILLA */ \
	    MAP(XK_otilde,        0x7D) /* U+00F5 LATIN SMALL LETTER O WITH TILDE */ \
	}

#define map_NRCS_Spanish(code) \
	switch (code) { \
	    MAP(0x23, XK_sterling)	/* U+00A3 POUND SIGN */ \
	    MAP(0x40, XK_section)	/* U+00A7 SECTION SIGN */ \
	    MAP(0x5B, XK_exclamdown)	/* U+00A1 INVERTED EXCLAMATION MARK */ \
	    MAP(0x5C, XK_Ntilde)	/* U+00D1 LATIN CAPITAL LETTER N WITH TILDE */ \
	    MAP(0x5D, XK_questiondown)	/* U+00BF INVERTED QUESTION MARK */ \
	    MAP(0x7B, XK_degree)	/* U+00B0 DEGREE SIGN */ \
	    MAP(0x7C, XK_ntilde)	/* U+00F1 LATIN SMALL LETTER N WITH TILDE */ \
	    MAP(0x7D, XK_ccedilla)	/* U+00E7 LATIN SMALL LETTER C WITH CEDILLA */ \
	}

#define unmap_NRCS_Spanish(code) \
	switch (code) { \
	    MAP(XK_Ntilde,        0x5C) /* U+00D1 LATIN CAPITAL LETTER N WITH TILDE */ \
	    MAP(XK_ccedilla,      0x7D) /* U+00E7 LATIN SMALL LETTER C WITH CEDILLA */ \
	    MAP(XK_degree,        0x7B) /* U+00B0 DEGREE SIGN */ \
	    MAP(XK_exclamdown,    0x5B) /* U+00A1 INVERTED EXCLAMATION MARK */ \
	    MAP(XK_ntilde,        0x7C) /* U+00F1 LATIN SMALL LETTER N WITH TILDE */ \
	    MAP(XK_questiondown,  0x5D) /* U+00BF INVERTED QUESTION MARK */ \
	    MAP(XK_section,       0x40) /* U+00A7 SECTION SIGN */ \
	    MAP(XK_sterling,      0x23) /* U+00A3 POUND SIGN */ \
	}

#define map_NRCS_Swedish(code) \
	switch (code) { \
	    MAP(0x40, XK_Eacute) \
	    MAP(0x5B, XK_Adiaeresis)	/* U+00C4 LATIN CAPITAL LETTER A WITH DIAERESIS */ \
	    MAP(0x5C, XK_Odiaeresis)	/* U+00D6 LATIN CAPITAL LETTER O WITH DIAERESIS */ \
	    MAP(0x5D, XK_Aring)	/* U+00C5 LATIN CAPITAL LETTER A WITH RING ABOVE */ \
	    MAP(0x5E, XK_Udiaeresis)	/* U+00DC LATIN CAPITAL LETTER U WITH DIAERESIS */ \
	    MAP(0x60, XK_eacute)	/* U+00E9 LATIN SMALL LETTER E WITH ACUTE */ \
	    MAP(0x7B, XK_adiaeresis)	/* U+00E4 LATIN SMALL LETTER A WITH DIAERESIS */ \
	    MAP(0x7C, XK_odiaeresis)	/* U+00F6 LATIN SMALL LETTER O WITH DIAERESIS */ \
	    MAP(0x7D, XK_aring)	/* U+00E5 LATIN SMALL LETTER A WITH RING ABOVE */ \
	    MAP(0x7E, XK_udiaeresis)	/* U+00FC LATIN SMALL LETTER U WITH DIAERESIS */ \
	}

#define unmap_NRCS_Swedish(code) \
	switch (code) { \
	    MAP(XK_Adiaeresis,    0x5B) /* U+00C4 LATIN CAPITAL LETTER A WITH DIAERESIS */ \
	    MAP(XK_Aring,         0x5D) /* U+00C5 LATIN CAPITAL LETTER A WITH RING ABOVE */ \
	    MAP(XK_Eacute,        0x40) \
	    MAP(XK_Odiaeresis,    0x5C) /* U+00D6 LATIN CAPITAL LETTER O WITH DIAERESIS */ \
	    MAP(XK_Udiaeresis,    0x5E) /* U+00DC LATIN CAPITAL LETTER U WITH DIAERESIS */ \
	    MAP(XK_adiaeresis,    0x7B) /* U+00E4 LATIN SMALL LETTER A WITH DIAERESIS */ \
	    MAP(XK_aring,         0x7D) /* U+00E5 LATIN SMALL LETTER A WITH RING ABOVE */ \
	    MAP(XK_eacute,        0x60) /* U+00E9 LATIN SMALL LETTER E WITH ACUTE */ \
	    MAP(XK_odiaeresis,    0x7C) /* U+00F6 LATIN SMALL LETTER O WITH DIAERESIS */ \
	    MAP(XK_udiaeresis,    0x7E) /* U+00FC LATIN SMALL LETTER U WITH DIAERESIS */ \
	}

#define map_NRCS_Swiss(code) \
	switch (code) { \
	    MAP(0x23, XK_ugrave)	/* U+00F9 LATIN SMALL LETTER U WITH GRAVE */ \
	    MAP(0x40, XK_agrave)	/* U+00E0 LATIN SMALL LETTER A WITH GRAVE */ \
	    MAP(0x5B, XK_eacute)	/* U+00E9 LATIN SMALL LETTER E WITH ACUTE */ \
	    MAP(0x5C, XK_ccedilla)	/* U+00E7 LATIN SMALL LETTER C WITH CEDILLA */ \
	    MAP(0x5D, XK_ecircumflex)	/* U+00EA LATIN SMALL LETTER E WITH CIRCUMFLEX */ \
	    MAP(0x5E, XK_icircumflex)	/* U+00EE LATIN SMALL LETTER I WITH CIRCUMFLEX */ \
	    MAP(0x5F, XK_egrave)	/* U+00E8 LATIN SMALL LETTER E WITH GRAVE */ \
	    MAP(0x60, XK_ocircumflex)	/* U+00F4 LATIN SMALL LETTER O WITH CIRCUMFLEX */ \
	    MAP(0x7B, XK_adiaeresis)	/* U+00E4 LATIN SMALL LETTER A WITH DIAERESIS */ \
	    MAP(0x7C, XK_odiaeresis)	/* U+00F6 LATIN SMALL LETTER O WITH DIAERESIS */ \
	    MAP(0x7D, XK_udiaeresis)	/* U+00FC LATIN SMALL LETTER U WITH DIAERESIS */ \
	    MAP(0x7E, XK_ucircumflex)	/* U+00FB LATIN SMALL LETTER U WITH CIRCUMFLEX */ \
	}

#define unmap_NRCS_Swiss(code) \
	switch (code) { \
	    MAP(XK_adiaeresis,    0x7B) /* U+00E4 LATIN SMALL LETTER A WITH DIAERESIS */ \
	    MAP(XK_agrave,        0x40) /* U+00E0 LATIN SMALL LETTER A WITH GRAVE */ \
	    MAP(XK_ccedilla,      0x5C) /* U+00E7 LATIN SMALL LETTER C WITH CEDILLA */ \
	    MAP(XK_eacute,        0x5B) /* U+00E9 LATIN SMALL LETTER E WITH ACUTE */ \
	    MAP(XK_ecircumflex,   0x5D) /* U+00EA LATIN SMALL LETTER E WITH CIRCUMFLEX */ \
	    MAP(XK_egrave,        0x5F) /* U+00E8 LATIN SMALL LETTER E WITH GRAVE */ \
	    MAP(XK_icircumflex,   0x5E) /* U+00EE LATIN SMALL LETTER I WITH CIRCUMFLEX */ \
	    MAP(XK_ocircumflex,   0x60) /* U+00F4 LATIN SMALL LETTER O WITH CIRCUMFLEX */ \
	    MAP(XK_odiaeresis,    0x7C) /* U+00F6 LATIN SMALL LETTER O WITH DIAERESIS */ \
	    MAP(XK_ucircumflex,   0x7E) /* U+00FB LATIN SMALL LETTER U WITH CIRCUMFLEX */ \
	    MAP(XK_udiaeresis,    0x7D) /* U+00FC LATIN SMALL LETTER U WITH DIAERESIS */ \
	    MAP(XK_ugrave,        0x23) /* U+00F9 LATIN SMALL LETTER U WITH GRAVE */ \
	}

/*
 * Unlike NRCS, which splices a few characters onto ISO-8859-1, the
 * supplementary character sets are complete, normally mapped to GR.  Most of
 * these mappings rely upon glyphs not found in ISO-8859-1.  We can display most
 * of those using Unicode, thereby supporting specialized applications that use
 * SCS with luit, subject to the limitation that select/paste will give
 * meaningless results in terms of the application which uses these mappings.
 *
 * Since the codepages introduced with VT320, etc, use 8-bit encodings, there is
 * no plausible argument to be made that these mappings "use" UTF-8, even though
 * there is a hidden step in the terminal emulator which relies upon UTF-8.
 */

#define map_DEC_Supp_Graphic(code,dft) \
	begin_CODEPAGE(94) \
	switch (code) { \
	    XXX(0x24, UNDEF) \
	    XXX(0x26, UNDEF) \
	    UNI(0x28, 0x00A4)	/* CURRENCY SIGN */ \
	    XXX(0x2C, UNDEF) \
	    XXX(0x2D, UNDEF) \
	    XXX(0x2E, UNDEF) \
	    XXX(0x2F, UNDEF) \
	    XXX(0x34, UNDEF) \
	    XXX(0x38, UNDEF) \
	    XXX(0x3E, UNDEF) \
	    XXX(0x50, UNDEF) \
	    UNI(0x57, 0x0152)	/* LATIN CAPITAL LIGATURE OE */ \
	    UNI(0x5D, 0x0178)	/* LATIN CAPITAL LETTER Y WITH DIAERESIS */ \
	    XXX(0x5E, UNDEF) \
	    XXX(0x70, UNDEF) \
	    UNI(0x77, 0x0153)	/* LATIN SMALL LIGATURE OE */ \
	    UNI(0x7D, 0x00FF)	/* LATIN SMALL LETTER Y WITH DIAERESIS */ \
	    XXX(0x7E, UNDEF) \
	    XXX(0x7F, UNDEF) \
	    default: dft; break; \
	} \
	end_CODEPAGE()

#define unmap_DEC_Supp_Graphic(code,dft)(code) \
	switch (code) { \
	    MAP(0x00A4, 0x28)	/* CURRENCY SIGN */ \
	    MAP(0x00FF, 0x7D)	/* LATIN SMALL LETTER Y WITH DIAERESIS */ \
	    MAP(0x0152, 0x57)	/* LATIN CAPITAL LIGATURE OE */ \
	    MAP(0x0153, 0x77)	/* LATIN SMALL LIGATURE OE */ \
	    MAP(0x0178, 0x5D)	/* LATIN CAPITAL LETTER Y WITH DIAERESIS */ \
	    MAP(UNDEF, 0x1B) \
	}

#if OPT_WIDE_CHARS

/*
 * derived from http://www.vt100.net/charsets/technical.html
 */
#define map_DEC_Technical(code) \
	begin_CODEPAGE(94) \
	switch (code) { \
	    UNI(0x21, 0x23B7)	/* RADICAL SYMBOL BOTTOM Centred left to right, so that it joins up with 02/02 */ \
	    UNI(0x22, 0x250C)	/* BOX DRAWINGS LIGHT DOWN AND RIGHT */ \
	    UNI(0x23, 0x2500)	/* BOX DRAWINGS LIGHT HORIZONTAL */ \
	    UNI(0x24, 0x2320)	/* TOP HALF INTEGRAL with the proviso that the stem is vertical, to join with 02/06 */ \
	    UNI(0x25, 0x2321)	/* BOTTOM HALF INTEGRAL with the proviso above. */ \
	    UNI(0x26, 0x2502)	/* BOX DRAWINGS LIGHT VERTICAL */ \
	    UNI(0x27, 0x23A1)	/* LEFT SQUARE BRACKET UPPER CORNER Joins vertically to 02/06, 02/08. Doesn't join to its right. */ \
	    UNI(0x28, 0x23A3)	/* LEFT SQUARE BRACKET LOWER CORNER Joins vertically to 02/06, 02/07. Doesn't join to its right. */ \
	    UNI(0x29, 0x23A4)	/* RIGHT SQUARE BRACKET UPPER CORNER Joins vertically to 026, 02a. Doesn't join to its left. */ \
	    UNI(0x2A, 0x23A6)	/* RIGHT SQUARE BRACKET LOWER CORNER Joins vertically to 026, 029. Doesn't join to its left. */ \
	    UNI(0x2B, 0x23A7)	/* LEFT CURLY BRACKET UPPER HOOK Joins vertically to 026, 02c, 02/15. Doesn't join to its right. */ \
	    UNI(0x2C, 0x23A9)	/* LEFT CURLY BRACKET LOWER HOOK Joins vertically to 026, 02b, 02/15. Doesn't join to its right. */ \
	    UNI(0x2D, 0x23AB)	/* RIGHT CURLY BRACKET UPPER HOOK Joins vertically to 026, 02e, 03/00. Doesn't join to its left. */ \
	    UNI(0x2E, 0x23AD)	/* RIGHT CURLY BRACKET LOWER HOOK Joins vertically to 026, 02d, 03/00. Doesn't join to its left. */ \
	    UNI(0x2F, 0x23A8)	/* LEFT CURLY BRACKET MIDDLE PIECE Joins vertically to 026, 02b, 02c. */ \
	    UNI(0x30, 0x23AC)	/* RIGHT CURLY BRACKET MIDDLE PIECE Joins vertically to 02/06, 02d, 02e. */ \
	    XXX(0x31, PUA(0))	/* Top Left Sigma. Joins to right with 02/03, 03/05. Joins diagonally below right with 03/03, 03/07. */ \
	    XXX(0x32, PUA(1))	/* Bottom Left Sigma. Joins to right with 02/03, 03/06. Joins diagonally above right with 03/04, 03/07. */ \
	    XXX(0x33, PUA(2))	/* Top Diagonal Sigma. Line for joining 03/01 to 03/04 or 03/07. */ \
	    XXX(0x34, PUA(3))	/* Bottom Diagonal Sigma. Line for joining 03/02 to 03/03 or 03/07. */ \
	    XXX(0x35, PUA(4))	/* Top Right Sigma. Joins to left with 02/03, 03/01. */ \
	    XXX(0x36, PUA(5))	/* Bottom Right Sigma. Joins to left with 02/03, 03/02. */ \
	    XXX(0x37, PUA(6))	/* Middle Sigma. Joins diagonally with 03/01, 03/02, 03/03, 03/04. */ \
	    XXX(0x38, UNDEF)	/* undefined */ \
	    XXX(0x39, UNDEF)	/* undefined */ \
	    XXX(0x3A, UNDEF)	/* undefined */ \
	    XXX(0x3B, UNDEF)	/* undefined */ \
	    UNI(0x3C, 0x2264)	/* LESS-THAN OR EQUAL TO */ \
	    UNI(0x3D, 0x2260)	/* NOT EQUAL TO */ \
	    UNI(0x3E, 0x2265)	/* GREATER-THAN OR EQUAL TO */ \
	    UNI(0x3F, 0x222B)	/* INTEGRAL */ \
	    UNI(0x40, 0x2234)	/* THEREFORE */ \
	    UNI(0x41, 0x221D)	/* PROPORTIONAL TO */ \
	    UNI(0x42, 0x221E)	/* INFINITY */ \
	    UNI(0x43, 0x00F7)	/* DIVISION SIGN */ \
	    UNI(0x44, 0x0394)	/* GREEK CAPITAL DELTA */ \
	    UNI(0x45, 0x2207)	/* NABLA */ \
	    UNI(0x46, 0x03A6)	/* GREEK CAPITAL LETTER PHI */ \
	    UNI(0x47, 0x0393)	/* GREEK CAPITAL LETTER GAMMA */ \
	    UNI(0x48, 0x223C)	/* TILDE OPERATOR */ \
	    UNI(0x49, 0x2243)	/* ASYMPTOTICALLY EQUAL TO */ \
	    UNI(0x4A, 0x0398)	/* GREEK CAPITAL LETTER THETA */ \
	    UNI(0x4B, 0x00D7)	/* MULTIPLICATION SIGN */ \
	    UNI(0x4C, 0x039B)	/* GREEK CAPITAL LETTER LAMDA */ \
	    UNI(0x4D, 0x21D4)	/* LEFT RIGHT DOUBLE ARROW */ \
	    UNI(0x4E, 0x21D2)	/* RIGHTWARDS DOUBLE ARROW */ \
	    UNI(0x4F, 0x2261)	/* IDENTICAL TO */ \
	    UNI(0x50, 0x03A0)	/* GREEK CAPITAL LETTER PI */ \
	    UNI(0x51, 0x03A8)	/* GREEK CAPITAL LETTER PSI */ \
	    XXX(0x52, UNDEF)	/* undefined */ \
	    UNI(0x53, 0x03A3)	/* GREEK CAPITAL LETTER SIGMA */ \
	    XXX(0x54, UNDEF)	/* undefined */ \
	    XXX(0x55, UNDEF)	/* undefined */ \
	    UNI(0x56, 0x221A)	/* SQUARE ROOT */ \
	    UNI(0x57, 0x03A9)	/* GREEK CAPITAL LETTER OMEGA */ \
	    UNI(0x58, 0x039E)	/* GREEK CAPITAL LETTER XI */ \
	    UNI(0x59, 0x03A5)	/* GREEK CAPITAL LETTER UPSILON */ \
	    UNI(0x5A, 0x2282)	/* SUBSET OF */ \
	    UNI(0x5B, 0x2283)	/* SUPERSET OF */ \
	    UNI(0x5C, 0x2229)	/* INTERSECTION */ \
	    UNI(0x5D, 0x222A)	/* UNION */ \
	    UNI(0x5E, 0x2227)	/* LOGICAL AND */ \
	    UNI(0x5F, 0x2228)	/* LOGICAL OR */ \
	    UNI(0x60, 0x00AC)	/* NOT SIGN */ \
	    UNI(0x61, 0x03B1)	/* GREEK SMALL LETTER ALPHA */ \
	    UNI(0x62, 0x03B2)	/* GREEK SMALL LETTER BETA */ \
	    UNI(0x63, 0x03C7)	/* GREEK SMALL LETTER CHI */ \
	    UNI(0x64, 0x03B4)	/* GREEK SMALL LETTER DELTA */ \
	    UNI(0x65, 0x03B5)	/* GREEK SMALL LETTER EPSILON */ \
	    UNI(0x66, 0x03C6)	/* GREEK SMALL LETTER PHI */ \
	    UNI(0x67, 0x03B3)	/* GREEK SMALL LETTER GAMMA */ \
	    UNI(0x68, 0x03B7)	/* GREEK SMALL LETTER ETA */ \
	    UNI(0x69, 0x03B9)	/* GREEK SMALL LETTER IOTA */ \
	    UNI(0x6A, 0x03B8)	/* GREEK SMALL LETTER THETA */ \
	    UNI(0x6B, 0x03BA)	/* GREEK SMALL LETTER KAPPA */ \
	    UNI(0x6C, 0x03BB)	/* GREEK SMALL LETTER LAMDA */ \
	    XXX(0x6D, UNDEF)	/* undefined */ \
	    UNI(0x6E, 0x03BD)	/* GREEK SMALL LETTER NU */ \
	    UNI(0x6F, 0x2202)	/* PARTIAL DIFFERENTIAL */ \
	    UNI(0x70, 0x03C0)	/* GREEK SMALL LETTER PI */ \
	    UNI(0x71, 0x03C8)	/* GREEK SMALL LETTER PSI */ \
	    UNI(0x72, 0x03C1)	/* GREEK SMALL LETTER RHO */ \
	    UNI(0x73, 0x03C3)	/* GREEK SMALL LETTER SIGMA */ \
	    UNI(0x74, 0x03C4)	/* GREEK SMALL LETTER TAU */ \
	    XXX(0x75, UNDEF)	/* undefined */ \
	    UNI(0x76, 0x0192)	/* LATIN SMALL LETTER F WITH HOOK Probably chosen for its meaning of "function" */ \
	    UNI(0x77, 0x03C9)	/* GREEK SMALL LETTER OMEGA */ \
	    UNI(0x78, 0x03BE)	/* GREEK SMALL LETTER XI */ \
	    UNI(0x79, 0x03C5)	/* GREEK SMALL LETTER UPSILON */ \
	    UNI(0x7A, 0x03B6)	/* GREEK SMALL LETTER ZETA */ \
	    UNI(0x7B, 0x2190)	/* LEFTWARDS ARROW */ \
	    UNI(0x7C, 0x2191)	/* UPWARDS ARROW */ \
	    UNI(0x7D, 0x2192)	/* RIGHTWARDS ARROW */ \
	    UNI(0x7E, 0x2193)	/* DOWNWARDS ARROW */ \
	} \
	end_CODEPAGE()

#define unmap_DEC_Technical(code) \
	switch (code) { \
	    MAP(0x00AC, 0xAC)	/* NOT SIGN */ \
	    MAP(0x00D7, 0xD7)	/* MULTIPLICATION SIGN */ \
	    MAP(0x00F7, 0xF7)	/* DIVISION SIGN */ \
	    MAP(0x0192, 0xFD)	/* LATIN SMALL LETTER F WITH HOOK Probably chosen for its meaning of "function" */ \
	    MAP(0x0393, 0x78)	/* GREEK CAPITAL LETTER GAMMA */ \
	    MAP(0x0394, 0xEE)	/* GREEK CAPITAL DELTA */ \
	    MAP(0x0398, 0x7D)	/* GREEK CAPITAL LETTER THETA */ \
	    MAP(0x039B, 0xA2)	/* GREEK CAPITAL LETTER LAMDA */ \
	    MAP(0x039E, 0xA5)	/* GREEK CAPITAL LETTER XI */ \
	    MAP(0x03A0, 0xA7)	/* GREEK CAPITAL LETTER PI */ \
	    MAP(0x03A3, 0xA9)	/* GREEK CAPITAL LETTER SIGMA */ \
	    MAP(0x03A5, 0xAB)	/* GREEK CAPITAL LETTER UPSILON */ \
	    MAP(0x03A6, 0xAC)	/* GREEK CAPITAL LETTER PHI */ \
	    MAP(0x03A8, 0xAE)	/* GREEK CAPITAL LETTER PSI */ \
	    MAP(0x03A9, 0xAF)	/* GREEK CAPITAL LETTER OMEGA */ \
	    MAP(0x03B1, 0xB7)	/* GREEK SMALL LETTER ALPHA */ \
	    MAP(0x03B2, 0xB8)	/* GREEK SMALL LETTER BETA */ \
	    MAP(0x03B3, 0xB9)	/* GREEK SMALL LETTER GAMMA */ \
	    MAP(0x03B4, 0xBA)	/* GREEK SMALL LETTER DELTA */ \
	    MAP(0x03B5, 0xBB)	/* GREEK SMALL LETTER EPSILON */ \
	    MAP(0x03B6, 0xBC)	/* GREEK SMALL LETTER ZETA */ \
	    MAP(0x03B7, 0xBD)	/* GREEK SMALL LETTER ETA */ \
	    MAP(0x03B8, 0xBE)	/* GREEK SMALL LETTER THETA */ \
	    MAP(0x03B9, 0xBF)	/* GREEK SMALL LETTER IOTA */ \
	    MAP(0x03BA, 0xC0)	/* GREEK SMALL LETTER KAPPA */ \
	    MAP(0x03BB, 0xC1)	/* GREEK SMALL LETTER LAMDA */ \
	    MAP(0x03BD, 0xC3)	/* GREEK SMALL LETTER NU */ \
	    MAP(0x03BE, 0xC4)	/* GREEK SMALL LETTER XI */ \
	    MAP(0x03C0, 0xC6)	/* GREEK SMALL LETTER PI */ \
	    MAP(0x03C1, 0xC7)	/* GREEK SMALL LETTER RHO */ \
	    MAP(0x03C3, 0xC9)	/* GREEK SMALL LETTER SIGMA */ \
	    MAP(0x03C4, 0xCA)	/* GREEK SMALL LETTER TAU */ \
	    MAP(0x03C5, 0xCB)	/* GREEK SMALL LETTER UPSILON */ \
	    MAP(0x03C6, 0xCC)	/* GREEK SMALL LETTER PHI */ \
	    MAP(0x03C7, 0xCD)	/* GREEK SMALL LETTER CHI */ \
	    MAP(0x03C8, 0xCE)	/* GREEK SMALL LETTER PSI */ \
	    MAP(0x03C9, 0xCF)	/* GREEK SMALL LETTER OMEGA */ \
	    MAP(0x2190, 0xFE)	/* LEFTWARDS ARROW */ \
	    MAP(0x2191, 0xFF)	/* UPWARDS ARROW */ \
	    MAP(0x2192, 0x100)	/* RIGHTWARDS ARROW */ \
	    MAP(0x2193, 0x02)	/* DOWNWARDS ARROW */ \
	    MAP(0x21D2, 0xF3)	/* RIGHTWARDS DOUBLE ARROW */ \
	    MAP(0x21D4, 0xF2)	/* LEFT RIGHT DOUBLE ARROW */ \
	    MAP(0x2202, 0xFC)	/* PARTIAL DIFFERENTIAL */ \
	    MAP(0x2207, 0xEF)	/* NABLA */ \
	    MAP(0x221A, 0xF5)	/* SQUARE ROOT */ \
	    MAP(0x221D, 0xEC)	/* PROPORTIONAL TO */ \
	    MAP(0x221E, 0xED)	/* INFINITY */ \
	    MAP(0x2227, 0xFA)	/* LOGICAL AND */ \
	    MAP(0x2228, 0xFB)	/* LOGICAL OR */ \
	    MAP(0x2229, 0xF8)	/* INTERSECTION */ \
	    MAP(0x222A, 0xF9)	/* UNION */ \
	    MAP(0x222B, 0xEA)	/* INTEGRAL */ \
	    MAP(0x2234, 0xEB)	/* THEREFORE */ \
	    MAP(0x223C, 0xF0)	/* TILDE OPERATOR */ \
	    MAP(0x2243, 0xF1)	/* ASYMPTOTICALLY EQUAL TO */ \
	    MAP(0x2260, 0x87)	/* NOT EQUAL TO */ \
	    MAP(0x2261, 0xF4)	/* IDENTICAL TO */ \
	    MAP(0x2264, 0x85)	/* LESS-THAN OR EQUAL TO */ \
	    MAP(0x2265, 0x86)	/* GREATER-THAN OR EQUAL TO */ \
	    MAP(0x2282, 0xF6)	/* SUBSET OF */ \
	    MAP(0x2283, 0xF7)	/* SUPERSET OF */ \
	    MAP(0x2320, 0xD7)	/* TOP HALF INTEGRAL with the proviso that the stem is vertical, to join with 02/06 */ \
	    MAP(0x2321, 0xD8)	/* BOTTOM HALF INTEGRAL with the proviso above. */ \
	    MAP(0x23A1, 0xD9)	/* LEFT SQUARE BRACKET UPPER CORNER Joins vertically to 02/06, 02/08. Doesn't join to its right. */ \
	    MAP(0x23A3, 0xDA)	/* LEFT SQUARE BRACKET LOWER CORNER Joins vertically to 02/06, 02/07. Doesn't join to its right. */ \
	    MAP(0x23A4, 0xDB)	/* RIGHT SQUARE BRACKET UPPER CORNER Joins vertically to 026, 02a. Doesn't join to its left. */ \
	    MAP(0x23A6, 0xDC)	/* RIGHT SQUARE BRACKET LOWER CORNER Joins vertically to 026, 029. Doesn't join to its left. */ \
	    MAP(0x23A7, 0xDD)	/* LEFT CURLY BRACKET UPPER HOOK Joins vertically to 026, 02c, 02/15. Doesn't join to its right. */ \
	    MAP(0x23A8, 0xE1)	/* LEFT CURLY BRACKET MIDDLE PIECE Joins vertically to 026, 02b, 02c. */ \
	    MAP(0x23A9, 0xDE)	/* LEFT CURLY BRACKET LOWER HOOK Joins vertically to 026, 02b, 02/15. Doesn't join to its right. */ \
	    MAP(0x23AB, 0xDF)	/* RIGHT CURLY BRACKET UPPER HOOK Joins vertically to 026, 02e, 03/00. Doesn't join to its left. */ \
	    MAP(0x23AC, 0xE2)	/* RIGHT CURLY BRACKET MIDDLE PIECE Joins vertically to 02/06, 02d, 02e. */ \
	    MAP(0x23AD, 0xE0)	/* RIGHT CURLY BRACKET LOWER HOOK Joins vertically to 026, 02d, 03/00. Doesn't join to its left. */ \
	    MAP(0x23B7, 0xD5)	/* RADICAL SYMBOL BOTTOM Centred left to right, so that it joins up with 02/02 */ \
	    MAP(0x2500, 0x1D)	/* BOX DRAWINGS LIGHT HORIZONTAL */ \
	    MAP(0x2502, 0x84)	/* BOX DRAWINGS LIGHT VERTICAL */ \
	    MAP(0x250C, 0xD6)	/* BOX DRAWINGS LIGHT DOWN AND RIGHT */ \
	    MAP(PUA(0), 0xE3)	/* Top Left Sigma. Joins to right with 02/03, 03/05. Joins diagonally below right with 03/03, 03/07. */ \
	    MAP(PUA(1), 0xE4)	/* Bottom Left Sigma. Joins to right with 02/03, 03/06. Joins diagonally above right with 03/04, 03/07. */ \
	    MAP(PUA(2), 0xE5)	/* Top Diagonal Sigma. Line for joining 03/01 to 03/04 or 03/07. */ \
	    MAP(PUA(3), 0xE6)	/* Bottom Diagonal Sigma. Line for joining 03/02 to 03/03 or 03/07. */ \
	    MAP(PUA(4), 0xE7)	/* Top Right Sigma. Joins to left with 02/03, 03/01. */ \
	    MAP(PUA(5), 0xE8)	/* Bottom Right Sigma. Joins to left with 02/03, 03/02. */ \
	    MAP(PUA(6), 0xE9)	/* Middle Sigma. Joins diagonally with 03/01, 03/02, 03/03, 03/04. */ \
	    MAP(UNDEF, 0x1B)	/* undefined */ \
	}

/*
 * ISO Latin/Cyrillic is 8859-5
 */
#define map_ISO_Latin_Cyrillic(code) \
	begin_CODEPAGE(96) \
	switch (code) { \
	    UNI(0x20, 0x00A0)	/* NO-BREAK SPACE */ \
	    UNI(0x21, 0x0401)	/* CYRILLIC CAPITAL LETTER IO */ \
	    UNI(0x22, 0x0402)	/* CYRILLIC CAPITAL LETTER DJE */ \
	    UNI(0x23, 0x0403)	/* CYRILLIC CAPITAL LETTER GJE */ \
	    UNI(0x24, 0x0404)	/* CYRILLIC CAPITAL LETTER UKRAINIAN IE */ \
	    UNI(0x25, 0x0405)	/* CYRILLIC CAPITAL LETTER DZE */ \
	    UNI(0x26, 0x0406)	/* CYRILLIC CAPITAL LETTER BYELORUSSIAN-UKRAINIAN I */ \
	    UNI(0x27, 0x0407)	/* CYRILLIC CAPITAL LETTER YI */ \
	    UNI(0x28, 0x0408)	/* CYRILLIC CAPITAL LETTER JE */ \
	    UNI(0x29, 0x0409)	/* CYRILLIC CAPITAL LETTER LJE */ \
	    UNI(0x2A, 0x040A)	/* CYRILLIC CAPITAL LETTER NJE */ \
	    UNI(0x2B, 0x040B)	/* CYRILLIC CAPITAL LETTER TSHE */ \
	    UNI(0x2C, 0x040C)	/* CYRILLIC CAPITAL LETTER KJE */ \
	    UNI(0x2D, 0x00AD)	/* SOFT HYPHEN */ \
	    UNI(0x2E, 0x040E)	/* CYRILLIC CAPITAL LETTER SHORT U */ \
	    UNI(0x2F, 0x040F)	/* CYRILLIC CAPITAL LETTER DZHE */ \
	    UNI(0x30, 0x0410)	/* CYRILLIC CAPITAL LETTER A */ \
	    UNI(0x31, 0x0411)	/* CYRILLIC CAPITAL LETTER BE */ \
	    UNI(0x32, 0x0412)	/* CYRILLIC CAPITAL LETTER VE */ \
	    UNI(0x33, 0x0413)	/* CYRILLIC CAPITAL LETTER GHE */ \
	    UNI(0x34, 0x0414)	/* CYRILLIC CAPITAL LETTER DE */ \
	    UNI(0x35, 0x0415)	/* CYRILLIC CAPITAL LETTER IE */ \
	    UNI(0x36, 0x0416)	/* CYRILLIC CAPITAL LETTER ZHE */ \
	    UNI(0x37, 0x0417)	/* CYRILLIC CAPITAL LETTER ZE */ \
	    UNI(0x38, 0x0418)	/* CYRILLIC CAPITAL LETTER I */ \
	    UNI(0x39, 0x0419)	/* CYRILLIC CAPITAL LETTER SHORT I */ \
	    UNI(0x3A, 0x041A)	/* CYRILLIC CAPITAL LETTER KA */ \
	    UNI(0x3B, 0x041B)	/* CYRILLIC CAPITAL LETTER EL */ \
	    UNI(0x3C, 0x041C)	/* CYRILLIC CAPITAL LETTER EM */ \
	    UNI(0x3D, 0x041D)	/* CYRILLIC CAPITAL LETTER EN */ \
	    UNI(0x3E, 0x041E)	/* CYRILLIC CAPITAL LETTER O */ \
	    UNI(0x3F, 0x041F)	/* CYRILLIC CAPITAL LETTER PE */ \
	    UNI(0x40, 0x0420)	/* CYRILLIC CAPITAL LETTER ER */ \
	    UNI(0x41, 0x0421)	/* CYRILLIC CAPITAL LETTER ES */ \
	    UNI(0x42, 0x0422)	/* CYRILLIC CAPITAL LETTER TE */ \
	    UNI(0x43, 0x0423)	/* CYRILLIC CAPITAL LETTER U */ \
	    UNI(0x44, 0x0424)	/* CYRILLIC CAPITAL LETTER EF */ \
	    UNI(0x45, 0x0425)	/* CYRILLIC CAPITAL LETTER HA */ \
	    UNI(0x46, 0x0426)	/* CYRILLIC CAPITAL LETTER TSE */ \
	    UNI(0x47, 0x0427)	/* CYRILLIC CAPITAL LETTER CHE */ \
	    UNI(0x48, 0x0428)	/* CYRILLIC CAPITAL LETTER SHA */ \
	    UNI(0x49, 0x0429)	/* CYRILLIC CAPITAL LETTER SHCHA */ \
	    UNI(0x4A, 0x042A)	/* CYRILLIC CAPITAL LETTER HARD SIGN */ \
	    UNI(0x4B, 0x042B)	/* CYRILLIC CAPITAL LETTER YERU */ \
	    UNI(0x4C, 0x042C)	/* CYRILLIC CAPITAL LETTER SOFT SIGN */ \
	    UNI(0x4D, 0x042D)	/* CYRILLIC CAPITAL LETTER E */ \
	    UNI(0x4E, 0x042E)	/* CYRILLIC CAPITAL LETTER YU */ \
	    UNI(0x4F, 0x042F)	/* CYRILLIC CAPITAL LETTER YA */ \
	    UNI(0x50, 0x0430)	/* CYRILLIC SMALL LETTER A */ \
	    UNI(0x51, 0x0431)	/* CYRILLIC SMALL LETTER BE */ \
	    UNI(0x52, 0x0432)	/* CYRILLIC SMALL LETTER VE */ \
	    UNI(0x53, 0x0433)	/* CYRILLIC SMALL LETTER GHE */ \
	    UNI(0x54, 0x0434)	/* CYRILLIC SMALL LETTER DE */ \
	    UNI(0x55, 0x0435)	/* CYRILLIC SMALL LETTER IE */ \
	    UNI(0x56, 0x0436)	/* CYRILLIC SMALL LETTER ZHE */ \
	    UNI(0x57, 0x0437)	/* CYRILLIC SMALL LETTER ZE */ \
	    UNI(0x58, 0x0438)	/* CYRILLIC SMALL LETTER I */ \
	    UNI(0x59, 0x0439)	/* CYRILLIC SMALL LETTER SHORT I */ \
	    UNI(0x5A, 0x043A)	/* CYRILLIC SMALL LETTER KA */ \
	    UNI(0x5B, 0x043B)	/* CYRILLIC SMALL LETTER EL */ \
	    UNI(0x5C, 0x043C)	/* CYRILLIC SMALL LETTER EM */ \
	    UNI(0x5D, 0x043D)	/* CYRILLIC SMALL LETTER EN */ \
	    UNI(0x5E, 0x043E)	/* CYRILLIC SMALL LETTER O */ \
	    UNI(0x5F, 0x043F)	/* CYRILLIC SMALL LETTER PE */ \
	    UNI(0x60, 0x0440)	/* CYRILLIC SMALL LETTER ER */ \
	    UNI(0x61, 0x0441)	/* CYRILLIC SMALL LETTER ES */ \
	    UNI(0x62, 0x0442)	/* CYRILLIC SMALL LETTER TE */ \
	    UNI(0x63, 0x0443)	/* CYRILLIC SMALL LETTER U */ \
	    UNI(0x64, 0x0444)	/* CYRILLIC SMALL LETTER EF */ \
	    UNI(0x65, 0x0445)	/* CYRILLIC SMALL LETTER HA */ \
	    UNI(0x66, 0x0446)	/* CYRILLIC SMALL LETTER TSE */ \
	    UNI(0x67, 0x0447)	/* CYRILLIC SMALL LETTER CHE */ \
	    UNI(0x68, 0x0448)	/* CYRILLIC SMALL LETTER SHA */ \
	    UNI(0x69, 0x0449)	/* CYRILLIC SMALL LETTER SHCHA */ \
	    UNI(0x6A, 0x044A)	/* CYRILLIC SMALL LETTER HARD SIGN */ \
	    UNI(0x6B, 0x044B)	/* CYRILLIC SMALL LETTER YERU */ \
	    UNI(0x6C, 0x044C)	/* CYRILLIC SMALL LETTER SOFT SIGN */ \
	    UNI(0x6D, 0x044D)	/* CYRILLIC SMALL LETTER E */ \
	    UNI(0x6E, 0x044E)	/* CYRILLIC SMALL LETTER YU */ \
	    UNI(0x6F, 0x044F)	/* CYRILLIC SMALL LETTER YA */ \
	    UNI(0x70, 0x2116)	/* NUMERO SIGN */ \
	    UNI(0x71, 0x0451)	/* CYRILLIC SMALL LETTER IO */ \
	    UNI(0x72, 0x0452)	/* CYRILLIC SMALL LETTER DJE */ \
	    UNI(0x73, 0x0453)	/* CYRILLIC SMALL LETTER GJE */ \
	    UNI(0x74, 0x0454)	/* CYRILLIC SMALL LETTER UKRAINIAN IE */ \
	    UNI(0x75, 0x0455)	/* CYRILLIC SMALL LETTER DZE */ \
	    UNI(0x76, 0x0456)	/* CYRILLIC SMALL LETTER BYELORUSSIAN-UKRAINIAN I */ \
	    UNI(0x77, 0x0457)	/* CYRILLIC SMALL LETTER YI */ \
	    UNI(0x78, 0x0458)	/* CYRILLIC SMALL LETTER JE */ \
	    UNI(0x79, 0x0459)	/* CYRILLIC SMALL LETTER LJE */ \
	    UNI(0x7A, 0x045A)	/* CYRILLIC SMALL LETTER NJE */ \
	    UNI(0x7B, 0x045B)	/* CYRILLIC SMALL LETTER TSHE */ \
	    UNI(0x7C, 0x045C)	/* CYRILLIC SMALL LETTER KJE */ \
	    UNI(0x7D, 0x00A7)	/* SECTION SIGN */ \
	    UNI(0x7E, 0x045E)	/* CYRILLIC SMALL LETTER SHORT U */ \
	    UNI(0x7F, 0x045F)	/* CYRILLIC SMALL LETTER DZHE */ \
	} \
	end_CODEPAGE()

#define unmap_ISO_Latin_Cyrillic(code) \
	switch (code) { \
	    MAP(0x00A0, 0x20)	/* NO-BREAK SPACE */ \
	    MAP(0x00A7, 0x7D)	/* SECTION SIGN */ \
	    MAP(0x00AD, 0x2D)	/* SOFT HYPHEN */ \
	    MAP(0x0401, 0x21)	/* CYRILLIC CAPITAL LETTER IO */ \
	    MAP(0x0402, 0x22)	/* CYRILLIC CAPITAL LETTER DJE */ \
	    MAP(0x0403, 0x23)	/* CYRILLIC CAPITAL LETTER GJE */ \
	    MAP(0x0404, 0x24)	/* CYRILLIC CAPITAL LETTER UKRAINIAN IE */ \
	    MAP(0x0405, 0x25)	/* CYRILLIC CAPITAL LETTER DZE */ \
	    MAP(0x0406, 0x26)	/* CYRILLIC CAPITAL LETTER BYELORUSSIAN-UKRAINIAN I */ \
	    MAP(0x0407, 0x27)	/* CYRILLIC CAPITAL LETTER YI */ \
	    MAP(0x0408, 0x28)	/* CYRILLIC CAPITAL LETTER JE */ \
	    MAP(0x0409, 0x29)	/* CYRILLIC CAPITAL LETTER LJE */ \
	    MAP(0x040A, 0x2A)	/* CYRILLIC CAPITAL LETTER NJE */ \
	    MAP(0x040B, 0x2B)	/* CYRILLIC CAPITAL LETTER TSHE */ \
	    MAP(0x040C, 0x2C)	/* CYRILLIC CAPITAL LETTER KJE */ \
	    MAP(0x040E, 0x2E)	/* CYRILLIC CAPITAL LETTER SHORT U */ \
	    MAP(0x040F, 0x2F)	/* CYRILLIC CAPITAL LETTER DZHE */ \
	    MAP(0x0410, 0x30)	/* CYRILLIC CAPITAL LETTER A */ \
	    MAP(0x0411, 0x31)	/* CYRILLIC CAPITAL LETTER BE */ \
	    MAP(0x0412, 0x32)	/* CYRILLIC CAPITAL LETTER VE */ \
	    MAP(0x0413, 0x33)	/* CYRILLIC CAPITAL LETTER GHE */ \
	    MAP(0x0414, 0x34)	/* CYRILLIC CAPITAL LETTER DE */ \
	    MAP(0x0415, 0x35)	/* CYRILLIC CAPITAL LETTER IE */ \
	    MAP(0x0416, 0x36)	/* CYRILLIC CAPITAL LETTER ZHE */ \
	    MAP(0x0417, 0x37)	/* CYRILLIC CAPITAL LETTER ZE */ \
	    MAP(0x0418, 0x38)	/* CYRILLIC CAPITAL LETTER I */ \
	    MAP(0x0419, 0x39)	/* CYRILLIC CAPITAL LETTER SHORT I */ \
	    MAP(0x041A, 0x3A)	/* CYRILLIC CAPITAL LETTER KA */ \
	    MAP(0x041B, 0x3B)	/* CYRILLIC CAPITAL LETTER EL */ \
	    MAP(0x041C, 0x3C)	/* CYRILLIC CAPITAL LETTER EM */ \
	    MAP(0x041D, 0x3D)	/* CYRILLIC CAPITAL LETTER EN */ \
	    MAP(0x041E, 0x3E)	/* CYRILLIC CAPITAL LETTER O */ \
	    MAP(0x041F, 0x3F)	/* CYRILLIC CAPITAL LETTER PE */ \
	    MAP(0x0420, 0x40)	/* CYRILLIC CAPITAL LETTER ER */ \
	    MAP(0x0421, 0x41)	/* CYRILLIC CAPITAL LETTER ES */ \
	    MAP(0x0422, 0x42)	/* CYRILLIC CAPITAL LETTER TE */ \
	    MAP(0x0423, 0x43)	/* CYRILLIC CAPITAL LETTER U */ \
	    MAP(0x0424, 0x44)	/* CYRILLIC CAPITAL LETTER EF */ \
	    MAP(0x0425, 0x45)	/* CYRILLIC CAPITAL LETTER HA */ \
	    MAP(0x0426, 0x46)	/* CYRILLIC CAPITAL LETTER TSE */ \
	    MAP(0x0427, 0x47)	/* CYRILLIC CAPITAL LETTER CHE */ \
	    MAP(0x0428, 0x48)	/* CYRILLIC CAPITAL LETTER SHA */ \
	    MAP(0x0429, 0x49)	/* CYRILLIC CAPITAL LETTER SHCHA */ \
	    MAP(0x042A, 0x4A)	/* CYRILLIC CAPITAL LETTER HARD SIGN */ \
	    MAP(0x042B, 0x4B)	/* CYRILLIC CAPITAL LETTER YERU */ \
	    MAP(0x042C, 0x4C)	/* CYRILLIC CAPITAL LETTER SOFT SIGN */ \
	    MAP(0x042D, 0x4D)	/* CYRILLIC CAPITAL LETTER E */ \
	    MAP(0x042E, 0x4E)	/* CYRILLIC CAPITAL LETTER YU */ \
	    MAP(0x042F, 0x4F)	/* CYRILLIC CAPITAL LETTER YA */ \
	    MAP(0x0430, 0x50)	/* CYRILLIC SMALL LETTER A */ \
	    MAP(0x0431, 0x51)	/* CYRILLIC SMALL LETTER BE */ \
	    MAP(0x0432, 0x52)	/* CYRILLIC SMALL LETTER VE */ \
	    MAP(0x0433, 0x53)	/* CYRILLIC SMALL LETTER GHE */ \
	    MAP(0x0434, 0x54)	/* CYRILLIC SMALL LETTER DE */ \
	    MAP(0x0435, 0x55)	/* CYRILLIC SMALL LETTER IE */ \
	    MAP(0x0436, 0x56)	/* CYRILLIC SMALL LETTER ZHE */ \
	    MAP(0x0437, 0x57)	/* CYRILLIC SMALL LETTER ZE */ \
	    MAP(0x0438, 0x58)	/* CYRILLIC SMALL LETTER I */ \
	    MAP(0x0439, 0x59)	/* CYRILLIC SMALL LETTER SHORT I */ \
	    MAP(0x043A, 0x5A)	/* CYRILLIC SMALL LETTER KA */ \
	    MAP(0x043B, 0x5B)	/* CYRILLIC SMALL LETTER EL */ \
	    MAP(0x043C, 0x5C)	/* CYRILLIC SMALL LETTER EM */ \
	    MAP(0x043D, 0x5D)	/* CYRILLIC SMALL LETTER EN */ \
	    MAP(0x043E, 0x5E)	/* CYRILLIC SMALL LETTER O */ \
	    MAP(0x043F, 0x5F)	/* CYRILLIC SMALL LETTER PE */ \
	    MAP(0x0440, 0x60)	/* CYRILLIC SMALL LETTER ER */ \
	    MAP(0x0441, 0x61)	/* CYRILLIC SMALL LETTER ES */ \
	    MAP(0x0442, 0x62)	/* CYRILLIC SMALL LETTER TE */ \
	    MAP(0x0443, 0x63)	/* CYRILLIC SMALL LETTER U */ \
	    MAP(0x0444, 0x64)	/* CYRILLIC SMALL LETTER EF */ \
	    MAP(0x0445, 0x65)	/* CYRILLIC SMALL LETTER HA */ \
	    MAP(0x0446, 0x66)	/* CYRILLIC SMALL LETTER TSE */ \
	    MAP(0x0447, 0x67)	/* CYRILLIC SMALL LETTER CHE */ \
	    MAP(0x0448, 0x68)	/* CYRILLIC SMALL LETTER SHA */ \
	    MAP(0x0449, 0x69)	/* CYRILLIC SMALL LETTER SHCHA */ \
	    MAP(0x044A, 0x6A)	/* CYRILLIC SMALL LETTER HARD SIGN */ \
	    MAP(0x044B, 0x6B)	/* CYRILLIC SMALL LETTER YERU */ \
	    MAP(0x044C, 0x6C)	/* CYRILLIC SMALL LETTER SOFT SIGN */ \
	    MAP(0x044D, 0x6D)	/* CYRILLIC SMALL LETTER E */ \
	    MAP(0x044E, 0x6E)	/* CYRILLIC SMALL LETTER YU */ \
	    MAP(0x044F, 0x6F)	/* CYRILLIC SMALL LETTER YA */ \
	    MAP(0x0451, 0x71)	/* CYRILLIC SMALL LETTER IO */ \
	    MAP(0x0452, 0x72)	/* CYRILLIC SMALL LETTER DJE */ \
	    MAP(0x0453, 0x73)	/* CYRILLIC SMALL LETTER GJE */ \
	    MAP(0x0454, 0x74)	/* CYRILLIC SMALL LETTER UKRAINIAN IE */ \
	    MAP(0x0455, 0x75)	/* CYRILLIC SMALL LETTER DZE */ \
	    MAP(0x0456, 0x76)	/* CYRILLIC SMALL LETTER BYELORUSSIAN-UKRAINIAN I */ \
	    MAP(0x0457, 0x77)	/* CYRILLIC SMALL LETTER YI */ \
	    MAP(0x0458, 0x78)	/* CYRILLIC SMALL LETTER JE */ \
	    MAP(0x0459, 0x79)	/* CYRILLIC SMALL LETTER LJE */ \
	    MAP(0x045A, 0x7A)	/* CYRILLIC SMALL LETTER NJE */ \
	    MAP(0x045B, 0x7B)	/* CYRILLIC SMALL LETTER TSHE */ \
	    MAP(0x045C, 0x7C)	/* CYRILLIC SMALL LETTER KJE */ \
	    MAP(0x045E, 0x7E)	/* CYRILLIC SMALL LETTER SHORT U */ \
	    MAP(0x045F, 0x7F)	/* CYRILLIC SMALL LETTER DZHE */ \
	    MAP(0x2116, 0x70)	/* NUMERO SIGN */ \
	}

/*
 * ISO Greek is 8859-7
 */
#define map_ISO_Greek_Supp(code) \
	begin_CODEPAGE(96) \
	switch (code) { \
	    UNI(0x20, 0x00A0)	/* NO-BREAK SPACE */ \
	    UNI(0x21, 0x2018)	/* LEFT SINGLE QUOTATION MARK */ \
	    UNI(0x22, 0x2019)	/* RIGHT SINGLE QUOTATION MARK */ \
	    UNI(0x23, 0x00A3)	/* POUND SIGN */ \
	    XXX(0x24, UNDEF)	/* undefined */ \
	    XXX(0x25, UNDEF)	/* undefined */ \
	    UNI(0x26, 0x00A6)	/* BROKEN BAR */ \
	    UNI(0x27, 0x00A7)	/* SECTION SIGN */ \
	    UNI(0x28, 0x00A8)	/* DIAERESIS */ \
	    UNI(0x29, 0x00A9)	/* COPYRIGHT SIGN */ \
	    XXX(0x2A, UNDEF)	/* undefined */ \
	    UNI(0x2B, 0x00AB)	/* LEFT-POINTING DOUBLE ANGLE QUOTATION MARK */ \
	    UNI(0x2C, 0x00AC)	/* NOT SIGN */ \
	    UNI(0x2D, 0x00AD)	/* SOFT HYPHEN */ \
	    XXX(0x2E, UNDEF)	/* undefined */ \
	    UNI(0x2F, 0x2015)	/* HORIZONTAL BAR */ \
	    UNI(0x30, 0x00B0)	/* DEGREE SIGN */ \
	    UNI(0x31, 0x00B1)	/* PLUS-MINUS SIGN */ \
	    UNI(0x32, 0x00B2)	/* SUPERSCRIPT TWO */ \
	    UNI(0x33, 0x00B3)	/* SUPERSCRIPT THREE */ \
	    UNI(0x34, 0x0384)	/* GREEK TONOS */ \
	    UNI(0x35, 0x0385)	/* GREEK DIALYTIKA TONOS */ \
	    UNI(0x36, 0x0386)	/* GREEK CAPITAL LETTER ALPHA WITH TONOS */ \
	    UNI(0x37, 0x00B7)	/* MIDDLE DOT */ \
	    UNI(0x38, 0x0388)	/* GREEK CAPITAL LETTER EPSILON WITH TONOS */ \
	    UNI(0x39, 0x0389)	/* GREEK CAPITAL LETTER ETA WITH TONOS */ \
	    UNI(0x3A, 0x038A)	/* GREEK CAPITAL LETTER IOTA WITH TONOS */ \
	    UNI(0x3B, 0x00BB)	/* RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK */ \
	    UNI(0x3C, 0x038C)	/* GREEK CAPITAL LETTER OMICRON WITH TONOS */ \
	    UNI(0x3D, 0x00BD)	/* VULGAR FRACTION ONE HALF */ \
	    UNI(0x3E, 0x038E)	/* GREEK CAPITAL LETTER UPSILON WITH TONOS */ \
	    UNI(0x3F, 0x038F)	/* GREEK CAPITAL LETTER OMEGA WITH TONOS */ \
	    UNI(0x40, 0x0390)	/* GREEK SMALL LETTER IOTA WITH DIALYTIKA AND TONOS */ \
	    UNI(0x41, 0x0391)	/* GREEK CAPITAL LETTER ALPHA */ \
	    UNI(0x42, 0x0392)	/* GREEK CAPITAL LETTER BETA */ \
	    UNI(0x43, 0x0393)	/* GREEK CAPITAL LETTER GAMMA */ \
	    UNI(0x44, 0x0394)	/* GREEK CAPITAL LETTER DELTA */ \
	    UNI(0x45, 0x0395)	/* GREEK CAPITAL LETTER EPSILON */ \
	    UNI(0x46, 0x0396)	/* GREEK CAPITAL LETTER ZETA */ \
	    UNI(0x47, 0x0397)	/* GREEK CAPITAL LETTER ETA */ \
	    UNI(0x48, 0x0398)	/* GREEK CAPITAL LETTER THETA */ \
	    UNI(0x49, 0x0399)	/* GREEK CAPITAL LETTER IOTA */ \
	    UNI(0x4A, 0x039A)	/* GREEK CAPITAL LETTER KAPPA */ \
	    UNI(0x4B, 0x039B)	/* GREEK CAPITAL LETTER LAMDA */ \
	    UNI(0x4C, 0x039C)	/* GREEK CAPITAL LETTER MU */ \
	    UNI(0x4D, 0x039D)	/* GREEK CAPITAL LETTER NU */ \
	    UNI(0x4E, 0x039E)	/* GREEK CAPITAL LETTER XI */ \
	    UNI(0x4F, 0x039F)	/* GREEK CAPITAL LETTER OMICRON */ \
	    UNI(0x50, 0x03A0)	/* GREEK CAPITAL LETTER PI */ \
	    UNI(0x51, 0x03A1)	/* GREEK CAPITAL LETTER RHO */ \
	    XXX(0x52, UNDEF)	/* undefined */ \
	    UNI(0x53, 0x03A3)	/* GREEK CAPITAL LETTER SIGMA */ \
	    UNI(0x54, 0x03A4)	/* GREEK CAPITAL LETTER TAU */ \
	    UNI(0x55, 0x03A5)	/* GREEK CAPITAL LETTER UPSILON */ \
	    UNI(0x56, 0x03A6)	/* GREEK CAPITAL LETTER PHI */ \
	    UNI(0x57, 0x03A7)	/* GREEK CAPITAL LETTER CHI */ \
	    UNI(0x58, 0x03A8)	/* GREEK CAPITAL LETTER PSI */ \
	    UNI(0x59, 0x03A9)	/* GREEK CAPITAL LETTER OMEGA */ \
	    UNI(0x5A, 0x03AA)	/* GREEK CAPITAL LETTER IOTA WITH DIALYTIKA */ \
	    UNI(0x5B, 0x03AB)	/* GREEK CAPITAL LETTER UPSILON WITH DIALYTIKA */ \
	    UNI(0x5C, 0x03AC)	/* GREEK SMALL LETTER ALPHA WITH TONOS */ \
	    UNI(0x5D, 0x03AD)	/* GREEK SMALL LETTER EPSILON WITH TONOS */ \
	    UNI(0x5E, 0x03AE)	/* GREEK SMALL LETTER ETA WITH TONOS */ \
	    UNI(0x5F, 0x03AF)	/* GREEK SMALL LETTER IOTA WITH TONOS */ \
	    UNI(0x60, 0x03B0)	/* GREEK SMALL LETTER UPSILON WITH DIALYTIKA AND TONOS */ \
	    UNI(0x61, 0x03B1)	/* GREEK SMALL LETTER ALPHA */ \
	    UNI(0x62, 0x03B2)	/* GREEK SMALL LETTER BETA */ \
	    UNI(0x63, 0x03B3)	/* GREEK SMALL LETTER GAMMA */ \
	    UNI(0x64, 0x03B4)	/* GREEK SMALL LETTER DELTA */ \
	    UNI(0x65, 0x03B5)	/* GREEK SMALL LETTER EPSILON */ \
	    UNI(0x66, 0x03B6)	/* GREEK SMALL LETTER ZETA */ \
	    UNI(0x67, 0x03B7)	/* GREEK SMALL LETTER ETA */ \
	    UNI(0x68, 0x03B8)	/* GREEK SMALL LETTER THETA */ \
	    UNI(0x69, 0x03B9)	/* GREEK SMALL LETTER IOTA */ \
	    UNI(0x6A, 0x03BA)	/* GREEK SMALL LETTER KAPPA */ \
	    UNI(0x6B, 0x03BB)	/* GREEK SMALL LETTER LAMDA */ \
	    UNI(0x6C, 0x03BC)	/* GREEK SMALL LETTER MU */ \
	    UNI(0x6D, 0x03BD)	/* GREEK SMALL LETTER NU */ \
	    UNI(0x6E, 0x03BE)	/* GREEK SMALL LETTER XI */ \
	    UNI(0x6F, 0x03BF)	/* GREEK SMALL LETTER OMICRON */ \
	    UNI(0x70, 0x03C0)	/* GREEK SMALL LETTER PI */ \
	    UNI(0x71, 0x03C1)	/* GREEK SMALL LETTER RHO */ \
	    UNI(0x72, 0x03C2)	/* GREEK SMALL LETTER FINAL SIGMA */ \
	    UNI(0x73, 0x03C3)	/* GREEK SMALL LETTER SIGMA */ \
	    UNI(0x74, 0x03C4)	/* GREEK SMALL LETTER TAU */ \
	    UNI(0x75, 0x03C5)	/* GREEK SMALL LETTER UPSILON */ \
	    UNI(0x76, 0x03C6)	/* GREEK SMALL LETTER PHI */ \
	    UNI(0x77, 0x03C7)	/* GREEK SMALL LETTER CHI */ \
	    UNI(0x78, 0x03C8)	/* GREEK SMALL LETTER PSI */ \
	    UNI(0x79, 0x03C9)	/* GREEK SMALL LETTER OMEGA */ \
	    UNI(0x7A, 0x03CA)	/* GREEK SMALL LETTER IOTA WITH DIALYTIKA */ \
	    UNI(0x7B, 0x03CB)	/* GREEK SMALL LETTER UPSILON WITH DIALYTIKA */ \
	    UNI(0x7C, 0x03CC)	/* GREEK SMALL LETTER OMICRON WITH TONOS */ \
	    UNI(0x7D, 0x03CD)	/* GREEK SMALL LETTER UPSILON WITH TONOS */ \
	    UNI(0x7E, 0x03CE)	/* GREEK SMALL LETTER OMEGA WITH TONOS */ \
	} \
	end_CODEPAGE()

#define unmap_ISO_Greek_Supp(code) \
	switch (code) { \
	    MAP(0x00A0, 0x20)	/* NO-BREAK SPACE */ \
	    MAP(0x00A3, 0x23)	/* POUND SIGN */ \
	    MAP(0x00A6, 0x26)	/* BROKEN BAR */ \
	    MAP(0x00A7, 0x27)	/* SECTION SIGN */ \
	    MAP(0x00A8, 0x28)	/* DIAERESIS */ \
	    MAP(0x00A9, 0x29)	/* COPYRIGHT SIGN */ \
	    MAP(0x00AB, 0x2B)	/* LEFT-POINTING DOUBLE ANGLE QUOTATION MARK */ \
	    MAP(0x00AC, 0x2C)	/* NOT SIGN */ \
	    MAP(0x00AD, 0x2D)	/* SOFT HYPHEN */ \
	    MAP(0x00B0, 0x30)	/* DEGREE SIGN */ \
	    MAP(0x00B1, 0x31)	/* PLUS-MINUS SIGN */ \
	    MAP(0x00B2, 0x32)	/* SUPERSCRIPT TWO */ \
	    MAP(0x00B3, 0x33)	/* SUPERSCRIPT THREE */ \
	    MAP(0x00B7, 0x37)	/* MIDDLE DOT */ \
	    MAP(0x00BB, 0x3B)	/* RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK */ \
	    MAP(0x00BD, 0x3D)	/* VULGAR FRACTION ONE HALF */ \
	    MAP(0x0384, 0x96)	/* GREEK TONOS */ \
	    MAP(0x0385, 0x95)	/* GREEK DIALYTIKA TONOS */ \
	    MAP(0x0386, 0x6E)	/* GREEK CAPITAL LETTER ALPHA WITH TONOS */ \
	    MAP(0x0388, 0x6F)	/* GREEK CAPITAL LETTER EPSILON WITH TONOS */ \
	    MAP(0x0389, 0x70)	/* GREEK CAPITAL LETTER ETA WITH TONOS */ \
	    MAP(0x038A, 0x71)	/* GREEK CAPITAL LETTER IOTA WITH TONOS */ \
	    MAP(0x038C, 0x72)	/* GREEK CAPITAL LETTER OMICRON WITH TONOS */ \
	    MAP(0x038E, 0x73)	/* GREEK CAPITAL LETTER UPSILON WITH TONOS */ \
	    MAP(0x038F, 0x74)	/* GREEK CAPITAL LETTER OMEGA WITH TONOS */ \
	    MAP(0x0390, 0x75)	/* GREEK SMALL LETTER IOTA WITH DIALYTIKA AND TONOS */ \
	    MAP(0x0391, 0x76)	/* GREEK CAPITAL LETTER ALPHA */ \
	    MAP(0x0392, 0x77)	/* GREEK CAPITAL LETTER BETA */ \
	    MAP(0x0393, 0x78)	/* GREEK CAPITAL LETTER GAMMA */ \
	    MAP(0x0394, 0x79)	/* GREEK CAPITAL LETTER DELTA */ \
	    MAP(0x0395, 0x7A)	/* GREEK CAPITAL LETTER EPSILON */ \
	    MAP(0x0396, 0x7B)	/* GREEK CAPITAL LETTER ZETA */ \
	    MAP(0x0397, 0x7C)	/* GREEK CAPITAL LETTER ETA */ \
	    MAP(0x0398, 0x7D)	/* GREEK CAPITAL LETTER THETA */ \
	    MAP(0x0399, 0x7E)	/* GREEK CAPITAL LETTER IOTA */ \
	    MAP(0x039A, 0x7F)	/* GREEK CAPITAL LETTER KAPPA */ \
	    MAP(0x039B, 0xA2)	/* GREEK CAPITAL LETTER LAMDA */ \
	    MAP(0x039C, 0xA3)	/* GREEK CAPITAL LETTER MU */ \
	    MAP(0x039D, 0xA4)	/* GREEK CAPITAL LETTER NU */ \
	    MAP(0x039E, 0xA5)	/* GREEK CAPITAL LETTER XI */ \
	    MAP(0x039F, 0xA6)	/* GREEK CAPITAL LETTER OMICRON */ \
	    MAP(0x03A0, 0xA7)	/* GREEK CAPITAL LETTER PI */ \
	    MAP(0x03A1, 0xA8)	/* GREEK CAPITAL LETTER RHO */ \
	    MAP(0x03A3, 0xA9)	/* GREEK CAPITAL LETTER SIGMA */ \
	    MAP(0x03A4, 0xAA)	/* GREEK CAPITAL LETTER TAU */ \
	    MAP(0x03A5, 0xAB)	/* GREEK CAPITAL LETTER UPSILON */ \
	    MAP(0x03A6, 0xAC)	/* GREEK CAPITAL LETTER PHI */ \
	    MAP(0x03A7, 0xAD)	/* GREEK CAPITAL LETTER CHI */ \
	    MAP(0x03A8, 0xAE)	/* GREEK CAPITAL LETTER PSI */ \
	    MAP(0x03A9, 0xAF)	/* GREEK CAPITAL LETTER OMEGA */ \
	    MAP(0x03AA, 0xB0)	/* GREEK CAPITAL LETTER IOTA WITH DIALYTIKA */ \
	    MAP(0x03AB, 0xB1)	/* GREEK CAPITAL LETTER UPSILON WITH DIALYTIKA */ \
	    MAP(0x03AC, 0xB2)	/* GREEK SMALL LETTER ALPHA WITH TONOS */ \
	    MAP(0x03AD, 0xB3)	/* GREEK SMALL LETTER EPSILON WITH TONOS */ \
	    MAP(0x03AE, 0xB4)	/* GREEK SMALL LETTER ETA WITH TONOS */ \
	    MAP(0x03AF, 0xB5)	/* GREEK SMALL LETTER IOTA WITH TONOS */ \
	    MAP(0x03B0, 0xB6)	/* GREEK SMALL LETTER UPSILON WITH DIALYTIKA AND TONOS */ \
	    MAP(0x03B1, 0xB7)	/* GREEK SMALL LETTER ALPHA */ \
	    MAP(0x03B2, 0xB8)	/* GREEK SMALL LETTER BETA */ \
	    MAP(0x03B3, 0xB9)	/* GREEK SMALL LETTER GAMMA */ \
	    MAP(0x03B4, 0xBA)	/* GREEK SMALL LETTER DELTA */ \
	    MAP(0x03B5, 0xBB)	/* GREEK SMALL LETTER EPSILON */ \
	    MAP(0x03B6, 0xBC)	/* GREEK SMALL LETTER ZETA */ \
	    MAP(0x03B7, 0xBD)	/* GREEK SMALL LETTER ETA */ \
	    MAP(0x03B8, 0xBE)	/* GREEK SMALL LETTER THETA */ \
	    MAP(0x03B9, 0xBF)	/* GREEK SMALL LETTER IOTA */ \
	    MAP(0x03BA, 0xC0)	/* GREEK SMALL LETTER KAPPA */ \
	    MAP(0x03BB, 0xC1)	/* GREEK SMALL LETTER LAMDA */ \
	    MAP(0x03BC, 0xC2)	/* GREEK SMALL LETTER MU */ \
	    MAP(0x03BD, 0xC3)	/* GREEK SMALL LETTER NU */ \
	    MAP(0x03BE, 0xC4)	/* GREEK SMALL LETTER XI */ \
	    MAP(0x03BF, 0xC5)	/* GREEK SMALL LETTER OMICRON */ \
	    MAP(0x03C0, 0xC6)	/* GREEK SMALL LETTER PI */ \
	    MAP(0x03C1, 0xC7)	/* GREEK SMALL LETTER RHO */ \
	    MAP(0x03C2, 0xC8)	/* GREEK SMALL LETTER FINAL SIGMA */ \
	    MAP(0x03C3, 0xC9)	/* GREEK SMALL LETTER SIGMA */ \
	    MAP(0x03C4, 0xCA)	/* GREEK SMALL LETTER TAU */ \
	    MAP(0x03C5, 0xCB)	/* GREEK SMALL LETTER UPSILON */ \
	    MAP(0x03C6, 0xCC)	/* GREEK SMALL LETTER PHI */ \
	    MAP(0x03C7, 0xCD)	/* GREEK SMALL LETTER CHI */ \
	    MAP(0x03C8, 0xCE)	/* GREEK SMALL LETTER PSI */ \
	    MAP(0x03C9, 0xCF)	/* GREEK SMALL LETTER OMEGA */ \
	    MAP(0x03CA, 0xD0)	/* GREEK SMALL LETTER IOTA WITH DIALYTIKA */ \
	    MAP(0x03CB, 0xD1)	/* GREEK SMALL LETTER UPSILON WITH DIALYTIKA */ \
	    MAP(0x03CC, 0xD2)	/* GREEK SMALL LETTER OMICRON WITH TONOS */ \
	    MAP(0x03CD, 0xD3)	/* GREEK SMALL LETTER UPSILON WITH TONOS */ \
	    MAP(0x03CE, 0xD4)	/* GREEK SMALL LETTER OMEGA WITH TONOS */ \
	    MAP(0x2015, 0x2D)	/* HORIZONTAL BAR */ \
	    MAP(0x2018, 0x60)	/* LEFT SINGLE QUOTATION MARK */ \
	    MAP(0x2019, 0x27)	/* RIGHT SINGLE QUOTATION MARK */ \
	    MAP(UNDEF, 0x1B)	/* undefined */ \
	}

/*
 * figure A-23 "ISO Latin-Hebrew Supplemental Character Set"
 */
#define map_ISO_Hebrew(code) \
	begin_CODEPAGE(96) \
	switch (code) { \
	    UNI(0x20, 0x00A0)	/* NO-BREAK SPACE */ \
	    XXX(0x21, UNDEF)	/* undefined */ \
	    UNI(0x22, 0x00A2)	/* CENT SIGN */ \
	    UNI(0x23, 0x00A3)	/* POUND SIGN */ \
	    UNI(0x24, 0x00A4)	/* CURRENCY SIGN */ \
	    UNI(0x25, 0x00A5)	/* YEN SIGN */ \
	    UNI(0x26, 0x00A6)	/* BROKEN BAR */ \
	    UNI(0x27, 0x00A7)	/* SECTION SIGN */ \
	    UNI(0x28, 0x00A8)	/* DIAERESIS */ \
	    UNI(0x29, 0x00A9)	/* COPYRIGHT SIGN */ \
	    UNI(0x2A, 0x00D7)	/* MULTIPLICATION SIGN */ \
	    UNI(0x2B, 0x00AB)	/* LEFT-POINTING DOUBLE ANGLE QUOTATION MARK */ \
	    UNI(0x2C, 0x00AC)	/* NOT SIGN */ \
	    UNI(0x2D, 0x00AD)	/* SOFT HYPHEN */ \
	    UNI(0x2E, 0x00AE)	/* REGISTERED SIGN */ \
	    UNI(0x2F, 0x00AF)	/* MACRON */ \
	    UNI(0x30, 0x00B0)	/* DEGREE SIGN */ \
	    UNI(0x31, 0x00B1)	/* PLUS-MINUS SIGN */ \
	    UNI(0x32, 0x00B2)	/* SUPERSCRIPT TWO */ \
	    UNI(0x33, 0x00B3)	/* SUPERSCRIPT THREE */ \
	    UNI(0x34, 0x00B4)	/* ACUTE ACCENT */ \
	    UNI(0x35, 0x00B5)	/* MICRO SIGN */ \
	    UNI(0x36, 0x00B6)	/* PILCROW SIGN */ \
	    UNI(0x37, 0x00B7)	/* MIDDLE DOT */ \
	    UNI(0x38, 0x00B8)	/* CEDILLA */ \
	    UNI(0x39, 0x00B9)	/* SUPERSCRIPT ONE */ \
	    UNI(0x3A, 0x00F7)	/* DIVISION SIGN */ \
	    UNI(0x3B, 0x00BB)	/* RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK */ \
	    UNI(0x3C, 0x00BC)	/* VULGAR FRACTION ONE QUARTER */ \
	    UNI(0x3D, 0x00BD)	/* VULGAR FRACTION ONE HALF */ \
	    UNI(0x3E, 0x00BE)	/* VULGAR FRACTION THREE QUARTERS */ \
	    XXX(0x3F, UNDEF)	/* undefined */ \
	    XXX(0x40, UNDEF)	/* undefined */ \
	    XXX(0x41, UNDEF)	/* undefined */ \
	    XXX(0x42, UNDEF)	/* undefined */ \
	    XXX(0x43, UNDEF)	/* undefined */ \
	    XXX(0x44, UNDEF)	/* undefined */ \
	    XXX(0x45, UNDEF)	/* undefined */ \
	    XXX(0x46, UNDEF)	/* undefined */ \
	    XXX(0x47, UNDEF)	/* undefined */ \
	    XXX(0x48, UNDEF)	/* undefined */ \
	    XXX(0x49, UNDEF)	/* undefined */ \
	    XXX(0x4A, UNDEF)	/* undefined */ \
	    XXX(0x4B, UNDEF)	/* undefined */ \
	    XXX(0x4C, UNDEF)	/* undefined */ \
	    XXX(0x4D, UNDEF)	/* undefined */ \
	    XXX(0x4E, UNDEF)	/* undefined */ \
	    XXX(0x4F, UNDEF)	/* undefined */ \
	    XXX(0x50, UNDEF)	/* undefined */ \
	    XXX(0x51, UNDEF)	/* undefined */ \
	    XXX(0x52, UNDEF)	/* undefined */ \
	    XXX(0x53, UNDEF)	/* undefined */ \
	    XXX(0x54, UNDEF)	/* undefined */ \
	    XXX(0x55, UNDEF)	/* undefined */ \
	    XXX(0x56, UNDEF)	/* undefined */ \
	    XXX(0x57, UNDEF)	/* undefined */ \
	    XXX(0x58, UNDEF)	/* undefined */ \
	    XXX(0x59, UNDEF)	/* undefined */ \
	    XXX(0x5A, UNDEF)	/* undefined */ \
	    XXX(0x5B, UNDEF)	/* undefined */ \
	    XXX(0x5C, UNDEF)	/* undefined */ \
	    XXX(0x5D, UNDEF)	/* undefined */ \
	    XXX(0x5E, UNDEF)	/* undefined */ \
	    UNI(0x5F, 0x2017)	/* DOUBLE LOW LINE */ \
	    UNI(0x60, 0x05D0)	/* HEBREW LETTER ALEF */ \
	    UNI(0x61, 0x05D1)	/* HEBREW LETTER BET */ \
	    UNI(0x62, 0x05D2)	/* HEBREW LETTER GIMEL */ \
	    UNI(0x63, 0x05D3)	/* HEBREW LETTER DALET */ \
	    UNI(0x64, 0x05D4)	/* HEBREW LETTER HE */ \
	    UNI(0x65, 0x05D5)	/* HEBREW LETTER VAV */ \
	    UNI(0x66, 0x05D6)	/* HEBREW LETTER ZAYIN */ \
	    UNI(0x67, 0x05D7)	/* HEBREW LETTER HET */ \
	    UNI(0x68, 0x05D8)	/* HEBREW LETTER TET */ \
	    UNI(0x69, 0x05D9)	/* HEBREW LETTER YOD */ \
	    UNI(0x6A, 0x05DA)	/* HEBREW LETTER FINAL KAF */ \
	    UNI(0x6B, 0x05DB)	/* HEBREW LETTER KAF */ \
	    UNI(0x6C, 0x05DC)	/* HEBREW LETTER LAMED */ \
	    UNI(0x6D, 0x05DD)	/* HEBREW LETTER FINAL MEM */ \
	    UNI(0x6E, 0x05DE)	/* HEBREW LETTER MEM */ \
	    UNI(0x6F, 0x05DF)	/* HEBREW LETTER FINAL NUN */ \
	    UNI(0x70, 0x05E0)	/* HEBREW LETTER NUN */ \
	    UNI(0x71, 0x05E1)	/* HEBREW LETTER SAMEKH */ \
	    UNI(0x72, 0x05E2)	/* HEBREW LETTER AYIN */ \
	    UNI(0x73, 0x05E3)	/* HEBREW LETTER FINAL PE */ \
	    UNI(0x74, 0x05E4)	/* HEBREW LETTER PE */ \
	    UNI(0x75, 0x05E5)	/* HEBREW LETTER FINAL TSADI */ \
	    UNI(0x76, 0x05E6)	/* HEBREW LETTER TSADI */ \
	    UNI(0x77, 0x05E7)	/* HEBREW LETTER QOF */ \
	    UNI(0x78, 0x05E8)	/* HEBREW LETTER RESH */ \
	    UNI(0x79, 0x05E9)	/* HEBREW LETTER SHIN */ \
	    UNI(0x7A, 0x05EA)	/* HEBREW LETTER TAV */ \
	    XXX(0x7B, UNDEF)	/* undefined */ \
	    XXX(0x7C, UNDEF)	/* undefined */ \
	    XXX(0x7D, UNDEF)	/* undefined */ \
	    XXX(0x7E, UNDEF)	/* undefined */ \
	} \
	end_CODEPAGE()

#define unmap_ISO_Hebrew(code) \
	switch (code) { \
	    MAP(0x00A0, 0x20)	/* NO-BREAK SPACE */ \
	    MAP(0x00A2, 0x22)	/* CENT SIGN */ \
	    MAP(0x00A3, 0x23)	/* POUND SIGN */ \
	    MAP(0x00A4, 0x24)	/* CURRENCY SIGN */ \
	    MAP(0x00A5, 0x25)	/* YEN SIGN */ \
	    MAP(0x00A6, 0x26)	/* BROKEN BAR */ \
	    MAP(0x00A7, 0x27)	/* SECTION SIGN */ \
	    MAP(0x00A8, 0x28)	/* DIAERESIS */ \
	    MAP(0x00A9, 0x29)	/* COPYRIGHT SIGN */ \
	    MAP(0x00AB, 0x2B)	/* LEFT-POINTING DOUBLE ANGLE QUOTATION MARK */ \
	    MAP(0x00AC, 0x2C)	/* NOT SIGN */ \
	    MAP(0x00AD, 0x2D)	/* SOFT HYPHEN */ \
	    MAP(0x00AE, 0x2E)	/* REGISTERED SIGN */ \
	    MAP(0x00AF, 0x2F)	/* MACRON */ \
	    MAP(0x00B0, 0x30)	/* DEGREE SIGN */ \
	    MAP(0x00B1, 0x31)	/* PLUS-MINUS SIGN */ \
	    MAP(0x00B2, 0x32)	/* SUPERSCRIPT TWO */ \
	    MAP(0x00B3, 0x33)	/* SUPERSCRIPT THREE */ \
	    MAP(0x00B4, 0x34)	/* ACUTE ACCENT */ \
	    MAP(0x00B5, 0x35)	/* MICRO SIGN */ \
	    MAP(0x00B6, 0x36)	/* PILCROW SIGN */ \
	    MAP(0x00B7, 0x37)	/* MIDDLE DOT */ \
	    MAP(0x00B8, 0x38)	/* CEDILLA */ \
	    MAP(0x00B9, 0x39)	/* SUPERSCRIPT ONE */ \
	    MAP(0x00BB, 0x3B)	/* RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK */ \
	    MAP(0x00BC, 0x3C)	/* VULGAR FRACTION ONE QUARTER */ \
	    MAP(0x00BD, 0x3D)	/* VULGAR FRACTION ONE HALF */ \
	    MAP(0x00BE, 0x3E)	/* VULGAR FRACTION THREE QUARTERS */ \
	    MAP(0x00D7, 0xD7)	/* MULTIPLICATION SIGN */ \
	    MAP(0x00F7, 0xF7)	/* DIVISION SIGN */ \
	    MAP(0x05D0, 0x53)	/* HEBREW LETTER ALEF */ \
	    MAP(0x05D1, 0x54)	/* HEBREW LETTER BET */ \
	    MAP(0x05D2, 0x55)	/* HEBREW LETTER GIMEL */ \
	    MAP(0x05D3, 0x56)	/* HEBREW LETTER DALET */ \
	    MAP(0x05D4, 0x57)	/* HEBREW LETTER HE */ \
	    MAP(0x05D5, 0x58)	/* HEBREW LETTER VAV */ \
	    MAP(0x05D6, 0x59)	/* HEBREW LETTER ZAYIN */ \
	    MAP(0x05D7, 0x5A)	/* HEBREW LETTER HET */ \
	    MAP(0x05D8, 0x5B)	/* HEBREW LETTER TET */ \
	    MAP(0x05D9, 0x5C)	/* HEBREW LETTER YOD */ \
	    MAP(0x05DA, 0x5D)	/* HEBREW LETTER FINAL KAF */ \
	    MAP(0x05DB, 0x5E)	/* HEBREW LETTER KAF */ \
	    MAP(0x05DC, 0x5F)	/* HEBREW LETTER LAMED */ \
	    MAP(0x05DD, 0x60)	/* HEBREW LETTER FINAL MEM */ \
	    MAP(0x05DE, 0x61)	/* HEBREW LETTER MEM */ \
	    MAP(0x05DF, 0x62)	/* HEBREW LETTER FINAL NUN */ \
	    MAP(0x05E0, 0x63)	/* HEBREW LETTER NUN */ \
	    MAP(0x05E1, 0x64)	/* HEBREW LETTER SAMEKH */ \
	    MAP(0x05E2, 0x65)	/* HEBREW LETTER AYIN */ \
	    MAP(0x05E3, 0x66)	/* HEBREW LETTER FINAL PE */ \
	    MAP(0x05E4, 0x67)	/* HEBREW LETTER PE */ \
	    MAP(0x05E5, 0x68)	/* HEBREW LETTER FINAL TSADI */ \
	    MAP(0x05E6, 0x69)	/* HEBREW LETTER TSADI */ \
	    MAP(0x05E7, 0x6A)	/* HEBREW LETTER QOF */ \
	    MAP(0x05E8, 0x6B)	/* HEBREW LETTER RESH */ \
	    MAP(0x05E9, 0x6C)	/* HEBREW LETTER SHIN */ \
	    MAP(0x05EA, 0x6D)	/* HEBREW LETTER TAV */ \
	    MAP(0x2017, 0x52)	/* DOUBLE LOW LINE */ \
	    MAP(UNDEF, 0x1B)	/* undefined */ \
	}

/*
 * ISO Latin-2 is 8859-2
 */
#define map_ISO_Latin_2(code) \
	begin_CODEPAGE(96) \
	switch (code) { \
	    UNI(0x20, 0x00A0)	/* NO-BREAK SPACE */ \
	    UNI(0x21, 0x0104)	/* LATIN CAPITAL LETTER A WITH OGONEK */ \
	    UNI(0x22, 0x02D8)	/* BREVE */ \
	    UNI(0x23, 0x0141)	/* LATIN CAPITAL LETTER L WITH STROKE */ \
	    UNI(0x24, 0x00A4)	/* CURRENCY SIGN */ \
	    UNI(0x25, 0x013D)	/* LATIN CAPITAL LETTER L WITH CARON */ \
	    UNI(0x26, 0x015A)	/* LATIN CAPITAL LETTER S WITH ACUTE */ \
	    UNI(0x27, 0x00A7)	/* SECTION SIGN */ \
	    UNI(0x28, 0x00A8)	/* DIAERESIS */ \
	    UNI(0x29, 0x0160)	/* LATIN CAPITAL LETTER S WITH CARON */ \
	    UNI(0x2A, 0x015E)	/* LATIN CAPITAL LETTER S WITH CEDILLA */ \
	    UNI(0x2B, 0x0164)	/* LATIN CAPITAL LETTER T WITH CARON */ \
	    UNI(0x2C, 0x0179)	/* LATIN CAPITAL LETTER Z WITH ACUTE */ \
	    UNI(0x2D, 0x00AD)	/* SOFT HYPHEN */ \
	    UNI(0x2E, 0x017D)	/* LATIN CAPITAL LETTER Z WITH CARON */ \
	    UNI(0x2F, 0x017B)	/* LATIN CAPITAL LETTER Z WITH DOT ABOVE */ \
	    UNI(0x30, 0x00B0)	/* DEGREE SIGN */ \
	    UNI(0x31, 0x0105)	/* LATIN SMALL LETTER A WITH OGONEK */ \
	    UNI(0x32, 0x02DB)	/* OGONEK */ \
	    UNI(0x33, 0x0142)	/* LATIN SMALL LETTER L WITH STROKE */ \
	    UNI(0x34, 0x00B4)	/* ACUTE ACCENT */ \
	    UNI(0x35, 0x013E)	/* LATIN SMALL LETTER L WITH CARON */ \
	    UNI(0x36, 0x015B)	/* LATIN SMALL LETTER S WITH ACUTE */ \
	    UNI(0x37, 0x02C7)	/* CARON */ \
	    UNI(0x38, 0x00B8)	/* CEDILLA */ \
	    UNI(0x39, 0x0161)	/* LATIN SMALL LETTER S WITH CARON */ \
	    UNI(0x3A, 0x015F)	/* LATIN SMALL LETTER S WITH CEDILLA */ \
	    UNI(0x3B, 0x0165)	/* LATIN SMALL LETTER T WITH CARON */ \
	    UNI(0x3C, 0x017A)	/* LATIN SMALL LETTER Z WITH ACUTE */ \
	    UNI(0x3D, 0x02DD)	/* DOUBLE ACUTE ACCENT */ \
	    UNI(0x3E, 0x017E)	/* LATIN SMALL LETTER Z WITH CARON */ \
	    UNI(0x3F, 0x017C)	/* LATIN SMALL LETTER Z WITH DOT ABOVE */ \
	    UNI(0x40, 0x0154)	/* LATIN CAPITAL LETTER R WITH ACUTE */ \
	    UNI(0x41, 0x00C1)	/* LATIN CAPITAL LETTER A WITH ACUTE */ \
	    UNI(0x42, 0x00C2)	/* LATIN CAPITAL LETTER A WITH CIRCUMFLEX */ \
	    UNI(0x43, 0x0102)	/* LATIN CAPITAL LETTER A WITH BREVE */ \
	    UNI(0x44, 0x00C4)	/* LATIN CAPITAL LETTER A WITH DIAERESIS */ \
	    UNI(0x45, 0x0139)	/* LATIN CAPITAL LETTER L WITH ACUTE */ \
	    UNI(0x46, 0x0106)	/* LATIN CAPITAL LETTER C WITH ACUTE */ \
	    UNI(0x47, 0x00C7)	/* LATIN CAPITAL LETTER C WITH CEDILLA */ \
	    UNI(0x48, 0x010C)	/* LATIN CAPITAL LETTER C WITH CARON */ \
	    UNI(0x49, 0x00C9)	/* LATIN CAPITAL LETTER E WITH ACUTE */ \
	    UNI(0x4A, 0x0118)	/* LATIN CAPITAL LETTER E WITH OGONEK */ \
	    UNI(0x4B, 0x00CB)	/* LATIN CAPITAL LETTER E WITH DIAERESIS */ \
	    UNI(0x4C, 0x011A)	/* LATIN CAPITAL LETTER E WITH CARON */ \
	    UNI(0x4D, 0x00CD)	/* LATIN CAPITAL LETTER I WITH ACUTE */ \
	    UNI(0x4E, 0x00CE)	/* LATIN CAPITAL LETTER I WITH CIRCUMFLEX */ \
	    UNI(0x4F, 0x010E)	/* LATIN CAPITAL LETTER D WITH CARON */ \
	    UNI(0x50, 0x0110)	/* LATIN CAPITAL LETTER D WITH STROKE */ \
	    UNI(0x51, 0x0143)	/* LATIN CAPITAL LETTER N WITH ACUTE */ \
	    UNI(0x52, 0x0147)	/* LATIN CAPITAL LETTER N WITH CARON */ \
	    UNI(0x53, 0x00D3)	/* LATIN CAPITAL LETTER O WITH ACUTE */ \
	    UNI(0x54, 0x00D4)	/* LATIN CAPITAL LETTER O WITH CIRCUMFLEX */ \
	    UNI(0x55, 0x0150)	/* LATIN CAPITAL LETTER O WITH DOUBLE ACUTE */ \
	    UNI(0x56, 0x00D6)	/* LATIN CAPITAL LETTER O WITH DIAERESIS */ \
	    UNI(0x57, 0x00D7)	/* MULTIPLICATION SIGN */ \
	    UNI(0x58, 0x0158)	/* LATIN CAPITAL LETTER R WITH CARON */ \
	    UNI(0x59, 0x016E)	/* LATIN CAPITAL LETTER U WITH RING ABOVE */ \
	    UNI(0x5A, 0x00DA)	/* LATIN CAPITAL LETTER U WITH ACUTE */ \
	    UNI(0x5B, 0x0170)	/* LATIN CAPITAL LETTER U WITH DOUBLE ACUTE */ \
	    UNI(0x5C, 0x00DC)	/* LATIN CAPITAL LETTER U WITH DIAERESIS */ \
	    UNI(0x5D, 0x00DD)	/* LATIN CAPITAL LETTER Y WITH ACUTE */ \
	    UNI(0x5E, 0x0162)	/* LATIN CAPITAL LETTER T WITH CEDILLA */ \
	    UNI(0x5F, 0x00DF)	/* LATIN SMALL LETTER SHARP S */ \
	    UNI(0x60, 0x0155)	/* LATIN SMALL LETTER R WITH ACUTE */ \
	    UNI(0x61, 0x00E1)	/* LATIN SMALL LETTER A WITH ACUTE */ \
	    UNI(0x62, 0x00E2)	/* LATIN SMALL LETTER A WITH CIRCUMFLEX */ \
	    UNI(0x63, 0x0103)	/* LATIN SMALL LETTER A WITH BREVE */ \
	    UNI(0x64, 0x00E4)	/* LATIN SMALL LETTER A WITH DIAERESIS */ \
	    UNI(0x65, 0x013A)	/* LATIN SMALL LETTER L WITH ACUTE */ \
	    UNI(0x66, 0x0107)	/* LATIN SMALL LETTER C WITH ACUTE */ \
	    UNI(0x67, 0x00E7)	/* LATIN SMALL LETTER C WITH CEDILLA */ \
	    UNI(0x68, 0x010D)	/* LATIN SMALL LETTER C WITH CARON */ \
	    UNI(0x69, 0x00E9)	/* LATIN SMALL LETTER E WITH ACUTE */ \
	    UNI(0x6A, 0x0119)	/* LATIN SMALL LETTER E WITH OGONEK */ \
	    UNI(0x6B, 0x00EB)	/* LATIN SMALL LETTER E WITH DIAERESIS */ \
	    UNI(0x6C, 0x011B)	/* LATIN SMALL LETTER E WITH CARON */ \
	    UNI(0x6D, 0x00ED)	/* LATIN SMALL LETTER I WITH ACUTE */ \
	    UNI(0x6E, 0x00EE)	/* LATIN SMALL LETTER I WITH CIRCUMFLEX */ \
	    UNI(0x6F, 0x010F)	/* LATIN SMALL LETTER D WITH CARON */ \
	    UNI(0x70, 0x0111)	/* LATIN SMALL LETTER D WITH STROKE */ \
	    UNI(0x71, 0x0144)	/* LATIN SMALL LETTER N WITH ACUTE */ \
	    UNI(0x72, 0x0148)	/* LATIN SMALL LETTER N WITH CARON */ \
	    UNI(0x73, 0x00F3)	/* LATIN SMALL LETTER O WITH ACUTE */ \
	    UNI(0x74, 0x00F4)	/* LATIN SMALL LETTER O WITH CIRCUMFLEX */ \
	    UNI(0x75, 0x0151)	/* LATIN SMALL LETTER O WITH DOUBLE ACUTE */ \
	    UNI(0x76, 0x00F6)	/* LATIN SMALL LETTER O WITH DIAERESIS */ \
	    UNI(0x77, 0x00F7)	/* DIVISION SIGN */ \
	    UNI(0x78, 0x0159)	/* LATIN SMALL LETTER R WITH CARON */ \
	    UNI(0x79, 0x016F)	/* LATIN SMALL LETTER U WITH RING ABOVE */ \
	    UNI(0x7A, 0x00FA)	/* LATIN SMALL LETTER U WITH ACUTE */ \
	    UNI(0x7B, 0x0171)	/* LATIN SMALL LETTER U WITH DOUBLE ACUTE */ \
	    UNI(0x7C, 0x00FC)	/* LATIN SMALL LETTER U WITH DIAERESIS */ \
	    UNI(0x7D, 0x00FD)	/* LATIN SMALL LETTER Y WITH ACUTE */ \
	    UNI(0x7E, 0x0163)	/* LATIN SMALL LETTER T WITH CEDILLA */ \
	    UNI(0x7F, 0x02D9)	/* DOT ABOVE */ \
	} \
	end_CODEPAGE()

#define unmap_ISO_Latin_2(code) \
	switch (code) { \
	    MAP(0x00A0, 0x20)	/* NO-BREAK SPACE */ \
	    MAP(0x00A4, 0x24)	/* CURRENCY SIGN */ \
	    MAP(0x00A7, 0x27)	/* SECTION SIGN */ \
	    MAP(0x00A8, 0x28)	/* DIAERESIS */ \
	    MAP(0x00AD, 0x2D)	/* SOFT HYPHEN */ \
	    MAP(0x00B0, 0x30)	/* DEGREE SIGN */ \
	    MAP(0x00B4, 0x34)	/* ACUTE ACCENT */ \
	    MAP(0x00B8, 0x38)	/* CEDILLA */ \
	    MAP(0x00C1, 0x41)	/* LATIN CAPITAL LETTER A WITH ACUTE */ \
	    MAP(0x00C2, 0x42)	/* LATIN CAPITAL LETTER A WITH CIRCUMFLEX */ \
	    MAP(0x00C4, 0x44)	/* LATIN CAPITAL LETTER A WITH DIAERESIS */ \
	    MAP(0x00C7, 0x47)	/* LATIN CAPITAL LETTER C WITH CEDILLA */ \
	    MAP(0x00C9, 0x49)	/* LATIN CAPITAL LETTER E WITH ACUTE */ \
	    MAP(0x00CB, 0x4B)	/* LATIN CAPITAL LETTER E WITH DIAERESIS */ \
	    MAP(0x00CD, 0x4D)	/* LATIN CAPITAL LETTER I WITH ACUTE */ \
	    MAP(0x00CE, 0x4E)	/* LATIN CAPITAL LETTER I WITH CIRCUMFLEX */ \
	    MAP(0x00D3, 0x53)	/* LATIN CAPITAL LETTER O WITH ACUTE */ \
	    MAP(0x00D4, 0x54)	/* LATIN CAPITAL LETTER O WITH CIRCUMFLEX */ \
	    MAP(0x00D6, 0x56)	/* LATIN CAPITAL LETTER O WITH DIAERESIS */ \
	    MAP(0x00D7, 0x57)	/* MULTIPLICATION SIGN */ \
	    MAP(0x00DA, 0x5A)	/* LATIN CAPITAL LETTER U WITH ACUTE */ \
	    MAP(0x00DC, 0x5C)	/* LATIN CAPITAL LETTER U WITH DIAERESIS */ \
	    MAP(0x00DD, 0x5D)	/* LATIN CAPITAL LETTER Y WITH ACUTE */ \
	    MAP(0x00DF, 0x5F)	/* LATIN SMALL LETTER SHARP S */ \
	    MAP(0x00E1, 0x61)	/* LATIN SMALL LETTER A WITH ACUTE */ \
	    MAP(0x00E2, 0x62)	/* LATIN SMALL LETTER A WITH CIRCUMFLEX */ \
	    MAP(0x00E4, 0x64)	/* LATIN SMALL LETTER A WITH DIAERESIS */ \
	    MAP(0x00E7, 0x67)	/* LATIN SMALL LETTER C WITH CEDILLA */ \
	    MAP(0x00E9, 0x69)	/* LATIN SMALL LETTER E WITH ACUTE */ \
	    MAP(0x00EB, 0x6B)	/* LATIN SMALL LETTER E WITH DIAERESIS */ \
	    MAP(0x00ED, 0x6D)	/* LATIN SMALL LETTER I WITH ACUTE */ \
	    MAP(0x00EE, 0x6E)	/* LATIN SMALL LETTER I WITH CIRCUMFLEX */ \
	    MAP(0x00F3, 0x73)	/* LATIN SMALL LETTER O WITH ACUTE */ \
	    MAP(0x00F4, 0x74)	/* LATIN SMALL LETTER O WITH CIRCUMFLEX */ \
	    MAP(0x00F6, 0x76)	/* LATIN SMALL LETTER O WITH DIAERESIS */ \
	    MAP(0x00F7, 0x77)	/* DIVISION SIGN */ \
	    MAP(0x00FA, 0x7A)	/* LATIN SMALL LETTER U WITH ACUTE */ \
	    MAP(0x00FC, 0x7C)	/* LATIN SMALL LETTER U WITH DIAERESIS */ \
	    MAP(0x00FD, 0x7D)	/* LATIN SMALL LETTER Y WITH ACUTE */ \
	    MAP(0x0102, 0x43)	/* LATIN CAPITAL LETTER A WITH BREVE */ \
	    MAP(0x0103, 0x63)	/* LATIN SMALL LETTER A WITH BREVE */ \
	    MAP(0x0104, 0x21)	/* LATIN CAPITAL LETTER A WITH OGONEK */ \
	    MAP(0x0105, 0x31)	/* LATIN SMALL LETTER A WITH OGONEK */ \
	    MAP(0x0106, 0x46)	/* LATIN CAPITAL LETTER C WITH ACUTE */ \
	    MAP(0x0107, 0x66)	/* LATIN SMALL LETTER C WITH ACUTE */ \
	    MAP(0x010C, 0x48)	/* LATIN CAPITAL LETTER C WITH CARON */ \
	    MAP(0x010D, 0x68)	/* LATIN SMALL LETTER C WITH CARON */ \
	    MAP(0x010E, 0x4F)	/* LATIN CAPITAL LETTER D WITH CARON */ \
	    MAP(0x010F, 0x6F)	/* LATIN SMALL LETTER D WITH CARON */ \
	    MAP(0x0110, 0x50)	/* LATIN CAPITAL LETTER D WITH STROKE */ \
	    MAP(0x0111, 0x70)	/* LATIN SMALL LETTER D WITH STROKE */ \
	    MAP(0x0118, 0x4A)	/* LATIN CAPITAL LETTER E WITH OGONEK */ \
	    MAP(0x0119, 0x6A)	/* LATIN SMALL LETTER E WITH OGONEK */ \
	    MAP(0x011A, 0x4C)	/* LATIN CAPITAL LETTER E WITH CARON */ \
	    MAP(0x011B, 0x6C)	/* LATIN SMALL LETTER E WITH CARON */ \
	    MAP(0x0139, 0x45)	/* LATIN CAPITAL LETTER L WITH ACUTE */ \
	    MAP(0x013A, 0x65)	/* LATIN SMALL LETTER L WITH ACUTE */ \
	    MAP(0x013D, 0x25)	/* LATIN CAPITAL LETTER L WITH CARON */ \
	    MAP(0x013E, 0x35)	/* LATIN SMALL LETTER L WITH CARON */ \
	    MAP(0x0141, 0x23)	/* LATIN CAPITAL LETTER L WITH STROKE */ \
	    MAP(0x0142, 0x33)	/* LATIN SMALL LETTER L WITH STROKE */ \
	    MAP(0x0143, 0x51)	/* LATIN CAPITAL LETTER N WITH ACUTE */ \
	    MAP(0x0144, 0x71)	/* LATIN SMALL LETTER N WITH ACUTE */ \
	    MAP(0x0147, 0x52)	/* LATIN CAPITAL LETTER N WITH CARON */ \
	    MAP(0x0148, 0x72)	/* LATIN SMALL LETTER N WITH CARON */ \
	    MAP(0x0150, 0x55)	/* LATIN CAPITAL LETTER O WITH DOUBLE ACUTE */ \
	    MAP(0x0151, 0x75)	/* LATIN SMALL LETTER O WITH DOUBLE ACUTE */ \
	    MAP(0x0154, 0x40)	/* LATIN CAPITAL LETTER R WITH ACUTE */ \
	    MAP(0x0155, 0x60)	/* LATIN SMALL LETTER R WITH ACUTE */ \
	    MAP(0x0158, 0x58)	/* LATIN CAPITAL LETTER R WITH CARON */ \
	    MAP(0x0159, 0x78)	/* LATIN SMALL LETTER R WITH CARON */ \
	    MAP(0x015A, 0x26)	/* LATIN CAPITAL LETTER S WITH ACUTE */ \
	    MAP(0x015B, 0x36)	/* LATIN SMALL LETTER S WITH ACUTE */ \
	    MAP(0x015E, 0x2A)	/* LATIN CAPITAL LETTER S WITH CEDILLA */ \
	    MAP(0x015F, 0x3A)	/* LATIN SMALL LETTER S WITH CEDILLA */ \
	    MAP(0x0160, 0x29)	/* LATIN CAPITAL LETTER S WITH CARON */ \
	    MAP(0x0161, 0x39)	/* LATIN SMALL LETTER S WITH CARON */ \
	    MAP(0x0162, 0x5E)	/* LATIN CAPITAL LETTER T WITH CEDILLA */ \
	    MAP(0x0163, 0x7E)	/* LATIN SMALL LETTER T WITH CEDILLA */ \
	    MAP(0x0164, 0x2B)	/* LATIN CAPITAL LETTER T WITH CARON */ \
	    MAP(0x0165, 0x3B)	/* LATIN SMALL LETTER T WITH CARON */ \
	    MAP(0x016E, 0x59)	/* LATIN CAPITAL LETTER U WITH RING ABOVE */ \
	    MAP(0x016F, 0x79)	/* LATIN SMALL LETTER U WITH RING ABOVE */ \
	    MAP(0x0170, 0x5B)	/* LATIN CAPITAL LETTER U WITH DOUBLE ACUTE */ \
	    MAP(0x0171, 0x7B)	/* LATIN SMALL LETTER U WITH DOUBLE ACUTE */ \
	    MAP(0x0179, 0x2C)	/* LATIN CAPITAL LETTER Z WITH ACUTE */ \
	    MAP(0x017A, 0x3C)	/* LATIN SMALL LETTER Z WITH ACUTE */ \
	    MAP(0x017B, 0x2F)	/* LATIN CAPITAL LETTER Z WITH DOT ABOVE */ \
	    MAP(0x017C, 0x3F)	/* LATIN SMALL LETTER Z WITH DOT ABOVE */ \
	    MAP(0x017D, 0x2E)	/* LATIN CAPITAL LETTER Z WITH CARON */ \
	    MAP(0x017E, 0x3E)	/* LATIN SMALL LETTER Z WITH CARON */ \
	    MAP(0x02C7, 0x37)	/* CARON */ \
	    MAP(0x02D8, 0x22)	/* BREVE */ \
	    MAP(0x02D9, 0x7F)	/* DOT ABOVE */ \
	    MAP(0x02DB, 0x32)	/* OGONEK */ \
	    MAP(0x02DD, 0x3D)	/* DOUBLE ACUTE ACCENT */ \
	}

/*
 * ISO Latin-5 is 8859-9
 */
#define map_ISO_Latin_5(code) \
	begin_CODEPAGE(96) \
	switch (code) { \
	    UNI(0x20, 0x00A0)	/* NO-BREAK SPACE */ \
	    UNI(0x21, 0x00A1)	/* INVERTED EXCLAMATION MARK */ \
	    UNI(0x22, 0x00A2)	/* CENT SIGN */ \
	    UNI(0x23, 0x00A3)	/* POUND SIGN */ \
	    UNI(0x24, 0x00A4)	/* CURRENCY SIGN */ \
	    UNI(0x25, 0x00A5)	/* YEN SIGN */ \
	    UNI(0x26, 0x00A6)	/* BROKEN BAR */ \
	    UNI(0x27, 0x00A7)	/* SECTION SIGN */ \
	    UNI(0x28, 0x00A8)	/* DIAERESIS */ \
	    UNI(0x29, 0x00A9)	/* COPYRIGHT SIGN */ \
	    UNI(0x2A, 0x00AA)	/* FEMININE ORDINAL INDICATOR */ \
	    UNI(0x2B, 0x00AB)	/* LEFT-POINTING DOUBLE ANGLE QUOTATION MARK */ \
	    UNI(0x2C, 0x00AC)	/* NOT SIGN */ \
	    UNI(0x2D, 0x00AD)	/* SOFT HYPHEN */ \
	    UNI(0x2E, 0x00AE)	/* REGISTERED SIGN */ \
	    UNI(0x2F, 0x00AF)	/* MACRON */ \
	    UNI(0x30, 0x00B0)	/* DEGREE SIGN */ \
	    UNI(0x31, 0x00B1)	/* PLUS-MINUS SIGN */ \
	    UNI(0x32, 0x00B2)	/* SUPERSCRIPT TWO */ \
	    UNI(0x33, 0x00B3)	/* SUPERSCRIPT THREE */ \
	    UNI(0x34, 0x00B4)	/* ACUTE ACCENT */ \
	    UNI(0x35, 0x00B5)	/* MICRO SIGN */ \
	    UNI(0x36, 0x00B6)	/* PILCROW SIGN */ \
	    UNI(0x37, 0x00B7)	/* MIDDLE DOT */ \
	    UNI(0x38, 0x00B8)	/* CEDILLA */ \
	    UNI(0x39, 0x00B9)	/* SUPERSCRIPT ONE */ \
	    UNI(0x3A, 0x00BA)	/* MASCULINE ORDINAL INDICATOR */ \
	    UNI(0x3B, 0x00BB)	/* RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK */ \
	    UNI(0x3C, 0x00BC)	/* VULGAR FRACTION ONE QUARTER */ \
	    UNI(0x3D, 0x00BD)	/* VULGAR FRACTION ONE HALF */ \
	    UNI(0x3E, 0x00BE)	/* VULGAR FRACTION THREE QUARTERS */ \
	    UNI(0x3F, 0x00BF)	/* INVERTED QUESTION MARK */ \
	    UNI(0x40, 0x00C0)	/* LATIN CAPITAL LETTER A WITH GRAVE */ \
	    UNI(0x41, 0x00C1)	/* LATIN CAPITAL LETTER A WITH ACUTE */ \
	    UNI(0x42, 0x00C2)	/* LATIN CAPITAL LETTER A WITH CIRCUMFLEX */ \
	    UNI(0x43, 0x00C3)	/* LATIN CAPITAL LETTER A WITH TILDE */ \
	    UNI(0x44, 0x00C4)	/* LATIN CAPITAL LETTER A WITH DIAERESIS */ \
	    UNI(0x45, 0x00C5)	/* LATIN CAPITAL LETTER A WITH RING ABOVE */ \
	    UNI(0x46, 0x00C6)	/* LATIN CAPITAL LETTER AE */ \
	    UNI(0x47, 0x00C7)	/* LATIN CAPITAL LETTER C WITH CEDILLA */ \
	    UNI(0x48, 0x00C8)	/* LATIN CAPITAL LETTER E WITH GRAVE */ \
	    UNI(0x49, 0x00C9)	/* LATIN CAPITAL LETTER E WITH ACUTE */ \
	    UNI(0x4A, 0x00CA)	/* LATIN CAPITAL LETTER E WITH CIRCUMFLEX */ \
	    UNI(0x4B, 0x00CB)	/* LATIN CAPITAL LETTER E WITH DIAERESIS */ \
	    UNI(0x4C, 0x00CC)	/* LATIN CAPITAL LETTER I WITH GRAVE */ \
	    UNI(0x4D, 0x00CD)	/* LATIN CAPITAL LETTER I WITH ACUTE */ \
	    UNI(0x4E, 0x00CE)	/* LATIN CAPITAL LETTER I WITH CIRCUMFLEX */ \
	    UNI(0x4F, 0x00CF)	/* LATIN CAPITAL LETTER I WITH DIAERESIS */ \
	    UNI(0x50, 0x011E)	/* LATIN CAPITAL LETTER G WITH BREVE */ \
	    UNI(0x51, 0x00D1)	/* LATIN CAPITAL LETTER N WITH TILDE */ \
	    UNI(0x52, 0x00D2)	/* LATIN CAPITAL LETTER O WITH GRAVE */ \
	    UNI(0x53, 0x00D3)	/* LATIN CAPITAL LETTER O WITH ACUTE */ \
	    UNI(0x54, 0x00D4)	/* LATIN CAPITAL LETTER O WITH CIRCUMFLEX */ \
	    UNI(0x55, 0x00D5)	/* LATIN CAPITAL LETTER O WITH TILDE */ \
	    UNI(0x56, 0x00D6)	/* LATIN CAPITAL LETTER O WITH DIAERESIS */ \
	    UNI(0x57, 0x00D7)	/* MULTIPLICATION SIGN */ \
	    UNI(0x58, 0x00D8)	/* LATIN CAPITAL LETTER O WITH STROKE */ \
	    UNI(0x59, 0x00D9)	/* LATIN CAPITAL LETTER U WITH GRAVE */ \
	    UNI(0x5A, 0x00DA)	/* LATIN CAPITAL LETTER U WITH ACUTE */ \
	    UNI(0x5B, 0x00DB)	/* LATIN CAPITAL LETTER U WITH CIRCUMFLEX */ \
	    UNI(0x5C, 0x00DC)	/* LATIN CAPITAL LETTER U WITH DIAERESIS */ \
	    UNI(0x5D, 0x0130)	/* LATIN CAPITAL LETTER I WITH DOT ABOVE */ \
	    UNI(0x5E, 0x015E)	/* LATIN CAPITAL LETTER S WITH CEDILLA */ \
	    UNI(0x5F, 0x00DF)	/* LATIN SMALL LETTER SHARP S */ \
	    UNI(0x60, 0x00E0)	/* LATIN SMALL LETTER A WITH GRAVE */ \
	    UNI(0x61, 0x00E1)	/* LATIN SMALL LETTER A WITH ACUTE */ \
	    UNI(0x62, 0x00E2)	/* LATIN SMALL LETTER A WITH CIRCUMFLEX */ \
	    UNI(0x63, 0x00E3)	/* LATIN SMALL LETTER A WITH TILDE */ \
	    UNI(0x64, 0x00E4)	/* LATIN SMALL LETTER A WITH DIAERESIS */ \
	    UNI(0x65, 0x00E5)	/* LATIN SMALL LETTER A WITH RING ABOVE */ \
	    UNI(0x66, 0x00E6)	/* LATIN SMALL LETTER AE */ \
	    UNI(0x67, 0x00E7)	/* LATIN SMALL LETTER C WITH CEDILLA */ \
	    UNI(0x68, 0x00E8)	/* LATIN SMALL LETTER E WITH GRAVE */ \
	    UNI(0x69, 0x00E9)	/* LATIN SMALL LETTER E WITH ACUTE */ \
	    UNI(0x6A, 0x00EA)	/* LATIN SMALL LETTER E WITH CIRCUMFLEX */ \
	    UNI(0x6B, 0x00EB)	/* LATIN SMALL LETTER E WITH DIAERESIS */ \
	    UNI(0x6C, 0x00EC)	/* LATIN SMALL LETTER I WITH GRAVE */ \
	    UNI(0x6D, 0x00ED)	/* LATIN SMALL LETTER I WITH ACUTE */ \
	    UNI(0x6E, 0x00EE)	/* LATIN SMALL LETTER I WITH CIRCUMFLEX */ \
	    UNI(0x6F, 0x00EF)	/* LATIN SMALL LETTER I WITH DIAERESIS */ \
	    UNI(0x70, 0x011F)	/* LATIN SMALL LETTER G WITH BREVE */ \
	    UNI(0x71, 0x00F1)	/* LATIN SMALL LETTER N WITH TILDE */ \
	    UNI(0x72, 0x00F2)	/* LATIN SMALL LETTER O WITH GRAVE */ \
	    UNI(0x73, 0x00F3)	/* LATIN SMALL LETTER O WITH ACUTE */ \
	    UNI(0x74, 0x00F4)	/* LATIN SMALL LETTER O WITH CIRCUMFLEX */ \
	    UNI(0x75, 0x00F5)	/* LATIN SMALL LETTER O WITH TILDE */ \
	    UNI(0x76, 0x00F6)	/* LATIN SMALL LETTER O WITH DIAERESIS */ \
	    UNI(0x77, 0x00F7)	/* DIVISION SIGN */ \
	    UNI(0x78, 0x00F8)	/* LATIN SMALL LETTER O WITH STROKE */ \
	    UNI(0x79, 0x00F9)	/* LATIN SMALL LETTER U WITH GRAVE */ \
	    UNI(0x7A, 0x00FA)	/* LATIN SMALL LETTER U WITH ACUTE */ \
	    UNI(0x7B, 0x00FB)	/* LATIN SMALL LETTER U WITH CIRCUMFLEX */ \
	    UNI(0x7C, 0x00FC)	/* LATIN SMALL LETTER U WITH DIAERESIS */ \
	    UNI(0x7D, 0x0131)	/* LATIN SMALL LETTER DOTLESS I */ \
	    UNI(0x7E, 0x015F)	/* LATIN SMALL LETTER S WITH CEDILLA */ \
	    UNI(0x7F, 0x00FF)	/* LATIN SMALL LETTER Y WITH DIAERESIS */ \
	} \
	end_CODEPAGE()

#define unmap_ISO_Latin_5(code) \
	switch (code) { \
	    MAP(0x00A0, 0x20)	/* NO-BREAK SPACE */ \
	    MAP(0x00A1, 0x21)	/* INVERTED EXCLAMATION MARK */ \
	    MAP(0x00A2, 0x22)	/* CENT SIGN */ \
	    MAP(0x00A3, 0x23)	/* POUND SIGN */ \
	    MAP(0x00A4, 0x24)	/* CURRENCY SIGN */ \
	    MAP(0x00A5, 0x25)	/* YEN SIGN */ \
	    MAP(0x00A6, 0x26)	/* BROKEN BAR */ \
	    MAP(0x00A7, 0x27)	/* SECTION SIGN */ \
	    MAP(0x00A8, 0x28)	/* DIAERESIS */ \
	    MAP(0x00A9, 0x29)	/* COPYRIGHT SIGN */ \
	    MAP(0x00AA, 0x2A)	/* FEMININE ORDINAL INDICATOR */ \
	    MAP(0x00AB, 0x2B)	/* LEFT-POINTING DOUBLE ANGLE QUOTATION MARK */ \
	    MAP(0x00AC, 0x2C)	/* NOT SIGN */ \
	    MAP(0x00AD, 0x2D)	/* SOFT HYPHEN */ \
	    MAP(0x00AE, 0x2E)	/* REGISTERED SIGN */ \
	    MAP(0x00AF, 0x2F)	/* MACRON */ \
	    MAP(0x00B0, 0x30)	/* DEGREE SIGN */ \
	    MAP(0x00B1, 0x31)	/* PLUS-MINUS SIGN */ \
	    MAP(0x00B2, 0x32)	/* SUPERSCRIPT TWO */ \
	    MAP(0x00B3, 0x33)	/* SUPERSCRIPT THREE */ \
	    MAP(0x00B4, 0x34)	/* ACUTE ACCENT */ \
	    MAP(0x00B5, 0x35)	/* MICRO SIGN */ \
	    MAP(0x00B6, 0x36)	/* PILCROW SIGN */ \
	    MAP(0x00B7, 0x37)	/* MIDDLE DOT */ \
	    MAP(0x00B8, 0x38)	/* CEDILLA */ \
	    MAP(0x00B9, 0x39)	/* SUPERSCRIPT ONE */ \
	    MAP(0x00BA, 0x3A)	/* MASCULINE ORDINAL INDICATOR */ \
	    MAP(0x00BB, 0x3B)	/* RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK */ \
	    MAP(0x00BC, 0x3C)	/* VULGAR FRACTION ONE QUARTER */ \
	    MAP(0x00BD, 0x3D)	/* VULGAR FRACTION ONE HALF */ \
	    MAP(0x00BE, 0x3E)	/* VULGAR FRACTION THREE QUARTERS */ \
	    MAP(0x00BF, 0x3F)	/* INVERTED QUESTION MARK */ \
	    MAP(0x00C0, 0x40)	/* LATIN CAPITAL LETTER A WITH GRAVE */ \
	    MAP(0x00C1, 0x41)	/* LATIN CAPITAL LETTER A WITH ACUTE */ \
	    MAP(0x00C2, 0x42)	/* LATIN CAPITAL LETTER A WITH CIRCUMFLEX */ \
	    MAP(0x00C3, 0x43)	/* LATIN CAPITAL LETTER A WITH TILDE */ \
	    MAP(0x00C4, 0x44)	/* LATIN CAPITAL LETTER A WITH DIAERESIS */ \
	    MAP(0x00C5, 0x45)	/* LATIN CAPITAL LETTER A WITH RING ABOVE */ \
	    MAP(0x00C6, 0x46)	/* LATIN CAPITAL LETTER AE */ \
	    MAP(0x00C7, 0x47)	/* LATIN CAPITAL LETTER C WITH CEDILLA */ \
	    MAP(0x00C8, 0x48)	/* LATIN CAPITAL LETTER E WITH GRAVE */ \
	    MAP(0x00C9, 0x49)	/* LATIN CAPITAL LETTER E WITH ACUTE */ \
	    MAP(0x00CA, 0x4A)	/* LATIN CAPITAL LETTER E WITH CIRCUMFLEX */ \
	    MAP(0x00CB, 0x4B)	/* LATIN CAPITAL LETTER E WITH DIAERESIS */ \
	    MAP(0x00CC, 0x4C)	/* LATIN CAPITAL LETTER I WITH GRAVE */ \
	    MAP(0x00CD, 0x4D)	/* LATIN CAPITAL LETTER I WITH ACUTE */ \
	    MAP(0x00CE, 0x4E)	/* LATIN CAPITAL LETTER I WITH CIRCUMFLEX */ \
	    MAP(0x00CF, 0x4F)	/* LATIN CAPITAL LETTER I WITH DIAERESIS */ \
	    MAP(0x00D1, 0x51)	/* LATIN CAPITAL LETTER N WITH TILDE */ \
	    MAP(0x00D2, 0x52)	/* LATIN CAPITAL LETTER O WITH GRAVE */ \
	    MAP(0x00D3, 0x53)	/* LATIN CAPITAL LETTER O WITH ACUTE */ \
	    MAP(0x00D4, 0x54)	/* LATIN CAPITAL LETTER O WITH CIRCUMFLEX */ \
	    MAP(0x00D5, 0x55)	/* LATIN CAPITAL LETTER O WITH TILDE */ \
	    MAP(0x00D6, 0x56)	/* LATIN CAPITAL LETTER O WITH DIAERESIS */ \
	    MAP(0x00D7, 0x57)	/* MULTIPLICATION SIGN */ \
	    MAP(0x00D8, 0x58)	/* LATIN CAPITAL LETTER O WITH STROKE */ \
	    MAP(0x00D9, 0x59)	/* LATIN CAPITAL LETTER U WITH GRAVE */ \
	    MAP(0x00DA, 0x5A)	/* LATIN CAPITAL LETTER U WITH ACUTE */ \
	    MAP(0x00DB, 0x5B)	/* LATIN CAPITAL LETTER U WITH CIRCUMFLEX */ \
	    MAP(0x00DC, 0x5C)	/* LATIN CAPITAL LETTER U WITH DIAERESIS */ \
	    MAP(0x00DF, 0x5F)	/* LATIN SMALL LETTER SHARP S */ \
	    MAP(0x00E0, 0x60)	/* LATIN SMALL LETTER A WITH GRAVE */ \
	    MAP(0x00E1, 0x61)	/* LATIN SMALL LETTER A WITH ACUTE */ \
	    MAP(0x00E2, 0x62)	/* LATIN SMALL LETTER A WITH CIRCUMFLEX */ \
	    MAP(0x00E3, 0x63)	/* LATIN SMALL LETTER A WITH TILDE */ \
	    MAP(0x00E4, 0x64)	/* LATIN SMALL LETTER A WITH DIAERESIS */ \
	    MAP(0x00E5, 0x65)	/* LATIN SMALL LETTER A WITH RING ABOVE */ \
	    MAP(0x00E6, 0x66)	/* LATIN SMALL LETTER AE */ \
	    MAP(0x00E7, 0x67)	/* LATIN SMALL LETTER C WITH CEDILLA */ \
	    MAP(0x00E8, 0x68)	/* LATIN SMALL LETTER E WITH GRAVE */ \
	    MAP(0x00E9, 0x69)	/* LATIN SMALL LETTER E WITH ACUTE */ \
	    MAP(0x00EA, 0x6A)	/* LATIN SMALL LETTER E WITH CIRCUMFLEX */ \
	    MAP(0x00EB, 0x6B)	/* LATIN SMALL LETTER E WITH DIAERESIS */ \
	    MAP(0x00EC, 0x6C)	/* LATIN SMALL LETTER I WITH GRAVE */ \
	    MAP(0x00ED, 0x6D)	/* LATIN SMALL LETTER I WITH ACUTE */ \
	    MAP(0x00EE, 0x6E)	/* LATIN SMALL LETTER I WITH CIRCUMFLEX */ \
	    MAP(0x00EF, 0x6F)	/* LATIN SMALL LETTER I WITH DIAERESIS */ \
	    MAP(0x00F1, 0x71)	/* LATIN SMALL LETTER N WITH TILDE */ \
	    MAP(0x00F2, 0x72)	/* LATIN SMALL LETTER O WITH GRAVE */ \
	    MAP(0x00F3, 0x73)	/* LATIN SMALL LETTER O WITH ACUTE */ \
	    MAP(0x00F4, 0x74)	/* LATIN SMALL LETTER O WITH CIRCUMFLEX */ \
	    MAP(0x00F5, 0x75)	/* LATIN SMALL LETTER O WITH TILDE */ \
	    MAP(0x00F6, 0x76)	/* LATIN SMALL LETTER O WITH DIAERESIS */ \
	    MAP(0x00F7, 0x77)	/* DIVISION SIGN */ \
	    MAP(0x00F8, 0x78)	/* LATIN SMALL LETTER O WITH STROKE */ \
	    MAP(0x00F9, 0x79)	/* LATIN SMALL LETTER U WITH GRAVE */ \
	    MAP(0x00FA, 0x7A)	/* LATIN SMALL LETTER U WITH ACUTE */ \
	    MAP(0x00FB, 0x7B)	/* LATIN SMALL LETTER U WITH CIRCUMFLEX */ \
	    MAP(0x00FC, 0x7C)	/* LATIN SMALL LETTER U WITH DIAERESIS */ \
	    MAP(0x00FF, 0x7F)	/* LATIN SMALL LETTER Y WITH DIAERESIS */ \
	    MAP(0x011E, 0x50)	/* LATIN CAPITAL LETTER G WITH BREVE */ \
	    MAP(0x011F, 0x70)	/* LATIN SMALL LETTER G WITH BREVE */ \
	    MAP(0x0130, 0x5D)	/* LATIN CAPITAL LETTER I WITH DOT ABOVE */ \
	    MAP(0x0131, 0x7D)	/* LATIN SMALL LETTER DOTLESS I */ \
	    MAP(0x015E, 0x5E)	/* LATIN CAPITAL LETTER S WITH CEDILLA */ \
	    MAP(0x015F, 0x7E)	/* LATIN SMALL LETTER S WITH CEDILLA */ \
	}

/*
 * DEC Cyrillic from screenshot
 */
#define map_DEC_Cyrillic(code) \
	begin_CODEPAGE(94) \
	switch (code) { \
	    UNI(0x20, 0x00A0)	/* NO-BREAK SPACE */ \
	    XXX(0x21, UNDEF)	/* undefined */ \
	    XXX(0x22, UNDEF)	/* undefined */ \
	    XXX(0x23, UNDEF)	/* undefined */ \
	    XXX(0x24, UNDEF)	/* undefined */ \
	    XXX(0x25, UNDEF)	/* undefined */ \
	    XXX(0x26, UNDEF)	/* undefined */ \
	    XXX(0x27, UNDEF)	/* undefined */ \
	    XXX(0x28, UNDEF)	/* undefined */ \
	    XXX(0x29, UNDEF)	/* undefined */ \
	    XXX(0x2A, UNDEF)	/* undefined */ \
	    XXX(0x2B, UNDEF)	/* undefined */ \
	    XXX(0x2C, UNDEF)	/* undefined */ \
	    XXX(0x2D, UNDEF)	/* undefined */ \
	    XXX(0x2E, UNDEF)	/* undefined */ \
	    XXX(0x2F, UNDEF)	/* undefined */ \
	    XXX(0x30, UNDEF)	/* undefined */ \
	    XXX(0x31, UNDEF)	/* undefined */ \
	    XXX(0x32, UNDEF)	/* undefined */ \
	    XXX(0x33, UNDEF)	/* undefined */ \
	    XXX(0x34, UNDEF)	/* undefined */ \
	    XXX(0x35, UNDEF)	/* undefined */ \
	    XXX(0x36, UNDEF)	/* undefined */ \
	    XXX(0x37, UNDEF)	/* undefined */ \
	    XXX(0x38, UNDEF)	/* undefined */ \
	    XXX(0x39, UNDEF)	/* undefined */ \
	    XXX(0x3A, UNDEF)	/* undefined */ \
	    XXX(0x3B, UNDEF)	/* undefined */ \
	    XXX(0x3C, UNDEF)	/* undefined */ \
	    XXX(0x3D, UNDEF)	/* undefined */ \
	    XXX(0x3E, UNDEF)	/* undefined */ \
	    XXX(0x3F, UNDEF)	/* undefined */ \
	    UNI(0x40, 0x044E)	/* CYRILLIC SMALL LETTER YU */ \
	    UNI(0x41, 0x0430)	/* CYRILLIC SMALL LETTER A */ \
	    UNI(0x42, 0x0431)	/* CYRILLIC SMALL LETTER BE */ \
	    UNI(0x43, 0x0446)	/* CYRILLIC SMALL LETTER TSE */ \
	    UNI(0x44, 0x0434)	/* CYRILLIC SMALL LETTER DE */ \
	    UNI(0x45, 0x0435)	/* CYRILLIC SMALL LETTER IE */ \
	    UNI(0x46, 0x0444)	/* CYRILLIC SMALL LETTER EF */ \
	    UNI(0x47, 0x0433)	/* CYRILLIC SMALL LETTER GHE */ \
	    UNI(0x48, 0x0445)	/* CYRILLIC SMALL LETTER HA */ \
	    UNI(0x49, 0x0438)	/* CYRILLIC SMALL LETTER I */ \
	    UNI(0x4A, 0x0439)	/* CYRILLIC SMALL LETTER SHORT I */ \
	    UNI(0x4B, 0x043A)	/* CYRILLIC SMALL LETTER KA */ \
	    UNI(0x4C, 0x043B)	/* CYRILLIC SMALL LETTER EL */ \
	    UNI(0x4D, 0x043C)	/* CYRILLIC SMALL LETTER EM */ \
	    UNI(0x4E, 0x043D)	/* CYRILLIC SMALL LETTER EN */ \
	    UNI(0x4F, 0x043E)	/* CYRILLIC SMALL LETTER O */ \
	    UNI(0x50, 0x043F)	/* CYRILLIC SMALL LETTER PE */ \
	    UNI(0x51, 0x044F)	/* CYRILLIC SMALL LETTER YA */ \
	    UNI(0x52, 0x0440)	/* CYRILLIC SMALL LETTER ER */ \
	    UNI(0x53, 0x0441)	/* CYRILLIC SMALL LETTER ES */ \
	    UNI(0x54, 0x0442)	/* CYRILLIC SMALL LETTER TE */ \
	    UNI(0x55, 0x0443)	/* CYRILLIC SMALL LETTER U */ \
	    UNI(0x56, 0x0436)	/* CYRILLIC SMALL LETTER ZHE */ \
	    UNI(0x57, 0x0432)	/* CYRILLIC SMALL LETTER VE */ \
	    UNI(0x58, 0x044C)	/* CYRILLIC SMALL LETTER SOFT SIGN */ \
	    UNI(0x59, 0x044B)	/* CYRILLIC SMALL LETTER YERU */ \
	    UNI(0x5A, 0x0437)	/* CYRILLIC SMALL LETTER ZE */ \
	    UNI(0x5B, 0x0448)	/* CYRILLIC SMALL LETTER SHA */ \
	    UNI(0x5C, 0x044D)	/* CYRILLIC SMALL LETTER E */ \
	    UNI(0x5D, 0x0449)	/* CYRILLIC SMALL LETTER SHCHA */ \
	    UNI(0x5E, 0x0447)	/* CYRILLIC SMALL LETTER CHE */ \
	    UNI(0x5F, 0x044A)	/* CYRILLIC SMALL LETTER HARD SIGN */ \
	    UNI(0x60, 0x042E)	/* CYRILLIC CAPITAL LETTER YU */ \
	    UNI(0x61, 0x0410)	/* CYRILLIC CAPITAL LETTER A */ \
	    UNI(0x62, 0x0411)	/* CYRILLIC CAPITAL LETTER BE */ \
	    UNI(0x63, 0x0426)	/* CYRILLIC CAPITAL LETTER TSE */ \
	    UNI(0x64, 0x0414)	/* CYRILLIC CAPITAL LETTER DE */ \
	    UNI(0x65, 0x0415)	/* CYRILLIC CAPITAL LETTER IE */ \
	    UNI(0x66, 0x0424)	/* CYRILLIC CAPITAL LETTER EF */ \
	    UNI(0x67, 0x0413)	/* CYRILLIC CAPITAL LETTER GHE */ \
	    UNI(0x68, 0x0425)	/* CYRILLIC CAPITAL LETTER HA */ \
	    UNI(0x69, 0x0418)	/* CYRILLIC CAPITAL LETTER I */ \
	    UNI(0x6A, 0x0419)	/* CYRILLIC CAPITAL LETTER SHORT I */ \
	    UNI(0x6B, 0x041A)	/* CYRILLIC CAPITAL LETTER KA */ \
	    UNI(0x6C, 0x041B)	/* CYRILLIC CAPITAL LETTER EL */ \
	    UNI(0x6D, 0x041C)	/* CYRILLIC CAPITAL LETTER EM */ \
	    UNI(0x6E, 0x041D)	/* CYRILLIC CAPITAL LETTER EN */ \
	    UNI(0x6F, 0x041E)	/* CYRILLIC CAPITAL LETTER O */ \
	    UNI(0x70, 0x041F)	/* CYRILLIC CAPITAL LETTER PE */ \
	    UNI(0x71, 0x042F)	/* CYRILLIC CAPITAL LETTER YA */ \
	    UNI(0x72, 0x0420)	/* CYRILLIC CAPITAL LETTER ER */ \
	    UNI(0x73, 0x0421)	/* CYRILLIC CAPITAL LETTER ES */ \
	    UNI(0x74, 0x0422)	/* CYRILLIC CAPITAL LETTER TE */ \
	    UNI(0x75, 0x0423)	/* CYRILLIC CAPITAL LETTER U */ \
	    UNI(0x76, 0x0416)	/* CYRILLIC CAPITAL LETTER ZHE */ \
	    UNI(0x77, 0x0412)	/* CYRILLIC CAPITAL LETTER VE */ \
	    UNI(0x78, 0x042C)	/* CYRILLIC CAPITAL LETTER SOFT SIGN */ \
	    UNI(0x79, 0x042B)	/* CYRILLIC CAPITAL LETTER YERU */ \
	    UNI(0x7A, 0x0417)	/* CYRILLIC CAPITAL LETTER ZE */ \
	    UNI(0x7B, 0x0428)	/* CYRILLIC CAPITAL LETTER SHA */ \
	    UNI(0x7C, 0x042D)	/* CYRILLIC CAPITAL LETTER E */ \
	    UNI(0x7D, 0x0429)	/* CYRILLIC CAPITAL LETTER SHCHA */ \
	    UNI(0x7E, 0x0427)	/* CYRILLIC CAPITAL LETTER CHE */ \
	    XXX(0x7F, UNDEF) \
	} \
	end_CODEPAGE()

#define unmap_DEC_Cyrillic(code) \
	switch (code) { \
	    MAP(0x00A0, 0x20)	/* NO-BREAK SPACE */ \
	    MAP(0x0410, 0x61)	/* CYRILLIC CAPITAL LETTER A */ \
	    MAP(0x0411, 0x62)	/* CYRILLIC CAPITAL LETTER BE */ \
	    MAP(0x0412, 0x77)	/* CYRILLIC CAPITAL LETTER VE */ \
	    MAP(0x0413, 0x67)	/* CYRILLIC CAPITAL LETTER GHE */ \
	    MAP(0x0414, 0x64)	/* CYRILLIC CAPITAL LETTER DE */ \
	    MAP(0x0415, 0x65)	/* CYRILLIC CAPITAL LETTER IE */ \
	    MAP(0x0416, 0x76)	/* CYRILLIC CAPITAL LETTER ZHE */ \
	    MAP(0x0417, 0x7A)	/* CYRILLIC CAPITAL LETTER ZE */ \
	    MAP(0x0418, 0x69)	/* CYRILLIC CAPITAL LETTER I */ \
	    MAP(0x0419, 0x6A)	/* CYRILLIC CAPITAL LETTER SHORT I */ \
	    MAP(0x041A, 0x6B)	/* CYRILLIC CAPITAL LETTER KA */ \
	    MAP(0x041B, 0x6C)	/* CYRILLIC CAPITAL LETTER EL */ \
	    MAP(0x041C, 0x6D)	/* CYRILLIC CAPITAL LETTER EM */ \
	    MAP(0x041D, 0x6E)	/* CYRILLIC CAPITAL LETTER EN */ \
	    MAP(0x041E, 0x6F)	/* CYRILLIC CAPITAL LETTER O */ \
	    MAP(0x041F, 0x70)	/* CYRILLIC CAPITAL LETTER PE */ \
	    MAP(0x0420, 0x72)	/* CYRILLIC CAPITAL LETTER ER */ \
	    MAP(0x0421, 0x73)	/* CYRILLIC CAPITAL LETTER ES */ \
	    MAP(0x0422, 0x74)	/* CYRILLIC CAPITAL LETTER TE */ \
	    MAP(0x0423, 0x75)	/* CYRILLIC CAPITAL LETTER U */ \
	    MAP(0x0424, 0x66)	/* CYRILLIC CAPITAL LETTER EF */ \
	    MAP(0x0425, 0x68)	/* CYRILLIC CAPITAL LETTER HA */ \
	    MAP(0x0426, 0x63)	/* CYRILLIC CAPITAL LETTER TSE */ \
	    MAP(0x0427, 0x7E)	/* CYRILLIC CAPITAL LETTER CHE */ \
	    MAP(0x0428, 0x7B)	/* CYRILLIC CAPITAL LETTER SHA */ \
	    MAP(0x0429, 0x7D)	/* CYRILLIC CAPITAL LETTER SHCHA */ \
	    MAP(0x042B, 0x79)	/* CYRILLIC CAPITAL LETTER YERU */ \
	    MAP(0x042C, 0x78)	/* CYRILLIC CAPITAL LETTER SOFT SIGN */ \
	    MAP(0x042D, 0x7C)	/* CYRILLIC CAPITAL LETTER E */ \
	    MAP(0x042E, 0x60)	/* CYRILLIC CAPITAL LETTER YU */ \
	    MAP(0x042F, 0x71)	/* CYRILLIC CAPITAL LETTER YA */ \
	    MAP(0x0430, 0x41)	/* CYRILLIC SMALL LETTER A */ \
	    MAP(0x0431, 0x42)	/* CYRILLIC SMALL LETTER BE */ \
	    MAP(0x0432, 0x57)	/* CYRILLIC SMALL LETTER VE */ \
	    MAP(0x0433, 0x47)	/* CYRILLIC SMALL LETTER GHE */ \
	    MAP(0x0434, 0x44)	/* CYRILLIC SMALL LETTER DE */ \
	    MAP(0x0435, 0x45)	/* CYRILLIC SMALL LETTER IE */ \
	    MAP(0x0436, 0x56)	/* CYRILLIC SMALL LETTER ZHE */ \
	    MAP(0x0437, 0x5A)	/* CYRILLIC SMALL LETTER ZE */ \
	    MAP(0x0438, 0x49)	/* CYRILLIC SMALL LETTER I */ \
	    MAP(0x0439, 0x4A)	/* CYRILLIC SMALL LETTER SHORT I */ \
	    MAP(0x043A, 0x4B)	/* CYRILLIC SMALL LETTER KA */ \
	    MAP(0x043B, 0x4C)	/* CYRILLIC SMALL LETTER EL */ \
	    MAP(0x043C, 0x4D)	/* CYRILLIC SMALL LETTER EM */ \
	    MAP(0x043D, 0x4E)	/* CYRILLIC SMALL LETTER EN */ \
	    MAP(0x043E, 0x4F)	/* CYRILLIC SMALL LETTER O */ \
	    MAP(0x043F, 0x50)	/* CYRILLIC SMALL LETTER PE */ \
	    MAP(0x0440, 0x52)	/* CYRILLIC SMALL LETTER ER */ \
	    MAP(0x0441, 0x53)	/* CYRILLIC SMALL LETTER ES */ \
	    MAP(0x0442, 0x54)	/* CYRILLIC SMALL LETTER TE */ \
	    MAP(0x0443, 0x55)	/* CYRILLIC SMALL LETTER U */ \
	    MAP(0x0444, 0x46)	/* CYRILLIC SMALL LETTER EF */ \
	    MAP(0x0445, 0x48)	/* CYRILLIC SMALL LETTER HA */ \
	    MAP(0x0446, 0x43)	/* CYRILLIC SMALL LETTER TSE */ \
	    MAP(0x0447, 0x5E)	/* CYRILLIC SMALL LETTER CHE */ \
	    MAP(0x0448, 0x5B)	/* CYRILLIC SMALL LETTER SHA */ \
	    MAP(0x0449, 0x5D)	/* CYRILLIC SMALL LETTER SHCHA */ \
	    MAP(0x044A, 0x5F)	/* CYRILLIC SMALL LETTER HARD SIGN */ \
	    MAP(0x044B, 0x59)	/* CYRILLIC SMALL LETTER YERU */ \
	    MAP(0x044C, 0x58)	/* CYRILLIC SMALL LETTER SOFT SIGN */ \
	    MAP(0x044D, 0x5C)	/* CYRILLIC SMALL LETTER E */ \
	    MAP(0x044E, 0x40)	/* CYRILLIC SMALL LETTER YU */ \
	    MAP(0x044F, 0x51)	/* CYRILLIC SMALL LETTER YA */ \
	    MAP(UNDEF, 0x1B) \
	}

/*
 * figure A-24 "DEC Greek Supplemental Character Set"
 */
#define map_DEC_Greek_Supp(code) \
	begin_CODEPAGE(94) \
	switch (code) { \
	    UNI(0x20, 0x00A0)	/* NO-BREAK SPACE */ \
	    UNI(0x21, 0x00A1)	/* LEFT SINGLE QUOTATION MARK */ \
	    UNI(0x22, 0x00A2)	/* RIGHT SINGLE QUOTATION MARK */ \
	    UNI(0x23, 0x00A3)	/* POUND SIGN */ \
	    XXX(0x24, UNDEF)	/* EURO SIGN */ \
	    UNI(0x25, 0x00A5)	/* YEN SIGN */ \
	    XXX(0x26, UNDEF)	/* BROKEN BAR */ \
	    UNI(0x27, 0x00A7)	/* SECTION SIGN */ \
	    UNI(0x28, 0x00A4)	/* CURRENCY SIGN */ \
	    UNI(0x29, 0x00A9)	/* COPYRIGHT SIGN */ \
	    UNI(0x2A, 0x00AA)	/* FEMININE ORDINAL INDICATOR */ \
	    UNI(0x2B, 0x00AB)	/* LEFT-POINTING DOUBLE ANGLE QUOTATION MARK */ \
	    XXX(0x2C, UNDEF)	/* reserved */ \
	    XXX(0x2D, UNDEF)	/* reserved */ \
	    XXX(0x2E, UNDEF)	/* reserved */ \
	    XXX(0x2F, UNDEF)	/* reserved */ \
	    UNI(0x30, 0x00B0)	/* DEGREE SIGN */ \
	    UNI(0x31, 0x00B1)	/* PLUS-MINUS SIGN */ \
	    UNI(0x32, 0x00B2)	/* SUPERSCRIPT TWO */ \
	    UNI(0x33, 0x00B3)	/* SUPERSCRIPT THREE */ \
	    XXX(0x34, UNDEF)	/* reserved */ \
	    UNI(0x35, 0x00B5)	/* MICRO SIGN */ \
	    UNI(0x36, 0x00B6)	/* PILCROW SIGN */ \
	    UNI(0x37, 0x00B7)	/* MIDDLE DOT */ \
	    XXX(0x38, UNDEF)	/* reserved */ \
	    UNI(0x39, 0x00B9)	/* SUPERSCRIPT ONE */ \
	    UNI(0x3A, 0x00BA)	/* MASCULINE ORDINAL INDICATOR */ \
	    UNI(0x3B, 0x00BB)	/* RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK */ \
	    UNI(0x3C, 0x00BC)	/* VULGAR FRACTION ONE QUARTER */ \
	    UNI(0x3D, 0x00BD)	/* VULGAR FRACTION ONE HALF */ \
	    XXX(0x3E, UNDEF)	/* reserved */ \
	    UNI(0x3F, 0x00BF)	/* INVERTED QUESTION MARK */ \
	    UNI(0x40, 0x03CA)	/* GREEK SMALL LETTER IOTA WITH DIALYTIKA */ \
	    UNI(0x41, 0x0391)	/* GREEK CAPITAL LETTER ALPHA */ \
	    UNI(0x42, 0x0392)	/* GREEK CAPITAL LETTER BETA */ \
	    UNI(0x43, 0x0393)	/* GREEK CAPITAL LETTER GAMMA */ \
	    UNI(0x44, 0x0394)	/* GREEK CAPITAL LETTER DELTA */ \
	    UNI(0x45, 0x0395)	/* GREEK CAPITAL LETTER EPSILON */ \
	    UNI(0x46, 0x0396)	/* GREEK CAPITAL LETTER ZETA */ \
	    UNI(0x47, 0x0397)	/* GREEK CAPITAL LETTER ETA */ \
	    UNI(0x48, 0x0398)	/* GREEK CAPITAL LETTER THETA */ \
	    UNI(0x49, 0x0399)	/* GREEK CAPITAL LETTER IOTA */ \
	    UNI(0x4A, 0x039A)	/* GREEK CAPITAL LETTER KAPPA */ \
	    UNI(0x4B, 0x039B)	/* GREEK CAPITAL LETTER LAMDA */ \
	    UNI(0x4C, 0x039C)	/* GREEK CAPITAL LETTER MU */ \
	    UNI(0x4D, 0x039D)	/* GREEK CAPITAL LETTER NU */ \
	    UNI(0x4E, 0x039E)	/* GREEK CAPITAL LETTER XI */ \
	    UNI(0x4F, 0x039F)	/* GREEK CAPITAL LETTER OMICRON */ \
	    XXX(0x50, UNDEF)	/* reserved */ \
	    UNI(0x51, 0x03A0)	/* GREEK CAPITAL LETTER PI */ \
	    UNI(0x52, 0x03A1)	/* GREEK CAPITAL LETTER RHO */ \
	    UNI(0x53, 0x03A3)	/* GREEK CAPITAL LETTER SIGMA */ \
	    UNI(0x54, 0x03A4)	/* GREEK CAPITAL LETTER TAU */ \
	    UNI(0x55, 0x03A5)	/* GREEK CAPITAL LETTER UPSILON */ \
	    UNI(0x56, 0x03A6)	/* GREEK CAPITAL LETTER PHI */ \
	    UNI(0x57, 0x03A7)	/* GREEK CAPITAL LETTER CHI */ \
	    UNI(0x58, 0x03A8)	/* GREEK CAPITAL LETTER PSI */ \
	    UNI(0x59, 0x03A9)	/* GREEK CAPITAL LETTER OMEGA */ \
	    UNI(0x5A, 0x03AC)	/* GREEK SMALL LETTER ALPHA WITH TONOS */ \
	    UNI(0x5B, 0x03AD)	/* GREEK SMALL LETTER EPSILON WITH TONOS */ \
	    UNI(0x5C, 0x03AE)	/* GREEK SMALL LETTER ETA WITH TONOS */ \
	    UNI(0x5D, 0x03AF)	/* GREEK SMALL LETTER IOTA WITH TONOS */ \
	    XXX(0x5E, UNDEF)	/* reserved */ \
	    UNI(0x5F, 0x03CC)	/* GREEK SMALL LETTER OMICRON WITH TONOS */ \
	    UNI(0x60, 0x03CB)	/* GREEK SMALL LETTER UPSILON WITH DIALYTIKA */ \
	    UNI(0x61, 0x03B1)	/* GREEK SMALL LETTER ALPHA */ \
	    UNI(0x62, 0x03B2)	/* GREEK SMALL LETTER BETA */ \
	    UNI(0x63, 0x03B3)	/* GREEK SMALL LETTER GAMMA */ \
	    UNI(0x64, 0x03B4)	/* GREEK SMALL LETTER DELTA */ \
	    UNI(0x65, 0x03B5)	/* GREEK SMALL LETTER EPSILON */ \
	    UNI(0x66, 0x03B6)	/* GREEK SMALL LETTER ZETA */ \
	    UNI(0x67, 0x03B7)	/* GREEK SMALL LETTER ETA */ \
	    UNI(0x68, 0x03B8)	/* GREEK SMALL LETTER THETA */ \
	    UNI(0x69, 0x03B9)	/* GREEK SMALL LETTER IOTA */ \
	    UNI(0x6A, 0x03BA)	/* GREEK SMALL LETTER KAPPA */ \
	    UNI(0x6B, 0x03BB)	/* GREEK SMALL LETTER LAMDA */ \
	    UNI(0x6C, 0x03BC)	/* GREEK SMALL LETTER MU */ \
	    UNI(0x6D, 0x03BD)	/* GREEK SMALL LETTER NU */ \
	    UNI(0x6E, 0x03BE)	/* GREEK SMALL LETTER XI */ \
	    UNI(0x6F, 0x03BF)	/* GREEK SMALL LETTER OMICRON */ \
	    XXX(0x70, UNDEF)	/* reserved */ \
	    UNI(0x71, 0x03C0)	/* GREEK SMALL LETTER PI */ \
	    UNI(0x72, 0x03C1)	/* GREEK SMALL LETTER RHO */ \
	    UNI(0x73, 0x03C3)	/* GREEK SMALL LETTER SIGMA */ \
	    UNI(0x74, 0x03C4)	/* GREEK SMALL LETTER TAU */ \
	    UNI(0x75, 0x03C5)	/* GREEK SMALL LETTER UPSILON */ \
	    UNI(0x76, 0x03C6)	/* GREEK SMALL LETTER PHI */ \
	    UNI(0x77, 0x03C7)	/* GREEK SMALL LETTER CHI */ \
	    UNI(0x78, 0x03C8)	/* GREEK SMALL LETTER PSI */ \
	    UNI(0x79, 0x03C9)	/* GREEK SMALL LETTER OMEGA */ \
	    UNI(0x7A, 0x03C2)	/* GREEK SMALL LETTER FINAL SIGMA */ \
	    UNI(0x7B, 0x03CD)	/* GREEK SMALL LETTER UPSILON WITH TONOS */ \
	    UNI(0x7C, 0x03CE)	/* GREEK SMALL LETTER OMEGA WITH TONOS */ \
	    UNI(0x7D, 0x0384)	/* GREEK TONOS */ \
	    XXX(0x7E, UNDEF)	/* reserved */ \
	} \
	end_CODEPAGE()

#define unmap_DEC_Greek_Supp(code) \
	switch (code) { \
	    MAP(0x00A0, 0x20)	/* NO-BREAK SPACE */ \
	    MAP(0x00A1, 0x21)	/* LEFT SINGLE QUOTATION MARK */ \
	    MAP(0x00A2, 0x22)	/* RIGHT SINGLE QUOTATION MARK */ \
	    MAP(0x00A3, 0x23)	/* POUND SIGN */ \
	    MAP(0x00A4, 0xA4)	/* CURRENCY SIGN */ \
	    MAP(0x00A5, 0x25)	/* YEN SIGN */ \
	    MAP(0x00A7, 0x27)	/* SECTION SIGN */ \
	    MAP(0x00A9, 0x29)	/* COPYRIGHT SIGN */ \
	    MAP(0x00AA, 0x2A)	/* FEMININE ORDINAL INDICATOR */ \
	    MAP(0x00AB, 0x2B)	/* LEFT-POINTING DOUBLE ANGLE QUOTATION MARK */ \
	    MAP(0x00B0, 0x30)	/* DEGREE SIGN */ \
	    MAP(0x00B1, 0x31)	/* PLUS-MINUS SIGN */ \
	    MAP(0x00B2, 0x32)	/* SUPERSCRIPT TWO */ \
	    MAP(0x00B3, 0x33)	/* SUPERSCRIPT THREE */ \
	    MAP(0x00B5, 0x35)	/* MICRO SIGN */ \
	    MAP(0x00B6, 0x36)	/* PILCROW SIGN */ \
	    MAP(0x00B7, 0x37)	/* MIDDLE DOT */ \
	    MAP(0x00B9, 0x39)	/* SUPERSCRIPT ONE */ \
	    MAP(0x00BA, 0x3A)	/* MASCULINE ORDINAL INDICATOR */ \
	    MAP(0x00BB, 0x3B)	/* RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK */ \
	    MAP(0x00BC, 0x3C)	/* VULGAR FRACTION ONE QUARTER */ \
	    MAP(0x00BD, 0x3D)	/* VULGAR FRACTION ONE HALF */ \
	    MAP(0x00BF, 0x3F)	/* INVERTED QUESTION MARK */ \
	    MAP(0x0384, 0x96)	/* GREEK TONOS */ \
	    MAP(0x0391, 0x76)	/* GREEK CAPITAL LETTER ALPHA */ \
	    MAP(0x0392, 0x77)	/* GREEK CAPITAL LETTER BETA */ \
	    MAP(0x0393, 0x78)	/* GREEK CAPITAL LETTER GAMMA */ \
	    MAP(0x0394, 0x79)	/* GREEK CAPITAL LETTER DELTA */ \
	    MAP(0x0395, 0x7A)	/* GREEK CAPITAL LETTER EPSILON */ \
	    MAP(0x0396, 0x7B)	/* GREEK CAPITAL LETTER ZETA */ \
	    MAP(0x0397, 0x7C)	/* GREEK CAPITAL LETTER ETA */ \
	    MAP(0x0398, 0x7D)	/* GREEK CAPITAL LETTER THETA */ \
	    MAP(0x0399, 0x7E)	/* GREEK CAPITAL LETTER IOTA */ \
	    MAP(0x039A, 0x7F)	/* GREEK CAPITAL LETTER KAPPA */ \
	    MAP(0x039B, 0xA2)	/* GREEK CAPITAL LETTER LAMDA */ \
	    MAP(0x039C, 0xA3)	/* GREEK CAPITAL LETTER MU */ \
	    MAP(0x039D, 0xA4)	/* GREEK CAPITAL LETTER NU */ \
	    MAP(0x039E, 0xA5)	/* GREEK CAPITAL LETTER XI */ \
	    MAP(0x039F, 0xA6)	/* GREEK CAPITAL LETTER OMICRON */ \
	    MAP(0x03A0, 0xA7)	/* GREEK CAPITAL LETTER PI */ \
	    MAP(0x03A1, 0xA8)	/* GREEK CAPITAL LETTER RHO */ \
	    MAP(0x03A3, 0xA9)	/* GREEK CAPITAL LETTER SIGMA */ \
	    MAP(0x03A4, 0xAA)	/* GREEK CAPITAL LETTER TAU */ \
	    MAP(0x03A5, 0xAB)	/* GREEK CAPITAL LETTER UPSILON */ \
	    MAP(0x03A6, 0xAC)	/* GREEK CAPITAL LETTER PHI */ \
	    MAP(0x03A7, 0xAD)	/* GREEK CAPITAL LETTER CHI */ \
	    MAP(0x03A8, 0xAE)	/* GREEK CAPITAL LETTER PSI */ \
	    MAP(0x03A9, 0xAF)	/* GREEK CAPITAL LETTER OMEGA */ \
	    MAP(0x03AC, 0xB2)	/* GREEK SMALL LETTER ALPHA WITH TONOS */ \
	    MAP(0x03AD, 0xB3)	/* GREEK SMALL LETTER EPSILON WITH TONOS */ \
	    MAP(0x03AE, 0xB4)	/* GREEK SMALL LETTER ETA WITH TONOS */ \
	    MAP(0x03AF, 0xB5)	/* GREEK SMALL LETTER IOTA WITH TONOS */ \
	    MAP(0x03B1, 0xB7)	/* GREEK SMALL LETTER ALPHA */ \
	    MAP(0x03B2, 0xB8)	/* GREEK SMALL LETTER BETA */ \
	    MAP(0x03B3, 0xB9)	/* GREEK SMALL LETTER GAMMA */ \
	    MAP(0x03B4, 0xBA)	/* GREEK SMALL LETTER DELTA */ \
	    MAP(0x03B5, 0xBB)	/* GREEK SMALL LETTER EPSILON */ \
	    MAP(0x03B6, 0xBC)	/* GREEK SMALL LETTER ZETA */ \
	    MAP(0x03B7, 0xBD)	/* GREEK SMALL LETTER ETA */ \
	    MAP(0x03B8, 0xBE)	/* GREEK SMALL LETTER THETA */ \
	    MAP(0x03B9, 0xBF)	/* GREEK SMALL LETTER IOTA */ \
	    MAP(0x03BA, 0xC0)	/* GREEK SMALL LETTER KAPPA */ \
	    MAP(0x03BB, 0xC1)	/* GREEK SMALL LETTER LAMDA */ \
	    MAP(0x03BC, 0xC2)	/* GREEK SMALL LETTER MU */ \
	    MAP(0x03BD, 0xC3)	/* GREEK SMALL LETTER NU */ \
	    MAP(0x03BE, 0xC4)	/* GREEK SMALL LETTER XI */ \
	    MAP(0x03BF, 0xC5)	/* GREEK SMALL LETTER OMICRON */ \
	    MAP(0x03C0, 0xC6)	/* GREEK SMALL LETTER PI */ \
	    MAP(0x03C1, 0xC7)	/* GREEK SMALL LETTER RHO */ \
	    MAP(0x03C2, 0xC8)	/* GREEK SMALL LETTER FINAL SIGMA */ \
	    MAP(0x03C3, 0xC9)	/* GREEK SMALL LETTER SIGMA */ \
	    MAP(0x03C4, 0xCA)	/* GREEK SMALL LETTER TAU */ \
	    MAP(0x03C5, 0xCB)	/* GREEK SMALL LETTER UPSILON */ \
	    MAP(0x03C6, 0xCC)	/* GREEK SMALL LETTER PHI */ \
	    MAP(0x03C7, 0xCD)	/* GREEK SMALL LETTER CHI */ \
	    MAP(0x03C8, 0xCE)	/* GREEK SMALL LETTER PSI */ \
	    MAP(0x03C9, 0xCF)	/* GREEK SMALL LETTER OMEGA */ \
	    MAP(0x03CA, 0xD0)	/* GREEK SMALL LETTER IOTA WITH DIALYTIKA */ \
	    MAP(0x03CB, 0xD1)	/* GREEK SMALL LETTER UPSILON WITH DIALYTIKA */ \
	    MAP(0x03CC, 0xD2)	/* GREEK SMALL LETTER OMICRON WITH TONOS */ \
	    MAP(0x03CD, 0xD3)	/* GREEK SMALL LETTER UPSILON WITH TONOS */ \
	    MAP(0x03CE, 0xD4)	/* GREEK SMALL LETTER OMEGA WITH TONOS */ \
	    MAP(UNDEF, 0x1B)	/* reserved */ \
	}

/*
 * figure A-22 "DEC Hebrew Supplemental Character Set"
 */
#define map_DEC_Hebrew_Supp(code) \
	begin_CODEPAGE(94) \
	switch (code) { \
	    UNI(0x21, 0x00A1)	/* INVERTED EXCLAMATION MARK */ \
	    UNI(0x22, 0x00A2)	/* CENT SIGN */ \
	    UNI(0x23, 0x00A3)	/* POUND SIGN */ \
	    XXX(0x24, UNDEF)	/* CURRENCY SIGN */ \
	    UNI(0x25, 0x00A5)	/* YEN SIGN */ \
	    XXX(0x26, UNDEF)	/* BROKEN BAR */ \
	    UNI(0x27, 0x00A7)	/* SECTION SIGN */ \
	    UNI(0x28, 0x00A8)	/* DIAERESIS */ \
	    UNI(0x29, 0x00A9)	/* COPYRIGHT SIGN */ \
	    UNI(0x2A, 0x00D7)	/* MULTIPLICATION SIGN */ \
	    UNI(0x2B, 0x00AB)	/* LEFT-POINTING DOUBLE ANGLE QUOTATION MARK */ \
	    XXX(0x2C, UNDEF)	/* NOT SIGN */ \
	    XXX(0x2D, UNDEF)	/* SOFT HYPHEN */ \
	    XXX(0x2E, UNDEF)	/* REGISTERED SIGN */ \
	    XXX(0x2F, UNDEF)	/* MACRON */ \
	    UNI(0x30, 0x00B0)	/* DEGREE SIGN */ \
	    UNI(0x31, 0x00B1)	/* PLUS-MINUS SIGN */ \
	    UNI(0x32, 0x00B2)	/* SUPERSCRIPT TWO */ \
	    UNI(0x33, 0x00B3)	/* SUPERSCRIPT THREE */ \
	    XXX(0x34, UNDEF)	/* ACUTE ACCENT */ \
	    UNI(0x35, 0x00B5)	/* MICRO SIGN */ \
	    UNI(0x36, 0x00B6)	/* PILCROW SIGN */ \
	    UNI(0x37, 0x00B7)	/* MIDDLE DOT */ \
	    XXX(0x38, UNDEF)	/* CEDILLA */ \
	    UNI(0x39, 0x00B9)	/* SUPERSCRIPT ONE */ \
	    UNI(0x3A, 0x00F7)	/* DIVISION SIGN */ \
	    UNI(0x3B, 0x00BB)	/* RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK */ \
	    UNI(0x3C, 0x00BC)	/* VULGAR FRACTION ONE QUARTER */ \
	    UNI(0x3D, 0x00BD)	/* VULGAR FRACTION ONE HALF */ \
	    XXX(0x3E, UNDEF)	/* VULGAR FRACTION THREE QUARTERS */ \
	    UNI(0x3F, 0x00BF)	/* INVERTED QUESTION MARK */ \
	    XXX(0x40, UNDEF)	/* reserved */ \
	    XXX(0x41, UNDEF)	/* reserved */ \
	    XXX(0x42, UNDEF)	/* reserved */ \
	    XXX(0x43, UNDEF)	/* reserved */ \
	    XXX(0x44, UNDEF)	/* reserved */ \
	    XXX(0x45, UNDEF)	/* reserved */ \
	    XXX(0x46, UNDEF)	/* reserved */ \
	    XXX(0x47, UNDEF)	/* reserved */ \
	    XXX(0x48, UNDEF)	/* reserved */ \
	    XXX(0x49, UNDEF)	/* reserved */ \
	    XXX(0x4A, UNDEF)	/* reserved */ \
	    XXX(0x4B, UNDEF)	/* reserved */ \
	    XXX(0x4C, UNDEF)	/* reserved */ \
	    XXX(0x4D, UNDEF)	/* reserved */ \
	    XXX(0x4E, UNDEF)	/* reserved */ \
	    XXX(0x4F, UNDEF)	/* reserved */ \
	    XXX(0x50, UNDEF)	/* reserved */ \
	    XXX(0x51, UNDEF)	/* reserved */ \
	    XXX(0x52, UNDEF)	/* reserved */ \
	    XXX(0x53, UNDEF)	/* reserved */ \
	    XXX(0x54, UNDEF)	/* reserved */ \
	    XXX(0x55, UNDEF)	/* reserved */ \
	    XXX(0x56, UNDEF)	/* reserved */ \
	    XXX(0x57, UNDEF)	/* reserved */ \
	    XXX(0x58, UNDEF)	/* reserved */ \
	    XXX(0x59, UNDEF)	/* reserved */ \
	    XXX(0x5A, UNDEF)	/* reserved */ \
	    XXX(0x5B, UNDEF)	/* reserved */ \
	    XXX(0x5C, UNDEF)	/* reserved */ \
	    XXX(0x5D, UNDEF)	/* reserved */ \
	    XXX(0x5E, UNDEF)	/* reserved */ \
	    XXX(0x5F, UNDEF)	/* reserved */ \
	    UNI(0x60, 0x05D0)	/* HEBREW LETTER ALEF */ \
	    UNI(0x61, 0x05D1)	/* HEBREW LETTER BET */ \
	    UNI(0x62, 0x05D2)	/* HEBREW LETTER GIMEL */ \
	    UNI(0x63, 0x05D3)	/* HEBREW LETTER DALET */ \
	    UNI(0x64, 0x05D4)	/* HEBREW LETTER HE */ \
	    UNI(0x65, 0x05D5)	/* HEBREW LETTER VAV */ \
	    UNI(0x66, 0x05D6)	/* HEBREW LETTER ZAYIN */ \
	    UNI(0x67, 0x05D7)	/* HEBREW LETTER HET */ \
	    UNI(0x68, 0x05D8)	/* HEBREW LETTER TET */ \
	    UNI(0x69, 0x05D9)	/* HEBREW LETTER YOD */ \
	    UNI(0x6A, 0x05DA)	/* HEBREW LETTER FINAL KAF */ \
	    UNI(0x6B, 0x05DB)	/* HEBREW LETTER KAF */ \
	    UNI(0x6C, 0x05DC)	/* HEBREW LETTER LAMED */ \
	    UNI(0x6D, 0x05DD)	/* HEBREW LETTER FINAL MEM */ \
	    UNI(0x6E, 0x05DE)	/* HEBREW LETTER MEM */ \
	    UNI(0x6F, 0x05DF)	/* HEBREW LETTER FINAL NUN */ \
	    UNI(0x70, 0x05E0)	/* HEBREW LETTER NUN */ \
	    UNI(0x71, 0x05E1)	/* HEBREW LETTER SAMEKH */ \
	    UNI(0x72, 0x05E2)	/* HEBREW LETTER AYIN */ \
	    UNI(0x73, 0x05E3)	/* HEBREW LETTER FINAL PE */ \
	    UNI(0x74, 0x05E4)	/* HEBREW LETTER PE */ \
	    UNI(0x75, 0x05E5)	/* HEBREW LETTER FINAL TSADI */ \
	    UNI(0x76, 0x05E6)	/* HEBREW LETTER TSADI */ \
	    UNI(0x77, 0x05E7)	/* HEBREW LETTER QOF */ \
	    UNI(0x78, 0x05E8)	/* HEBREW LETTER RESH */ \
	    UNI(0x79, 0x05E9)	/* HEBREW LETTER SHIN */ \
	    UNI(0x7A, 0x05EA)	/* HEBREW LETTER TAV */ \
	    XXX(0x7B, UNDEF)	/* reserved */ \
	    XXX(0x7C, UNDEF)	/* reserved */ \
	    XXX(0x7D, UNDEF)	/* reserved */ \
	    XXX(0x7E, UNDEF)	/* reserved */ \
	} \
	end_CODEPAGE()

#define unmap_DEC_Hebrew_Supp(code) \
	switch (code) { \
	    MAP(0x00A1, 0x21)	/* INVERTED EXCLAMATION MARK */ \
	    MAP(0x00A2, 0x22)	/* CENT SIGN */ \
	    MAP(0x00A3, 0x23)	/* POUND SIGN */ \
	    MAP(0x00A5, 0x25)	/* YEN SIGN */ \
	    MAP(0x00A7, 0x27)	/* SECTION SIGN */ \
	    MAP(0x00A8, 0xA4)	/* DIAERESIS */ \
	    MAP(0x00A9, 0x29)	/* COPYRIGHT SIGN */ \
	    MAP(0x00AB, 0x2B)	/* LEFT-POINTING DOUBLE ANGLE QUOTATION MARK */ \
	    MAP(0x00B0, 0x30)	/* DEGREE SIGN */ \
	    MAP(0x00B1, 0x31)	/* PLUS-MINUS SIGN */ \
	    MAP(0x00B2, 0x32)	/* SUPERSCRIPT TWO */ \
	    MAP(0x00B3, 0x33)	/* SUPERSCRIPT THREE */ \
	    MAP(0x00B5, 0x35)	/* MICRO SIGN */ \
	    MAP(0x00B6, 0x36)	/* PILCROW SIGN */ \
	    MAP(0x00B7, 0x37)	/* MIDDLE DOT */ \
	    MAP(0x00B9, 0x39)	/* SUPERSCRIPT ONE */ \
	    MAP(0x00BB, 0x3B)	/* RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK */ \
	    MAP(0x00BC, 0x3C)	/* VULGAR FRACTION ONE QUARTER */ \
	    MAP(0x00BD, 0x3D)	/* VULGAR FRACTION ONE HALF */ \
	    MAP(0x00BF, 0x3F)	/* INVERTED QUESTION MARK */ \
	    MAP(0x00D7, 0x2A)	/* MULTIPLICATION SIGN */ \
	    MAP(0x00F7, 0x3A)	/* DIVISION SIGN */ \
	    MAP(0x05D0, 0x53)	/* HEBREW LETTER ALEF */ \
	    MAP(0x05D1, 0x54)	/* HEBREW LETTER BET */ \
	    MAP(0x05D2, 0x55)	/* HEBREW LETTER GIMEL */ \
	    MAP(0x05D3, 0x56)	/* HEBREW LETTER DALET */ \
	    MAP(0x05D4, 0x57)	/* HEBREW LETTER HE */ \
	    MAP(0x05D5, 0x58)	/* HEBREW LETTER VAV */ \
	    MAP(0x05D6, 0x59)	/* HEBREW LETTER ZAYIN */ \
	    MAP(0x05D7, 0x5A)	/* HEBREW LETTER HET */ \
	    MAP(0x05D8, 0x5B)	/* HEBREW LETTER TET */ \
	    MAP(0x05D9, 0x5C)	/* HEBREW LETTER YOD */ \
	    MAP(0x05DA, 0x5D)	/* HEBREW LETTER FINAL KAF */ \
	    MAP(0x05DB, 0x5E)	/* HEBREW LETTER KAF */ \
	    MAP(0x05DC, 0x5F)	/* HEBREW LETTER LAMED */ \
	    MAP(0x05DD, 0x60)	/* HEBREW LETTER FINAL MEM */ \
	    MAP(0x05DE, 0x61)	/* HEBREW LETTER MEM */ \
	    MAP(0x05DF, 0x62)	/* HEBREW LETTER FINAL NUN */ \
	    MAP(0x05E0, 0x63)	/* HEBREW LETTER NUN */ \
	    MAP(0x05E1, 0x64)	/* HEBREW LETTER SAMEKH */ \
	    MAP(0x05E2, 0x65)	/* HEBREW LETTER AYIN */ \
	    MAP(0x05E3, 0x66)	/* HEBREW LETTER FINAL PE */ \
	    MAP(0x05E4, 0x67)	/* HEBREW LETTER PE */ \
	    MAP(0x05E5, 0x68)	/* HEBREW LETTER FINAL TSADI */ \
	    MAP(0x05E6, 0x69)	/* HEBREW LETTER TSADI */ \
	    MAP(0x05E7, 0x6A)	/* HEBREW LETTER QOF */ \
	    MAP(0x05E8, 0x6B)	/* HEBREW LETTER RESH */ \
	    MAP(0x05E9, 0x6C)	/* HEBREW LETTER SHIN */ \
	    MAP(0x05EA, 0x6D)	/* HEBREW LETTER TAV */ \
	    MAP(UNDEF, 0x1B)	/* reserved */ \
	}

/*
 * figure A-27 "DEC 8-Bit Turkish Supplemental Character Set"
 */
#define map_DEC_Turkish_Supp(code) \
	begin_CODEPAGE(94) \
	switch (code) { \
	    UNI(0x21, 0x00A1)	/* INVERTED EXCLAMATION MARK */ \
	    UNI(0x22, 0x00A2)	/* CENT SIGN */ \
	    UNI(0x23, 0x00A3)	/* POUND SIGN */ \
	    XXX(0x24, UNDEF)	/* reserved */ \
	    UNI(0x25, 0x00A5)	/* YEN SIGN */ \
	    XXX(0x26, UNDEF)	/* reserved */ \
	    UNI(0x27, 0x00A7)	/* SECTION SIGN */ \
	    UNI(0x28, 0x00A8)	/* DIAERESIS */ \
	    UNI(0x29, 0x00A9)	/* COPYRIGHT SIGN */ \
	    UNI(0x2A, 0x00AA)	/* FEMININE ORDINAL INDICATOR */ \
	    UNI(0x2B, 0x00AB)	/* LEFT-POINTING DOUBLE ANGLE QUOTATION MARK */ \
	    XXX(0x2C, UNDEF)	/* reserved */ \
	    XXX(0x2D, UNDEF)	/* reserved */ \
	    UNI(0x2E, 0x0130)	/* LATIN CAPITAL LETTER I WITH DOT ABOVE */ \
	    XXX(0x2F, UNDEF)	/* reserved */ \
	    UNI(0x30, 0x00B0)	/* DEGREE SIGN */ \
	    UNI(0x31, 0x00B1)	/* PLUS-MINUS SIGN */ \
	    UNI(0x32, 0x00B2)	/* SUPERSCRIPT TWO */ \
	    UNI(0x33, 0x00B3)	/* SUPERSCRIPT THREE */ \
	    XXX(0x34, UNDEF)	/* reserved */ \
	    UNI(0x35, 0x00B5)	/* MICRO SIGN */ \
	    UNI(0x36, 0x00B6)	/* PILCROW SIGN */ \
	    UNI(0x37, 0x00B7)	/* MIDDLE DOT */ \
	    XXX(0x38, UNDEF)	/* reserved */ \
	    UNI(0x39, 0x00B9)	/* SUPERSCRIPT ONE */ \
	    UNI(0x3A, 0x00BA)	/* MASCULINE ORDINAL INDICATOR */ \
	    UNI(0x3B, 0x00BB)	/* RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK */ \
	    UNI(0x3C, 0x00BC)	/* VULGAR FRACTION ONE QUARTER */ \
	    UNI(0x3D, 0x00BD)	/* VULGAR FRACTION ONE HALF */ \
	    UNI(0x3E, 0x0131)	/* LATIN SMALL LETTER DOTLESS I */ \
	    UNI(0x3F, 0x00BF)	/* INVERTED QUESTION MARK */ \
	    UNI(0x40, 0x00C0)	/* LATIN CAPITAL LETTER A WITH GRAVE */ \
	    UNI(0x41, 0x00C1)	/* LATIN CAPITAL LETTER A WITH ACUTE */ \
	    UNI(0x42, 0x00C2)	/* LATIN CAPITAL LETTER A WITH CIRCUMFLEX */ \
	    UNI(0x43, 0x00C3)	/* LATIN CAPITAL LETTER A WITH TILDE */ \
	    UNI(0x44, 0x00C4)	/* LATIN CAPITAL LETTER A WITH DIAERESIS */ \
	    UNI(0x45, 0x00C5)	/* LATIN CAPITAL LETTER A WITH RING ABOVE */ \
	    UNI(0x46, 0x00C6)	/* LATIN CAPITAL LETTER AE */ \
	    UNI(0x47, 0x00C7)	/* LATIN CAPITAL LETTER C WITH CEDILLA */ \
	    UNI(0x48, 0x00C8)	/* LATIN CAPITAL LETTER E WITH GRAVE */ \
	    UNI(0x49, 0x00C9)	/* LATIN CAPITAL LETTER E WITH ACUTE */ \
	    UNI(0x4A, 0x00CA)	/* LATIN CAPITAL LETTER E WITH CIRCUMFLEX */ \
	    UNI(0x4B, 0x00CB)	/* LATIN CAPITAL LETTER E WITH DIAERESIS */ \
	    UNI(0x4C, 0x00CC)	/* LATIN CAPITAL LETTER I WITH GRAVE */ \
	    UNI(0x4D, 0x00CD)	/* LATIN CAPITAL LETTER I WITH ACUTE */ \
	    UNI(0x4E, 0x00CE)	/* LATIN CAPITAL LETTER I WITH CIRCUMFLEX */ \
	    UNI(0x4F, 0x00CF)	/* LATIN CAPITAL LETTER I WITH DIAERESIS */ \
	    UNI(0x50, 0x011E)	/* LATIN CAPITAL LETTER G WITH BREVE */ \
	    UNI(0x51, 0x00D1)	/* LATIN CAPITAL LETTER N WITH TILDE */ \
	    UNI(0x52, 0x00D2)	/* LATIN CAPITAL LETTER O WITH GRAVE */ \
	    UNI(0x53, 0x00D3)	/* LATIN CAPITAL LETTER O WITH ACUTE */ \
	    UNI(0x54, 0x00D4)	/* LATIN CAPITAL LETTER O WITH CIRCUMFLEX */ \
	    UNI(0x55, 0x00D5)	/* LATIN CAPITAL LETTER O WITH TILDE */ \
	    UNI(0x56, 0x00D6)	/* LATIN CAPITAL LETTER O WITH DIAERESIS */ \
	    UNI(0x57, 0x0152)	/* LATIN CAPITAL LIGATURE OE */ \
	    UNI(0x58, 0x00D8)	/* LATIN CAPITAL LETTER O WITH STROKE */ \
	    UNI(0x59, 0x00D9)	/* LATIN CAPITAL LETTER U WITH GRAVE */ \
	    UNI(0x5A, 0x00DA)	/* LATIN CAPITAL LETTER U WITH ACUTE */ \
	    UNI(0x5B, 0x00DB)	/* LATIN CAPITAL LETTER U WITH CIRCUMFLEX */ \
	    UNI(0x5C, 0x00DC)	/* LATIN CAPITAL LETTER U WITH DIAERESIS */ \
	    UNI(0x5D, 0x0178)	/* LATIN CAPITAL LETTER Y WITH DIAERESIS */ \
	    UNI(0x5E, 0x015E)	/* LATIN CAPITAL LETTER S WITH CEDILLA */ \
	    UNI(0x5F, 0x00DF)	/* LATIN SMALL LETTER SHARP S */ \
	    UNI(0x60, 0x00E0)	/* LATIN SMALL LETTER A WITH GRAVE */ \
	    UNI(0x61, 0x00E1)	/* LATIN SMALL LETTER A WITH ACUTE */ \
	    UNI(0x62, 0x00E2)	/* LATIN SMALL LETTER A WITH CIRCUMFLEX */ \
	    UNI(0x63, 0x00E3)	/* LATIN SMALL LETTER A WITH TILDE */ \
	    UNI(0x64, 0x00E4)	/* LATIN SMALL LETTER A WITH DIAERESIS */ \
	    UNI(0x65, 0x00E5)	/* LATIN SMALL LETTER A WITH RING ABOVE */ \
	    UNI(0x66, 0x00E6)	/* LATIN SMALL LETTER AE */ \
	    UNI(0x67, 0x00E7)	/* LATIN SMALL LETTER C WITH CEDILLA */ \
	    UNI(0x68, 0x00E8)	/* LATIN SMALL LETTER E WITH GRAVE */ \
	    UNI(0x69, 0x00E9)	/* LATIN SMALL LETTER E WITH ACUTE */ \
	    UNI(0x6A, 0x00EA)	/* LATIN SMALL LETTER E WITH CIRCUMFLEX */ \
	    UNI(0x6B, 0x00EB)	/* LATIN SMALL LETTER E WITH DIAERESIS */ \
	    UNI(0x6C, 0x00EC)	/* LATIN SMALL LETTER I WITH GRAVE */ \
	    UNI(0x6D, 0x00ED)	/* LATIN SMALL LETTER I WITH ACUTE */ \
	    UNI(0x6E, 0x00EE)	/* LATIN SMALL LETTER I WITH CIRCUMFLEX */ \
	    UNI(0x6F, 0x00EF)	/* LATIN SMALL LETTER I WITH DIAERESIS */ \
	    UNI(0x70, 0x011F)	/* LATIN SMALL LETTER G WITH BREVE */ \
	    UNI(0x71, 0x00F1)	/* LATIN SMALL LETTER N WITH TILDE */ \
	    UNI(0x72, 0x00F2)	/* LATIN SMALL LETTER O WITH GRAVE */ \
	    UNI(0x73, 0x00F3)	/* LATIN SMALL LETTER O WITH ACUTE */ \
	    UNI(0x74, 0x00F4)	/* LATIN SMALL LETTER O WITH CIRCUMFLEX */ \
	    UNI(0x75, 0x00F5)	/* LATIN SMALL LETTER O WITH TILDE */ \
	    UNI(0x76, 0x00F6)	/* LATIN SMALL LETTER O WITH DIAERESIS */ \
	    UNI(0x77, 0x0153)	/* LATIN SMALL LIGATURE OE */ \
	    UNI(0x78, 0x00F8)	/* LATIN SMALL LETTER O WITH STROKE */ \
	    UNI(0x79, 0x00F9)	/* LATIN SMALL LETTER U WITH GRAVE */ \
	    UNI(0x7A, 0x00FA)	/* LATIN SMALL LETTER U WITH ACUTE */ \
	    UNI(0x7B, 0x00FB)	/* LATIN SMALL LETTER U WITH CIRCUMFLEX */ \
	    UNI(0x7C, 0x00FC)	/* LATIN SMALL LETTER U WITH DIAERESIS */ \
	    UNI(0x7D, 0x00FF)	/* LATIN SMALL LETTER Y WITH DIAERESIS */ \
	    UNI(0x7E, 0x015F)	/* LATIN SMALL LETTER S WITH CEDILLA */ \
	} \
	end_CODEPAGE()

#define unmap_DEC_Turkish_Supp(code) \
	switch (code) { \
	    MAP(0x00A1, 0x21)	/* INVERTED EXCLAMATION MARK */ \
	    MAP(0x00A2, 0x22)	/* CENT SIGN */ \
	    MAP(0x00A3, 0x23)	/* POUND SIGN */ \
	    MAP(0x00A5, 0x25)	/* YEN SIGN */ \
	    MAP(0x00A7, 0x27)	/* SECTION SIGN */ \
	    MAP(0x00A8, 0xA4)	/* DIAERESIS */ \
	    MAP(0x00A9, 0x29)	/* COPYRIGHT SIGN */ \
	    MAP(0x00AA, 0x2A)	/* FEMININE ORDINAL INDICATOR */ \
	    MAP(0x00AB, 0x2B)	/* LEFT-POINTING DOUBLE ANGLE QUOTATION MARK */ \
	    MAP(0x00B0, 0x30)	/* DEGREE SIGN */ \
	    MAP(0x00B1, 0x31)	/* PLUS-MINUS SIGN */ \
	    MAP(0x00B2, 0x32)	/* SUPERSCRIPT TWO */ \
	    MAP(0x00B3, 0x33)	/* SUPERSCRIPT THREE */ \
	    MAP(0x00B5, 0x35)	/* MICRO SIGN */ \
	    MAP(0x00B6, 0x36)	/* PILCROW SIGN */ \
	    MAP(0x00B7, 0x37)	/* MIDDLE DOT */ \
	    MAP(0x00B9, 0x39)	/* SUPERSCRIPT ONE */ \
	    MAP(0x00BA, 0x3A)	/* MASCULINE ORDINAL INDICATOR */ \
	    MAP(0x00BB, 0x3B)	/* RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK */ \
	    MAP(0x00BC, 0x3C)	/* VULGAR FRACTION ONE QUARTER */ \
	    MAP(0x00BD, 0x3D)	/* VULGAR FRACTION ONE HALF */ \
	    MAP(0x00BF, 0x3F)	/* INVERTED QUESTION MARK */ \
	    MAP(0x00C0, 0x40)	/* LATIN CAPITAL LETTER A WITH GRAVE */ \
	    MAP(0x00C1, 0x41)	/* LATIN CAPITAL LETTER A WITH ACUTE */ \
	    MAP(0x00C2, 0x42)	/* LATIN CAPITAL LETTER A WITH CIRCUMFLEX */ \
	    MAP(0x00C3, 0x43)	/* LATIN CAPITAL LETTER A WITH TILDE */ \
	    MAP(0x00C4, 0x44)	/* LATIN CAPITAL LETTER A WITH DIAERESIS */ \
	    MAP(0x00C5, 0x45)	/* LATIN CAPITAL LETTER A WITH RING ABOVE */ \
	    MAP(0x00C6, 0x46)	/* LATIN CAPITAL LETTER AE */ \
	    MAP(0x00C7, 0x47)	/* LATIN CAPITAL LETTER C WITH CEDILLA */ \
	    MAP(0x00C8, 0x48)	/* LATIN CAPITAL LETTER E WITH GRAVE */ \
	    MAP(0x00C9, 0x49)	/* LATIN CAPITAL LETTER E WITH ACUTE */ \
	    MAP(0x00CA, 0x4A)	/* LATIN CAPITAL LETTER E WITH CIRCUMFLEX */ \
	    MAP(0x00CB, 0x4B)	/* LATIN CAPITAL LETTER E WITH DIAERESIS */ \
	    MAP(0x00CC, 0x4C)	/* LATIN CAPITAL LETTER I WITH GRAVE */ \
	    MAP(0x00CD, 0x4D)	/* LATIN CAPITAL LETTER I WITH ACUTE */ \
	    MAP(0x00CE, 0x4E)	/* LATIN CAPITAL LETTER I WITH CIRCUMFLEX */ \
	    MAP(0x00CF, 0x4F)	/* LATIN CAPITAL LETTER I WITH DIAERESIS */ \
	    MAP(0x00D1, 0x51)	/* LATIN CAPITAL LETTER N WITH TILDE */ \
	    MAP(0x00D2, 0x52)	/* LATIN CAPITAL LETTER O WITH GRAVE */ \
	    MAP(0x00D3, 0x53)	/* LATIN CAPITAL LETTER O WITH ACUTE */ \
	    MAP(0x00D4, 0x54)	/* LATIN CAPITAL LETTER O WITH CIRCUMFLEX */ \
	    MAP(0x00D5, 0x55)	/* LATIN CAPITAL LETTER O WITH TILDE */ \
	    MAP(0x00D6, 0x56)	/* LATIN CAPITAL LETTER O WITH DIAERESIS */ \
	    MAP(0x00D8, 0x58)	/* LATIN CAPITAL LETTER O WITH STROKE */ \
	    MAP(0x00D9, 0x59)	/* LATIN CAPITAL LETTER U WITH GRAVE */ \
	    MAP(0x00DA, 0x5A)	/* LATIN CAPITAL LETTER U WITH ACUTE */ \
	    MAP(0x00DB, 0x5B)	/* LATIN CAPITAL LETTER U WITH CIRCUMFLEX */ \
	    MAP(0x00DC, 0x5C)	/* LATIN CAPITAL LETTER U WITH DIAERESIS */ \
	    MAP(0x00DF, 0x5F)	/* LATIN SMALL LETTER SHARP S */ \
	    MAP(0x00E0, 0x60)	/* LATIN SMALL LETTER A WITH GRAVE */ \
	    MAP(0x00E1, 0x61)	/* LATIN SMALL LETTER A WITH ACUTE */ \
	    MAP(0x00E2, 0x62)	/* LATIN SMALL LETTER A WITH CIRCUMFLEX */ \
	    MAP(0x00E3, 0x63)	/* LATIN SMALL LETTER A WITH TILDE */ \
	    MAP(0x00E4, 0x64)	/* LATIN SMALL LETTER A WITH DIAERESIS */ \
	    MAP(0x00E5, 0x65)	/* LATIN SMALL LETTER A WITH RING ABOVE */ \
	    MAP(0x00E6, 0x66)	/* LATIN SMALL LETTER AE */ \
	    MAP(0x00E7, 0x67)	/* LATIN SMALL LETTER C WITH CEDILLA */ \
	    MAP(0x00E8, 0x68)	/* LATIN SMALL LETTER E WITH GRAVE */ \
	    MAP(0x00E9, 0x69)	/* LATIN SMALL LETTER E WITH ACUTE */ \
	    MAP(0x00EA, 0x6A)	/* LATIN SMALL LETTER E WITH CIRCUMFLEX */ \
	    MAP(0x00EB, 0x6B)	/* LATIN SMALL LETTER E WITH DIAERESIS */ \
	    MAP(0x00EC, 0x6C)	/* LATIN SMALL LETTER I WITH GRAVE */ \
	    MAP(0x00ED, 0x6D)	/* LATIN SMALL LETTER I WITH ACUTE */ \
	    MAP(0x00EE, 0x6E)	/* LATIN SMALL LETTER I WITH CIRCUMFLEX */ \
	    MAP(0x00EF, 0x6F)	/* LATIN SMALL LETTER I WITH DIAERESIS */ \
	    MAP(0x00F1, 0x71)	/* LATIN SMALL LETTER N WITH TILDE */ \
	    MAP(0x00F2, 0x72)	/* LATIN SMALL LETTER O WITH GRAVE */ \
	    MAP(0x00F3, 0x73)	/* LATIN SMALL LETTER O WITH ACUTE */ \
	    MAP(0x00F4, 0x74)	/* LATIN SMALL LETTER O WITH CIRCUMFLEX */ \
	    MAP(0x00F5, 0x75)	/* LATIN SMALL LETTER O WITH TILDE */ \
	    MAP(0x00F6, 0x76)	/* LATIN SMALL LETTER O WITH DIAERESIS */ \
	    MAP(0x00F8, 0x78)	/* LATIN SMALL LETTER O WITH STROKE */ \
	    MAP(0x00F9, 0x79)	/* LATIN SMALL LETTER U WITH GRAVE */ \
	    MAP(0x00FA, 0x7A)	/* LATIN SMALL LETTER U WITH ACUTE */ \
	    MAP(0x00FB, 0x7B)	/* LATIN SMALL LETTER U WITH CIRCUMFLEX */ \
	    MAP(0x00FC, 0x7C)	/* LATIN SMALL LETTER U WITH DIAERESIS */ \
	    MAP(0x00FF, 0xFF)	/* LATIN SMALL LETTER Y WITH DIAERESIS */ \
	    MAP(0x011E, 0x4E)	/* LATIN CAPITAL LETTER G WITH BREVE */ \
	    MAP(0x011F, 0x50)	/* LATIN SMALL LETTER G WITH BREVE */ \
	    MAP(0x0130, 0x4F)	/* LATIN CAPITAL LETTER I WITH DOT ABOVE */ \
	    MAP(0x0131, 0x51)	/* LATIN SMALL LETTER DOTLESS I */ \
	    MAP(0x0152, 0x97)	/* LATIN CAPITAL LIGATURE OE */ \
	    MAP(0x0153, 0x99)	/* LATIN SMALL LIGATURE OE */ \
	    MAP(0x015E, 0x9F)	/* LATIN CAPITAL LETTER S WITH CEDILLA */ \
	    MAP(0x015F, 0x2A)	/* LATIN SMALL LETTER S WITH CEDILLA */ \
	    MAP(0x0178, 0x98)	/* LATIN CAPITAL LETTER Y WITH DIAERESIS */ \
	    MAP(UNDEF, 0x1B)	/* reserved */ \
	}

/*
 * mentioned, but not documented in VT510 manual, etc., this uses
 * the ELOT927 table from Kermit 95:
 */
#define map_NRCS_Greek(code) \
	switch (code) { \
	    UNI(0x61, 0x0391)	/* CAPITAL GREEK LETTER ALPHA */ \
	    UNI(0x62, 0x0392)	/* CAPITAL GREEK LETTER BETA */ \
	    UNI(0x63, 0x0393)	/* CAPITAL GREEK LETTER GAMMA */ \
	    UNI(0x64, 0x0394)	/* CAPITAL GREEK LETTER DELTA */ \
	    UNI(0x65, 0x0395)	/* CAPITAL GREEK LETTER EPSILON */ \
	    UNI(0x66, 0x0396)	/* CAPITAL GREEK LETTER ZETA */ \
	    UNI(0x67, 0x0397)	/* CAPITAL GREEK LETTER ETA */ \
	    UNI(0x68, 0x0398)	/* CAPITAL GREEK LETTER THETA */ \
	    UNI(0x69, 0x0399)	/* CAPITAL GREEK LETTER IOTA */ \
	    UNI(0x6a, 0x039A)	/* CAPITAL GREEK LETTER KAPPA */ \
	    UNI(0x6b, 0x039B)	/* CAPITAL GREEK LETTER LAMDA */ \
	    UNI(0x6c, 0x039C)	/* CAPITAL GREEK LETTER MU */ \
	    UNI(0x6d, 0x039D)	/* CAPITAL GREEK LETTER NU */ \
	    UNI(0x6e, 0x03A7)	/* CAPITAL GREEK LETTER KSI (CHI) */ \
	    UNI(0x6f, 0x039F)	/* CAPITAL GREEK LETTER OMICRON */ \
	    UNI(0x70, 0x03A0)	/* CAPITAL GREEK LETTER PI */ \
	    UNI(0x71, 0x03A1)	/* CAPITAL GREEK LETTER RHO */ \
	    UNI(0x72, 0x03A3)	/* CAPITAL GREEK LETTER SIGMA */ \
	    UNI(0x73, 0x03A4)	/* CAPITAL GREEK LETTER TAU */ \
	    UNI(0x74, 0x03A5)	/* CAPITAL GREEK LETTER UPSILON */ \
	    UNI(0x75, 0x03A6)	/* CAPITAL GREEK LETTER FI (PHI) */ \
	    UNI(0x76, 0x039E)	/* CAPITAL GREEK LETTER XI */ \
	    UNI(0x77, 0x03A8)	/* CAPITAL GREEK LETTER PSI */ \
	    UNI(0x78, 0x03A9)	/* CAPITAL GREEK LETTER OMEGA */ \
	    XXX(0x79, UNDEF)	/* unused */ \
	    XXX(0x7a, UNDEF)	/* unused */ \
	}

#define unmap_NRCS_Greek(code) \
	switch (code) { \
	    MAP(0x0391,           0x61) /* CAPITAL GREEK LETTER ALPHA */ \
	    MAP(0x0392,           0x62) /* CAPITAL GREEK LETTER BETA */ \
	    MAP(0x0393,           0x63) /* CAPITAL GREEK LETTER GAMMA */ \
	    MAP(0x0394,           0x64) /* CAPITAL GREEK LETTER DELTA */ \
	    MAP(0x0395,           0x65) /* CAPITAL GREEK LETTER EPSILON */ \
	    MAP(0x0396,           0x66) /* CAPITAL GREEK LETTER ZETA */ \
	    MAP(0x0397,           0x67) /* CAPITAL GREEK LETTER ETA */ \
	    MAP(0x0398,           0x68) /* CAPITAL GREEK LETTER THETA */ \
	    MAP(0x0399,           0x69) /* CAPITAL GREEK LETTER IOTA */ \
	    MAP(0x039A,           0x6a) /* CAPITAL GREEK LETTER KAPPA */ \
	    MAP(0x039B,           0x6b) /* CAPITAL GREEK LETTER LAMDA */ \
	    MAP(0x039C,           0x6c) /* CAPITAL GREEK LETTER MU */ \
	    MAP(0x039D,           0x6d) /* CAPITAL GREEK LETTER NU */ \
	    MAP(0x039E,           0x76) /* CAPITAL GREEK LETTER XI */ \
	    MAP(0x039F,           0x6f) /* CAPITAL GREEK LETTER OMICRON */ \
	    MAP(0x03A0,           0x70) /* CAPITAL GREEK LETTER PI */ \
	    MAP(0x03A1,           0x71) /* CAPITAL GREEK LETTER RHO */ \
	    MAP(0x03A3,           0x72) /* CAPITAL GREEK LETTER SIGMA */ \
	    MAP(0x03A4,           0x73) /* CAPITAL GREEK LETTER TAU */ \
	    MAP(0x03A5,           0x74) /* CAPITAL GREEK LETTER UPSILON */ \
	    MAP(0x03A6,           0x75) /* CAPITAL GREEK LETTER FI (PHI) */ \
	    MAP(0x03A7,           0x6e) /* CAPITAL GREEK LETTER KSI (CHI) */ \
	    MAP(0x03A8,           0x77) /* CAPITAL GREEK LETTER PSI */ \
	    MAP(0x03A9,           0x78) /* CAPITAL GREEK LETTER OMEGA */ \
	    MAP(UNDEF,            0x1B) /* unused */ \
	}

/*
 * figure A-21 "DEC 7-Bit Hebrew Character Set"
 */
#define map_NRCS_Hebrew(code) \
	switch (code) { \
	    UNI(0x60, 0x05D0)	/* HEBREW LETTER ALEF */ \
	    UNI(0x61, 0x05D1)	/* HEBREW LETTER BET */ \
	    UNI(0x62, 0x05D2)	/* HEBREW LETTER GIMEL */ \
	    UNI(0x63, 0x05D3)	/* HEBREW LETTER DALET */ \
	    UNI(0x64, 0x05D4)	/* HEBREW LETTER HE */ \
	    UNI(0x65, 0x05D5)	/* HEBREW LETTER VAV */ \
	    UNI(0x66, 0x05D6)	/* HEBREW LETTER ZAYIN */ \
	    UNI(0x67, 0x05D7)	/* HEBREW LETTER HET */ \
	    UNI(0x68, 0x05D8)	/* HEBREW LETTER TET */ \
	    UNI(0x69, 0x05D9)	/* HEBREW LETTER YOD */ \
	    UNI(0x6a, 0x05DA)	/* HEBREW LETTER FINAL KAF */ \
	    UNI(0x6b, 0x05DB)	/* HEBREW LETTER KAF */ \
	    UNI(0x6c, 0x05DC)	/* HEBREW LETTER LAMED */ \
	    UNI(0x6d, 0x05DD)	/* HEBREW LETTER FINAL MEM */ \
	    UNI(0x6e, 0x05DE)	/* HEBREW LETTER MEM */ \
	    UNI(0x6f, 0x05DF)	/* HEBREW LETTER FINAL NUN */ \
	    UNI(0x70, 0x05E0)	/* HEBREW LETTER NUN */ \
	    UNI(0x71, 0x05E1)	/* HEBREW LETTER SAMEKH */ \
	    UNI(0x72, 0x05E2)	/* HEBREW LETTER AYIN */ \
	    UNI(0x73, 0x05E3)	/* HEBREW LETTER FINAL PE */ \
	    UNI(0x74, 0x05E4)	/* HEBREW LETTER PE */ \
	    UNI(0x75, 0x05E5)	/* HEBREW LETTER FINAL TSADI */ \
	    UNI(0x76, 0x05E6)	/* HEBREW LETTER TSADI */ \
	    UNI(0x77, 0x05E7)	/* HEBREW LETTER QOF */ \
	    UNI(0x78, 0x05E8)	/* HEBREW LETTER RESH */ \
	    UNI(0x79, 0x05E9)	/* HEBREW LETTER SHIN */ \
	    UNI(0x7a, 0x05EA)	/* HEBREW LETTER TAV */ \
	}

#define unmap_NRCS_Hebrew(code) \
	switch (code) { \
	    MAP(0x05D0,           0x60) /* HEBREW LETTER ALEF */ \
	    MAP(0x05D1,           0x61) /* HEBREW LETTER BET */ \
	    MAP(0x05D2,           0x62) /* HEBREW LETTER GIMEL */ \
	    MAP(0x05D3,           0x63) /* HEBREW LETTER DALET */ \
	    MAP(0x05D4,           0x64) /* HEBREW LETTER HE */ \
	    MAP(0x05D5,           0x65) /* HEBREW LETTER VAV */ \
	    MAP(0x05D6,           0x66) /* HEBREW LETTER ZAYIN */ \
	    MAP(0x05D7,           0x67) /* HEBREW LETTER HET */ \
	    MAP(0x05D8,           0x68) /* HEBREW LETTER TET */ \
	    MAP(0x05D9,           0x69) /* HEBREW LETTER YOD */ \
	    MAP(0x05DA,           0x6a) /* HEBREW LETTER FINAL KAF */ \
	    MAP(0x05DB,           0x6b) /* HEBREW LETTER KAF */ \
	    MAP(0x05DC,           0x6c) /* HEBREW LETTER LAMED */ \
	    MAP(0x05DD,           0x6d) /* HEBREW LETTER FINAL MEM */ \
	    MAP(0x05DE,           0x6e) /* HEBREW LETTER MEM */ \
	    MAP(0x05DF,           0x6f) /* HEBREW LETTER FINAL NUN */ \
	    MAP(0x05E0,           0x70) /* HEBREW LETTER NUN */ \
	    MAP(0x05E1,           0x71) /* HEBREW LETTER SAMEKH */ \
	    MAP(0x05E2,           0x72) /* HEBREW LETTER AYIN */ \
	    MAP(0x05E3,           0x73) /* HEBREW LETTER FINAL PE */ \
	    MAP(0x05E4,           0x74) /* HEBREW LETTER PE */ \
	    MAP(0x05E5,           0x75) /* HEBREW LETTER FINAL TSADI */ \
	    MAP(0x05E6,           0x76) /* HEBREW LETTER TSADI */ \
	    MAP(0x05E7,           0x77) /* HEBREW LETTER QOF */ \
	    MAP(0x05E8,           0x78) /* HEBREW LETTER RESH */ \
	    MAP(0x05E9,           0x79) /* HEBREW LETTER SHIN */ \
	    MAP(0x05EA,           0x7a) /* HEBREW LETTER TAV */ \
	}

/*
 * figure A-26 "DEC 7-Bit Turkish Character Set"
 */
#define map_NRCS_Turkish(code) \
	switch (code) { \
	    UNI(0x26, 0x011F)	/* LATIN SMALL LETTER G WITH BREVE */ \
	    UNI(0x40, 0x0130)	/* LATIN CAPITAL LETTER I WITH DOT ABOVE */ \
	    UNI(0x5b, 0x015E)	/* LATIN CAPITAL LETTER S WITH CEDILLA */ \
	    UNI(0x5c, 0x00D6)	/* LATIN CAPITAL LETTER O WITH DIAERESIS */ \
	    UNI(0x5d, 0x00C7)	/* LATIN CAPITAL LETTER C WITH CEDILLA */ \
	    UNI(0x5e, 0x00dC)	/* LATIN CAPITAL LETTER U WITH DIAERESIS */ \
	    UNI(0x60, 0x011E)	/* LATIN CAPITAL LETTER G WITH BREVE */ \
	    UNI(0x7b, 0x015F)	/* LATIN SMALL LETTER S WITH CEDILLA */ \
	    UNI(0x7c, 0x00F6)	/* LATIN SMALL LETTER O WITH DIAERESIS */ \
	    UNI(0x7d, 0x00E7)	/* LATIN SMALL LETTER C WITH CEDILLA */ \
	    UNI(0x7e, 0x00FC)	/* LATIN SMALL LETTER U WITH DIAERESIS */ \
	}

#define unmap_NRCS_Turkish(code) \
	switch (code) { \
	    MAP(0x00C7,           0x5d) /* LATIN CAPITAL LETTER C WITH CEDILLA */ \
	    MAP(0x00D6,           0x5c) /* LATIN CAPITAL LETTER O WITH DIAERESIS */ \
	    MAP(0x00E7,           0x7d) /* LATIN SMALL LETTER C WITH CEDILLA */ \
	    MAP(0x00F6,           0x7c) /* LATIN SMALL LETTER O WITH DIAERESIS */ \
	    MAP(0x00FC,           0x7e) /* LATIN SMALL LETTER U WITH DIAERESIS */ \
	    MAP(0x00dC,           0x5e) /* LATIN CAPITAL LETTER U WITH DIAERESIS */ \
	    MAP(0x011E,           0x60) /* LATIN CAPITAL LETTER G WITH BREVE */ \
	    MAP(0x011F,           0x26) /* LATIN SMALL LETTER G WITH BREVE */ \
	    MAP(0x0130,           0x40) /* LATIN CAPITAL LETTER I WITH DOT ABOVE */ \
	    MAP(0x015E,           0x5b) /* LATIN CAPITAL LETTER S WITH CEDILLA */ \
	    MAP(0x015F,           0x7b) /* LATIN SMALL LETTER S WITH CEDILLA */ \
	}
#else
#define map_DEC_Cyrillic(code)	/* nothing */
#define map_DEC_Greek_Supp(code)	/* nothing */
#define map_DEC_Hebrew_Supp(code)	/* nothing */
#define map_DEC_Technical(code)	/* nothing */
#define map_DEC_Turkish_Supp(code)	/* nothing */
#define map_ISO_Greek_Supp(code)	/* nothing */
#define map_ISO_Hebrew(code)	/* nothing */
#define map_ISO_Latin_2(code)	/* nothing */
#define map_ISO_Latin_5(code)	/* nothing */
#define map_ISO_Latin_Cyrillic(code)	/* nothing */
#define map_NRCS_Greek(code)	/* nothing */
#define map_NRCS_Hebrew(code)	/* nothing */
#define map_NRCS_Turkish(code)	/* nothing */
#endif /* OPT_WIDE_CHARS */

#endif /* included_charsets_h */
