/*
 *	$XConsortium: ptyx.h /main/67 1996/11/29 10:34:19 swick $
 *	$XFree86: xc/programs/xterm/ptyx.h,v 3.19.2.2 1997/05/25 05:07:00 dawes Exp $
 */

/*
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

/* ptyx.h */
/* @(#)ptyx.h	X10/6.6	11/10/86 */

#include <X11/IntrinsicP.h>
#include <X11/Xmu/Misc.h>	/* For Max() and Min(). */
#include <X11/Xfuncs.h>
#include <X11/Xosdefs.h>

#ifdef AMOEBA
/* Avoid name clashes with standard Amoeba types: */
#define event    am_event_t
#define interval am_interval_t
#include <amoeba.h>
#include <semaphore.h>
#include <circbuf.h>
#undef event
#undef interval
#endif

/* Extra Xlib definitions */
#define AllButtonsUp(detail, ignore)  (\
		((ignore) == Button1) ? \
				(((detail)&(Button2Mask|Button3Mask)) == 0) \
				: \
		 (((ignore) == Button2) ? \
		  		(((detail)&(Button1Mask|Button3Mask)) == 0) \
				: \
		  		(((detail)&(Button1Mask|Button2Mask)) == 0)) \
		)

#define MAX_COLS	200
#define MAX_ROWS	128

/*
** System V definitions
*/

#ifdef SYSV
#ifdef X_NOT_POSIX
#ifndef CRAY
#define	dup2(fd1,fd2)	((fd1 == fd2) ? fd1 : \
				(close(fd2), fcntl(fd1, F_DUPFD, fd2)))
#endif
#endif
#endif /* SYSV */

/*
** allow for mobility of the pty master/slave directories
*/
#ifndef PTYDEV
#ifdef hpux
#define	PTYDEV		"/dev/ptym/ptyxx"
#else	/* !hpux */
#ifndef __osf__
#define	PTYDEV		"/dev/ptyxx"
#endif
#endif	/* !hpux */
#endif	/* !PTYDEV */

#ifndef TTYDEV
#ifdef hpux
#define TTYDEV		"/dev/pty/ttyxx"
#else	/* !hpux */
#ifdef __osf__
#define TTYDEV		"/dev/ttydirs/xxx/xxxxxxxxxxxxxx"
#else
#define	TTYDEV		"/dev/ttyxx"
#endif
#endif	/* !hpux */
#endif	/* !TTYDEV */

#ifndef PTYCHAR1
#ifdef hpux
#define PTYCHAR1	"zyxwvutsrqp"
#else	/* !hpux */
#ifdef __EMX__
#define PTYCHAR1	"pq"
#else
#define	PTYCHAR1	"pqrstuvwxyzPQRSTUVWXYZ"
#endif  /* !__EMX__ */
#endif	/* !hpux */
#endif	/* !PTYCHAR1 */

#ifndef PTYCHAR2
#ifdef hpux
#define	PTYCHAR2	"fedcba9876543210"
#else	/* !hpux */
#ifdef __FreeBSD__
#define	PTYCHAR2	"0123456789abcdefghijklmnopqrstuv"
#else /* !__FreeBSD__ */
#define	PTYCHAR2	"0123456789abcdef"
#endif /* !__FreeBSD__ */
#endif	/* !hpux */
#endif	/* !PTYCHAR2 */

#ifndef TTYFORMAT
#ifdef CRAY
#define TTYFORMAT "/dev/ttyp%03d"
#else
#define TTYFORMAT "/dev/ttyp%d"
#endif
#endif

#ifndef PTYFORMAT
#ifdef CRAY
#define PTYFORMAT "/dev/pty/%03d"
#else
#define PTYFORMAT "/dev/ptyp%d"
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

typedef enum {NORMAL, LEFTEXTENSION, RIGHTEXTENSION} EventMode;

/*
 * The origin of a screen is 0, 0.  Therefore, the number of rows
 * on a screen is screen->max_row + 1, and similarly for columns.
 */

typedef unsigned char Char;		/* to support 8 bit chars */
typedef Char **ScrnBuf;

/*
 * ANSI emulation.
 */
