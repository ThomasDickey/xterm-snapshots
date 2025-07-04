/* $XTermId: ptyx.h,v 1.1152 2025/06/23 23:54:48 tom Exp $ */

/*
 * Copyright 1999-2024,2025 by Thomas E. Dickey
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

#ifndef included_ptyx_h
#define included_ptyx_h 1

#ifdef HAVE_CONFIG_H
#include <xtermcfg.h>
#endif

/* ptyx.h */
/* *INDENT-OFF* */
/* @(#)ptyx.h	X10/6.6	11/10/86 */

#include <X11/IntrinsicP.h>
#include <X11/Shell.h>		/* for XtNdieCallback, etc. */
#include <X11/StringDefs.h>	/* for standard resource names */
#include <X11/Xmu/Misc.h>	/* For Max() and Min(). */
#include <X11/cursorfont.h>


#undef bcopy
#undef bzero
#include <X11/Xfuncs.h>

#include <X11/Xosdefs.h>
#include <X11/Xmu/Converters.h>
#ifdef XRENDERFONT
#include <X11/Xft/Xft.h>
#endif

#include <stdio.h>
#include <limits.h>

#if defined(HAVE_STDINT_H) || !defined(HAVE_CONFIG_H)
#include <stdint.h>
#define DECONST(type,s) ((type *)(intptr_t)(const type *)(s))
#else
#define DECONST(type,s) ((type *)(s))
#endif

/* adapted from IntrinsicI.h */
#define MyStackAlloc(size, stack_cache_array)     \
    ((size) <= sizeof(stack_cache_array)	  \
    ?  (stack_cache_array)			  \
    :  (char*)malloc((size_t)(size)))

#define MyStackFree(pointer, stack_cache_array) \
    if ((pointer) != ((char *)(stack_cache_array))) free(pointer)

/* adapted from vile (vi-like-emacs) */
#define TypeCallocN(type,n)	(type *)calloc((size_t) (n), sizeof(type))
#define TypeCalloc(type)	TypeCallocN(type, 1)

#define TypeMallocN(type,n)	(type *)malloc(sizeof(type) * (size_t) (n))
#define TypeMalloc(type)	TypeMallocN(type, 1)

#define TypeRealloc(type,n,p)	(type *)realloc(p, (n) * sizeof(type))

#define TypeXtReallocN(t,p,n)	(t *)(void *)XtRealloc((char *)(p), (Cardinal)(sizeof(t) * (size_t) (n)))

#define TypeXtMallocX(type,n)	(type *)(void *)XtMalloc((Cardinal)(sizeof(type) + (size_t) (n)))
#define TypeXtMallocN(type,n)	(type *)(void *)XtMalloc((Cardinal)(sizeof(type) * (size_t) (n)))
#define TypeXtMalloc(type)	TypeXtMallocN(type, 1)

#define CastMalloc(type)	(type *)malloc(sizeof(type))

#define BumpBuffer(type, buffer, size, want) \
	if (want >= size) { \
	    size = 1 + (want * 2); \
	    buffer = TypeRealloc(type, size, buffer); \
	}

#define BfBuf(type) screen->bf_buf_##type
#define BfLen(type) screen->bf_len_##type

#define TypedBuffer(type) \
	type		*bf_buf_##type; \
	Cardinal	bf_len_##type

#define BumpTypedBuffer(type, want) \
	BumpBuffer(type, BfBuf(type), BfLen(type), want)

#define FreeTypedBuffer(type) \
	do { \
	    FreeAndNull(BfBuf(type)); \
	    BfLen(type) = 0; \
	} while (0)

#define FreeAndNull(value) \
	do { \
	    free((void *)(value)); \
	    value = NULL; \
	} while (0)

/*
** System V definitions
*/

#ifdef att
#define ATT
#endif

#ifdef SVR4
#undef  SYSV			/* predefined on Solaris 2.4 */
#define SYSV			/* SVR4 is (approx) superset of SVR3 */
#define ATT
#endif

#ifdef SYSV
#ifdef X_NOT_POSIX
#if !defined(CRAY) && !defined(SVR4)
#define	dup2(fd1,fd2)	((fd1 == fd2) ? fd1 : \
				(close(fd2), fcntl(fd1, F_DUPFD, fd2)))
#endif
#endif
#endif /* SYSV */

/*
 * Newer versions of <X11/Xft/Xft.h> have a version number.  We use certain
 * features from that.
 */
#if defined(XRENDERFONT) && defined(XFT_VERSION) && XFT_VERSION >= 20100
#define HAVE_TYPE_FCCHAR32	1	/* compatible: XftChar16 */
#define HAVE_TYPE_XFTCHARSPEC	1	/* new type XftCharSpec */
#endif

/*
** Definitions to simplify ifdef's for pty's.
*/
#define USE_PTY_DEVICE 1
#define USE_PTY_SEARCH 1

#if defined(__osf__) || (defined(linux) && defined(__GLIBC__) && (__GLIBC__ >= 2) && (__GLIBC_MINOR__ >= 1)) || defined(__DragonFly__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__APPLE__)
#undef USE_PTY_DEVICE
#undef USE_PTY_SEARCH
#define USE_PTS_DEVICE 1
#elif defined(PUCC_PTYD)
#undef USE_PTY_SEARCH
#elif (defined(sun) && defined(SVR4)) || defined(_ALL_SOURCE) || defined(__CYGWIN__)
#undef USE_PTY_SEARCH
#elif defined(__OpenBSD__)
#undef USE_PTY_SEARCH
#undef USE_PTY_DEVICE
#endif

#if (defined (__GLIBC__) && ((__GLIBC__ > 2) || (__GLIBC__ == 2) && (__GLIBC_MINOR__ >= 1)))
#define USE_HANDSHAKE 0	/* "recent" Linux systems do not require handshaking */
#endif

#if (defined (__GLIBC__) && ((__GLIBC__ > 2) || (__GLIBC__ == 2) && (__GLIBC_MINOR__ >= 1)))
#define USE_USG_PTYS
#elif (defined(ATT) && !defined(__sgi)) || (defined(SYSV) && defined(i386))
#define USE_USG_PTYS
#endif

/*
 * More systems than not require pty-handshaking.
 */
#ifndef USE_HANDSHAKE
#define USE_HANDSHAKE 1
#endif

/*
** allow for mobility of the pty master/slave directories
*/
#ifndef PTYDEV
#if defined(__hpux)
#define	PTYDEV		"/dev/ptym/ptyxx"
#else
#define	PTYDEV		"/dev/ptyxx"
#endif
#endif	/* !PTYDEV */

#ifndef TTYDEV
#if defined(__hpux)
#define TTYDEV		"/dev/pty/ttyxx"
#elif defined(USE_PTS_DEVICE)
#define TTYDEV		"/dev/pts/0"
#else
#define	TTYDEV		"/dev/ttyxx"
#endif
#endif	/* !TTYDEV */

#ifndef PTYCHAR1
#ifdef __hpux
#define PTYCHAR1	"zyxwvutsrqp"
#else	/* !__hpux */
#define	PTYCHAR1	"pqrstuvwxyzPQRSTUVWXYZ"
#endif	/* !__hpux */
#endif	/* !PTYCHAR1 */

#ifndef PTYCHAR2
#ifdef __hpux
#define	PTYCHAR2	"fedcba9876543210"
#else	/* !__hpux */
#if defined(__DragonFly__) || defined(__FreeBSD__)
#define	PTYCHAR2	"0123456789abcdefghijklmnopqrstuv"
#else /* !__FreeBSD__ */
#define	PTYCHAR2	"0123456789abcdef"
#endif /* !__FreeBSD__ */
#endif	/* !__hpux */
#endif	/* !PTYCHAR2 */

#ifndef TTYFORMAT
#if defined(CRAY)
#define TTYFORMAT "/dev/ttyp%03d"
#else
#define TTYFORMAT "/dev/ttyp%d"
#endif
#endif /* TTYFORMAT */

#ifndef PTYFORMAT
#ifdef CRAY
#define PTYFORMAT "/dev/pty/%03d"
#else
#define PTYFORMAT "/dev/ptyp%d"
#endif
#endif /* PTYFORMAT */

#ifndef PTYCHARLEN
#ifdef CRAY
#define PTYCHARLEN 3
#else
#define PTYCHARLEN 2
#endif
#endif

#ifndef MAXPTTYS
#ifdef CRAY
#define MAXPTTYS 256
#else
#define MAXPTTYS 2048
#endif
#endif

/* Until the translation manager comes along, I have to do my own translation of
 * mouse events into the proper routines. */

typedef enum {
    NORMAL = 0
    , LEFTEXTENSION
    , RIGHTEXTENSION
} EventMode;

/*
 * The origin of a screen is 0, 0.  Therefore, the number of rows
 * on a screen is screen->max_row + 1, and similarly for columns.
 */
#define MaxCols(screen)		((screen)->max_col + 1)
#define MaxRows(screen)		((screen)->max_row + 1)

#define MaxUChar 255
typedef unsigned char Char;		/* to support 8 bit chars */
typedef Char *ScrnPtr;
typedef ScrnPtr *ScrnBuf;

/*
 * Declare an X String, but for unsigned chars.
 */
#ifdef _CONST_X_STRING
typedef const Char *UString;
#else
typedef Char *UString;
#endif

#define IsEmpty(s) ((s) == NULL || *(s) == '\0')
#define IsSpace(c) ((c) == ' ' || (c) == '\t' || (c) == '\r' || (c) == '\n')

/*
 * Check strtol result, using "FullS2L" when no more data is expected, and
 * "PartS2L" when more data may follow in the string.
 */
#define FullS2L(s,d) (PartS2L(s,d) && (*(d) == '\0'))
#define PartS2L(s,d) (isdigit(CharOf(*(s))) && (d) != (s) && (d) != NULL)

#define CASETYPE(name) case name: result = #name; break

#define AsciiOf(n) (0x7f & (n))		/* extract 7-bit character */
#define CharOf(n) ((Char)(n))		/* extract 8-bit character */

typedef struct {
    int row;
    int col;
} CELL;

typedef struct {
    Char  *data_buffer;			/* the current selection */
    size_t data_limit;			/* size of allocated buffer */
    size_t data_length;			/* number of significant bytes */
} SelectedCells;

#define isSameRow(a,b)		((a)->row == (b)->row)
#define isSameCol(a,b)		((a)->col == (b)->col)
#define isSameCELL(a,b)		(isSameRow(a,b) && isSameCol(a,b))

#define xBIT(n)         (1 << (n))

/*
 * ANSI emulation, special character codes
 */
#define ANSI_EOT	0x04
#define ANSI_BEL	0x07
#define ANSI_BS		0x08
#define ANSI_HT		0x09
#define ANSI_LF		0x0A
#define ANSI_VT		0x0B
#define	ANSI_FF		0x0C		/* C0, C1 control names		*/
#define ANSI_CR		0x0D
#define ANSI_SO		0x0E
#define ANSI_SI		0x0F
#define	ANSI_XON	0x11		/* DC1 */
#define	ANSI_XOFF	0x13		/* DC3 */
#define	ANSI_NAK	0x15
#define	ANSI_CAN	0x18
#define	ANSI_ESC	0x1B
#define	ANSI_SPA	0x20
#define XTERM_POUND	0x1E		/* internal mapping for '#'	*/
#define	ANSI_DEL	0x7F
#define	ANSI_SS2	0x8E
#define	ANSI_SS3	0x8F
#define	ANSI_DCS	0x90
#define	ANSI_SOS	0x98
#define	ANSI_CSI	0x9B
#define	ANSI_ST		0x9C
#define	ANSI_OSC	0x9D
#define	ANSI_PM		0x9E
#define	ANSI_APC	0x9F
#define XTERM_PUA	0xEEEE		/* internal mapping for DEC Technical */

#define BAD_ASCII	'?'
#define NonLatin1(c)	(((c) != ANSI_LF) && \
			 ((c) != ANSI_HT) && \
			 (((c) < ANSI_SPA) || \
			  ((c) >= ANSI_DEL && (c) <= ANSI_APC)))
#define OnlyLatin1(c)	(NonLatin1(c) ? BAD_ASCII : (c))

#define L_BLOK		'['
#define R_BLOK		']'

#define L_CURL		'{'
#define R_CURL		'}'

#define MIN_DECID  52			/* can emulate VT52 */
#define MAX_DECID 525			/* ...through VT525 */

#ifndef DFT_DECID
#define DFT_DECID "420"			/* default VT420 */
#endif

#ifndef DFT_KBD_DIALECT
#define DFT_KBD_DIALECT "B"		/* default USASCII */
#endif

#define MAX_I_PARAM	65535		/* parameters */
#define MAX_I_DELAY	32767		/* time-delay in ReGIS */
#define MAX_U_COLOR	65535u		/* colors */
#define MAX_U_COORD	32767u		/* coordinates */
#define MAX_U_STRING	65535u		/* string-length */

/* constants used for utf8 mode */
#define UCS_REPL	0xfffd
#define UCS_LIMIT	0x80000000U	/* both limit and flag for non-UCS */

#define TERMCAP_SIZE 1500		/* 1023 is standard; 'screen' exceeds */

#define MAX_XLFD_FONTS	1
#define MAX_XFT_FONTS	2
#define NMENUFONTS	10		/* font entries in fontMenu */

#define	NBOX	5			/* Number of Points in box	*/
#define	NPARAM	30			/* Max. parameters		*/

typedef struct {
	String opt;
	String desc;
} OptionHelp;

typedef	struct {
	int	count;			/* number of values in params[]	*/
	int	has_subparams;		/* true if there are any sub's	*/
	int	is_sub[NPARAM];		/* true for subparam		*/
	int	params[NPARAM];		/* parameter value		*/
} PARAMS;

typedef short ParmType;
typedef unsigned short UParm;		/* unparseputn passes ParmType	*/

#define MaxSParm   0x7fff		/* limit if a signed value is needed */
#define MaxUParm   0xffff		/* limit if unsigned value is needed */

#define SParmOf(n) ((int)(ParmType)(n))
#define UParmOf(n) ((unsigned)(UParm)(n))

typedef struct {
	Char		a_type;		/* CSI, etc., see unparseq()	*/
	Char		a_pintro;	/* private-mode char, if any	*/
	const char *	a_delim;	/* between parameters (;)	*/
	Char		a_inters;	/* special (before final-char)	*/
	Char		a_final;	/* final-char			*/
	ParmType	a_nparam;	/* # of parameters		*/
	ParmType	a_param[NPARAM]; /* Parameters			*/
	Char		a_radix[NPARAM]; /* Parameters			*/
} ANSI;

#define TEK_FONT_LARGE 0
#define TEK_FONT_2 1
#define TEK_FONT_3 2
#define TEK_FONT_SMALL 3
#define	TEKNUMFONTS 4

/* Actually there are 5 types of lines, but four are non-solid lines */
#define	TEKNUMLINES	4

typedef struct {
	int	x;
	int	y;
	int	fontsize;
	unsigned linetype;
} Tmodes;

typedef struct {
	int Twidth;
	int Theight;
} T_fontsize;

typedef struct {
	short *bits;
	int x;
	int y;
	int width;
	int height;
} BitmapBits;

/* bit-assignments for extensions to DECRQCRA, to omit DEC features */
typedef enum {
    csDEC = 0
    ,csPOSITIVE = xBIT(0)	/* do not negate the result */
    ,csATTRIBS = xBIT(1)	/* do not report the VT100 video attributes */
    ,csNOTRIM = xBIT(2)		/* do not omit checksum for blanks */
    ,csDRAWN = xBIT(3)		/* do not skip uninitialized cells */
    ,csBYTE = xBIT(4)		/* do not mask cell value to 8 bits or ignore combining chars */
} CSBITS;

#define EXCHANGE(a,b,tmp) tmp = a; a = b; b = tmp

/***====================================================================***/

#if (XtSpecificationRelease < 6)
#ifndef NO_ACTIVE_ICON
#define NO_ACTIVE_ICON 1 /* Note: code relies on an X11R6 function */
#endif
#endif

#ifndef OPT_AIX_COLORS
#define OPT_AIX_COLORS  1 /* true if xterm is configured with AIX (16) colors */
#endif

#ifndef OPT_ALLOW_XXX_OPS
#define OPT_ALLOW_XXX_OPS 1 /* true if xterm adds "Allow XXX Ops" submenu */
#endif

#ifndef OPT_BLINK_CURS
#define OPT_BLINK_CURS  1 /* true if xterm has blinking cursor capability */
#endif

#ifndef OPT_BLINK_TEXT
#define OPT_BLINK_TEXT  OPT_BLINK_CURS /* true if xterm has blinking text capability */
#endif

#ifndef OPT_BLOCK_SELECT
#define OPT_BLOCK_SELECT 0 /* true if block-select is supported */
#endif

#ifndef OPT_BOX_CHARS
#define OPT_BOX_CHARS	1 /* true if xterm can simulate box-characters */
#endif

#ifndef OPT_BUILTIN_XPMS
#define OPT_BUILTIN_XPMS 0 /* true if all xpm data is compiled-in */
#endif

#ifndef OPT_BROKEN_OSC
#ifdef linux
#define OPT_BROKEN_OSC	1 /* man console_codes, 1st paragraph - cf: ECMA-48 */
#else
#define OPT_BROKEN_OSC	0 /* true if xterm allows Linux's broken OSC parsing */
#endif
#endif

#ifndef OPT_BROKEN_ST
#define OPT_BROKEN_ST	1 /* true if xterm allows old/broken OSC parsing */
#endif

#ifndef OPT_C1_PRINT
#define OPT_C1_PRINT	1 /* true if xterm allows C1 controls to be printable */
#endif

#ifndef OPT_CLIP_BOLD
#define OPT_CLIP_BOLD	1 /* true if xterm uses clipping to avoid bold-trash */
#endif

#ifndef OPT_COLOR_CLASS
#define OPT_COLOR_CLASS 1 /* true if xterm uses separate color-resource classes */
#endif

#ifndef OPT_DABBREV
#define OPT_DABBREV 0	/* dynamic abbreviations */
#endif

#ifndef OPT_DEC_CHRSET
#define OPT_DEC_CHRSET  1 /* true if xterm is configured for DEC charset */
#endif

#ifndef OPT_DEC_LOCATOR
#define	OPT_DEC_LOCATOR 0 /* true if xterm supports VT220-style mouse events */
#endif

#ifndef OPT_DEC_RECTOPS
#define OPT_DEC_RECTOPS 1 /* true if xterm is configured for VT420 rectangles */
#endif

#ifndef OPT_SET_XPROP
#define OPT_SET_XPROP 1 /* true if xterm can set X properties */
#endif

#ifndef OPT_SGR2_HASH
#define OPT_SGR2_HASH 1 /* true if xterm hashes color-lookups for faint color */
#endif

#ifndef OPT_SIXEL_GRAPHICS
#define OPT_SIXEL_GRAPHICS 0 /* true if xterm supports VT240-style sixel graphics */
#endif

#ifndef OPT_PRINT_GRAPHICS
#define OPT_PRINT_GRAPHICS 0 /* true if xterm supports screen dumps as sixel graphics */
#endif