#define INQ	0x05
#define BEL	0x07
#define	FF	0x0C			/* C0, C1 control names		*/
#define	LS1	0x0E
#define	LS0	0x0F
#define	NAK	0x15
#define	CAN	0x18
#define	SUB	0x1A
#define	ESC	0x1B
#define US	0x1F
#define	DEL	0x7F
#define HTS     ('H'+0x40)
#define	RI	0x8D
#define	SS2	0x8E
#define	SS3	0x8F
#define	DCS	0x90
#define	SPA	0x96
#define	EPA	0x97
#define	SOS	0x98
#define	OLDID	0x9A			/* ESC Z			*/
#define	CSI	0x9B
#define	ST	0x9C
#define	OSC	0x9D
#define	PM	0x9E
#define	APC	0x9F
#define	RDEL	0xFF

#define MIN_DECID  52			/* can emulate VT52 */
#define MAX_DECID 420			/* ...through VT420 */

#ifndef DFT_DECID
#define DFT_DECID 100			/* default VT100 */
#endif

#define NMENUFONTS 9			/* entries in fontMenu */

#define	NBOX	5			/* Number of Points in box	*/
#define	NPARAM	10			/* Max. parameters		*/

#define	MINHILITE	32

typedef struct {
	unsigned char	a_type;
	unsigned char	a_pintro;
	unsigned char	a_final;
	unsigned char	a_inters;
	char	a_nparam;		/* # of parameters		*/
	char	a_dflt[NPARAM];		/* Default value flags		*/
	short	a_param[NPARAM];	/* Parameters			*/
	char	a_nastyf;		/* Error flag			*/
} ANSI;

typedef struct {
	int		row;
	int		col;
	unsigned	flags;	/* Vt100 saves graphics rendition. Ugh! */
	char		curgl;
	char		curgr;
	char		gsets[4];
} SavedCursor;

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
	int	linetype;
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

#define	SAVELINES		64      /* default # lines to save      */
#define SCROLLLINES 1			/* default # lines to scroll    */

/***====================================================================***/

#define	TEXT_FG		0
#define	TEXT_BG		1
#define	TEXT_CURSOR	2
#define	MOUSE_FG	3
#define	MOUSE_BG	4
#define	TEK_FG		5
#define	TEK_BG		6
#define	NCOLORS		7

#define	COLOR_DEFINED(s,w)	((s)->which&(1<<(w)))
#define	COLOR_VALUE(s,w)	((s)->colors[w])
#define	SET_COLOR_VALUE(s,w,v)	(((s)->colors[w]=(v)),((s)->which|=(1<<(w))))

#define	COLOR_NAME(s,w)		((s)->names[w])
#define	SET_COLOR_NAME(s,w,v)	(((s)->names[w]=(v)),((s)->which|=(1<<(w))))

#define	UNDEFINE_COLOR(s,w)	((s)->which&=(~((w)<<1)))
#define	OPPOSITE_COLOR(n)	(((n)==TEXT_FG?TEXT_BG:\
				 ((n)==TEXT_BG?TEXT_FG:\
				 ((n)==MOUSE_FG?MOUSE_BG:\
				 ((n)==MOUSE_BG?MOUSE_FG:\
				 ((n)==TEK_FG?TEK_BG:\
				 ((n)==TEXT_BG?TEK_FG:(n))))))))

typedef struct {
	unsigned	which;	/* must have NCOLORS bits */
	Pixel		colors[NCOLORS];
	char		*names[NCOLORS];
} ScrnColors;

/***====================================================================***/

#ifndef OPT_AIX_COLORS
#define OPT_AIX_COLORS  1 /* true if xterm is configured with AIX (16) colors */
#endif

#define OPT_BLINK_CURS  0 /* FIXME: do this later (96/7/31) */

#ifndef OPT_ISO_COLORS
#define OPT_ISO_COLORS  1 /* true if xterm is configured with ISO colors */
#endif

#ifndef OPT_VT52_MODE
#define OPT_VT52_MODE   1 /* true if xterm supports VT52 emulation */
#endif

/***====================================================================***/

#if OPT_AIX_COLORS && !OPT_ISO_COLORS
fixme: You must have ANSI/ISO colors to support AIX colors
#endif

/***====================================================================***/

#if OPT_ISO_COLORS
#define if_OPT_ISO_COLORS(screen, code) if(screen->colorMode) code
#define TERM_COLOR_FLAGS (term->flags & (FG_COLOR|BG_COLOR))
#define MAXCOLORS 18
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
#define COLOR_BD	16
#define COLOR_UL	17
#ifndef DFT_COLORMODE
#define DFT_COLORMODE FALSE	/* default colorMode resource */
#endif
#else
#define if_OPT_ISO_COLORS(screen, code) /* nothing */
#define TERM_COLOR_FLAGS 0
#endif	/* OPT_ISO_COLORS */

#if OPT_AIX_COLORS
#define if_OPT_AIX_COLORS(screen, code) if(screen->colorMode) code
#else
#define if_OPT_AIX_COLORS(screen, code) /* nothing */
#endif

	/* the number of pointers per row in 'ScrnBuf' */
#if OPT_ISO_COLORS
#define MAX_PTRS term->num_ptrs
#else
#define MAX_PTRS 2
#endif

#if OPT_VT52_MODE
#define if_OPT_VT52_MODE(screen, code) if(screen->ansi_level == 0) code
#else
#define if_OPT_VT52_MODE(screen, code) /* nothing */
#endif

	/* ScrnBuf-level macros */
#define BUF_CHARS(buf, row) (buf[MAX_PTRS * (row) + 0])
#define BUF_ATTRS(buf, row) (buf[MAX_PTRS * (row) + 1])

#if OPT_ISO_COLORS
#define BUF_COLOR(buf, row) (buf[MAX_PTRS * (row) + 2])
#endif

	/* TScreen-level macros */
#define SCRN_BUF_CHARS(screen, row) BUF_CHARS(screen->buf, row)
#define SCRN_BUF_ATTRS(screen, row) BUF_ATTRS(screen->buf, row)

#if OPT_ISO_COLORS
#define SCRN_BUF_COLOR(screen, row) BUF_COLOR(screen->buf, row)
#endif