#ifndef OPT_SCREEN_DUMPS
#define OPT_SCREEN_DUMPS 1 /* true if xterm supports screen dumps */
#endif

#ifndef OPT_REGIS_GRAPHICS
#define OPT_REGIS_GRAPHICS 0 /* true if xterm supports VT125/VT240/VT330 ReGIS graphics */
#endif

#ifndef OPT_GRAPHICS
#define OPT_GRAPHICS 0 /* true if xterm is configured for any type of graphics */
#endif

#ifndef OPT_DEC_SOFTFONT
#define OPT_DEC_SOFTFONT 0 /* true if xterm is configured for VT220 softfonts */
#endif

#ifndef OPT_DOUBLE_BUFFER
#define OPT_DOUBLE_BUFFER 0 /* true if using double-buffering */
#endif

#ifndef OPT_EXEC_SELECTION
#define OPT_EXEC_SELECTION 1 /* true if xterm can exec to process selection */
#endif

#ifndef OPT_EXEC_XTERM
#define OPT_EXEC_XTERM 0 /* true if xterm can fork/exec copies of itself */
#endif

#ifndef OPT_EXTRA_PASTE
#define OPT_EXTRA_PASTE 1
#endif

#ifndef OPT_FOCUS_EVENT
#define OPT_FOCUS_EVENT	1 /* focus in/out events */
#endif

#ifndef OPT_HP_FUNC_KEYS
#define OPT_HP_FUNC_KEYS 0 /* true if xterm supports HP-style function keys */
#endif

#ifndef OPT_I18N_SUPPORT
#if (XtSpecificationRelease >= 5)
#define OPT_I18N_SUPPORT 1 /* true if xterm uses internationalization support */
#else
#define OPT_I18N_SUPPORT 0
#endif
#endif

#ifndef OPT_INITIAL_ERASE
#define OPT_INITIAL_ERASE 1 /* use pty's erase character if it's not 128 */
#endif

#ifndef OPT_INPUT_METHOD
#if (XtSpecificationRelease >= 6)
#define OPT_INPUT_METHOD OPT_I18N_SUPPORT /* true if xterm uses input-method support */
#else
#define OPT_INPUT_METHOD 0
#endif
#endif

#ifndef OPT_ISO_COLORS
#define OPT_ISO_COLORS  1 /* true if xterm is configured with ISO colors */
#endif

#ifndef OPT_DIRECT_COLOR
#define OPT_DIRECT_COLOR  OPT_ISO_COLORS /* true if xterm is configured with direct-colors */
#endif

#ifndef OPT_256_COLORS
#define OPT_256_COLORS  1 /* true if xterm is configured with 256 colors */
#endif

#ifndef OPT_88_COLORS
#define OPT_88_COLORS	1 /* true if xterm is configured with 88 colors */
#endif

#ifndef OPT_HIGHLIGHT_COLOR
#define OPT_HIGHLIGHT_COLOR 1 /* true if xterm supports color highlighting */
#endif

#ifndef OPT_LOAD_VTFONTS
#define OPT_LOAD_VTFONTS 0 /* true if xterm has load-vt-fonts() action */
#endif

#ifndef OPT_LUIT_PROG
#define OPT_LUIT_PROG   1 /* true if xterm supports luit */
#endif

#ifndef OPT_MAXIMIZE
#define OPT_MAXIMIZE	1 /* add actions for iconify ... maximize */
#endif

#ifndef OPT_MINI_LUIT
#define OPT_MINI_LUIT   0 /* true if xterm supports built-in mini-luit */
#endif

#ifndef OPT_MOD_FKEYS
#define OPT_MOD_FKEYS	1 /* modify cursor- and function-keys in normal mode */
#endif

#ifndef OPT_NUM_LOCK
#define OPT_NUM_LOCK	1 /* use NumLock key only for numeric-keypad */
#endif

#ifndef OPT_PASTE64
#define OPT_PASTE64	1 /* program control of select/paste via base64 */
#endif

#ifndef OPT_PC_COLORS
#define OPT_PC_COLORS   1 /* true if xterm supports PC-style (bold) colors */
#endif

#ifndef OPT_PRINT_ON_EXIT
#define OPT_PRINT_ON_EXIT 1 /* true allows xterm to dump screen on X error */
#endif

#ifndef OPT_PTY_HANDSHAKE
#define OPT_PTY_HANDSHAKE USE_HANDSHAKE	/* avoid pty races on older systems */
#endif

#ifndef OPT_PRINT_COLORS
#define OPT_PRINT_COLORS 1 /* true if we print color information */
#endif

#ifndef OPT_QUERY_ALLOW
#define OPT_QUERY_ALLOW	1 /* report allowed/disallowed features */
#endif

#ifndef OPT_READLINE
#define OPT_READLINE	0 /* mouse-click/paste support for readline */
#endif

#ifndef OPT_RENDERFONT
#ifdef XRENDERFONT
#define OPT_RENDERFONT 1
#else
#define OPT_RENDERFONT 0
#endif
#endif

#ifndef OPT_RENDERWIDE
#if OPT_RENDERFONT && OPT_WIDE_CHARS && defined(HAVE_TYPE_XFTCHARSPEC)
#define OPT_RENDERWIDE 1
#else
#define OPT_RENDERWIDE 0
#endif
#endif

#ifndef OPT_REPORT_CCLASS
#define OPT_REPORT_CCLASS  1 /* provide "-report-charclass" option */
#endif

#ifndef OPT_REPORT_COLORS
#define OPT_REPORT_COLORS  1 /* provide "-report-colors" option */
#endif

#ifndef OPT_REPORT_FONTS
#define OPT_REPORT_FONTS   1 /* provide "-report-fonts" option */
#endif

#ifndef OPT_REPORT_ICONS
#define OPT_REPORT_ICONS   1 /* provide "-report-icons" option */
#endif

#ifndef OPT_SAME_NAME
#define OPT_SAME_NAME   1 /* suppress redundant updates of title, icon, etc. */
#endif

#ifndef OPT_SCO_FUNC_KEYS
#define OPT_SCO_FUNC_KEYS 0 /* true if xterm supports SCO-style function keys */
#endif

#ifndef OPT_SUN_FUNC_KEYS
#define OPT_SUN_FUNC_KEYS 1 /* true if xterm supports Sun-style function keys */
#endif

#ifndef OPT_SCROLL_LOCK
#define OPT_SCROLL_LOCK 1 /* true if xterm interprets fontsize-shifting */
#endif

#ifndef OPT_SELECT_REGEX
#define OPT_SELECT_REGEX 1 /* true if xterm supports regular-expression selects */
#endif

#ifndef OPT_SELECTION_OPS
#define OPT_SELECTION_OPS 1 /* true if xterm supports operations on selection */
#endif

#ifndef OPT_SESSION_MGT
#if defined(XtNdieCallback) && defined(XtNsaveCallback)
#define OPT_SESSION_MGT 1
#else
#define OPT_SESSION_MGT 0
#endif
#endif

#ifndef OPT_SHIFT_FONTS
#define OPT_SHIFT_FONTS 1 /* true if xterm interprets fontsize-shifting */
#endif

#ifndef OPT_STATUS_LINE
#define OPT_STATUS_LINE	0 /* true if xterm supports status-line controls */
#endif

#ifndef OPT_SUNPC_KBD
#define OPT_SUNPC_KBD	1 /* true if xterm supports Sun/PC keyboard map */
#endif

#ifndef OPT_TCAP_FKEYS
#define OPT_TCAP_FKEYS	1 /* true for termcap function-keys */
#endif

#ifndef OPT_TCAP_QUERY
#define OPT_TCAP_QUERY	1 /* true for termcap query */
#endif

#ifndef OPT_TEK4014
#define OPT_TEK4014     1 /* true if we're using tek4014 emulation */
#endif

#ifndef OPT_TITLE_MODES
#define OPT_TITLE_MODES 1 /* true if we provide title-stack */
#endif

#ifndef OPT_TOOLBAR
#define OPT_TOOLBAR	0 /* true if xterm supports toolbar menus */
#endif

#ifndef OPT_TRACE
#define OPT_TRACE       0 /* true if we're using debugging traces */
#endif

#ifndef OPT_TRACE_FLAGS
#define OPT_TRACE_FLAGS 0 /* additional tracing used for SCRN_BUF_FLAGS */
#endif

#ifndef OPT_TRACE_UNIQUE
#define OPT_TRACE_UNIQUE 0 /* true if we're using multiple trace files */
#endif

#ifndef OPT_VT52_MODE
#define OPT_VT52_MODE   1 /* true if xterm supports VT52 emulation */
#endif

#ifndef OPT_WIDE_ATTRS
#define OPT_WIDE_ATTRS  1 /* true if xterm supports 16-bit attributes */
#endif

#ifndef OPT_VT525_COLORS
#define OPT_VT525_COLORS 1 /* true if xterm is configured for VT525 colors */
#endif

#ifndef OPT_WIDE_CHARS
#define OPT_WIDE_CHARS  1 /* true if xterm supports 16-bit characters */
#endif

#ifndef OPT_WIDER_ICHAR
#define OPT_WIDER_ICHAR 1 /* true if xterm uses 32-bits for wide-chars */
#endif

#ifndef OPT_XMC_GLITCH
#define OPT_XMC_GLITCH	0 /* true if xterm supports xmc (magic cookie glitch) */
#endif

#ifndef OPT_XRES_QUERY
#define OPT_XRES_QUERY	1 /* true for resource query */
#endif

#ifndef OPT_XTERM_SGR
#define OPT_XTERM_SGR   1 /* true if xterm supports private SGR controls */
#endif

#ifndef OPT_ZICONBEEP
#define OPT_ZICONBEEP   1 /* true if xterm supports "-ziconbeep" option */
#endif

/***====================================================================***/

#if OPT_AIX_COLORS && !OPT_ISO_COLORS
/* You must have ANSI/ISO colors to support AIX colors */
#undef  OPT_AIX_COLORS
#define OPT_AIX_COLORS 0
#endif

#if OPT_PC_COLORS && !OPT_ISO_COLORS
/* You must have ANSI/ISO colors to support PC colors */
#undef  OPT_PC_COLORS
#define OPT_PC_COLORS 0
#endif

#if OPT_PRINT_COLORS && !OPT_ISO_COLORS
/* You must have ANSI/ISO colors to be able to print them */
#undef  OPT_PRINT_COLORS
#define OPT_PRINT_COLORS 0
#endif

#if OPT_256_COLORS && !OPT_ISO_COLORS
/* You must have ANSI/ISO colors to support 256 colors */
#undef  OPT_256_COLORS
#define OPT_256_COLORS 0
#endif

#if OPT_88_COLORS && !OPT_ISO_COLORS
/* You must have ANSI/ISO colors to support 88 colors */
#undef  OPT_88_COLORS
#define OPT_88_COLORS 0
#endif

#if OPT_88_COLORS && OPT_256_COLORS
/* 256 colors supersedes 88 colors */
#undef  OPT_88_COLORS
#define OPT_88_COLORS 0
#endif

/***====================================================================***/

/*
 * Indices for menu_font_names[][]
 */
typedef enum {
    fNorm = 0			/* normal font */
    , fBold			/* bold font */
#if OPT_WIDE_ATTRS || OPT_RENDERWIDE
    , fItal			/* italic font */
    , fBtal			/* bold-italic font */
#endif
#if OPT_WIDE_CHARS
    , fWide			/* double-width font */
    , fWBold			/* double-width bold font */
    , fWItal			/* double-width italic font */
    , fWBtal			/* double-width bold-italic font */
#endif
    , fMAX
} VTFontEnum;

/*
 * Indices for cachedGCs.c (unrelated to VTFontEnum).
 */
typedef enum {
    gcNorm = 0
    , gcBold
    , gcNormReverse
    , gcBoldReverse
    , gcFiller
    , gcBorder
#if OPT_BOX_CHARS || OPT_WIDE_CHARS
    , gcLine
    , gcDots
#endif
#if OPT_DEC_CHRSET
    , gcCNorm
    , gcCBold
#endif
#if OPT_WIDE_CHARS
    , gcWide
    , gcWBold
    , gcWideReverse
    , gcWBoldReverse
#endif
    , gcVTcursNormal
    , gcVTcursFilled
    , gcVTcursReverse
    , gcVTcursOutline
#if OPT_TEK4014
    , gcTKcurs
#endif
    , gcMAX
} CgsEnum;

#define for_each_text_gc(n) for (n = gcNorm; n < gcVTcursNormal; ++n)
#define for_each_curs_gc(n) for (n = gcVTcursNormal; n <= gcVTcursOutline; ++n)
#define for_each_gc(n)      for (n = gcNorm; n < gcMAX; ++n)

/*
 * Indices for the normal terminal colors in screen.Tcolors[].
 * See also OscTextColors, which has corresponding values.
 */
typedef enum {
    TEXT_FG = 0			/* text foreground */
    , TEXT_BG			/* text background */
    , TEXT_CURSOR		/* text cursor */
    , MOUSE_FG			/* mouse foreground */
    , MOUSE_BG			/* mouse background */
#if OPT_TEK4014
    , TEK_FG = 5		/* tektronix foreground */
    , TEK_BG			/* tektronix background */
#endif
#if OPT_HIGHLIGHT_COLOR
    , HIGHLIGHT_BG = 7		/* highlight background */
#endif
#if OPT_TEK4014
    , TEK_CURSOR = 8		/* tektronix cursor */
#endif
#if OPT_HIGHLIGHT_COLOR
    , HIGHLIGHT_FG = 9		/* highlight foreground */
#endif
    , NCOLORS			/* total number of colors */
} TermColors;

/*
 * Enum corresponding to the actual OSC codes rather than the internal
 * array indices.  Compare with TermColors.
 */
typedef enum {
    OSC_TEXT_FG = 10
    ,OSC_TEXT_BG
    ,OSC_TEXT_CURSOR
    ,OSC_MOUSE_FG
    ,OSC_MOUSE_BG
#if OPT_TEK4014
    ,OSC_TEK_FG = 15
    ,OSC_TEK_BG
#endif
#if OPT_HIGHLIGHT_COLOR
    ,OSC_HIGHLIGHT_BG = 17
#endif
#if OPT_TEK4014
    ,OSC_TEK_CURSOR = 18
#endif
#if OPT_HIGHLIGHT_COLOR
    ,OSC_HIGHLIGHT_FG = 19
#endif
    ,OSC_NCOLORS
} OscTextColors;

/*
 * Definitions for exec-formatted and insert-formatted actions.
 */
typedef void (*FormatSelect) (Widget, char *, char *, CELL *, CELL *);

typedef struct {
    Boolean done;
    char *format;
    char *buffer;
    FormatSelect format_select;
#if OPT_PASTE64
    Cardinal base64_paste;
#endif
#if OPT_PASTE64 || OPT_READLINE
    unsigned paste_brackets;
#endif
} InternalSelect;

/*
 * Constants for titleModes resource
 */
typedef enum {
    tmSetBase16 = 1		/* set title using hex-string */
    , tmGetBase16 = 2		/* get title using hex-string */
#if OPT_WIDE_CHARS
#define MAX_TITLEMODE 3
    , tmSetUtf8 = 4		/* like utf8Title, but controllable */
    , tmGetUtf8 = 8		/* retrieve title encoded as UTF-8 */
#else
#define MAX_TITLEMODE 1
#endif
} TitleModes;

#define ValidTitleMode(code) ((code) >= 0 && (code) <= MAX_TITLEMODE)
#define IsTitleMode(xw,mode) (((xw)->screen.title_modes & mode) != 0)

#define IsSetUtf8Title(xw) (IsTitleMode(xw, tmSetUtf8) \
			 || ((xw)->screen.utf8_title) \
			 || ((xw)->screen.c1_printable))

#include <xcharmouse.h>

/*
 * For readability...
 */
#define nrc_percent   100
#define nrc_dquote    200
#define nrc_ampersand 300
typedef enum {
    nrc_ASCII = 0
    ,nrc_British		/* vt100 */
    ,nrc_British_Latin_1	/* vt3xx */
    ,nrc_DEC_Cyrillic		/* vt5xx */
    ,nrc_DEC_Spec_Graphic	/* vt100 */
    ,nrc_DEC_Alt_Chars		/* vt100 */
    ,nrc_DEC_Alt_Graphics	/* vt100 */
    ,nrc_DEC_Supp		/* vt2xx */
    ,nrc_DEC_Supp_Graphic	/* vt3xx */
    ,nrc_DEC_Technical		/* vt3xx */
    ,nrc_DEC_UPSS		/* vt3xx */
    ,nrc_Dutch			/* vt2xx */
    ,nrc_Finnish		/* vt2xx */
    ,nrc_Finnish2		/* vt2xx */
    ,nrc_French			/* vt2xx */
    ,nrc_French2		/* vt2xx */
    ,nrc_French_Canadian	/* vt2xx */
    ,nrc_French_Canadian2	/* vt3xx */
    ,nrc_German			/* vt2xx */
    ,nrc_Greek			/* vt5xx */
    ,nrc_DEC_Greek_Supp		/* vt5xx */
    ,nrc_ISO_Greek_Supp		/* vt5xx */
    ,nrc_DEC_Hebrew_Supp	/* vt5xx */
    ,nrc_Hebrew			/* vt5xx */
    ,nrc_ISO_Hebrew_Supp	/* vt5xx */
    ,nrc_Italian		/* vt2xx */
    ,nrc_ISO_Latin_1_Supp	/* vt3xx */
    ,nrc_ISO_Latin_2_Supp	/* vt5xx */
    ,nrc_ISO_Latin_5_Supp	/* vt5xx */
    ,nrc_ISO_Latin_Cyrillic	/* vt5xx */
    ,nrc_JIS_Katakana		/* vt382 */
    ,nrc_JIS_Roman		/* vt382 */
    ,nrc_Norwegian_Danish	/* vt3xx */
    ,nrc_Norwegian_Danish2	/* vt2xx */
    ,nrc_Norwegian_Danish3	/* vt2xx */
    ,nrc_Portugese		/* vt3xx */
    ,nrc_Russian		/* vt5xx */
    ,nrc_SCS_NRCS		/* vt5xx - probably Serbo/Croatian */
    ,nrc_Spanish		/* vt2xx */
    ,nrc_Swedish		/* vt2xx */
    ,nrc_Swedish2		/* vt2xx */
    ,nrc_Swiss			/* vt2xx */
    ,nrc_DEC_Turkish_Supp	/* vt5xx */
    ,nrc_Turkish		/* vt5xx */
    ,nrc_Unknown
} DECNRCM_codes;

/*
 * Default and alternate codes for user-preferred supplemental set.
 */
#define DFT_UPSS nrc_DEC_Supp_Graphic
#define ALT_UPSS nrc_ISO_Latin_1_Supp
#define PreferredUPSS(screen)	((screen)->prefer_latin1 ? ALT_UPSS : DFT_UPSS)

/*
 * Use this enumerated type to check consistency among dpmodes(), savemodes()
 * restoremodes() and do_dec_rqm().
 */
typedef enum {
    srm_DECCKM = 1		/* Cursor Keys Mode */
    ,srm_DECANM = 2		/* ANSI Mode */
    ,srm_DECCOLM = 3		/* Column Mode */
    ,srm_DECSCLM = 4		/* Scrolling Mode */
    ,srm_DECSCNM = 5		/* Screen Mode */
    ,srm_DECOM = 6		/* Origin Mode */
    ,srm_DECAWM = 7		/* Autowrap Mode */
    ,srm_DECARM = 8		/* Autorepeat Mode */
    ,srm_X10_MOUSE = SET_X10_MOUSE
#if OPT_TOOLBAR
    ,srm_RXVT_TOOLBAR = 10
#else
    ,srm_DECEDM = 10		/* vt330:edit */
#endif
    ,srm_DECLTM = 11		/* vt330:line transmit */
#if OPT_BLINK_CURS
    ,srm_ATT610_BLINK = 12
    ,srm_CURSOR_BLINK_OPS = 13
    ,srm_XOR_CURSOR_BLINKS = 14
#else
    ,srm_DECKANAM = 12		/* vt382:Katakana shift */
    ,srm_DECSCFDM = 13		/* vt330:space compression field delimiter */
    ,srm_DECTEM = 14		/* vt330:transmission execution */
#endif
    ,srm_DECEKEM = 16		/* vt330:edit key execution */
    ,srm_DECPFF = 18		/* vt220:Print Form Feed Mode */
    ,srm_DECPEX = 19		/* vt220:Printer Extent Mode */
    ,srm_DECTCEM = 25		/* Text Cursor Enable Mode */
    ,srm_RXVT_SCROLLBAR = 30
    ,srm_DECRLM = 34		/* vt510:Cursor Right to Left Mode */
#if OPT_SHIFT_FONTS
    ,srm_RXVT_FONTSIZE = 35	/* also vt520:DECHEBM */
#else
    ,srm_DECHEBM = 35		/* vt520:Hebrew keyboard mapping */
#endif
    ,srm_DECHEM = 36		/* vt510:Hebrew Encoding Mode */
#if OPT_TEK4014
    ,srm_DECTEK = 38
#endif
    ,srm_132COLS = 40
    ,srm_CURSES_HACK = 41
    ,srm_DECNRCM = 42		/* National Replacement Character Set Mode */
#if OPT_PRINT_GRAPHICS
    ,srm_DECGEPM = 43		/* Graphics Expanded Print Mode */
#endif
    ,srm_MARGIN_BELL = 44	/* also DECGPCM (Graphics Print Color Mode) */
    ,srm_REVERSEWRAP = 45	/* also DECGPCS (Graphics Print Color Syntax) */
#ifdef ALLOWLOGGING
    ,srm_ALLOWLOGGING = 46	/* also DECGPBM (Graphics Print Background Mode) */
#elif OPT_PRINT_GRAPHICS
    ,srm_DECGPBM = 46		/* Graphics Print Background Mode */
#endif
    ,srm_ALTBUF = 47		/* also DECGRPM (Graphics Rotated Print Mode) */
    ,srm_DEC131TM = 53		/* vt330:VT131 transmit */
    ,srm_DECNAKB = 57		/* vt510:Greek/N-A Keyboard Mapping */
    ,srm_DECIPEM = 58		/* vt510:IBM ProPrinter Emulation Mode */
    ,srm_DECKKDM = 59   	/* vt382:Kanji/Katakana */
    ,srm_DECHCCM = 60		/* vt420:Horizontal Cursor-Coupling Mode */
    ,srm_DECVCCM = 61		/* vt420:Vertical Cursor-Coupling Mode */
    ,srm_DECPCCM = 64		/* vt420:Page Cursor-Coupling Mode */
    ,srm_DECNKM = 66		/* vt420:Numeric Keypad Mode */
    ,srm_DECBKM = 67		/* vt420:Backarrow Key mode */
    ,srm_DECKBUM = 68		/* vt420:Keyboard Usage mode */
    ,srm_DECLRMM = 69		/* vt420:Vertical Split Screen Mode (DECVSSM) */
    ,srm_DECXRLM = 73		/* vt420:Transmit Rate Limiting */
#if OPT_SIXEL_GRAPHICS
    ,srm_DECSDM = 80		/* vt320:Sixel Display Mode */
#endif
    ,srm_DECKPM = 81		/* vt420:Key Position Mode */
    ,srm_DECNCSM = 95		/* vt510:No Clearing Screen On Column Change */
    ,srm_DECRLCM = 96		/* vt510:Right-to-Left Copy */
    ,srm_DECCRTSM = 97		/* vt510:CRT Save Mode */
    ,srm_DECARSM = 98		/* vt510:Auto Resize Mode */
    ,srm_DECMCM = 99		/* vt510:Modem Control Mode */
    ,srm_DECAAM = 100		/* vt510:Auto Answerback Mode */
    ,srm_DECCANSM = 101		/* vt510:Conceal Answerback Message Mode */
    ,srm_DECNULM = 102		/* vt510:Ignoring Null Mode */
    ,srm_DECHDPXM = 103		/* vt510:Half-Duplex Mode */
    ,srm_DECESKM = 104		/* vt510:enable secondary keyboard language */
    ,srm_DECOSCNM = 106		/* vt510:Overscan Mode */
    ,srm_DECNUMLK = 108		/* vt510:Num Lock Mode */
    ,srm_DECCAPSLK = 109	/* vt510:Caps Lock Mode */
    ,srm_DECKLHIM = 110		/* vt510:Keyboard LEDs Host Indicator Mode */
    ,srm_DECFWM = 111		/* vt520:Framed Windows Mode */
    ,srm_DECRPL = 112		/* vt520:Review Previous Lines */
    ,srm_DECHWUM = 113		/* vt520:Host Wake-Up */
    ,srm_DECATCUM = 114		/* vt520:Alternate Text Color Underline */
    ,srm_DECATCBM = 115		/* vt520:Alternate Text Color Blink */
    ,srm_DECBBSM = 116		/* vt520:Bold and Blink Style Mode */
    ,srm_DECECM = 117		/* vt520:Erase Color Mode */
    ,srm_VT200_MOUSE = SET_VT200_MOUSE
    ,srm_VT200_HIGHLIGHT_MOUSE = SET_VT200_HIGHLIGHT_MOUSE
    ,srm_BTN_EVENT_MOUSE = SET_BTN_EVENT_MOUSE
    ,srm_ANY_EVENT_MOUSE = SET_ANY_EVENT_MOUSE
#if OPT_FOCUS_EVENT
    ,srm_FOCUS_EVENT_MOUSE = SET_FOCUS_EVENT_MOUSE
#endif
    ,srm_EXT_MODE_MOUSE = SET_EXT_MODE_MOUSE
    ,srm_SGR_EXT_MODE_MOUSE = SET_SGR_EXT_MODE_MOUSE
    ,srm_URXVT_EXT_MODE_MOUSE = SET_URXVT_EXT_MODE_MOUSE
    ,srm_PIXEL_POSITION_MOUSE = SET_PIXEL_POSITION_MOUSE
    ,srm_ALTERNATE_SCROLL = SET_ALTERNATE_SCROLL
    ,srm_RXVT_SCROLL_TTY_OUTPUT = 1010
    ,srm_RXVT_SCROLL_TTY_KEYPRESS = 1011
    ,srm_FAST_SCROLL = 1014
    ,srm_EIGHT_BIT_META = 1034
#if OPT_NUM_LOCK
    ,srm_REAL_NUMLOCK = 1035
    ,srm_META_SENDS_ESC = 1036
#endif
    ,srm_DELETE_IS_DEL = 1037
#if OPT_NUM_LOCK
    ,srm_ALT_SENDS_ESC = 1039
#endif
    ,srm_KEEP_SELECTION = 1040
    ,srm_SELECT_TO_CLIPBOARD = 1041
    ,srm_BELL_IS_URGENT = 1042
    ,srm_POP_ON_BELL = 1043
    ,srm_KEEP_CLIPBOARD = 1044
    ,srm_REVERSEWRAP2 = 1045	/* reverse-wrap without limits */
    ,srm_ALLOW_ALTBUF = 1046
    ,srm_OPT_ALTBUF = 1047
    ,srm_SAVE_CURSOR = 1048
    ,srm_OPT_ALTBUF_CURSOR = 1049
#if OPT_TCAP_FKEYS
    ,srm_TCAP_FKEYS = 1050
#endif
#if OPT_SUN_FUNC_KEYS
    ,srm_SUN_FKEYS = 1051
#endif
#if OPT_HP_FUNC_KEYS
    ,srm_HP_FKEYS = 1052
#endif
#if OPT_SCO_FUNC_KEYS
    ,srm_SCO_FKEYS = 1053
#endif
    ,srm_LEGACY_FKEYS = 1060
#if OPT_SUNPC_KBD
    ,srm_VT220_FKEYS = 1061
#endif
#if OPT_GRAPHICS
    ,srm_PRIVATE_COLOR_REGISTERS = 1070
#endif
#if OPT_PASTE64 || OPT_READLINE
    ,srm_PASTE_IN_BRACKET = SET_PASTE_IN_BRACKET
#endif
#if OPT_READLINE
    ,srm_BUTTON1_MOVE_POINT = SET_BUTTON1_MOVE_POINT
    ,srm_BUTTON2_MOVE_POINT = SET_BUTTON2_MOVE_POINT
    ,srm_DBUTTON3_DELETE = SET_DBUTTON3_DELETE
    ,srm_PASTE_QUOTE = SET_PASTE_QUOTE
    ,srm_PASTE_LITERAL_NL = SET_PASTE_LITERAL_NL
#endif				/* OPT_READLINE */
#if OPT_SIXEL_GRAPHICS
    ,srm_SIXEL_SCROLLS_RIGHT = 8452
#endif
} DECSET_codes;

/* internal codes for selection atoms */
typedef enum {
    PRIMARY_CODE = 0
    ,CLIPBOARD_CODE
    ,SECONDARY_CODE
    ,MAX_SELECTION_CODES
} SelectionCodes;

/* indices for mapping multiple clicks to selection types */
typedef enum {
    Select_CHAR=0
    ,Select_WORD
    ,Select_LINE
    ,Select_GROUP
    ,Select_PAGE
    ,Select_ALL
#if OPT_SELECT_REGEX
    ,Select_REGEX
#endif
    ,NSELECTUNITS
} SelectUnit;

#if OPT_BLINK_CURS
typedef enum {
    cbFalse = 0
    , cbTrue
    , cbAlways
    , cbNever
    , cbLAST
} BlinkOps;
#endif

typedef enum {
    ecSetColor = 1
    , ecGetColor
    , ecGetAnsiColor
    , ecLAST
} ColorOps;

typedef enum {
    efSetFont = 1
    , efGetFont
    , efLAST
} FontOps;

typedef enum {
    esFalse = 0
    , esTrue
    , esAlways
    , esNever
    , esLAST
} FullscreenOps;

#ifndef NO_ACTIVE_ICON
typedef enum {
    eiFalse = 0
    , eiTrue
    , eiDefault
    , eiLAST
} AIconOps;
#endif

typedef enum {
    emX10 = 1
    , emLocator
    , emVT200Click
    , emVT200Hilite
    , emAnyButton
    , emAnyEvent
    , emFocusEvent
    , emExtended
    , emSGR
    , emURXVT
    , emAlternateScroll
    , emLAST
} MouseOps;

typedef enum {
#define DATA(name) ep##name
    DATA(NUL) = 0
    , DATA(SOH) =  1
    , DATA(STX) =  2
    , DATA(ETX) =  3
    , DATA(EOT) =  4
    , DATA(ENQ) =  5
    , DATA(ACK) =  6
    , DATA(BEL) =  7
    , DATA(BS)  =  8
    , DATA(HT)  =  9
    , DATA(LF)  = 10
    , DATA(VT)  = 11
    , DATA(FF)  = 12
    , DATA(CR)  = 13
    , DATA(SO)  = 14
    , DATA(SI)  = 15
    , DATA(DLE) = 16
    , DATA(DC1) = 17
    , DATA(DC2) = 18
    , DATA(DC3) = 19
    , DATA(DC4) = 20
    , DATA(NAK) = 21
    , DATA(SYN) = 22
    , DATA(ETB) = 23
    , DATA(CAN) = 24
    , DATA(EM)  = 25
    , DATA(SUB) = 26
    , DATA(ESC) = 27
    , DATA(FS)  = 28
    , DATA(GS)  = 29
    , DATA(RS)  = 30
    , DATA(US)  = 31
    /* aliases */
    , DATA(C0)
    , DATA(DEL)
    , DATA(STTY)
#undef DATA
    , epLAST
} PasteControls;

typedef enum {			/* legal values for keyboard.shift_escape */
    ssFalse = 0
    , ssTrue = 1
    , ssAlways = 2
    , ssNever = 3
    , ssLAST
} ShiftEscapeOps;

/*
 * xterm uses these codes for the its push-SGR feature.  They match where
 * possible the corresponding SGR coding.  The foreground and background colors
 * do not fit into that scheme (because they are a set of ranges), so those are
 * chosen arbitrarily -TD
 */
typedef enum {
    psBOLD = 1
#if OPT_WIDE_ATTRS
    , psATR_FAINT = 2
    , psATR_ITALIC = 3
#endif
    , psUNDERLINE = 4
    , psBLINK = 5
    , psINVERSE = 7
    , psINVISIBLE = 8
#if OPT_WIDE_ATTRS
    , psATR_STRIKEOUT = 9
#endif
    /* SGR 10-19 correspond to primary/alternate fonts, currently unused */
#if OPT_ISO_COLORS
    , psFG_COLOR_obs = 10
    , psBG_COLOR_obs = 11
#endif
#if OPT_WIDE_ATTRS
    , psATR_DBL_UNDER = 21
#endif
    /* SGR 22-29 mostly are used to reset SGR 1-9 */
#if OPT_ISO_COLORS
    , psFG_COLOR = 30	/* stack maps many colors to one state */
    , psBG_COLOR = 31
#endif
    , MAX_PUSH_SGR
} PushSGR;

typedef enum {
    etSetTcap = 1
    , etGetTcap
    , etLAST
} TcapOps;

typedef enum {
    /* 1-23 are chosen to be the same as the control-sequence coding */
    ewRestoreWin = 1
    , ewMinimizeWin = 2
    , ewSetWinPosition = 3
    , ewSetWinSizePixels = 4
    , ewRaiseWin = 5
    , ewLowerWin = 6
    , ewRefreshWin = 7
    , ewSetWinSizeChars = 8
#if OPT_MAXIMIZE
    , ewMaximizeWin = 9
    , ewFullscreenWin = 10
#endif
    , ewGetWinState = 11
    , ewGetWinPosition = 13
    , ewGetWinSizePixels = 14
#if OPT_MAXIMIZE
    , ewGetScreenSizePixels = 15
    , ewGetCharSizePixels = 16
#endif
    , ewGetWinSizeChars = 18
#if OPT_MAXIMIZE
    , ewGetScreenSizeChars = 19
#endif
    , ewGetIconTitle = 20
    , ewGetWinTitle = 21
#if OPT_TITLE_MODES
    , ewPushTitle = 22
    , ewPopTitle = 23
#endif
    /* these do not fit into that scheme, which is why we use an array */
    , ewSetWinLines
    , ewColumnMode
#if OPT_SET_XPROP
    , ewSetXprop
#endif
#if OPT_PASTE64
    , ewGetSelection
    , ewSetSelection
#endif
#if OPT_DEC_RECTOPS
    , ewGetChecksum
    , ewSetChecksum
#endif
#if OPT_STATUS_LINE
    , ewStatusLine
#endif
    /* get the size of the array... */
    , ewLAST
} WindowOps;

/***====================================================================***/

#define	COLOR_DEFINED(s,w)	((s)->which & (unsigned) (1<<(w)))
#define	COLOR_VALUE(s,w)	((s)->colors[w])
#define	SET_COLOR_VALUE(s,w,v)	(((s)->colors[w] = (v)), UIntSet((s)->which, (1<<(w))))

#define	COLOR_NAME(s,w)		((s)->names[w])
#define	SET_COLOR_NAME(s,w,v)	(((s)->names[w] = (v)), ((s)->which |= (unsigned) (1<<(w))))

#define	UNDEFINE_COLOR(s,w)	((s)->which &= (~((w)<<1)))

/***====================================================================***/

#if OPT_ISO_COLORS
#if OPT_WIDE_ATTRS
#define COLOR_FLAGS		(FG_COLOR | BG_COLOR | ATR_DIRECT_FG | ATR_DIRECT_BG)
#else
#define COLOR_FLAGS		(FG_COLOR | BG_COLOR)
#endif
#define TERM_COLOR_FLAGS(xw)	((xw)->flags & COLOR_FLAGS)
#define COLOR_0		0
#define COLOR_1		1
#define COLOR_2		2
#define COLOR_3		3
#define COLOR_4		4
#define COLOR_5		5
#define COLOR_6		6
#define COLOR_7		7
#define COLOR_8		8
#define COLOR_9		9
#define COLOR_10	10
#define COLOR_11	11
#define COLOR_12	12
#define COLOR_13	13
#define COLOR_14	14
#define COLOR_15	15
#define MIN_ANSI_COLORS 16

#if OPT_256_COLORS
# define NUM_ANSI_COLORS 256
#elif OPT_88_COLORS
# define NUM_ANSI_COLORS 88
#else
# define NUM_ANSI_COLORS MIN_ANSI_COLORS
#endif

#define okIndexedColor(n) ((n) >= 0 && (n) < NUM_ANSI_COLORS)

#if NUM_ANSI_COLORS > MIN_ANSI_COLORS
# define OPT_EXT_COLORS  1
#else
# define OPT_EXT_COLORS  0
#endif

#define COLOR_BD	(NUM_ANSI_COLORS)	/* BOLD */
#define COLOR_UL	(NUM_ANSI_COLORS+1)	/* UNDERLINE */
#define COLOR_BL	(NUM_ANSI_COLORS+2)	/* BLINK */
#define COLOR_RV	(NUM_ANSI_COLORS+3)	/* REVERSE */

#if OPT_WIDE_ATTRS
#define COLOR_IT	(NUM_ANSI_COLORS+4)	/* ITALIC */
#define MAXCOLORS	(NUM_ANSI_COLORS+5)
#else
#define MAXCOLORS	(NUM_ANSI_COLORS+4)
#endif

#ifndef DFT_COLORMODE
#define DFT_COLORMODE True	/* default colorMode resource */
#endif

#define UseItalicFont(screen) (!(screen)->colorITMode)

#define ReverseOrHilite(screen,flags,hilite) \
		(( screen->colorRVMode && hilite ) || \
		    ( !screen->colorRVMode && \
		      (( (flags & INVERSE) && !hilite) || \
		       (!(flags & INVERSE) &&  hilite)) ))

#else	/* !OPT_ISO_COLORS */

#define TERM_COLOR_FLAGS(xw) 0

#define UseItalicFont(screen) True
#define ReverseOrHilite(screen,flags,hilite) \
		      (( (flags & INVERSE) && !hilite) || \
		       (!(flags & INVERSE) &&  hilite))

#endif	/* OPT_ISO_COLORS */

typedef enum {
	XK_TCAPNAME = 3
	/* Define fake XK codes, we need those for the fake color response in
	 * xtermcapKeycode().
	 */
#if OPT_ISO_COLORS
	, XK_COLORS
	, XK_RGB
#endif
} TcapQuery;