typedef struct {
/* These parameters apply to both windows */
	Display		*display;	/* X display for screen		*/
	int		respond;	/* socket for responses
					   (position report, etc.)	*/
#ifdef AMOEBA
	capability      proccap;        /* process capability           */
	struct circbuf  *tty_inq;       /* tty server input queue       */
	struct circbuf  *tty_outq;      /* tty server output queue      */
#endif
	long		pid;		/* pid of process on far side   */
	int		uid;		/* user id of actual person	*/
	int		gid;		/* group id of actual person	*/
	GC		cursorGC;	/* normal cursor painting	*/
	GC		reversecursorGC;/* reverse cursor painting	*/
	GC		cursoroutlineGC;/* for painting lines around    */
	Pixel		foreground;	/* foreground color		*/
	Pixel		cursorcolor;	/* Cursor color			*/
	Pixel		mousecolor;	/* Mouse color			*/
	Pixel		mousecolorback;	/* Mouse color background	*/
#if OPT_ISO_COLORS
	Pixel		Acolors[MAXCOLORS]; /* ANSI color emulation	*/
	Boolean		colorMode;	/* are we using color mode?	*/
	Boolean		colorULMode;	/* use color for underline?	*/
	Boolean		colorBDMode;	/* use color for bold?		*/
#endif
	int		border;		/* inner border			*/
	Cursor		arrow;		/* arrow cursor			*/
	unsigned short	send_mouse_pos;	/* user wants mouse transition  */
					/* and position information	*/
	int		select;		/* xterm selected		*/
	Boolean		visualbell;	/* visual bell mode		*/
	Boolean		allowSendEvents;/* SendEvent mode		*/
	Boolean		awaitInput;	/* select-timeout mode		*/
	Boolean		grabbedKbd;	/* keyboard is grabbed		*/
#ifdef ALLOWLOGGING
	int		logging;	/* logging mode			*/
	int		logfd;		/* file descriptor of log	*/
	char		*logfile;	/* log file name		*/
	unsigned char	*logstart;	/* current start of log buffer	*/
#endif
	int		inhibit;	/* flags for inhibiting changes	*/

/* VT window parameters */
	struct _vtwin {
		Window	window;		/* X window id			*/
		int	width;		/* width of columns		*/
		int	height;		/* height of rows		*/
		int	fullwidth;	/* full width of window		*/
		int	fullheight;	/* full height of window	*/
		int	f_width;	/* width of fonts in pixels	*/
		int	f_height;	/* height of fonts in pixels	*/
		int	scrollbar;	/* if > 0, width of scrollbar, and
						scrollbar is showing	*/
		GC	normalGC;	/* normal painting		*/
		GC	reverseGC;	/* reverse painting		*/
		GC	normalboldGC;	/* normal painting, bold font	*/
		GC	reverseboldGC;	/* reverse painting, bold font	*/
	} fullVwin;
#ifndef NO_ACTIVE_ICON
	struct _vtwin iconVwin;
	struct _vtwin *whichVwin;
#endif /* NO_ACTIVE_ICON */
	Cursor pointer_cursor;		/* pointer cursor in window	*/

	/* Terminal fonts must be of the same size and of fixed width */
	XFontStruct	*fnt_norm;	/* normal font of terminal	*/
	XFontStruct	*fnt_bold;	/* bold font of terminal	*/
#ifndef NO_ACTIVE_ICON
	XFontStruct	*fnt_icon;	/* icon font */
#endif /* NO_ACTIVE_ICON */
	int		enbolden;	/* overstrike for bold font	*/
	XPoint		*box;		/* draw unselected cursor	*/

	int		cursor_state;	/* ON, OFF, or BLINKED_OFF	*/
#if OPT_BLINK_CURS
	int		cursor_blink;	/* blink-rate (0=off) msecs	*/
	XtIntervalId	cursor_timer;	/* timer-id for cursor-proc	*/
#endif
	int		cursor_set;	/* requested state		*/
	int		cursor_col;	/* previous cursor column	*/
	int		cursor_row;	/* previous cursor row		*/
	int		cur_col;	/* current cursor column	*/
	int		cur_row;	/* current cursor row		*/
	int		max_col;	/* rightmost column		*/
	int		max_row;	/* bottom row			*/
	int		top_marg;	/* top line of scrolling region */
	int		bot_marg;	/* bottom line of  "	    "	*/
	Widget		scrollWidget;	/* pointer to scrollbar struct	*/
	int		topline;	/* line number of top, <= 0	*/
	int		savedlines;     /* number of lines that've been saved */
	int		savelines;	/* number of lines off top to save */
	int		scrolllines;	/* number of lines to button scroll */
	Boolean		scrollttyoutput; /* scroll to bottom on tty output */
	Boolean		scrollkey;	/* scroll to bottom on key	*/
	
	ScrnBuf		buf;		/* ptr to visible screen buf (main) */
	ScrnBuf		allbuf;		/* screen buffer (may include
					   lines scrolled off top)	*/
	Char		*sbuf_address;	/* main screen memory address   */
	ScrnBuf		altbuf;		/* alternate screen buffer	*/
	Char		*abuf_address;	/* alternate screen memory address */
	Boolean		alternate;	/* true if using alternate buf	*/
	unsigned short	do_wrap;	/* true if cursor in last column
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
	Boolean		c132;		/* allow change to 132 columns	*/
	Boolean		curses;		/* kludge line wrap for more	*/
	Boolean		hp_ll_bc;	/* kludge HP-style ll for xdb	*/
	Boolean		marginbell;	/* true if margin bell on	*/
	int		nmarginbell;	/* columns from right margin	*/
	int		bellarmed;	/* cursor below bell margin	*/
	Boolean 	multiscroll;	/* true if multi-scroll		*/
	int		scrolls;	/* outstanding scroll count,
					    used only with multiscroll	*/
	SavedCursor	sc;		/* data for restore cursor	*/
	int		save_modes[19];	/* save dec private modes	*/

	/* Improved VT100 emulation stuff.				*/
	char		gsets[4];	/* G0 through G3.		*/
	char		curgl;		/* Current GL setting.		*/
	char		curgr;		/* Current GR setting.		*/
	char		curss;		/* Current single shift.	*/
	int		terminal_id;	/* 100=vt100, 220=vt220, etc.	*/
	int		ansi_level;	/* 0=vt100, 1,2,3 = vt100 ... vt320 */
	int		scroll_amt;	/* amount to scroll		*/
	int		refresh_amt;	/* amount to refresh		*/
	int		protected_mode;	/* 0=off, 1=DEC, 2=ISO		*/
	Boolean		jumpscroll;	/* whether we should jumpscroll */
	Boolean         always_highlight; /* whether to highlight cursor */
	Boolean		underline;	/* whether to underline text	*/

/* Tektronix window parameters */
	GC		TnormalGC;	/* normal painting		*/
	GC		TcursorGC;	/* normal cursor painting	*/
	Pixel		Tforeground;	/* foreground color		*/
	Pixel		Tbackground;	/* Background color		*/
	Pixel		Tcursorcolor;	/* Cursor color			*/
	int		Tcolor;		/* colors used			*/
	Boolean		Vshow;		/* VT window showing		*/
	Boolean		Tshow;		/* Tek window showing		*/
	Boolean		waitrefresh;	/* postpone refresh		*/
	struct _tekwin {
		Window	window;		/* X window id			*/
		int	width;		/* width of columns		*/
		int	height;		/* height of rows		*/
		int	fullwidth;	/* full width of window		*/
		int	fullheight;	/* full height of window	*/
		double	tekscale;	/* scale factor Tek -> vs100	*/
	} fullTwin;
#ifndef NO_ACTIVE_ICON
	struct _tekwin iconTwin;
	struct _tekwin *whichTwin;
#endif /* NO_ACTIVE_ICON */
	int		xorplane;	/* z plane for inverts		*/
	GC		linepat[TEKNUMLINES]; /* line patterns		*/
	Boolean		TekEmu;		/* true if Tektronix emulation	*/
	int		cur_X;		/* current x			*/
	int		cur_Y;		/* current y			*/
	Tmodes		cur;		/* current tek modes		*/
	Tmodes		page;		/* starting tek modes on page	*/
	int		margin;		/* 0 -> margin 1, 1 -> margin 2	*/
	int		pen;		/* current Tektronix pen 0=up, 1=dn */
	char		*TekGIN;	/* nonzero if Tektronix GIN mode*/
	int		gin_terminator; /* Tek strap option */

	int		multiClickTime;	 /* time between multiclick selects */
	int		bellSuppressTime; /* msecs after Bell before another allowed */
	Boolean		bellInProgress; /* still ringing/flashing prev bell? */
	char		*charClass;	/* for overriding word selection */
	Boolean		cutNewline;	/* whether or not line cut has \n */
	Boolean		cutToBeginningOfLine;  /* line cuts to BOL? */
	Boolean		highlight_selection; /* controls appearance of selection */
	char		*selection;	/* the current selection */
	int		selection_size; /* size of allocated buffer */
	int		selection_length; /* number of significant bytes */
	Time		selection_time;	/* latest event timestamp */
	int		startHRow, startHCol, /* highlighted text */
			endHRow, endHCol,
			startHCoord, endHCoord;
	Atom*		selection_atoms; /* which selections we own */
	Cardinal	sel_atoms_size;	/*  how many atoms allocated */
	Cardinal	selection_count; /* how many atoms in use */
	Boolean		input_eight_bits;/* use 8th bit instead of ESC prefix */
	Boolean		output_eight_bits; /* honor all bits or strip */
	Boolean		control_eight_bits; /* send CSI as 8-bits */
	Pixmap		menu_item_bitmap;	/* mask for checking items */
	Widget		mainMenu, vtMenu, tekMenu, fontMenu;
	char*		menu_font_names[NMENUFONTS];
	int		menu_font_number;
	XIC		xic;
} TScreen;