#if OPT_AIX_COLORS
#define if_OPT_AIX_COLORS(screen, code) if(screen->colorMode) code
#else
#define if_OPT_AIX_COLORS(screen, code) /* nothing */
#endif

#if OPT_256_COLORS || OPT_88_COLORS || OPT_ISO_COLORS
# define if_OPT_ISO_COLORS(screen, code) if (screen->colorMode) code
#else
# define if_OPT_ISO_COLORS(screen, code) /* nothing */
#endif

#if OPT_DIRECT_COLOR
# define if_OPT_DIRECT_COLOR(screen, code) if (screen->direct_color) code
# define if_OPT_DIRECT_COLOR2(screen, test, code) if (screen->direct_color && (test)) code
#else
# define if_OPT_DIRECT_COLOR(screen, code) /* nothing */
# define if_OPT_DIRECT_COLOR2(screen, test, code) /* nothing */
#endif

#define if_OPT_DIRECT_COLOR2_else(cond, test, stmt) \
	if_OPT_DIRECT_COLOR2(cond, test, stmt else)

#if OPT_REPORT_COLORS
#define if_OPT_REPORT_COLORS(stmt) if (resource.reportColors) stmt
#else
#define if_OPT_REPORT_COLORS(stmt) /* nothing */
#endif

#define COLOR_RES_NAME(root) "color" root

#if OPT_COLOR_CLASS
#define COLOR_RES_CLASS(root) "Color" root
#else
#define COLOR_RES_CLASS(root) XtCForeground
#endif

#define COLOR_RES(root,offset,value) Sres(COLOR_RES_NAME(root), COLOR_RES_CLASS(root), offset.resource, value)
#define COLOR_RES2(name,class,offset,value) Sres(name, class, offset.resource, value)

#define CLICK_RES_NAME(count)  "on" count "Clicks"
#define CLICK_RES_CLASS(count) "On" count "Clicks"
#define CLICK_RES(count,offset,value) Sres(CLICK_RES_NAME(count), CLICK_RES_CLASS(count), offset, value)

/***====================================================================***/

#if OPT_DEC_CHRSET
#define if_OPT_DEC_CHRSET(code) code
	/* Use 2 bits for encoding the double high/wide sense of characters */
#define CSET_SWL        0	/* character set: single-width line */
#define CSET_DHL_TOP    1	/* character set: double-height top line */
#define CSET_DHL_BOT    2	/* character set: double-height bottom line */
#define CSET_DWL        3	/* character set: double-width line */
#define NUM_CHRSET      8	/* normal/bold and 4 CSET_xxx values */

	/* Use remaining bits for encoding the other character-sets */
#define CSET_NORMAL(code)  ((code) == CSET_SWL)
#define CSET_DOUBLE(code)  (!CSET_NORMAL(code) && !CSET_EXTEND(code))
#define CSET_EXTEND(code)  ((int)(code) > CSET_DWL)

#define DBLCS_BITS            4
#define DBLCS_MASK            BITS2MASK(DBLCS_BITS)

#define GetLineDblCS(ld)      ((ld) != NULL ? (((ld)->bufHead >> LINEFLAG_BITS) & DBLCS_MASK) : 0)
#define SetLineDblCS(ld,cs)   (ld)->bufHead = (RowData) ((ld->bufHead & LINEFLAG_MASK) | (cs << LINEFLAG_BITS))

#define LineCharSet(screen, ld) \
	(unsigned) ((CSET_DOUBLE(GetLineDblCS(ld))) \
		    ? GetLineDblCS(ld) \
		    : (screen)->cur_chrset)
#define LineMaxCol(screen, ld) \
	(CSET_DOUBLE(GetLineDblCS(ld)) \
	 ? (screen->max_col / 2) \
	 : (screen->max_col))
#define LineCursorX(screen, ld, col) \
	(CSET_DOUBLE(GetLineDblCS(ld)) \
	 ? CursorX(screen, 2*(col)) \
	 : CursorX(screen, (col)))
#define LineFontWidth(screen, ld) \
	(CSET_DOUBLE(GetLineDblCS(ld)) \
	 ? 2*FontWidth(screen) \
	 : FontWidth(screen))
#else

#define if_OPT_DEC_CHRSET(code) /*nothing*/
#define CSET_SWL                        0
#define GetLineDblCS(ld)                0U
#define LineCharSet(screen, ld)         0U
#define LineMaxCol(screen, ld)          screen->max_col
#define LineCursorX(screen, ld, col)    CursorX(screen, col)
#define LineFontWidth(screen, ld)       FontWidth(screen)

#endif

#if OPT_LUIT_PROG && !OPT_WIDE_CHARS
/* Luit requires the wide-chars configuration */
#undef OPT_LUIT_PROG
#define OPT_LUIT_PROG 0
#endif

/***====================================================================***/

#if OPT_DEC_RECTOPS
#define if_OPT_DEC_RECTOPS(stmt) stmt
#else
#define if_OPT_DEC_RECTOPS(stmt) /* nothing */
#endif

/***====================================================================***/

#define CONTROL(a) ((a) & 037)

#define XTERM_ERASE CONTROL('H')
#define XTERM_LNEXT CONTROL('V')

/***====================================================================***/

#if OPT_TEK4014
#define TEK4014_ACTIVE(xw)      ((xw)->misc.TekEmu)
#define TEK4014_SHOWN(xw)       ((xw)->misc.Tshow)
#define CURRENT_EMU_VAL(tek,vt) (TEK4014_ACTIVE(term) ? tek : vt)
#define CURRENT_EMU()           CURRENT_EMU_VAL((Widget)tekWidget, (Widget)term)
#else
#define TEK4014_ACTIVE(screen)  0
#define TEK4014_SHOWN(xw)       0
#define CURRENT_EMU_VAL(tek,vt) (vt)
#define CURRENT_EMU()           ((Widget)term)
#endif

/***====================================================================***/

#if OPT_TOOLBAR
#define SHELL_OF(widget) XtParent(XtParent(widget))
#else
#define SHELL_OF(widget) XtParent(widget)
#endif

/***====================================================================***/

#if OPT_VT52_MODE
#define if_OPT_VT52_MODE(screen, code) if(screen->vtXX_level == 0) code
#else
#define if_OPT_VT52_MODE(screen, code) /* nothing */
#endif

/***====================================================================***/

#if OPT_XMC_GLITCH
#define if_OPT_XMC_GLITCH(screen, code) if(screen->xmc_glitch) code
#define XMC_GLITCH 1	/* the character we'll show */
#define XMC_FLAGS (INVERSE|UNDERLINE|BOLD|BLINK)
#else
#define if_OPT_XMC_GLITCH(screen, code) /* nothing */
#endif

/***====================================================================***/

typedef unsigned IFlags;	/* at least 32 bits */

#if OPT_WIDE_ATTRS
typedef unsigned short IAttr;	/* at least 16 bits */
#else
typedef unsigned char IAttr;	/* at least 8 bits */
#endif

/***====================================================================***/

#define LO_BYTE(ch) CharOf((ch) & 0xff)
#define HI_BYTE(ch) CharOf((ch) >> 8)

#if OPT_WIDE_CHARS
#define if_OPT_WIDE_CHARS(screen, code) if(screen->wide_chars) code
#define if_WIDE_OR_NARROW(screen, wide, narrow) if(screen->wide_chars) wide else narrow
#define NARROW_ICHAR    0xffff
#if OPT_WIDER_ICHAR
#define is_NON_CHAR(c)          (((c) >= 0xffd0 && (c) <= 0xfdef) || \
                                 (((c) & 0xffff) >= 0xfffe))
#define is_UCS_SPECIAL(c)       ((c) >= 0xfff0 && (c) <= 0xffff)
#define WIDEST_ICHAR    0x1fffff
typedef unsigned IChar;         /* for 8-21 bit characters */
#else
#define is_NON_CHAR(c)          (((c) >= 0xffd0 && (c) <= 0xfdef) || \
                                 ((c) >= 0xfffe && (c) <= 0xffff))
#define is_UCS_SPECIAL(c)       ((c) >= 0xfff0)
#define WIDEST_ICHAR    NARROW_ICHAR
typedef unsigned short IChar;	/* for 8-16 bit characters */
#endif
#else	/* !OPT_WIDE_CHARS */
#undef OPT_WIDER_ICHAR
#define OPT_WIDER_ICHAR 0
#define is_NON_CHAR(c)          ((c) > 255)
#define if_OPT_WIDE_CHARS(screen, code) /* nothing */
#define if_WIDE_OR_NARROW(screen, wide, narrow) narrow
typedef unsigned char IChar;	/* for 8-bit characters */
#endif

/***====================================================================***/

#ifndef RES_OFFSET
#define RES_OFFSET(offset) XtOffsetOf(XtermWidgetRec, offset)
#endif

#define RES_NAME(name) name
#define RES_CLASS(name) name

#define Bres(name, class, offset, dftvalue) \
	{RES_NAME(name), RES_CLASS(class), XtRBoolean, sizeof(Boolean), \
	 RES_OFFSET(offset), XtRImmediate, (XtPointer) dftvalue}

#define Cres(name, class, offset, dftvalue) \
	{RES_NAME(name), RES_CLASS(class), XtRPixel, sizeof(Pixel), \
	 RES_OFFSET(offset), XtRString, DECONST(char,dftvalue)}

#define Tres(name, class, offset, dftvalue) \
	COLOR_RES2(name, class, screen.Tcolors[offset], dftvalue) \

#define Fres(name, class, offset, dftvalue) \
	{RES_NAME(name), RES_CLASS(class), XtRFontStruct, sizeof(XFontStruct *), \
	 RES_OFFSET(offset), XtRString, DECONST(char,dftvalue)}

#define Ires(name, class, offset, dftvalue) \
	{RES_NAME(name), RES_CLASS(class), XtRInt, sizeof(int), \
	 RES_OFFSET(offset), XtRImmediate, (XtPointer) dftvalue}

#define Dres(name, class, offset, dftvalue) \
	{RES_NAME(name), RES_CLASS(class), XtRFloat, sizeof(float), \
	 RES_OFFSET(offset), XtRString, DECONST(char,dftvalue)}

#define Sres(name, class, offset, dftvalue) \
	{RES_NAME(name), RES_CLASS(class), XtRString, sizeof(char *), \
	 RES_OFFSET(offset), XtRString, DECONST(char,dftvalue)}

#define Wres(name, class, offset, dftvalue) \
	{RES_NAME(name), RES_CLASS(class), XtRWidget, sizeof(Widget), \
	 RES_OFFSET(offset), XtRWidget, (XtPointer) dftvalue}

/***====================================================================***/

#define FRG_SIZE resource.minBufSize
#define BUF_SIZE resource.maxBufSize

typedef struct {
	Char    *next;
	Char    *last;
	int      update;	/* HandleInterpret */
#if OPT_WIDE_CHARS
	IChar    utf_data;	/* resulting character */
	size_t   utf_size;	/* ...number of bytes decoded */
	Char    *write_buf;
	size_t   write_len;
#endif
	Char     buffer[1];
} PtyData;

/***====================================================================***/

/*
 * Pixel (and its components) are declared as unsigned long, but even for RGB
 * we need no more than 32-bits.
 */
typedef uint32_t MyPixel;
typedef int32_t MyColor;

#if OPT_ISO_COLORS
#if OPT_DIRECT_COLOR
typedef struct {
    MyColor fg;
    MyColor bg;
} CellColor;

#define isSameCColor(p,q) (!memcmp(&(p), &(q), sizeof(CellColor)))

#elif OPT_256_COLORS || OPT_88_COLORS

#define COLOR_BITS 8
typedef unsigned short CellColor;

#else

#define COLOR_BITS 4
typedef Char CellColor;

#endif
#else
typedef unsigned CellColor;
#endif

#define NO_COLOR		((unsigned)-1)

#ifndef isSameCColor
#define isSameCColor(p,q)	((p) == (q))
#endif

#define BITS2MASK(b)		(xBIT(b) - 1)

#define COLOR_MASK		BITS2MASK(COLOR_BITS)

#if OPT_DIRECT_COLOR
#define clrDirectFG(flags)	UIntClr(flags, ATR_DIRECT_FG)
#define clrDirectBG(flags)	UIntClr(flags, ATR_DIRECT_BG)
#define GetCellColorFG(data)	((data).fg)
#define GetCellColorBG(data)	((data).bg)
#define hasDirectFG(flags)	((flags) & ATR_DIRECT_FG)
#define hasDirectBG(flags)	((flags) & ATR_DIRECT_BG)
#define setDirectFG(flags,test)	if (test) UIntSet(flags, ATR_DIRECT_FG); else UIntClr(flags, ATR_DIRECT_FG)
#define setDirectBG(flags,test)	if (test) UIntSet(flags, ATR_DIRECT_BG); else UIntClr(flags, ATR_DIRECT_BG)
#elif OPT_ISO_COLORS
#define clrDirectFG(flags)	/* nothing */
#define clrDirectBG(flags)	/* nothing */
#define GetCellColorFG(data)	((data) & COLOR_MASK)
#define GetCellColorBG(data)	(((data) >> COLOR_BITS) & COLOR_MASK)
#define hasDirectFG(flags)	0
#define hasDirectBG(flags)	0
#define setDirectFG(flags,test)	(void)(test)
#define setDirectBG(flags,test)	(void)(test)
#else
#define GetCellColorFG(data)	7
#define GetCellColorBG(data)	0
#endif
extern CellColor blank_cell_color;

typedef Char RowData;		/* wrap/blink, and DEC single-double chars */

#define LINEFLAG_BITS		4
#define LINEFLAG_MASK		BITS2MASK(LINEFLAG_BITS)

#define GetLineFlags(ld)	((ld)->bufHead & LINEFLAG_MASK)

#if OPT_DEC_CHRSET
#define SetLineFlags(ld,xx)	(ld)->bufHead = (RowData) ((ld->bufHead & (DBLCS_MASK << LINEFLAG_BITS)) | (xx & LINEFLAG_MASK))
#else
#define SetLineFlags(ld,xx)	(ld)->bufHead = (RowData) (xx & LINEFLAG_MASK)
#endif

typedef IChar CharData;

/*
 * This is the xterm line-data/scrollback structure.
 */
typedef struct {
	Dimension lineSize;	/* number of columns in this row */
	RowData	 bufHead;	/* flag for wrapped lines */
#if OPT_WIDE_CHARS
	Char	 combSize;	/* number of items in combData[] */
#endif
#if OPT_DEC_RECTOPS
	Char	 *charSets;	/* SCS code (DECNRCM_codes) */
	Char     *charSeen;	/* pre-SCS value */
#endif
	IAttr	 *attribs;	/* video attributes */
#if OPT_ISO_COLORS
	CellColor *color;	/* foreground+background color numbers */
#endif
	CharData *charData;	/* cell's base character */
	CharData *combData[1];	/* first field past fixed-offsets */
} LineData;

typedef const LineData CLineData;

/*
 * We use CellData in a few places, when copying a cell's data to a temporary
 * variable.
 */
typedef struct {
	IAttr    attribs;	/* video attributes */
#if OPT_WIDE_CHARS
	Char     combSize;	/* number of items in combData[] */
#endif
#if OPT_DEC_RECTOPS
	Char     charSets;	/* SCS code (DECNRCM_codes) */
	Char     charSeen;	/* pre-SCS value */
#endif
#if OPT_ISO_COLORS
	CellColor color;	/* foreground+background color numbers */
#endif
	CharData charData;	/* cell's base character */
	CharData combData[1];	/* array of combining chars */
} CellData;

#define for_each_combData(off, ld) for (off = 0; off < ld->combSize; ++off)

#define Clear1Cell(ld, x) \
	do { \
	    ld->charData[x] = ' '; \
	    do { \
	    if_OPT_WIDE_CHARS(screen, { \
		size_t z; \
		for_each_combData(z, ld) { \
		    ld->combData[z][x] = '\0'; \
		} \
	    }) } while (0); \
	} while (0)

#define Clear2Cell(dst, src, x) \
	do { \
	    dst->charData[x] = ' '; \
	    dst->attribs[x] = src->attribs[x]; \
	    do { \
	    if_OPT_ISO_COLORS(screen, { \
		dst->color[x] = src->color[x]; \
	    }) } while (0); \
	    do { \
	    if_OPT_WIDE_CHARS(screen, { \
		size_t z; \
		for_each_combData(z, dst) { \
		    dst->combData[z][x] = '\0'; \
		} \
	    }) } while (0); \
	} while (0)

/*
 * Accommodate older compilers by not using variable-length arrays.
 */
#define SizeOfLineData  offsetof(LineData, combData)
#define SizeOfCellData  offsetof(CellData, combData)

/*
 * CellData size depends on the "combiningChars" resource.
 */
#define CellDataSize(screen) (SizeOfCellData + screen->cellExtra)

	/*
	 * A "row" is the index within the visible part of the screen, and an
	 * "inx" is the index within the whole set of scrollable lines.
	 */
#define ROW2INX(screen, row)	((row) + (screen)->topline)
#define INX2ROW(screen, inx)	((inx) - (screen)->topline)

/* these are unused but could be useful for debugging */
#if 0
#define ROW2ABS(screen, row)	((row) + (screen)->savedlines)
#define INX2ABS(screen, inx)	ROW2ABS(screen, INX2ROW(screen, inx))
#endif

#define okScrnRow(screen, row) \
	((row) <= ((screen)->max_row - (screen)->topline) \
      && (row) >= -((screen)->savedlines))

	/*
	 * Cache data for "proportional" and other fonts containing a mixture
	 * of widths.
	 */
typedef struct {
	Bool		mixed;
	Dimension	min_width;	/* nominal cell width for 0..255 */
	Dimension	max_width;	/* maximum cell width */
} XTermFontInfo;

	/*
	 * Map of characters to simplify/speed-up the checks for missing glyphs
	 * at runtime.
	 *
	 * FIXME: initially implement for Xft, but replace known_missing[] in
	 * X11 fonts as well.
	 */
typedef Char XTfontNum;
typedef struct {
	int		depth;		/* number of fonts merged for map */
	size_t		limit;		/* allocated size of per_font, etc */
	size_t		first_char;	/* merged first-character index */
	size_t		last_char;	/* merged last-character index */
	XTfontNum *	per_font;	/* index 1-n of first font with char */
} XTermFontMap;

typedef enum {
	fwNever = 0,
	fwResource,
	fwAlways
} fontWarningTypes;

typedef struct {
	unsigned	chrset;
	unsigned	flags;
	fontWarningTypes warn;
	XFontStruct *	fs;
	char *		fn;
	XTermFontInfo	font_info;
	Char		known_missing[MaxUChar + 1];
} XTermFonts;

#if OPT_RENDERFONT
typedef enum {
	erFalse = 0
	, erTrue
	, erDefault
	, erDefaultOff
	, erLast
} RenderFont;

#define DefaultRenderFont(xw) \
	if ((xw)->work.render_font == erDefault) \
	    (xw)->work.render_font = erFalse