typedef struct _TekPart {
    XFontStruct *Tfont[TEKNUMFONTS];
    int		tobaseline[TEKNUMFONTS]; /* top to baseline for each font */
    char	*initial_font;		/* large, 2, 3, small */
    char	*gin_terminator_str;	/* ginTerminator resource */
} TekPart;



/* meaning of bits in screen.select flag */
#define	INWINDOW	01	/* the mouse is in one of the windows */
#define	FOCUS		02	/* one of the windows is the focus window */

#define MULTICLICKTIME 250	/* milliseconds */

typedef struct
{
	unsigned	flags;
} TKeyboard;

typedef struct _Misc {
    char *geo_metry;
    char *T_geometry;
    char *f_n;
    char *f_b;
#ifdef ALLOWLOGGING
    Boolean log_on;
#endif
    Boolean login_shell;
    Boolean re_verse;
    int resizeGravity;
    Boolean reverseWrap;
    Boolean autoWrap;
    Boolean logInhibit;
    Boolean signalInhibit;
    Boolean tekInhibit;
    Boolean scrollbar;
    Boolean titeInhibit;
    Boolean tekSmall;	/* start tek window in small size */
    Boolean appcursorDefault;
    Boolean appkeypadDefault;
#if XtSpecificationRelease >= 6
    char* input_method;
    char* preedit_type;
    Boolean open_im;
#endif
    Boolean dynamicColors;
    Boolean shared_ic;
#ifndef NO_ACTIVE_ICON
    Boolean active_icon;	/* use application icon window  */
    int icon_border_width;
    Pixel icon_border_pixel;
#endif /* NO_ACTIVE_ICON */
} Misc;

typedef struct {int foo;} XtermClassPart, TekClassPart;

typedef struct _XtermClassRec {
    CoreClassPart  core_class;
    XtermClassPart xterm_class;
} XtermClassRec;

typedef struct _TekClassRec {
    CoreClassPart core_class;
    TekClassPart tek_class;
} TekClassRec;

/* define masks for keyboard.flags */
#define MODE_DECKPAM	0x02	/* keypad application mode */
#define MODE_DECCKM	0x04	/* cursor keys */


#define N_MARGINBELL	10
#define MAX_TABS	320
#define TAB_ARRAY_SIZE	10	/* number of ints to provide MAX_TABS bits */

typedef unsigned Tabs [TAB_ARRAY_SIZE];

typedef struct _XtermWidgetRec {
    CorePart	core;
    TKeyboard	keyboard;	/* terminal keyboard		*/
    TScreen	screen;		/* terminal screen		*/
    unsigned	flags;		/* mode flags			*/
    unsigned    cur_foreground;	/* current foreground color	*/
    unsigned    cur_background;	/* current background color	*/
#if OPT_ISO_COLORS
    unsigned    sgr_foreground;	/* current SGR foreground color	*/
    int         num_ptrs;	/* number of pointers per row in 'ScrnBuf' */
#endif
    unsigned	initflags;	/* initial mode flags		*/
    Tabs	tabs;		/* tabstops of the terminal	*/
    Misc	misc;		/* miscellaneous parameters	*/
} XtermWidgetRec, *XtermWidget;

typedef struct _TekWidgetRec {
    CorePart core;
    TekPart tek;
} TekWidgetRec, *TekWidget;

#define BUF_SIZE 4096

/*
 * terminal flags
 * There are actually two namespaces mixed together here.
 * One is the set of flags that can go in screen->buf attributes
 * and which must fit in a char.
 * The other is the global setting stored in
 * term->flags and screen->save_modes.  This need only fit in an unsigned.
 */

/* global flags and character flags (visible character attributes) */
#define INVERSE		0x01	/* invert the characters to be output */
#define UNDERLINE	0x02	/* true if underlining */
#define BOLD		0x04
/* global flags (also character attributes) */
#define BG_COLOR	0x08  /* true if background set */
#define FG_COLOR	0x10  /* true if foreground set */

/* character flags (internal attributes) */
#define PROTECTED	0x20	/* a character is drawn that cannot be erased */
#define LINEWRAPPED	0x40	/* used on the first character in a
				 * line to indicate that it wraps onto
				 * the next line so we can tell the
				 * difference between lines that have
				 * wrapped around and lines that have
				 * ended naturally with a CR at column
				 * max_col.
				 */
#define CHARDRAWN	0x80    /* a character has been drawn here on the
				   screen.  Used to distinguish blanks from
				   empty parts of the screen when selecting */

			/* mask: user-visible attributes */
#define	ATTRIBUTES	(INVERSE|UNDERLINE|BOLD|BG_COLOR|FG_COLOR|INVISIBLE|PROTECTED)

#define WRAPAROUND	0x400	/* true if auto wraparound mode */
#define	REVERSEWRAP	0x800	/* true if reverse wraparound mode */
#define REVERSE_VIDEO	0x1000	/* true if screen white on black */
#define LINEFEED	0x2000	/* true if in auto linefeed mode */
#define ORIGIN		0x4000	/* true if in origin mode */
#define INSERT		0x8000	/* true if in insert mode */
#define SMOOTHSCROLL	0x10000	/* true if in smooth scroll mode */
#define IN132COLUMNS	0x20000	/* true if in 132 column mode */
#define INVISIBLE	0x40000	/* true if writing invisible text */

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
#define CursorX(screen,col) ((col) * FontWidth(screen) + screen->border \
			+ Scrollbar(screen))