typedef enum {
    	xcEmpty = 0			/* slot is unused */
	, xcBogus			/* ignore this pattern */
	, xcOpened			/* slot has open font descriptor */
	, xcUnused			/* opened, but unused so far */
} XTermXftState;

typedef struct {
	XftFont *	font;
	XTermXftState	usage;
} XTermXftCache;

#define MaxXftCache	MaxUChar

typedef struct {
	XftPattern *	pattern;	/* pattern for main font */
	XftFontSet *	fontset;	/* ordered list of fallback patterns */
	XTermXftCache	cache[MaxXftCache + 1]; /* list of open font pointers */
	int		fs_size;	/* last usable index of cache[] */
	Char		opened;		/* number in cache[] with xcOpened */
	XTermFontInfo	font_info;	/* summary of font metrics */
	XTermFontMap	font_map;	/* map of glyphs provided in fontset */
} XTermXftFonts;

#define XftFpN(p,n)	(p)->cache[(n)].font
#define XftIsN(p,n)	(p)->cache[(n)].usage

#define XftFp(p)	XftFpN(p,0)
#define XftIs(p)	XftIsN(p,0)

typedef	struct _ListXftFonts {
	struct _ListXftFonts *next;
	XftFont *	font;
} ListXftFonts;
#endif

typedef struct {
	int		top;
	int		left;
	int		bottom;
	int		right;
} XTermRect;

/***====================================================================***/

	/* indices into save_modes[] */
typedef enum {
	DP_ALLOW_ALTBUF,
	DP_ALTERNATE_SCROLL,
	DP_ALT_SENDS_ESC,
	DP_BELL_IS_URGENT,
	DP_CRS_VISIBLE,
	DP_DECANM,
	DP_DECARM,
	DP_DECAWM,
	DP_DECBKM,
	DP_DECCKM,
	DP_DECCOLM,	/* IN132COLUMNS */
	DP_DECKPAM,
	DP_DECNRCM,
	DP_DECOM,
	DP_DECPEX,
	DP_DECPFF,
	DP_DECSCLM,
	DP_DECSCNM,
	DP_DECTCEM,
	DP_DELETE_IS_DEL,
	DP_EIGHT_BIT_META,
	DP_FAST_SCROLL,
	DP_KEEP_CLIPBOARD,
	DP_KEEP_SELECTION,
	DP_KEYBOARD_TYPE,
	DP_POP_ON_BELL,
	DP_PRN_EXTENT,
	DP_PRN_FORMFEED,
	DP_RXVT_SCROLLBAR,
	DP_RXVT_SCROLL_TTY_KEYPRESS,
	DP_RXVT_SCROLL_TTY_OUTPUT,
	DP_SELECT_TO_CLIPBOARD,
	DP_X_ALTBUF,
	DP_X_DECCOLM,
	DP_X_EXT_MOUSE,
	DP_X_LOGGING,
	DP_X_LRMM,
	DP_X_MARGIN,
	DP_X_MORE,
	DP_X_MOUSE,
	DP_X_NCSM,
	DP_X_REVWRAP,
	DP_X_REVWRAP2,
	DP_X_X10MSE,
#if OPT_BLINK_CURS
	DP_CRS_BLINK,
#endif
#if OPT_FOCUS_EVENT
	DP_X_FOCUS,
#endif
#if OPT_NUM_LOCK
	DP_REAL_NUMLOCK,
	DP_META_SENDS_ESC,
#endif
#if OPT_SHIFT_FONTS
	DP_RXVT_FONTSIZE,
#endif
#if OPT_SIXEL_GRAPHICS
	DP_DECSDM,
#endif
#if OPT_TEK4014
	DP_DECTEK,
#endif
#if OPT_TOOLBAR
	DP_TOOLBAR,
#endif
#if OPT_GRAPHICS
	DP_X_PRIVATE_COLOR_REGISTERS,
#endif
#if OPT_SIXEL_GRAPHICS
	DP_SIXEL_SCROLLS_RIGHT,
#endif
#if OPT_PRINT_GRAPHICS
	DP_DECGEPM,  /* Graphics Expanded Print Mode */
	DP_DECGPCM,  /* Graphics Print Color Mode */
	DP_DECGPCS,  /* Graphics Print Color Syntax */
	DP_DECGPBM,  /* Graphics Print Background Mode */
	DP_DECGRPM,  /* Graphics Rotated Print Mode */
#endif
	DP_LAST
} SaveModes;

#define DoSM(code,value)  screen->save_modes[code] = (unsigned) (value)
#define DoRM(code,value)  value = (Boolean) screen->save_modes[code]
#define DoRM0(code,value) value = screen->save_modes[code]

	/* index into vt_shell[] or tek_shell[] */
typedef enum {
	noMenu = -1
	,mainMenu
	,vtMenu
	,fontMenu
#if OPT_TEK4014
	,tekMenu
#endif
} MenuIndex;

typedef enum {
	bvOff = -1,
	bvLow = 0,
	bvHigh
} BellVolume;

#define NUM_POPUP_MENUS 4

typedef struct {
	String		resource;
	Pixel		value;
	unsigned short red, green, blue;
	int		mode;		/* -1=invalid, 0=unset, 1=set   */
} ColorRes;

/* these are set in getPrinterFlags */
typedef struct {
	int	printer_extent;		/* print complete page		*/
	int	printer_formfeed;	/* print formfeed per function	*/
	int	printer_newline;	/* print newline per function	*/
	int	print_attributes;	/* 0=off, 1=normal, 2=color	*/
	int	print_everything;	/* 0=all, 1=dft, 2=alt, 3=saved */
} PrinterFlags;

typedef struct {
	FILE *	fp;			/* output file/pipe used	*/
	Boolean isOpen;			/* output was opened/tried	*/
	Boolean toFile;			/* true when directly to file	*/
	Boolean printer_checked;	/* printer_command is checked	*/
	String	printer_command;	/* pipe/shell command string	*/
	Boolean printer_autoclose;	/* close printer when offline	*/
	Boolean printer_extent;		/* print complete page		*/
	Boolean printer_formfeed;	/* print formfeed per function	*/
	Boolean printer_newline;	/* print newline per function	*/
	int	printer_controlmode;	/* 0=off, 1=auto, 2=controller	*/
	int	print_attributes;	/* 0=off, 1=normal, 2=color	*/
	int	print_everything;	/* 0=all, 1=dft, 2=alt, 3=saved */
} PrinterState;

typedef struct {
	unsigned	which;		/* must have NCOLORS bits */
	Pixel		colors[NCOLORS];
	char		*names[NCOLORS];
} ScrnColors;

#define NUM_GSETS 4
#define NUM_GSETS2 (NUM_GSETS + 1)	/* include user-preferred */
#define gsets_upss	gsets[4]

#define SAVED_CURSORS 2

typedef struct {
	Boolean		saved;
	int		row;
	int		col;
	IFlags		flags;		/* VTxxx saves graphics rendition */
	Char		curgl;
	Char		curgr;
	DECNRCM_codes	gsets[NUM_GSETS2];
	Boolean		wrap_flag;
#if OPT_ISO_COLORS
	int		cur_foreground;  /* current foreground color	*/
	int		cur_background;  /* current background color	*/
	int		sgr_foreground;  /* current SGR foreground color */
	int		sgr_background;  /* current SGR background color */
	Boolean		sgr_38_xcolors;  /* true if ISO 8613 extension	*/
#endif
} SavedCursor;

typedef struct _SaveTitle {
	struct _SaveTitle *next;
	char		*iconName;
	char		*windowName;
} SaveTitle;

#define MAX_SAVED_TITLES 10

typedef struct {
	int		used;		/* index to current item	*/
	SaveTitle	data[MAX_SAVED_TITLES];
} SavedTitles;

typedef struct {
	int		width;		/* if > 0, width of scrollbar,	*/
					/* and scrollbar is showing	*/
	Boolean		rv_cached;	/* see ScrollBarReverseVideo	*/
	int		rv_active;	/* ...current reverse-video	*/
	Pixel		bg;		/* ...cached background color	*/
	Pixel		fg;		/* ...cached foreground color	*/
	Pixel		bdr;		/* ...cached border color	*/
	Pixmap		bdpix;		/* ...cached border pixmap	*/
} SbInfo;

#if OPT_TOOLBAR
typedef struct {
	Widget		menu_bar;	/* toolbar, if initialized	*/
	Dimension	menu_height;	/* ...and its height		*/
	Dimension	menu_border;	/* ...and its border		*/
} TbInfo;
#define VT100_TB_INFO(name) screen.fullVwin.tb_info.name
#endif

typedef struct {
	Window		window;		/* X window id			*/
	int		width;		/* width of columns in pixels	*/
	int		height;		/* height of rows in pixels	*/
	Dimension	fullwidth;	/* full width of window		*/
	Dimension	fullheight;	/* full height of window	*/
	int		f_width;	/* width of fonts in pixels	*/
	int		f_height;	/* height of fonts in pixels	*/
	int		f_ascent;	/* ascent of font in pixels	*/
	int		f_descent;	/* descent of font in pixels	*/
	SbInfo		sb_info;
	GC		filler_gc;	/* filler's fg/bg		*/
	GC		border_gc;	/* inner border's fg/bg		*/
	GC		marker_gc[2];	/* wrap-marks			*/
#if USE_DOUBLE_BUFFER
	Drawable	drawable;	/* X drawable id                */
#endif
#if OPT_TOOLBAR
	Boolean		active;		/* true if toolbars are used	*/
	TbInfo		tb_info;	/* toolbar information		*/
#endif
} VTwin;

typedef struct {
	Window		window;		/* X window id			*/
	int		width;		/* width of columns		*/
	int		height;		/* height of rows		*/
	Dimension	fullwidth;	/* full width of window		*/
	Dimension	fullheight;	/* full height of window	*/
	double		tekscale;	/* scale factor Tek -> vs100	*/
} TKwin;

typedef struct {
    char *f_n;			/* the normal font */
    char *f_b;			/* the bold font */
#if OPT_WIDE_CHARS
    char *f_w;			/* the normal wide font */
    char *f_wb;			/* the bold wide font */
#endif
} VTFontNames;

typedef struct {
    char **list_n;		/* the normal font */
    char **list_b;		/* the bold font */
#if OPT_WIDE_ATTRS || OPT_RENDERWIDE
    char **list_i;		/* italic font (Xft only) */
    char **list_bi;		/* bold-italic font (Xft only) */
#endif
#if OPT_WIDE_CHARS
    char **list_w;		/* the normal wide font */
    char **list_wb;		/* the bold wide font */
    char **list_wi;		/* wide italic font (Xft only) */
    char **list_wbi;		/* wide bold-italic font (Xft only) */
#endif
} VTFontList;

typedef struct {
    VTFontList x11;
#if OPT_RENDERFONT
    VTFontList xft;
#endif
} XtermFontNames;

typedef struct {
    VTFontNames default_font;
    String menu_font_names[NMENUFONTS][fMAX];
    XtermFontNames fonts;
} SubResourceRec;

#if OPT_INPUT_METHOD
#define NINPUTWIDGETS	3
typedef struct {
	Widget		w;
	XIM		xim;		/* input method attached to 'w' */
	XIC		xic;		/* input context attached to 'xim' */
} TInput;
#endif

typedef enum {
	CURSOR_BLOCK = 2
	, CURSOR_UNDERLINE = 4
	, CURSOR_BAR = 6
} XtCursorShape;

#define isCursorBlock(s)	((s)->cursor_shape == CURSOR_BLOCK)
#define isCursorUnderline(s)	((s)->cursor_shape == CURSOR_UNDERLINE)
#define isCursorBar(s)		((s)->cursor_shape == CURSOR_BAR)

typedef enum {
	DEFAULT_STYLE = 0
	, BLINK_BLOCK
	, STEADY_BLOCK
	, BLINK_UNDERLINE
	, STEADY_UNDERLINE
	, BLINK_BAR
	, STEADY_BAR
} XtCursorStyle;

#if OPT_GRAPHICS
#define GraphicsTermId(screen) (\
	(screen)->graphics_termid \
	 ? (screen)->graphics_termid \
	 : (screen)->terminal_id)
#else
#define GraphicsTermId(screen) (screen)->terminal_id
#endif

#if OPT_REGIS_GRAPHICS
#define optRegisGraphics(screen) \
	(GraphicsTermId(screen) == 125 || \
	 GraphicsTermId(screen) == 240 || \
	 GraphicsTermId(screen) == 241 || \
	 GraphicsTermId(screen) == 330 || \
	 GraphicsTermId(screen) == 340)
#else
#define optRegisGraphics(screen) False
#endif

#if OPT_SIXEL_GRAPHICS
#define optSixelGraphics(screen) \
	(GraphicsTermId(screen) == 240 || \
	 GraphicsTermId(screen) == 241 || \
	 GraphicsTermId(screen) == 330 || \
	 GraphicsTermId(screen) == 340 || \
	 GraphicsTermId(screen) == 382)
#else
#define optSixelGraphics(screen) False
#endif

#if OPT_PRINT_GRAPHICS
#define if_PRINT_GRAPHICS2(statement) if (optRegisGraphics(screen)) { statement; } else
#else
#define if_PRINT_GRAPHICS2(statement) /* nothing */
#endif

typedef struct {
/* These parameters apply to both windows */
	Display		*display;	/* X display for screen		*/
	int		respond;	/* socket for responses
					   (position report, etc.)	*/
	int		nextEventDelay;	/* msecs to delay for x-events  */
/* These parameters apply to VT100 window */
	IChar		*unparse_bfr;
	unsigned	unparse_len;
	unsigned	unparse_max;	/* limitResponse resource	*/
	unsigned	strings_max;	/* maxStringParse resource	*/

#if OPT_TCAP_QUERY
	int		tc_query_code;
	Bool		tc_query_fkey;
#endif
	pid_t		pid;		/* pid of process on far side   */
	uid_t		uid;		/* user id of actual person	*/
	gid_t		gid;		/* group id of actual person	*/
	ColorRes	Tcolors[NCOLORS]; /* terminal colors		*/
#if OPT_HIGHLIGHT_COLOR
	Boolean		hilite_color;	/* hilite colors override	*/
	Boolean		hilite_reverse;	/* hilite overrides reverse	*/
#endif
#if OPT_ISO_COLORS
	XColor *	cmap_data;	/* color table			*/
	unsigned	cmap_size;
	ColorRes	Acolors[MAXCOLORS]; /* ANSI color emulation	*/
	int		veryBoldColors;	/* modifier for boldColors	*/
	Boolean		boldColors;	/* can we make bold colors?	*/
	Boolean		colorMode;	/* are we using color mode?	*/
	Boolean		colorULMode;	/* use color for underline?	*/
	Boolean		italicULMode;	/* italic font for underline?	*/
	Boolean		colorBDMode;	/* use color for bold?		*/
	Boolean		colorBLMode;	/* use color for blink?		*/
	Boolean		colorRVMode;	/* use color for reverse?	*/
	Boolean		colorAttrMode;	/* prefer colorUL/BD to SGR	*/
#if OPT_WIDE_ATTRS
	Boolean		colorITMode;	/* use color for italics?	*/
#endif
#if OPT_DIRECT_COLOR
	Boolean		direct_color;	/* direct-color enabled?	*/
#endif
#if OPT_WIDE_ATTRS && OPT_SGR2_HASH
	Boolean		faint_relative;	/* faint is relative?		*/
#endif
#if OPT_VT525_COLORS
	int		assigned_fg;	/* DECAC			*/
	int		assigned_bg;
	struct {
	    int fg;			/* 0..15			*/
	    int bg;			/* 0..15			*/
	} alt_colors[16];		/* DECATC if DECSTGLT is 1 or 2	*/
#endif
#endif /* OPT_ISO_COLORS */
#if OPT_DEC_CHRSET
	Boolean		font_doublesize;/* enable font-scaling		*/
	int		cache_doublesize;/* limit of our cache		*/
	Char		cur_chrset;	/* character-set index & code	*/
	int		fonts_used;	/* count items in double_fonts	*/
	XTermFonts	double_fonts[NUM_CHRSET];
#if OPT_RENDERFONT
	XTermXftFonts	double_xft_fonts[NUM_CHRSET];
#endif
#endif /* OPT_DEC_CHRSET */
#if OPT_DEC_RECTOPS
	int		cur_decsace;	/* parameter for DECSACE	*/
	int		checksum_ext;	/* extensions for DECRQCRA	*/
	int		checksum_ext0;	/* initial checksumExtension	*/
#endif
#if OPT_WIDE_CHARS
	Boolean		wide_chars;	/* true when 16-bit chars	*/
	Boolean		vt100_graphics;	/* true to allow vt100-graphics	*/
	Boolean		utf8_inparse;	/* true to enable UTF-8 parser	*/
	Boolean		normalized_c;	/* true to precompose to Form C */
	char *		utf8_mode_s;	/* use UTF-8 decode/encode	*/
	char *		utf8_fonts_s;	/* use UTF-8 decode/encode	*/
	char *		utf8_title_s;	/* use UTF-8 titles		*/
	int		utf8_nrc_mode;	/* saved UTF-8 mode for DECNRCM */
	Boolean		utf8_always;	/* special case for wideChars	*/
	int		utf8_mode;	/* use UTF-8 decode/encode: 0-2	*/
	int		utf8_fonts;	/* use UTF-8 fonts: 0-2		*/
	int		utf8_title;	/* use UTF-8 EWHM props: 0-2	*/
	int		max_combining;	/* maximum # of combining chars	*/
	Boolean		utf8_latin1;	/* use UTF-8 with Latin-1 bias	*/
	Boolean		utf8_weblike;	/* use UTF-8 with browser bias	*/
	int		latin9_mode;	/* poor man's luit, latin9	*/
	int		unicode_font;	/* font uses unicode encoding	*/
	int		utf_count;	/* state of utf_char		*/
	IChar		utf_char;	/* in-progress character	*/
	Boolean		char_was_written;
	int		last_written_col;
	int		last_written_row;
#endif
	TypedBuffer(IChar);
	TypedBuffer(Char);
	TypedBuffer(XChar2b);
#if OPT_BROKEN_OSC
	Boolean		brokenLinuxOSC; /* true to ignore Linux palette ctls */
#endif
#if OPT_BROKEN_ST
	Boolean		brokenStringTerm; /* true to match old OSC parse */
#endif
#if OPT_C1_PRINT || OPT_WIDE_CHARS
	Boolean		c1_printable;	/* true if we treat C1 as print	*/
#endif
	int		border;		/* inner border			*/
	int		scrollBarBorder; /* scrollBar border		*/
	long		event_mask;
	unsigned	send_mouse_pos;	/* user wants mouse transition  */
					/* and position information	*/
	int		extend_coords;	/* support large terminals	*/
#if OPT_FOCUS_EVENT
	Boolean		send_focus_pos; /* user wants focus in/out info */
#endif
	Boolean		quiet_grab;	/* true if no cursor change on focus */
#if OPT_PASTE64
	Cardinal	base64_paste;	/* set to send paste in base64	*/
	int		base64_final;	/* string-terminator for paste	*/
	/* _qWriteSelectionData expects these to be initialized to zero.
	 * base64_flush() is the last step of the conversion, it clears these
	 * variables.
	 */
	unsigned	base64_accu;
	unsigned	base64_count;
	unsigned	base64_pad;
#endif
#if OPT_PASTE64 || OPT_READLINE
	unsigned	paste_brackets;
	/* not part of bracketed-paste, these are here to simplify ifdefs */
	unsigned	dclick3_deletes;
	unsigned	paste_literal_nl;
#endif
#if OPT_READLINE
	unsigned	click1_moves;
	unsigned	paste_moves;
	unsigned	paste_quotes;
#endif	/* OPT_READLINE */
#if OPT_DEC_LOCATOR
	Boolean		locator_reset;	/* turn mouse off after 1 report? */
	Boolean		locator_pixels;	/* report in pixels?		*/
					/* if false, report in cells	*/
	unsigned	locator_events;	/* what events to report	*/
	Boolean		loc_filter;	/* is filter rectangle active?	*/
	int		loc_filter_top;	/* filter rectangle for DEC Locator */
	int		loc_filter_left;
	int		loc_filter_bottom;
	int		loc_filter_right;
#endif	/* OPT_DEC_LOCATOR */
	int		mouse_button;	/* current button pressed	*/
	int		mouse_row;	/* ...and its row		*/
	int		mouse_col;	/* ...and its column		*/
	int		select;		/* xterm selected		*/
	Boolean		bellOnReset;	/* bellOnReset			*/
	Boolean		visualbell;	/* visual bell mode		*/
	Boolean		poponbell;	/* pop on bell mode		*/

	Boolean		eraseSavedLines; /* eraseSavedLines option	*/
	Boolean		eraseSavedLines0; /* initial eraseSavedLines	*/
	Boolean		tabCancelsWrap; /* tabCancelsWrap option	*/

	Boolean		allowPasteControls; /* PasteControls mode	*/
	Boolean		allowColorOps;	/* ColorOps mode		*/
	Boolean		allowFontOps;	/* FontOps mode			*/
	Boolean		allowMouseOps;	/* MouseOps mode		*/
	Boolean		allowSendEvents;/* SendEvent mode		*/
	Boolean		allowTcapOps;	/* TcapOps mode			*/
	Boolean		allowTitleOps;	/* TitleOps mode		*/
	Boolean		allowWindowOps;	/* WindowOps mode		*/

	Boolean		allowPasteControl0; /* PasteControls mode	*/
	Boolean		allowColorOp0;	/* initial ColorOps mode	*/
	Boolean		allowFontOp0;	/* initial FontOps mode		*/
	Boolean		allowMouseOp0;	/* initial MouseOps mode	*/
	Boolean		allowSendEvent0;/* initial SendEvent mode	*/
	Boolean		allowTcapOp0;	/* initial TcapOps mode		*/
	Boolean		allowTitleOp0;	/* initial TitleOps mode	*/
	Boolean		allowWindowOp0;	/* initial WindowOps mode	*/

	String		disallowedColorOps;
	char		disallow_color_ops[ecLAST];

	String		disallowedFontOps;
	char		disallow_font_ops[efLAST];

	String		disallowedMouseOps;
	char		disallow_mouse_ops[emLAST];

	String		disallowedPasteOps;
	char		disallow_paste_ops[epLAST];

	String		disallowedTcapOps;
	char		disallow_tcap_ops[etLAST];

	String		disallowedWinOps;
	char		disallow_win_ops[ewLAST];

	char		color_events[1+OSC_NCOLORS]; /* ok OSC codes	*/
	char *		colorEvents;	/* initial color-event codes	*/

	Boolean		awaitInput;	/* select-timeout mode		*/
	Boolean		grabbedKbd;	/* keyboard is grabbed		*/
#ifdef ALLOWLOGGING
	int		logging;	/* logging mode			*/
	int		logfd;		/* file descriptor of log	*/
	char		*logfile;	/* log file name		*/
	Char		*logstart;	/* current start of log buffer	*/
#endif
	int		inhibit;	/* flags for inhibiting changes	*/

/* VT window parameters */
	Boolean		Vshow;		/* VT window showing		*/
	VTwin		fullVwin;
	int		needSwap;
#ifndef NO_ACTIVE_ICON
	VTwin		iconVwin;
	VTwin		*whichVwin;
#endif /* NO_ACTIVE_ICON */

	int		pointer_mode;	/* when to use hidden_cursor	*/
	int		pointer_mode0;	/* ...initial value             */
	Boolean 	hide_pointer;	/* true to use "hidden_cursor"  */
	String		pointer_shape;	/* name of shape in cursor font */
	Cursor		pointer_cursor;	/* current pointer cursor	*/
	Cursor		hidden_cursor;	/* hidden cursor in window	*/

	String		answer_back;	/* response to ENQ		*/
	Boolean		prefer_latin1;	/* preference for UPSS		*/

	PrinterState	printer_state;	/* actual printer state		*/
	PrinterFlags	printer_flags;	/* working copy of printer flags */
	Boolean		print_rawchars;	/* true to ignore printer check	*/
#if OPT_PRINT_ON_EXIT
	Boolean		write_error;
#endif

	Boolean		fnt_prop;	/* true if proportional fonts	*/
	unsigned	fnt_boxes;	/* 0=no boxes, 1=old, 2=unicode */
	Boolean		force_packed;	/* true to override proportional */
#if OPT_BOX_CHARS
	Boolean		force_box_chars;/* true if we assume no boxchars */
	Boolean		broken_box_chars;/* true if broken boxchars	*/
	Boolean		assume_all_chars;/* true to allow missing chars */
	Boolean		allow_packing;	/* true to allow packed-fonts	*/
#endif
#if OPT_BOX_CHARS || OPT_WIDE_CHARS
	Boolean		force_all_chars;/* true to outline missing chars */
#endif

	Dimension	fnt_wide;
	Dimension	fnt_high;
	float		scale_height;	/* scaling for font-height	*/
	XTermFonts	fnts[fMAX];	/* normal/bold/etc for terminal	*/
	Boolean		free_bold_box;	/* same_font_size's austerity	*/
	Boolean		allowBoldFonts;	/* do we use bold fonts at all? */
#if OPT_WIDE_ATTRS
	XTermFonts	ifnts[fMAX];	/* normal/bold/etc italic fonts */
	Boolean		ifnts_ok;	/* true if ifnts[] is cached	*/
#endif
#ifndef NO_ACTIVE_ICON
	XTermFonts	fnt_icon;	/* icon font			*/
	String		icon_fontname;	/* name of icon font		*/
	int		icon_fontnum;	/* number to use for icon font	*/
#endif /* NO_ACTIVE_ICON */
	int		enbolden;	/* overstrike for bold font	*/
	XPoint		*box;		/* draw unselected cursor	*/

	int		cursor_state;	/* ON, OFF, or BLINKED_OFF	*/
	int		cursor_busy;	/* do not redraw...		*/
	Boolean		cursor_underline; /* true if cursor is in underline mode */
	Boolean         cursor_bar;     /* true if cursor is in bar mode */
	XtCursorShape	cursor_shape;
#if OPT_BLINK_CURS
	BlinkOps	cursor_blink;	/* cursor blink enable		*/
	BlinkOps	cursor_blink_i;	/* save cursor blink enable	*/
	char *		cursor_blink_s;	/* ...resource cursorBlink	*/
	int		cursor_blink_esc; /* cursor blink escape-state	*/
	Boolean		cursor_blink_xor; /* how to merge menu/escapes	*/
#endif
#if OPT_BLINK_TEXT
	Boolean		blink_as_bold;	/* text blink disable		*/
#endif
#if OPT_BLINK_CURS || OPT_BLINK_TEXT
	int		blink_state;	/* ON, OFF, or BLINKED_OFF	*/
	int		blink_on;	/* cursor on time (msecs)	*/
	int		blink_off;	/* cursor off time (msecs)	*/
	XtIntervalId	blink_timer;	/* timer-id for cursor-proc	*/
#endif
#if OPT_ZICONBEEP
	Boolean		zIconBeep_flagged; /* True if icon name was changed */
#endif /* OPT_ZICONBEEP */
	int		cursor_GC;	/* see ShowCursor()		*/
	int		cursor_set;	/* requested state		*/
	CELL		cursorp;	/* previous cursor row/column	*/
	int		cur_col;	/* current cursor column	*/
	int		cur_row;	/* current cursor row		*/
	int		max_col;	/* rightmost column		*/
	int		max_row;	/* bottom row			*/
	int		top_marg;	/* top line of scrolling region */
	int		bot_marg;	/* bottom line of  "	    "	*/
	int		lft_marg;	/* left column of "	    "	*/
	int		rgt_marg;	/* right column of "	    "	*/
	Widget		scrollWidget;	/* pointer to scrollbar struct	*/
#if USE_DOUBLE_BUFFER
	int		buffered_sb;	/* nonzero when pending update	*/
	struct timeval	buffered_at;	/* reference time, for FPS	*/
#define DbeMsecs(xw)	(1000L / (long) resource.buffered_fps)
#endif
	/*
	 * Indices used to keep track of the top of the vt100 window and
	 * the saved lines, taking scrolling into account.
	 */
	int		topline;	/* line number of top, <= 0	*/
	long		saved_fifo;     /* number of lines that've ever been saved */
	int		savedlines;     /* number of lines that've been saved */
	int		savelines;	/* number of lines off top to save */
	int		scroll_amt;	/* amount to scroll		*/
	int		refresh_amt;	/* amount to refresh		*/
	/*
	 * Working variables for getLineData().
	 */
	size_t		lineExtra;	/* extra space for combining chars */
	size_t		cellExtra;	/* extra space for combining chars */
	/*
	 * Pointer to the current visible buffer.
	 */
	ScrnBuf		visbuf;		/* ptr to visible screen buf (main) */
	/*
	 * Data for the normal buffer, which may have saved lines to which
	 * the user can scroll.
	 */
	ScrnBuf		saveBuf_index;
	Char		*saveBuf_data;
	/*
	 * Data for visible and alternate buffer.
	 */
	ScrnBuf		editBuf_index[2];
	Char		*editBuf_data[2];
	int		whichBuf;	/* 0/1 for normal/alternate buf */
	Boolean		is_running;	/* true when buffers are legal	*/
	/*
	 * Workspace used for screen operations.
	 */
	Char		**save_ptr;	/* workspace for save-pointers  */
	size_t		save_len;	/* ...and its length		*/

	int		scrolllines;	/* number of lines to button scroll */
	Boolean		alternateScroll; /* scroll-actions become keys */
	Boolean		scrollttyoutput; /* scroll to bottom on tty output */
	Boolean		scrollkey;	/* scroll to bottom on key	*/
	Boolean		cursor_moved;	/* scrolling makes cursor move	*/

	Boolean		do_wrap;	/* true if cursor in last column
					    and character just output    */

	int		incopy;		/* 0 idle; 1 XCopyArea issued;
					    -1 first GraphicsExpose seen,
					    but last not seen		*/
	int		copy_src_x;	/* params from last XCopyArea ... */
	int		copy_src_y;
	unsigned int	copy_width;
	unsigned int	copy_height;
	int		copy_dest_x;
	int		copy_dest_y;

	Dimension	embed_wide;
	Dimension	embed_high;

	Boolean		c132;		/* allow change to 132 columns	*/
	Boolean		curses;		/* kludge line wrap for more	*/
	Boolean		hp_ll_bc;	/* kludge HP-style ll for xdb	*/
	Boolean		marginbell;	/* true if margin bell on	*/
	int		nmarginbell;	/* columns from right margin	*/
	int		bellArmed;	/* cursor below bell margin	*/
	BellVolume	marginVolume;	/* margin-bell volume           */
	BellVolume	warningVolume;	/* warning-bell volume          */
	Boolean		multiscroll;	/* true if multi-scroll		*/
	int		scrolls;	/* outstanding scroll count,
					    used only with multiscroll	*/
	SavedCursor	sc[SAVED_CURSORS]; /* data for restore cursor	*/
	IFlags		save_modes[DP_LAST]; /* save dec/xterm private modes */

	int		title_modes;	/* control set/get of titles	*/
	int		title_modes0;	/* ...initial value         	*/
#if OPT_TITLE_MODES
	SavedTitles	saved_titles;
#endif

	/* Improved VT100 emulation stuff.				*/
	String		keyboard_dialect; /* default keyboard dialect	*/
	DECNRCM_codes	gsets[NUM_GSETS2]; /* G0 through G3, plus UPSS	*/
	Char		curgl;		/* Current GL setting.		*/
	Char		curgr;		/* Current GR setting.		*/
	Char		curss;		/* Current single shift.	*/
	String		term_id;	/* resource for terminal_id	*/
	int		terminal_id;	/* 100=vt100, 220=vt220, etc.	*/
	int		display_da1;	/* 100=vt100, 220=vt220, etc.	*/
	int		vtXX_level;	/* 0=vt52, 1,2,3 = vt100 ... vt320 */
	int		ansi_level;	/* dpANSI levels 1,2,3		*/
	int		protected_mode;	/* 0=off, 1=DEC, 2=ISO		*/
	Boolean		always_bold_mode; /* compare normal/bold font	*/
	Boolean		always_highlight; /* whether to highlight cursor */
	Boolean		bold_mode;	/* use bold font or overstrike	*/
	Boolean		delete_is_del;	/* true for compatible Delete key */
	Boolean		jumpscroll;	/* whether we should jumpscroll */
	Boolean		fastscroll;	/* whether we should fastscroll */
	Boolean		old_fkeys;	/* true for compatible fkeys	*/
	Boolean		old_fkeys0;	/* ...initial value         	*/
	Boolean		underline;	/* whether to underline text	*/

#if OPT_MAXIMIZE
	Boolean		restore_data;
	int		restore_x;
	int		restore_y;
	unsigned	restore_width;
	unsigned	restore_height;
#endif

#if OPT_REGIS_GRAPHICS
	String		graphics_regis_default_font; /* font for "builtin" */

	String		graphics_regis_screensize; /* given a size in pixels */
	Dimension	graphics_regis_def_wide; /* ...corresponding width   */
	Dimension	graphics_regis_def_high; /* ...and height            */
#endif

#if OPT_GRAPHICS
	String		graph_termid;		/* resource for graphics_termid */
	int		graphics_termid;	/* based on terminal_id   */
	String		graphics_max_size;	/* given a size in pixels */
	Dimension	graphics_max_wide;	/* ...corresponding width */
	Dimension	graphics_max_high;	/* ...and height          */
#endif

#if OPT_SCROLL_LOCK
	Boolean		allowScrollLock;/* ScrollLock mode		*/
	Boolean		allowScrollLock0;/* initial ScrollLock mode	*/
	Boolean		autoScrollLock; /* Auto ScrollLock mode		*/
	Boolean		scroll_lock;	/* true to keep buffer in view	*/
	Boolean		scroll_dirty;	/* scrolling makes screen dirty	*/
#endif

#if OPT_SIXEL_GRAPHICS
	Boolean		sixel_scrolling; /* sixel scrolling             */
	Boolean		sixel_scrolls_right; /* sixel scrolling moves cursor to right */
	Boolean		sixel_scrolls_right0; /* initial sixelScrolling mode */
#endif

#if OPT_GRAPHICS
	int		numcolorregisters; /* number of supported color registers */
	Boolean		privatecolorregisters; /* private color registers for each graphic */
	Boolean		privatecolorregisters0; /* initial privateColorRegisters */
	Boolean		incremental_graphics; /* draw graphics incrementally */
#endif

	/* Graphics Printing */
#if OPT_PRINT_GRAPHICS
	Boolean		graphics_print_to_host;
	Boolean		graphics_expanded_print_mode;
	Boolean		graphics_print_color_mode;
	Boolean		graphics_print_color_syntax;
	Boolean		graphics_print_background_mode;
	Boolean		graphics_rotated_print_mode;
#endif

#define StatusLineRows	1		/* number of rows in status-line */

#if OPT_STATUS_LINE
#define AddStatusLineRows(nrow)         nrow += StatusLineRows
#define LastRowNumber(screen) \
	( (screen)->max_row \
	 + (IsStatusShown(screen) ? StatusLineRows : 0) )
#define FirstRowNumber(screen) \
	( (screen)->status_active \
	   ? LastRowNumber(screen) \
	   : 0 )
#define IsStatusShown(screen) \
	( ( (screen)->status_type == 2) || \
	  ( (screen)->status_type == 1) )
#define PlusStatusLine(screen,expr) \
	( (screen)->status_shown \
	  ? (expr) + StatusLineRows \
	  : (expr) )
#define if_STATUS_LINE(screen,stmt) \
	if (IsStatusShown(screen) && (screen)->status_active) stmt

	Boolean		status_timeout;	/* status timeout needs service	*/
	int		status_active;	/* DECSASD */
	int		status_type;	/* DECSSDT */
	int		status_shown;	/* last-displayed type */
	SavedCursor	status_data[2];	/* main- and status-cursors */
	char *		status_fmt;	/* format for indicator-status	*/

#else /* !OPT_STATUS_LINE */

#define AddStatusLineRows(nrow)         /* nothing */
#define LastRowNumber(screen)           (screen)->max_row
#define FirstRowNumber(screen)          0
#define IsStatusShown(screen) False
#define PlusStatusLine(screen,expr)     (expr)
#define if_STATUS_LINE(screen,stmt)	/* nothing */

#endif /* OPT_STATUS_LINE */

#if OPT_VT52_MODE
	IFlags		vt52_save_flags;
	Char		vt52_save_curgl;
	Char		vt52_save_curgr;
	Char		vt52_save_curss;
	DECNRCM_codes	vt52_save_gsets[NUM_GSETS2];
#endif
	/* Testing */
#if OPT_XMC_GLITCH
	unsigned	xmc_glitch;	/* # of spaces to pad on SGR's	*/
	IAttr		xmc_attributes;	/* attrs that make a glitch	*/
	Boolean		xmc_inline;	/* SGR's propagate only to eol	*/
	Boolean		move_sgr_ok;	/* SGR is reset on move		*/
#endif

	/*
	 * Bell
	 */
	int		visualBellDelay; /* msecs to delay for visibleBell */
	int		bellSuppressTime; /* msecs after Bell before another allowed */
	Boolean		bellInProgress; /* still ringing/flashing prev bell? */
	Boolean		bellIsUrgent;	/* set XUrgency WM hint on bell */
	Boolean		flash_line;	/* show visualBell as current line */
	/*
	 * Select/paste state.
	 */
	Boolean		selectToClipboard; /* primary vs clipboard */
	String		*mappedSelect;	/* mapping for "SELECT" to "PRIMARY" */

	Boolean		waitingForTrackInfo;
	int		numberOfClicks;
	int		maxClicks;
	int		multiClickTime;	/* time between multiclick selects */
	SelectUnit	selectUnit;
	SelectUnit	selectMap[NSELECTUNITS];
	String		onClick[NSELECTUNITS + 1];

	char		*charClass;	/* for overriding word selection */
	Boolean		cutNewline;	/* whether or not line cut has \n */
	Boolean		cutToBeginningOfLine;  /* line cuts to BOL? */
	Boolean		highlight_selection; /* controls appearance of selection */
	Boolean		show_wrap_marks; /* show lines which are wrapped */
	Boolean		trim_selection; /* controls trimming of selection */
	Boolean		i18nSelections;
	Boolean		brokenSelections;
	Boolean		keepClipboard;	/* retain data sent to clipboard */
	Boolean		keepSelection;	/* do not lose selection on output */
	Boolean		replyToEmacs;	/* Send emacs escape code when done selecting or extending? */

	SelectedCells	clipboard_data;	/* what we sent to the clipboard */

	EventMode	eventMode;
	Time		selection_time;	/* latest event timestamp */
	Time		lastButtonUpTime;
	unsigned	lastButton;

#define MAX_CUT_BUFFER  8		/* CUT_BUFFER0 to CUT_BUFFER7 */
#define MAX_SELECTIONS	(MAX_SELECTION_CODES + MAX_CUT_BUFFER)
	SelectedCells	selected_cells[MAX_SELECTIONS];

	CELL		rawPos;		/* raw position for selection start */
	CELL		startRaw;	/* area before selectUnit processing */
	CELL		endRaw;		/* " " */
	CELL		startSel;	/* area after selectUnit processing */
	CELL		endSel;		/* " " */
	CELL		startH;		/* start highlighted text */
	CELL		endH;		/* end highlighted text */
	CELL		saveStartW;	/* saved WORD state, for LINE */
	CELL		startExt;	/* Start, end of extension */
	CELL		endExt;		/* " " */
	CELL		saveStartR;	/* Saved values of raw selection for extend to restore to */
	CELL		saveEndR;	/* " " */
	int		startHCoord, endHCoord;
	int		firstValidRow;	/* Valid rows for selection clipping */
	int		lastValidRow;	/* " " */

#if OPT_BLOCK_SELECT
	int		lastSelectWasBlock;
	int		blockSelecting;	/* non-zero if block selection */
#endif

	Boolean		selectToBuffer;	/* copy selection to buffer	*/
	InternalSelect	internal_select;

	String		default_string;
	String		eightbit_select_types;
	Atom*		selection_targets_8bit;
#if OPT_WIDE_CHARS
	String		utf8_select_types;
	Atom*		selection_targets_utf8;
#endif
	Atom*		selection_atoms; /* which selections we own */
	Cardinal	sel_atoms_size;	 /* how many atoms allocated */
	Cardinal	selection_count; /* how many atoms in use */
#if OPT_SELECT_REGEX
	char *		selectExpr[NSELECTUNITS];
#endif
	/*
	 * Input/output state.
	 */
	Boolean		input_eight_bits;	/* do not send ESC when meta pressed */
	int		eight_bit_meta;		/* use 8th bit when meta pressed */
	char *		eight_bit_meta_s;	/* ...resource eightBitMeta */
	Boolean		output_eight_bits;	/* honor all bits or strip */
	Boolean		control_eight_bits;	/* send CSI as 8-bits */
	Boolean		backarrow_key;		/* backspace/delete */
	Boolean		alt_is_not_meta;	/* use both Alt- and Meta-key */
	Boolean		alt_sends_esc;		/* Alt-key sends ESC prefix */
	Boolean		meta_sends_esc;		/* Meta-key sends ESC prefix */
	/*
	 * Fonts
	 */
	Pixmap		menu_item_bitmap;	/* mask for checking items */
	String		initial_font;
	char *		menu_font_names[NMENUFONTS][fMAX];
#define MenuFontName(n) menu_font_names[n][fNorm]
#define EscapeFontName() MenuFontName(fontMenu_fontescape)
#define SelectFontName() MenuFontName(fontMenu_fontsel)
	long		menu_font_sizes[NMENUFONTS];
	int		menu_font_number;
#if OPT_LOAD_VTFONTS || OPT_WIDE_CHARS
	Boolean		savedVTFonts;
	Boolean		mergedVTFonts;
	SubResourceRec	cacheVTFonts;
#endif
#if OPT_CLIP_BOLD
	Boolean		use_border_clipping;
	Boolean		use_clipping;
#endif
	void *		main_cgs_cache;
#ifndef NO_ACTIVE_ICON
	void *		icon_cgs_cache;
#endif
#if OPT_RENDERFONT
	int		xft_max_glyph_memory;
	int		xft_max_unref_fonts;
	Boolean		xft_track_mem_usage;
	Boolean		force_xft_height;
	ListXftFonts	*list_xft_fonts;
	XTermXftFonts	renderFontNorm[NMENUFONTS];
	XTermXftFonts	renderFontBold[NMENUFONTS];
#if OPT_WIDE_ATTRS || OPT_RENDERWIDE
	XTermXftFonts	renderFontItal[NMENUFONTS];
	XTermXftFonts	renderFontBtal[NMENUFONTS];
#endif
#if OPT_RENDERWIDE
	XTermXftFonts	renderWideNorm[NMENUFONTS];
	XTermXftFonts	renderWideBold[NMENUFONTS];
	XTermXftFonts	renderWideItal[NMENUFONTS];
	XTermXftFonts	renderWideBtal[NMENUFONTS];
	TypedBuffer(XftCharSpec);
#else
	TypedBuffer(XftChar8);
#endif
	XftDraw *	renderDraw;
#endif
#if OPT_DABBREV
	Boolean		dabbrev_working;	/* nonzero during dabbrev process */
	unsigned char	dabbrev_erase_char;	/* used for deleting inserted completion */
#endif
	char		tcapbuf[TERMCAP_SIZE];
	char		tcap_area[TERMCAP_SIZE];
#if OPT_TCAP_FKEYS
	char **		tcap_fkeys;
#endif
	String		cursor_font_name;	/* alternate cursor font */
} TScreen;