#define CursorY(screen,row) ((((row) - screen->topline) * FontHeight(screen)) \
			+ screen->border)

#ifndef NO_ACTIVE_ICON
#define IsIcon(screen)		((screen)->whichVwin == &(screen)->iconVwin)
#define VWindow(screen)		((screen)->whichVwin->window)
#define VShellWindow		term->core.parent->core.window
#define TextWindow(screen)      ((screen)->whichVwin->window)
#define TWindow(screen)		((screen)->whichTwin->window)
#define TShellWindow		tekWidget->core.parent->core.window
#define Width(screen)		((screen)->whichVwin->width)
#define Height(screen)		((screen)->whichVwin->height)
#define FullWidth(screen)	((screen)->whichVwin->fullwidth)
#define FullHeight(screen)	((screen)->whichVwin->fullheight)
#define FontWidth(screen)	((screen)->whichVwin->f_width)
#define FontHeight(screen)	((screen)->whichVwin->f_height)
#define FontAscent(screen)	(IsIcon(screen) ? (screen)->fnt_icon->ascent \
						: (screen)->fnt_norm->ascent)
#define Scrollbar(screen)	((screen)->whichVwin->scrollbar)
#define NormalGC(screen)	((screen)->whichVwin->normalGC)
#define ReverseGC(screen)	((screen)->whichVwin->reverseGC)
#define NormalBoldGC(screen)	((screen)->whichVwin->normalboldGC)
#define ReverseBoldGC(screen)	((screen)->whichVwin->reverseboldGC)
#define TWidth(screen)		((screen)->whichTwin->width)
#define THeight(screen)		((screen)->whichTwin->height)
#define TFullWidth(screen)	((screen)->whichTwin->fullwidth)
#define TFullHeight(screen)	((screen)->whichTwin->fullheight)
#define TekScale(screen)	((screen)->whichTwin->tekscale)

#else /* NO_ACTIVE_ICON */

#define IsIcon(screen)		(False)
#define VWindow(screen)		((screen)->fullVwin.window)
#define VShellWindow		term->core.parent->core.window
#define TextWindow(screen)      ((screen)->fullVwin.window)
#define TWindow(screen)		((screen)->fullTwin.window)
#define TShellWindow		tekWidget->core.parent->core.window
#define Width(screen)		((screen)->fullVwin.width)
#define Height(screen)		((screen)->fullVwin.height)
#define FullWidth(screen)	((screen)->fullVwin.fullwidth)
#define FullHeight(screen)	((screen)->fullVwin.fullheight)
#define FontWidth(screen)	((screen)->fullVwin.f_width)
#define FontHeight(screen)	((screen)->fullVwin.f_height)
#define FontAscent(screen)	((screen)->fnt_norm->ascent)
#define Scrollbar(screen)	((screen)->fullVwin.scrollbar)
#define NormalGC(screen)	((screen)->fullVwin.normalGC)
#define ReverseGC(screen)	((screen)->fullVwin.reverseGC)
#define NormalBoldGC(screen)	((screen)->fullVwin.normalboldGC)
#define ReverseBoldGC(screen)	((screen)->fullVwin.reverseboldGC)
#define TWidth(screen)		((screen)->fullTwin.width)
#define THeight(screen)		((screen)->fullTwin.height)
#define TFullWidth(screen)	((screen)->fullTwin.fullwidth)
#define TFullHeight(screen)	((screen)->fullTwin.fullheight)
#define TekScale(screen)	((screen)->fullTwin.tekscale)

#endif /* NO_ACTIVE_ICON */

#define	TWINDOWEVENTS	(KeyPressMask | ExposureMask | ButtonPressMask |\
			 ButtonReleaseMask | StructureNotifyMask |\
			 EnterWindowMask | LeaveWindowMask | FocusChangeMask)

#define	WINDOWEVENTS	(TWINDOWEVENTS | PointerMotionMask)


#define TEK_LINK_BLOCK_SIZE 1024

typedef struct Tek_Link
{
	struct Tek_Link	*next;	/* pointer to next TekLink in list
				   NULL <=> this is last TekLink */
	short fontsize;		/* character size, 0-3 */
	short count;		/* number of chars in data */
	char *ptr;		/* current pointer into data */
	char data [TEK_LINK_BLOCK_SIZE];
} TekLink;

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