typedef XTermFonts *(*MyGetFont) (TScreen *, int);

typedef struct _TekPart {
	XFontStruct *	Tfont[TEKNUMFONTS];
	int		tobaseline[TEKNUMFONTS]; /* top-baseline, each font */
	char *		initial_font;		/* large, 2, 3, small */
	char *		gin_terminator_str;	/* ginTerminator resource */
#if OPT_TOOLBAR
	TbInfo		tb_info;	/* toolbar information		*/
#endif
} TekPart;

/* Tektronix window parameters */
typedef struct _TekScreen {
	GC		TnormalGC;	/* normal painting		*/
	GC		TcursorGC;	/* normal cursor painting	*/

	Boolean		waitrefresh;	/* postpone refresh		*/
	TKwin		fullTwin;
#ifndef NO_ACTIVE_ICON
	TKwin		iconTwin;
	TKwin		*whichTwin;
#endif /* NO_ACTIVE_ICON */

	Cursor		arrow;		/* arrow cursor			*/
	GC		linepat[TEKNUMLINES]; /* line patterns		*/
	int		cur_X;		/* current x			*/
	int		cur_Y;		/* current y			*/
	Tmodes		cur;		/* current tek modes		*/
	Tmodes		page;		/* starting tek modes on page	*/
	int		margin;		/* 0 -> margin 1, 1 -> margin 2	*/
	int		pen;		/* current Tektronix pen 0=up, 1=dn */
	char		*TekGIN;	/* nonzero if Tektronix GIN mode*/
	int		gin_terminator; /* Tek strap option */
	char		tcapbuf[TERMCAP_SIZE];
} TekScreen;

#if OPT_PASTE64 || OPT_READLINE
#define SCREEN_FLAG(screenp,f)		(1&(screenp)->f)
#define SCREEN_FLAG_set(screenp,f)	((screenp)->f |= 1)
#define SCREEN_FLAG_unset(screenp,f)	((screenp)->f &= (unsigned) ~1L)
#define SCREEN_FLAG_save(screenp,f)	\
	((screenp)->f = (((screenp)->f)<<1) | SCREEN_FLAG(screenp,f))
#define SCREEN_FLAG_restore(screenp,f)	((screenp)->f = (((screenp)->f)>>1))
#else
#define SCREEN_FLAG(screenp,f)		(0)
#endif

/*
 * After screen-updates, reset the flag that tells us we should do wrapping.
 * Likewise, reset (in wide-character mode) the flag that tells us where the
 * "previous" character was written.
 */
#if OPT_WIDE_CHARS
#define ResetWrap(screen) \
    (screen)->do_wrap = \
    (screen)->char_was_written = False
#else
#define ResetWrap(screen) \
    (screen)->do_wrap = False
#endif

/* meaning of bits in screen.select flag */
#define	INWINDOW	01	/* the mouse is in one of the windows */
#define	FOCUS		02	/* one of the windows is the focus window */

#define MULTICLICKTIME 250	/* milliseconds */

typedef struct {
    const char *name;
    int code;
} FlagList;

typedef enum {
    keyboardIsLegacy,		/* bogus vt220 codes for F1-F4, etc. */
    keyboardIsDefault,
    keyboardIsHP,
    keyboardIsSCO,
    keyboardIsSun,
    keyboardIsTermcap,
    keyboardIsVT220
} xtermKeyboardType;

typedef enum {			/* legal values for screen.pointer_mode */
    pNever = 0
    , pNoMouse = 1
    , pAlways = 2
    , pFocused = 3
} pointerModeTypes;

typedef enum {			/* legal values for screen.utf8_mode */
    uFalse = 0
    , uTrue = 1
    , uAlways = 2
    , uDefault = 3
    , uLast
} utf8ModeTypes;

typedef enum {			/* legal values for screen.eight_bit_meta */
    ebFalse = 0
    , ebTrue = 1
    , ebNever = 2
    , ebLocale = 3
    , ebLast
} ebMetaModeTypes;

typedef enum {			/* legal values for misc.cdXtraScroll */
    edFalse = 0
    , edTrue = 1
    , edTrim = 2
    , edLast
} edXtraScrollTypes;

#define NAME_OLD_KT " legacy"

#if OPT_HP_FUNC_KEYS
#define NAME_HP_KT " hp"
#else
#define NAME_HP_KT /*nothing*/
#endif

#if OPT_SCO_FUNC_KEYS
#define NAME_SCO_KT " sco"
#else
#define NAME_SCO_KT /*nothing*/
#endif

#if OPT_SUN_FUNC_KEYS
#define NAME_SUN_KT " sun"
#else
#define NAME_SUN_KT /*nothing*/
#endif

#if OPT_SUNPC_KBD
#define NAME_VT220_KT " vt220"
#else
#define NAME_VT220_KT /*nothing*/
#endif

#if OPT_TCAP_FKEYS
#define NAME_TCAP_KT " tcap"
#else
#define NAME_TCAP_KT /*nothing*/
#endif

#define KEYBOARD_TYPES NAME_TCAP_KT NAME_HP_KT NAME_SCO_KT NAME_SUN_KT NAME_VT220_KT

#if OPT_TRACE
#define TRACE_RC(code,func) code = func
#else
#define TRACE_RC(code,func) func
#endif

extern	const char * visibleKeyboardType(xtermKeyboardType);

typedef struct
{
    int allow_keys;		/* how to handle legacy/vt220 keyboard */
    int cursor_keys;		/* how to handle cursor-key modifiers */
    int function_keys;		/* how to handle function-key modifiers */
    int keypad_keys;		/* how to handle keypad key-modifiers */
    int other_keys;		/* how to handle other key-modifiers */
    int string_keys;		/* how to handle string() modifiers */
    int modify_keys;		/* how to handle modifier key-modifiers */
    int special_keys;		/* how to handle special key-modifiers */
} TModify;

typedef enum {
    mfkNone = -1		/* disable the feature */
    , mfkOriginal = 0		/* original/obsolete format */
    , mfkControl  = 1		/* prefix modified controls with CSI */
    , mfkParam    = 2		/* modifier parameter is second */
    , mfkPrivate  = 3		/* mark this as a private control */
    , mfkExtended = 4		/* modify all special keys */
} MfkModes;			/* also modifyCursorKeys, modifyKeypadKeys */

typedef enum {
    mokNone = 0			/* disable the feature */
    , mokUser     = 1		/* user-friendly modified-keys */
    , mokProgram  = 2		/* program-friendly modified-keys */
    , mokExtended = 3		/* modify all ordinary keys */
} MokModes;			/* modifyOtherKeys */

typedef enum {
    modifyKeyboard = 0		/* mode */
    , modifyCursorKeys   = 1	/* keysym parameterized mode */
    , modifyFunctionKeys = 2	/* keysym parameterized mode */
    , modifyKeypadKeys   = 3	/* keysym non-parameterized mode */
    , modifyOtherKeys    = 4	/* keysym semi-parameterized mode */
    , modifyStringKeys   = 5	/* reserved */
    , modifyModifierKeys = 6	/* used by Xutf8LookupString, etc */
    , modifySpecialKeys  = 7	/* other special/non-parameterized keysyms */
} ModkeyModes;

typedef struct
{
    xtermKeyboardType type;
    IFlags flags;
    char *shell_translations;	/* shell's translations, for input check */
    char *xterm_translations;	/* xterm's translations, for input check */
    char *extra_translations;
    char *print_translations;	/* printable translations for buttons */
    unsigned shift_buttons;	/* special shift-modifier for mouse-buttons */
    int shift_escape;		/* working value of shiftEscape */
    char * shift_escape_s;	/* resource for shiftEscape */
#if OPT_INITIAL_ERASE
    int	reset_DECBKM;		/* reset should set DECBKM */
#endif
#if OPT_MOD_FKEYS
    TModify modify_now;		/* current modifier value */
    TModify modify_1st;		/* original modifier value, for resets */
    TModify format_now;		/* current formatter value */
    TModify format_1st;		/* original formatter value, for resets */
    TModify ignore_now;		/* limit modifiers used in modifyXxxKeys */
    TModify ignore_1st;		/* original limit on modifiers */
#endif
} TKeyboard;

#define GravityIsNorthWest(w) ((w)->misc.resizeGravity == NorthWestGravity)
#define GravityIsSouthWest(w) ((w)->misc.resizeGravity == SouthWestGravity)

typedef struct _Misc {
    VTFontNames default_font;
    char *geo_metry;
    char *T_geometry;
#if OPT_WIDE_CHARS
    Boolean cjk_width;		/* true for built-in CJK wcwidth() */
    Boolean mk_width;		/* true for simpler built-in wcwidth() */
    int mk_samplesize;
    int mk_samplepass;
#endif
#if OPT_LUIT_PROG
    Boolean callfilter;		/* true to invoke luit */
    Boolean use_encoding;	/* true to use -encoding option for luit */
    char *locale_str;		/* "locale" resource */
    char *localefilter;		/* path for luit */
#endif
    fontWarningTypes fontWarnings;
    int limit_resize;
#ifdef ALLOWLOGGING
    Boolean log_on;
#endif
    Boolean color_inner_border;
    Boolean login_shell;
    Boolean re_verse;
    Boolean re_verse0;		/* initial value of "-rv" */
    Boolean resizeByPixel;
    XtGravity resizeGravity;
    Boolean reverseWrap;
    Boolean autoWrap;
    Boolean logInhibit;
    Boolean signalInhibit;
#if OPT_TEK4014
    Boolean tekInhibit;
    Boolean tekSmall;		/* start tek window in small size */
    Boolean TekEmu;		/* true if Tektronix emulation	*/
    Boolean Tshow;		/* Tek window showing		*/
#endif
    Boolean scrollbar;
#ifdef SCROLLBAR_RIGHT
    Boolean useRight;
#endif
    Boolean titeInhibit;
    Boolean appcursorDefault;
    Boolean appkeypadDefault;
    int cdXtraScroll;		/* scroll on cd (clear-display) */
    char *cdXtraScroll_s;
    int tiXtraScroll;		/* scroll on ti/te (init/end-cup) */
    char *tiXtraScroll_s;
#if OPT_INPUT_METHOD
    char* f_x;			/* font for XIM */
    char* input_method;
    char* preedit_type;
    Boolean open_im;		/* true if input-method is opened */
    int retry_im;
#endif
    Boolean dynamicColors;
#ifndef NO_ACTIVE_ICON
    char *active_icon_s;	/* use application icon window  */
    unsigned icon_border_width;
    Pixel icon_border_pixel;
#endif /* NO_ACTIVE_ICON */
#if OPT_DEC_SOFTFONT
    Boolean font_loadable;
#endif
#if OPT_SHIFT_FONTS
    Boolean shift_fonts;	/* true if we interpret fontsize-shifting */
#endif
#if OPT_SUNPC_KBD
    int ctrl_fkeys;		/* amount to add to XK_F1 for ctrl modifier */
#endif
#if OPT_NUM_LOCK
    Boolean real_NumLock;	/* true if we treat NumLock key specially */
    Boolean alwaysUseMods;	/* true if we always want f-key modifiers */
#endif
#if OPT_RENDERFONT
    VTFontNames default_xft;
    float face_size[NMENUFONTS];
    char *render_font_s;
    int limit_fontsets;
    int limit_fontheight;
    int limit_fontwidth;
#endif
} Misc;

typedef struct _Work {
    int dummy;
#ifdef SunXK_F36
#define MAX_UDK 37
#else
#define MAX_UDK 35
#endif
    struct {
	char *str;
	int len;
    } user_keys[MAX_UDK];
#define SET_MIN_MOD(xw, value) xw->work.min_mod = (value >= 3) ? 0 : 1
    int min_mod;		/* zero for modifying control-keys */
#define MAX_POINTER (XC_num_glyphs/2)
    Cursor pointer_cursors[MAX_POINTER]; /* saved cursors	*/
#ifndef NO_ACTIVE_ICON
    int active_icon;		/* use application icon window  */
    char *wm_name;
#endif /* NO_ACTIVE_ICON */
#if OPT_INPUT_METHOD
    Boolean cannot_im;		/* true if we cannot use input-method */
    XFontSet xim_fs;		/* fontset for XIM preedit */
    int xim_fs_ascent;		/* ascent of fs */
    TInput inputs[NINPUTWIDGETS];
#endif
    Boolean doing_resize;	/* currently in RequestResize */
#if OPT_MAXIMIZE
#define MAX_EWMH_MODE 3
#define MAX_EWMH_DATA (1 + OPT_TEK4014)
    struct {
	int mode;		/* fullscreen, etc.		*/
	Boolean checked[MAX_EWMH_MODE + 1];
	Boolean allowed[MAX_EWMH_MODE + 1];
    } ewmh[MAX_EWMH_DATA];
#endif
#if OPT_NUM_LOCK
    unsigned num_lock;		/* modifier for Num_Lock */
    unsigned alt_mods;		/* modifier for Alt_L or Alt_R */
    unsigned meta_mods;		/* modifier for Meta_L or Meta_R */
#endif
    XtermFontNames fonts;
    Boolean force_wideFont;	/* true to single-step wideFont	*/
#if OPT_RENDERFONT
    Boolean render_font;
    FcPattern *xft_defaults;
    unsigned max_fontsets;
#endif
#if OPT_DABBREV
#define MAX_DABBREV	1024	/* maximum word length as in tcsh */
    char dabbrev_data[MAX_DABBREV];
#endif
    ScrnColors *oldColors;
    Boolean palette_changed;
    Boolean broken_box_chars;
    /* data write dotext/WriteText */
    IChar *write_text;		/* points to print_area */
#if OPT_DEC_RECTOPS
    Char *write_sums;		/* if non-null, points to buffer_sums */
    Char *buffer_sums;		/* data for ->charSeen[] */
    Char *buffer_sets;		/* data for ->charSets[] */
    size_t sizeof_sums;		/* allocated size of buffer_sums */
#endif
} Work;

typedef struct {int foo;} XtermClassPart, TekClassPart;

typedef struct _XtermClassRec {
    CoreClassPart  core_class;
    XtermClassPart xterm_class;
} XtermClassRec;

extern WidgetClass xtermWidgetClass;

#define IsXtermWidget(w) (XtClass(w) == xtermWidgetClass)

#if OPT_TEK4014
typedef struct _TekClassRec {
    CoreClassPart core_class;
    TekClassPart tek_class;
} TekClassRec;

extern WidgetClass tekWidgetClass;

#define IsTekWidget(w) (XtClass(w) == tekWidgetClass)

#endif

/* define masks for keyboard.flags */
#define MODE_KAM	xBIT(0)	/* mode 2: keyboard action mode */
#define MODE_DECKPAM	xBIT(1)	/* keypad application mode */
#define MODE_DECCKM	xBIT(2)	/* private mode 1: cursor keys */
#define MODE_SRM	xBIT(3)	/* mode 12: send-receive mode */
#define MODE_DECBKM	xBIT(4)	/* private mode 67: backarrow */
#define MODE_DECSDM	xBIT(5)	/* private mode 80: sixel DISPLAY mode -- note, when SDM is off, the terminal is in sixel SCROLLING mode  */

#define N_MARGINBELL	10

#define TAB_BITS_SHIFT	5	/* FIXME: 2**5 == 32 (should derive) */
#define TAB_BITS_WIDTH	(1 << TAB_BITS_SHIFT)
#define TAB_ARRAY_SIZE	(1024 / TAB_BITS_WIDTH)
#define MAX_TABS	(TAB_BITS_WIDTH * TAB_ARRAY_SIZE)

#define OkTAB(c)	((c) > 0 && (c) < MAX_TABS)

typedef unsigned Tabs [TAB_ARRAY_SIZE];

#if OPT_XTERM_SGR
#define MAX_SAVED_SGR	10
typedef	struct {
    int		used;
    struct	{
	IFlags	mask;
	IFlags	flags;
#if OPT_ISO_COLORS
	int	sgr_foreground;
	int	sgr_background;
	Boolean	sgr_38_xcolors;
#endif
    } stack[MAX_SAVED_SGR];
} SavedSGR;

typedef struct {
    ScrnColors base;
    ColorRes ansi[1];
} ColorSlot;

typedef struct {
    int		used;		/* currently saved or restored	*/
    int		last;		/* maximum number of saved palettes */
    ColorSlot	*palettes[MAX_SAVED_SGR];
} SavedColors;
#endif /* OPT_XTERM_SGR */

typedef struct _XtermWidgetRec {
    CorePart	core;
    XSizeHints	hints;
    XVisualInfo *visInfo;
    int		numVisuals;
    unsigned	rgb_shifts[3];
    unsigned	rgb_widths[3];
    Bool	has_rgb;
    Bool	init_menu;
    TKeyboard	keyboard;	/* terminal keyboard		*/
    TScreen	screen;		/* terminal screen		*/
    IFlags	flags;		/* mode flags			*/
    int		cur_foreground; /* current foreground color	*/
    int		cur_background; /* current background color	*/
    Pixel	dft_foreground; /* default foreground color	*/
    Pixel	dft_background; /* default background color	*/
    Pixel	old_foreground; /* original foreground color	*/
    Pixel	old_background; /* original background color	*/
#if OPT_ISO_COLORS
    int		sgr_foreground; /* current SGR foreground color */
    int		sgr_background; /* current SGR background color */
    Boolean	sgr_38_xcolors;	/* true if ISO 8613 extension	*/
#endif
    IFlags	initflags;	/* initial mode flags		*/
    Tabs	tabs;		/* tabstops of the terminal	*/
    Misc	misc;		/* miscellaneous parameters	*/
    Work	work;		/* workspace (no resources)	*/
#if OPT_XTERM_SGR
    SavedSGR	saved_sgr;
    SavedColors	saved_colors;
#endif
} XtermWidgetRec, *XtermWidget;

#if OPT_TEK4014
typedef struct _TekWidgetRec {
    CorePart	core;
    XtermWidget vt;		/* main widget has border, etc. */
    TekPart	tek;		/* contains resources */
    TekScreen	screen;		/* contains working data (no resources) */
    Bool	init_menu;
    XSizeHints	hints;
} TekWidgetRec, *TekWidget;
#endif /* OPT_TEK4014 */

/*
 * terminal flags
 * There are actually two namespaces mixed together here:
 * a) One is the set of flags that can go in screen->visbuf attributes and
 *    which must fit in an IAttr (either a char or short, depending on whether
 *    wide-attributes are used).
 * b) The other is the global setting stored in term->flags and
 *    screen->save_modes, which fits in an unsigned (IFlags).
 */

#define AttrBIT(n)	xBIT(n)		/* text-attributes */
#define MiscBIT(n)	xBIT(n + 16)	/* miscellaneous state flags */

/* global flags and character flags (visible character attributes) */
#define INVERSE		AttrBIT(0)	/* invert the characters to be output */
#define UNDERLINE	AttrBIT(1)	/* true if underlining */
#define BOLD		AttrBIT(2)
#define BLINK		AttrBIT(3)
/* global flags (also character attributes) */
#define BG_COLOR	AttrBIT(4)	/* true if background set */
#define FG_COLOR	AttrBIT(5)	/* true if foreground set */

/* character flags (internal attributes) */
#define PROTECTED	AttrBIT(6)	/* a character that cannot be erased */
#define CHARDRAWN	AttrBIT(7)	/* a character has been drawn here on
					   the screen.  Used to distinguish
					   blanks from empty parts of the
					   screen when selecting */
/*
 * This does not fit in a byte with the other (more important) attributes, but
 * if wide-attributes are configured, it is possible to maintain it there.
 */
#define INVISIBLE	AttrBIT(8)	/* true if writing invisible text */

#if OPT_WIDE_ATTRS
#define ATR_FAINT	AttrBIT(9)
#define ATR_ITALIC	AttrBIT(10)
#define ATR_STRIKEOUT	AttrBIT(11)
#define ATR_DBL_UNDER	AttrBIT(12)
#define ATR_DIRECT_FG	AttrBIT(13)
#define ATR_DIRECT_BG	AttrBIT(14)
#define SGR_MASK2       (ATR_FAINT | ATR_ITALIC | ATR_STRIKEOUT | ATR_DBL_UNDER | ATR_DIRECT_FG | ATR_DIRECT_BG)
#define AttrEND         15
#else
#define SGR_MASK2       0
#define AttrEND         9
#endif

/*
 * Other flags
 */
#define REVERSE_VIDEO	MiscBIT(0)	/* true if screen white on black */
#define WRAPAROUND	MiscBIT(1)	/* true if auto wraparound mode */
#define	REVERSEWRAP	MiscBIT(2)	/* true if reverse wraparound mode */
#define	REVERSEWRAP2	MiscBIT(3)	/* true if extended reverse wraparound */
#define LINEFEED	MiscBIT(4)	/* true if in auto linefeed mode */
#define ORIGIN		MiscBIT(5)	/* true if in origin mode */
#define INSERT		MiscBIT(6)	/* true if in insert mode */
#define SMOOTHSCROLL	MiscBIT(7)	/* true if in smooth scroll mode */
#define IN132COLUMNS	MiscBIT(8)	/* true if in 132 column mode */
#define NATIONAL        MiscBIT(9)	/* true if writing national charset */
#define LEFT_RIGHT      MiscBIT(10)	/* true if left/right margin mode */
#define NOCLEAR_COLM    MiscBIT(11)	/* true if no clear on DECCOLM change */

/*
 * Drawing-bits start after the video/color attributes, and are independent
 * of the miscellaneous flags.
 */
#define DrawBIT(n)	xBIT(n + AttrEND) /* XTermDraw.draw_flags */
/* The following attributes are used in the argument of drawXtermText()  */
#define NOBACKGROUND	DrawBIT(0)	/* Used for overstrike */
#define NOTRANSLATION	DrawBIT(1)	/* No scan for chars missing in font */
#define DOUBLEWFONT	DrawBIT(2)	/* The actual X-font is double-width */
#define DOUBLEHFONT	DrawBIT(3)	/* The actual X-font is double-height */
#define DOUBLEFIRST	DrawBIT(4)	/* Draw chars one-by-one */
#define CHARBYCHAR	DrawBIT(5)	/* Draw chars one-by-one */

/* The following attribute is used in the argument of xtermSpecialFont etc */
#define NORESOLUTION	DrawBIT(6)	/* find the font without resolution */

/*
 * Groups of attributes
 */
			/* mask for video-attributes only */
#define SGR_MASK	(BOLD | BLINK | UNDERLINE | INVERSE)

			/* mask: user-visible attributes */
#define	ATTRIBUTES	(SGR_MASK | SGR_MASK2 | BG_COLOR | FG_COLOR | INVISIBLE | PROTECTED)

/* The toplevel-call to drawXtermText() should have text-attributes guarded: */
#define DRAWX_MASK	(ATTRIBUTES | CHARDRAWN)

/*
 * BOLDATTR is not only nonzero when we will use bold font, but uses the bits
 * for BOLD/BLINK to match against the video attributes which were originally
 * requested.
 */
#define USE_BOLD(screen) ((screen)->allowBoldFonts)

#if OPT_BLINK_TEXT
#define BOLDATTR(screen) (unsigned) (USE_BOLD(screen) ? (BOLD | ((screen)->blink_as_bold ? BLINK : 0)) : 0)
#else
#define BOLDATTR(screen) (unsigned) (USE_BOLD(screen) ? (BOLD | BLINK) : 0)
#endif

/*
 * Sixel scrolling is on when Sixel Display Mode is off, and vice versa.
 * (Note: DEC erroneously conflates the two in the VT330/340 manual).
 */
#define SixelScrolling(xw) (!((xw)->keyboard.flags & MODE_DECSDM))

/*
 * Per-line flags
 */
#define LINEWRAPPED	AttrBIT(0)
/* used once per line to indicate that it wraps onto the next line so we can
 * tell the difference between lines that have wrapped around and lines that
 * have ended naturally with a CR at column max_col.
 */
#define LINEBLINKED	AttrBIT(1)
/* set when the line contains blinking text.
 */

#if OPT_ZICONBEEP || OPT_TOOLBAR || (USE_DOUBLE_BUFFER && OPT_RENDERFONT)
#define HANDLE_STRUCT_NOTIFY 1
#else
#define HANDLE_STRUCT_NOTIFY 0
#endif

/*
 * If we've set protected attributes with the DEC-style DECSCA, then we'll have
 * to use DECSED or DECSEL to erase preserving protected text.  (The normal ED,
 * EL won't preserve protected-text).  If we've used SPA, then normal ED and EL
 * will preserve protected-text.  To keep things simple, just remember the last
 * control that was used to begin protected-text, and use that to determine how
 * erases are performed (otherwise we'd need 2 bits per protected character).
 */
#define OFF_PROTECT 0
#define DEC_PROTECT 1
#define ISO_PROTECT 2

/***====================================================================***/

/*
 * Reduce parameter-count of drawXtermText by putting less-modified data here.
 */
typedef struct {
	XtermWidget	xw;
	unsigned	attr_flags;
	unsigned	draw_flags;
	unsigned	this_chrset;
	unsigned	real_chrset;
	int		on_wide;
} XTermDraw;

/***====================================================================***/

#define TScreenOf(xw)	(&(xw)->screen)
#define TekScreenOf(tw) (&(tw)->screen)

#define PrinterOf(screen) (screen)->printer_state

#ifdef SCROLLBAR_RIGHT
#define OriginX(screen) (((term->misc.useRight)?0:ScrollbarWidth(screen)) + screen->border)
#else
#define OriginX(screen) (ScrollbarWidth(screen) + screen->border)
#endif

#define OriginY(screen) (screen->border)

#define CursorMoved(screen) \
		((screen)->cursor_moved || \
		    ((screen)->cursorp.col != (screen)->cur_col || \
		     (screen)->cursorp.row != (screen)->cur_row))

#define CursorX2(screen,col,fw) ((col) * (int)(fw) + OriginX(screen))
#define CursorX(screen,col)     CursorX2(screen, col, FontWidth(screen))
#define CursorY2(screen,row)    (((row) * FontHeight(screen)) + screen->border)
#define CursorY(screen,row)     CursorY2(screen, INX2ROW(screen, row))

/*
 * These definitions depend on whether xterm supports active-icon.
 */
#ifndef NO_ACTIVE_ICON
#define IsIconWin(screen,win)	((win) == &(screen)->iconVwin)
#define IsIcon(screen)		(WhichVWin(screen) == &(screen)->iconVwin)
#define WhichVWin(screen)	((screen)->whichVwin)
#define WhichTWin(screen)	((screen)->whichTwin)

#define WhichVFont(screen,name)	(IsIcon(screen) ? getIconicFont(screen) \
						: getNormalFont(screen, (int)(name)))->fs
#define FontAscent(screen)	(IsIcon(screen) ? getIconicFont(screen)->fs->ascent \
						: WhichVWin(screen)->f_ascent)
#define FontDescent(screen)	(IsIcon(screen) ? getIconicFont(screen)->fs->descent \
						: WhichVWin(screen)->f_descent)
#else /* NO_ACTIVE_ICON */

#define IsIconWin(screen,win)	(False)
#define IsIcon(screen)		(False)
#define WhichVWin(screen)	(&((screen)->fullVwin))
#define WhichTWin(screen)	(&((screen)->fullTwin))

#define WhichVFont(screen,name)	getNormalFont(screen, (int)(name))->fs
#define FontAscent(screen)	WhichVWin(screen)->f_ascent
#define FontDescent(screen)	WhichVWin(screen)->f_descent

#endif /* NO_ACTIVE_ICON */

#define okFont(font) ((font) != NULL && (font)->fid != 0)

/*
 * Macro to check if we are iconified; do not use render for that case.
 */
#define UsingRenderFont(xw)	(((xw)->work.render_font == True) && !IsIcon(TScreenOf(xw)))

/*
 * These definitions do not depend on whether xterm supports active-icon.
 */
#define VWindow(screen)		WhichVWin(screen)->window
#define VShellWindow(xw)	XtWindow(SHELL_OF(xw))
#define TWindow(screen)		WhichTWin(screen)->window
#define TShellWindow		XtWindow(SHELL_OF(tekWidget))

#if USE_DOUBLE_BUFFER
extern Window VDrawable(TScreen * /* screen */);
#else
#define VDrawable(screen)	VWindow(screen)
#endif

#define Width(screen)		WhichVWin(screen)->width
#define Height(screen)		WhichVWin(screen)->height
#define FullWidth(screen)	WhichVWin(screen)->fullwidth
#define FullHeight(screen)	WhichVWin(screen)->fullheight
#define FontWidth(screen)	WhichVWin(screen)->f_width
#define FontHeight(screen)	WhichVWin(screen)->f_height

#define NormalFont(screen)	WhichVFont(screen, fNorm)
#define BoldFont(screen)	WhichVFont(screen, fBold)

#if OPT_WIDE_CHARS
#define NormalWFont(screen)	WhichVFont(screen, fWide)
#define BoldWFont(screen)	WhichVFont(screen, fWBold)
#endif

#define ScrollbarWidth(screen)	WhichVWin(screen)->sb_info.width

/* y -> y_shift, to center text versus the cursor */
#define ScaleShift(screen) \
	    (int) ((IsIcon(screen) || (screen->scale_height <= 1.0f)) \
	           ? 0.0f \
	           : ((float) WhichVWin(screen)->f_height \
		      * ((float) screen->scale_height - 1.0f) / 2.0f))

#define BorderGC(w,sp)		WhichVWin(sp)->border_gc
#define FillerGC(w,sp)		WhichVWin(sp)->filler_gc
#define NormalGC(w,sp)		getCgsGC(w, WhichVWin(sp), gcNorm)
#define ReverseGC(w,sp)		getCgsGC(w, WhichVWin(sp), gcNormReverse)
#define NormalBoldGC(w,sp)	getCgsGC(w, WhichVWin(sp), gcBold)
#define ReverseBoldGC(w,sp)	getCgsGC(w, WhichVWin(sp), gcBoldReverse)

#define TWidth(screen)		WhichTWin(screen)->width
#define THeight(screen)		WhichTWin(screen)->height
#define TFullWidth(screen)	WhichTWin(screen)->fullwidth
#define TFullHeight(screen)	WhichTWin(screen)->fullheight
#define TekScale(screen)	WhichTWin(screen)->tekscale

/* use these before tek4014 is realized, good enough for default "9x15" font */
#define TDefaultRows		37
#define TDefaultCols		75

#define BorderWidth(w)		((w)->core.border_width)
#define BorderPixel(w)		((w)->core.border_pixel)

#define AllowXtermOps(w,name)	(TScreenOf(w)->name && !TScreenOf(w)->allowSendEvents)

#define AllowColorOps(w,name)	(AllowXtermOps(w, allowColorOps) || \
				 !TScreenOf(w)->disallow_color_ops[name])

#define AllowFontOps(w,name)	(AllowXtermOps(w, allowFontOps) || \
				 !TScreenOf(w)->disallow_font_ops[name])

#define AllowMouseOps(w,name)	(AllowXtermOps(w, allowMouseOps) || \
				 !TScreenOf(w)->disallow_mouse_ops[name])

#define AllowTcapOps(w,name)	(AllowXtermOps(w, allowTcapOps) || \
				 !TScreenOf(w)->disallow_tcap_ops[name])

#define AllowTitleOps(w)	AllowXtermOps(w, allowTitleOps)

#define AllowXResOps(w)		True

#define SpecialWindowOps(w,name) (!TScreenOf(w)->disallow_win_ops[name])
#define AllowWindowOps(w,name)	(AllowXtermOps(w, allowWindowOps) || \
				 SpecialWindowOps(w,name))

#if OPT_TOOLBAR
#define ToolbarHeight(w)	((resource.toolBar) \
				 ? ((w)->VT100_TB_INFO(menu_height) \
				  + (w)->VT100_TB_INFO(menu_border) * 2) \
				 : 0)
#else
#define ToolbarHeight(w) 0
#endif

#if OPT_TEK4014
#define TEK_LINK_BLOCK_SIZE 1024

typedef struct Tek_Link
{
	struct Tek_Link	*next;	/* pointer to next TekLink in list
				   NULL <=> this is last TekLink */
	unsigned short fontsize;/* character size, 0-3 */
	unsigned short count;	/* number of chars in data */
	char *ptr;		/* current pointer into data */
	char data [TEK_LINK_BLOCK_SIZE];
} TekLink;
#endif /* OPT_TEK4014 */

/* flags for cursors */
#define	OFF		0
#define	ON		1
#define	BLINKED_OFF	2
#define	CLEAR		0
#define	TOGGLE		1

/* flags for inhibit */
#ifdef ALLOWLOGGING
#define	I_LOG		0x01
#endif
#define	I_SIGNAL	0x02
#define	I_TEK		0x04

/* *INDENT-ON* */

#endif /* included_ptyx_h */
