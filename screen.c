/*
 *	$XConsortium: screen.c /main/35 1996/12/01 23:47:05 swick $
 *	$XFree86: xc/programs/xterm/screen.c,v 3.12.2.1 1997/05/23 09:24:42 dawes Exp $
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

/* screen.c */

#ifdef HAVE_CONFIG_H
#include <xtermcfg.h>
#endif

#include "ptyx.h"
#include "error.h"
#include "data.h"

#include "xterm.h"

#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#else
extern Char *calloc(), *malloc(), *realloc();
extern void free();
#endif

#include <stdio.h>
#include <signal.h>
#ifdef SVR4
#include <termios.h>
#else
#include <sys/ioctl.h>
#endif

#ifdef att
#include <sys/termio.h>
#include <sys/stream.h>			/* get typedef used in ptem.h */
#include <sys/ptem.h>
#endif

#ifdef MINIX
#include <termios.h>
#endif

#ifdef ISC
#include <sys/termio.h>
#define TIOCGPGRP TCGETPGRP
#define TIOCSPGRP TCSETPGRP
#endif

#ifdef __EMX__
extern int ptioctl(int fd, int func, void* data);
#define ioctl ptioctl
#define TIOCSWINSZ	113
#define TIOCGWINSZ	117
struct winsize {
        unsigned short  ws_row;         /* rows, in characters */
        unsigned short  ws_col;         /* columns, in characters */
        unsigned short  ws_xpixel;      /* horizontal size, pixels */
        unsigned short  ws_ypixel;      /* vertical size, pixels */
};
#endif

static int Reallocate PROTO((ScrnBuf *sbuf, Char **sbufaddr, int nrow, int ncol, int oldrow, int oldcol));
static void ScrnClearLines PROTO((char **save, ScrnBuf sb, int where, int n, int size));


ScrnBuf Allocate (nrow, ncol, addr)
/*
   allocates memory for a 2-dimensional array of chars and returns a pointer
   thereto
   each line is formed from a pair of char arrays.  The first (even) one is
   the actual character array and the second (odd) one is the attributes.
   each line is formed from four char arrays.  The first one is the actual
   character array, the second one is the attributes, the third is the
   foreground color, and the fourth is the background color.
 */
register int nrow, ncol;
Char **addr;
{
	register ScrnBuf base;
	register Char *tmp;
	register int i;

	if ((base = (ScrnBuf) calloc ((unsigned)(nrow *= MAX_PTRS), sizeof (char *))) == 0)
		SysError (ERROR_SCALLOC);

	if ((tmp = (Char *)calloc ((unsigned) (nrow * ncol), sizeof(Char))) == 0)
		SysError (ERROR_SCALLOC2);

	*addr = tmp;
	for (i = 0; i < nrow; i++, tmp += ncol)
		base[i] = tmp;

	return (base);
}

/*
 *  This is called when the screen is resized.
 *  Returns the number of lines the text was moved down (neg for up).
 *  (Return value only necessary with SouthWestGravity.)
 */
static int
Reallocate(sbuf, sbufaddr, nrow, ncol, oldrow, oldcol)
    ScrnBuf *sbuf;
    Char **sbufaddr;
    int nrow, ncol, oldrow, oldcol;
{
	register ScrnBuf base;
	register Char *tmp;
	register int i, minrows;
	register Size_t mincols;
	Char *oldbuf;
	int move_down = 0, move_up = 0;

	if (sbuf == NULL || *sbuf == NULL)
		return 0;

	oldrow *= MAX_PTRS;
	oldbuf = *sbufaddr;

	/*
	 * Special case if oldcol == ncol - straight forward realloc and
	 * update of the additional lines in sbuf
	 */

	/* this is a good idea, but doesn't seem to be implemented.  -gildea */

	/* 
	 * realloc sbuf, the pointers to all the lines.
	 * If the screen shrinks, remove lines off the top of the buffer
	 * if resizeGravity resource says to do so.
	 */
	nrow *= MAX_PTRS;
	if (nrow < oldrow  &&  term->misc.resizeGravity == SouthWestGravity) {
	    /* Remove lines off the top of the buffer if necessary. */
	    move_up = oldrow-nrow 
		        - MAX_PTRS*(term->screen.max_row - term->screen.cur_row);
	    if (move_up < 0)
		move_up = 0;
	    /* Overlapping memmove here! */
	    memmove( *sbuf, *sbuf+move_up, 
		  (oldrow-move_up)*sizeof((*sbuf)[0]) );
	}
	*sbuf = (ScrnBuf) realloc((char *) (*sbuf),
				  (unsigned) (nrow * sizeof(char *)));
	if (*sbuf == 0)
	    SysError(ERROR_RESIZE);
	base = *sbuf;

	/* 
	 *  create the new buffer space and copy old buffer contents there
	 *  line by line.
	 */
	if ((tmp = (Char *)calloc((unsigned) (nrow * ncol), sizeof(Char))) == 0)
		SysError(ERROR_SREALLOC);
	*sbufaddr = tmp;
	minrows = (oldrow < nrow) ? oldrow : nrow;
	mincols = (oldcol < ncol) ? oldcol : ncol;
	if (nrow > oldrow  &&  term->misc.resizeGravity == SouthWestGravity) {
	    /* move data down to bottom of expanded screen */
	    move_down = Min(nrow-oldrow, MAX_PTRS*term->screen.savedlines);
	    tmp += ncol*move_down;
	}
	for (i = 0; i < minrows; i++, tmp += ncol) {
		memcpy( tmp, base[i], mincols);
	}
	/*
	 * update the pointers in sbuf
	 */
	for (i = 0, tmp = *sbufaddr; i < nrow; i++, tmp += ncol)
	    base[i] = tmp;

        /* Now free the old buffer */
	free(oldbuf);

	return move_down ? move_down/MAX_PTRS : -move_up/MAX_PTRS; /* convert to rows */
}

void
ScreenWrite (screen, str, flags, cur_fg, cur_bg, length)
/*
   Writes str into buf at row row and column col.  Characters are set to match
   flags.
 */
TScreen *screen;
char *str;
register unsigned flags;
register unsigned cur_fg, cur_bg;
register int length;		/* length of string */
{
#if OPT_ISO_COLORS
	register Char *fb = 0;
#endif
	register Char *attrs;
	register int avail  = screen->max_col - screen->cur_col + 1;
	register Char *col;
	register int wrappedbit;

	if (length > avail)
	    length = avail;
	if (length <= 0)
		return;

	col   = SCRN_BUF_CHARS(screen, screen->cur_row) + screen->cur_col;
	attrs = SCRN_BUF_ATTRS(screen, screen->cur_row) + screen->cur_col;

	if_OPT_ISO_COLORS(screen,{
		fb = SCRN_BUF_COLOR(screen, screen->cur_row) + screen->cur_col;
	})

	wrappedbit = *attrs & LINEWRAPPED;

	/* write blanks if we're writing invisible text */
	if (flags & INVISIBLE) {
		bzero(col,   length);
	} else {
		memcpy( col,   str,    length);
	}

	flags &= ATTRIBUTES;
	flags |= CHARDRAWN;
	memset( attrs, flags,  length);

	if_OPT_ISO_COLORS(screen,{
		memset( fb,   makeColorPair(cur_fg, cur_bg), length);
	})

	if (wrappedbit)
	    *attrs |= LINEWRAPPED;
}

static void
ScrnClearLines (save, sb, where, n, size)
/*
 * Saves pointers to the n lines beginning at sb + where, and clears the lines
 */
char **save;
ScrnBuf sb;
int where, n, size;
{
	register int i;

	/* save n lines at where */
	memcpy ( (char *)save,
		 (char *) &sb[MAX_PTRS * where],
		 MAX_PTRS * sizeof(char *) * n);

	/* clear contents of old rows */
	if (TERM_COLOR_FLAGS) {
		int last = (n * MAX_PTRS);
		int flags = TERM_COLOR_FLAGS;
		for (i = 0; i < last; i += MAX_PTRS) {
			bzero(save[i+0], size);
			memset(save[i+1], flags, size);
			memset(save[i+2], xtermColorPair(), size);
		}
	} else {
		for (i = MAX_PTRS * n - 1 ; i >= 0 ; i--)
			bzero (save[i], size);
	}
}

void
ScrnInsertLine (sb, last, where, n, size)
/*
   Inserts n blank lines at sb + where, treating last as a bottom margin.
   Size is the size of each entry in sb.
   Requires: 0 <= where < where + n <= last
   	     n <= MAX_ROWS
 */
register ScrnBuf sb;
int last;
register int where, n, size;
{
	char *save [4 /* MAX_PTRS */ * MAX_ROWS];

	/* save n lines at bottom */
	ScrnClearLines(save, sb, (last -= n - 1), n, size);

	/*
	 * WARNING, overlapping copy operation.  Move down lines (pointers).
	 *
	 *   +----|---------|--------+
	 *
	 * is copied in the array to:
	 *
	 *   +--------|---------|----+
	 */
	memmove( (char *) &sb [MAX_PTRS * (where + n)],
		 (char *) &sb [MAX_PTRS * where], 
		MAX_PTRS * sizeof (char *) * (last - where));

	/* reuse storage for new lines at where */
	memcpy ( (char *) &sb[MAX_PTRS * where],
		 (char *)save,
		 MAX_PTRS * sizeof(char *) * n);
}

void
ScrnDeleteLine (sb, last, where, n, size)
/*
   Deletes n lines at sb + where, treating last as a bottom margin.
   Size is the size of each entry in sb.
   Requires 0 <= where < where + n < = last
   	    n <= MAX_ROWS
 */
register ScrnBuf sb;
register int n, last, size;
int where;
{
	char *save [4 /* MAX_PTRS */ * MAX_ROWS];

	ScrnClearLines(save, sb, where, n, size);

	/* move up lines */
	memmove( (char *) &sb[MAX_PTRS * where],
		 (char *) &sb[MAX_PTRS * (where + n)], 
		MAX_PTRS * sizeof (char *) * ((last -= n - 1) - where));

	/* reuse storage for new bottom lines */
	memcpy ( (char *) &sb[MAX_PTRS * last],
		 (char *)save, 
		MAX_PTRS * sizeof(char *) * n);
}

void
ScrnInsertChar (screen, n, size)
    /*
     * Inserts n blanks in screen at current row, col.  Size is the size of each
     * row.
     */
    register TScreen *screen;
    register int n;
    int size;
{
	ScrnBuf sb = screen->buf;
	int row = screen->cur_row;
	int col = screen->cur_col;
	register int i, j;
	register Char *ptr = BUF_CHARS(sb, row);
	register Char *attrs = BUF_ATTRS(sb, row);
	int wrappedbit = attrs[0]&LINEWRAPPED;
	int flags = CHARDRAWN | TERM_COLOR_FLAGS;

	attrs[0] &= ~LINEWRAPPED; /* make sure the bit isn't moved */
	for (i = size - 1; i >= col + n; i--) {
		ptr[i] = ptr[j = i - n];
		attrs[i] = attrs[j];
	}

	for (i=col; i<col+n; i++)
	    ptr[i] = ' ';
	for (i=col; i<col+n; i++)
	    attrs[i] = flags;
	if_OPT_ISO_COLORS(screen,{
	    memset(BUF_COLOR(sb, row) + col, xtermColorPair(), n);
	})

	if (wrappedbit)
	    attrs[0] |= LINEWRAPPED;
}

void
ScrnDeleteChar (screen, n, size)
    /*
      Deletes n characters at current row, col. Size is the size of each row.
      */
    register TScreen *screen;
    register int size;
    register int n;
{
	ScrnBuf sb = screen->buf;
	int row = screen->cur_row;
	int col = screen->cur_col;
	register Char *ptr = BUF_CHARS(sb, row);
	register Char *attrs = BUF_ATTRS(sb, row);
	register Size_t nbytes = (size - n - col);
	int wrappedbit = attrs[0]&LINEWRAPPED;

	memcpy (ptr   + col, ptr   + col + n, nbytes);
	memcpy (attrs + col, attrs + col + n, nbytes);
	bzero  (ptr + size - n, n);
	memset (attrs + size - n, TERM_COLOR_FLAGS, n);

	if_OPT_ISO_COLORS(screen,{
	    memset(BUF_COLOR(sb, row) + size - n, xtermColorPair(), n);
	})
	if (wrappedbit)
	    attrs[0] |= LINEWRAPPED;
}

void
ScrnRefresh (screen, toprow, leftcol, nrows, ncols, force)
/*
   Repaints the area enclosed by the parameters.
   Requires: (toprow, leftcol), (toprow + nrows, leftcol + ncols) are
   	     coordinates of characters in screen;
	     nrows and ncols positive.
 */
register TScreen *screen;
int toprow, leftcol, nrows, ncols;
Boolean force;			/* ... leading/trailing spaces */
{
	int y = toprow * FontHeight(screen) + screen->border;
	register int row;
	register int topline = screen->topline;
	int maxrow = toprow + nrows - 1;
	int scrollamt = screen->scroll_amt;
	int max = screen->max_row;
	int gc_changes = 0;

	if(screen->cursor_col >= leftcol && screen->cursor_col <=
	 (leftcol + ncols - 1) && screen->cursor_row >= toprow + topline &&
	 screen->cursor_row <= maxrow + topline)
		screen->cursor_state = OFF;
	for (row = toprow; row <= maxrow; y += FontHeight(screen), row++) {
#if OPT_ISO_COLORS
	   register Char *fb = 0;
#endif
	   register Char *chars;
	   register Char *attrs;
	   register int col = leftcol;
	   int maxcol = leftcol + ncols - 1;
	   int hi_col = maxcol;
	   int lastind;
	   int flags;
	   int fg = 0, bg = 0;
	   int x;
	   GC gc;
	   Boolean hilite;	

	   if (row < screen->top_marg || row > screen->bot_marg)
		lastind = row;
	   else
		lastind = row - scrollamt;

	   if (lastind < 0 || lastind > max)
	   	continue;

	   chars = SCRN_BUF_CHARS(screen, lastind + topline);
	   attrs = SCRN_BUF_ATTRS(screen, lastind + topline);

	   if_OPT_ISO_COLORS(screen,{
		   fb = SCRN_BUF_COLOR(screen, lastind + topline);
	   })

	   if (row < screen->startHRow || row > screen->endHRow ||
	       (row == screen->startHRow && maxcol < screen->startHCol) ||
	       (row == screen->endHRow && col >= screen->endHCol))
	   {
	       /* row does not intersect selection; don't hilite */
	       if (!force) {
		   while (col <= maxcol && (attrs[col] & ~BOLD) == 0 &&
			  (chars[col] & ~040) == 0)
		       col++;

		   while (col <= maxcol && (attrs[maxcol] & ~BOLD) == 0 &&
			  (chars[maxcol] & ~040) == 0)
		       maxcol--;
	       }
	       hilite = False;
	   }
	   else {
	       /* row intersects selection; split into pieces of single type */
	       if (row == screen->startHRow && col < screen->startHCol) {
		   ScrnRefresh(screen, row, col, 1, screen->startHCol - col,
			       force);
		   col = screen->startHCol;
	       }
	       if (row == screen->endHRow && maxcol >= screen->endHCol) {
		   ScrnRefresh(screen, row, screen->endHCol, 1,
			       maxcol - screen->endHCol + 1, force);
		   maxcol = screen->endHCol - 1;
	       }

	       /*
		* If we're highlighting because the user is doing cut/paste,
		* trim the trailing blanks from the highlighted region so we're
		* showing the actual extent of the text that'll be cut.  If
		* we're selecting a blank line, we'll highlight one column
		* anyway.
		*
		* We don't do this if the mouse-hilite mode is set because that
		* would be too confusing.
		*
		* The default if the highlightSelection resource isn't set will
		* highlight the whole width of the terminal, which is easy to
		* see, but harder to use (because trailing blanks aren't as
		* apparent).
	        */
	       if (screen->highlight_selection
/*	        && maxcol >= screen->max_col */
		&& screen->send_mouse_pos != 3) {
		   hi_col = screen->max_col;
	           while (hi_col > 0 && !(attrs[hi_col] & CHARDRAWN))
                       hi_col--;
	       }

	       /* remaining piece should be hilited */
	       hilite = True;
	   }

	   if (col > maxcol) continue;

	   flags = attrs[col];
	   if_OPT_ISO_COLORS(screen,{
		fg = extract_fg(fb[col], flags);
		bg = extract_bg(fb[col]);
	   })
	   gc = updatedXtermGC(screen, flags, fg, bg, hilite);
	   gc_changes |= (flags & (FG_COLOR|BG_COLOR));

	   x = CursorX(screen, col);
	   lastind = col;

	   for (; col <= maxcol; col++) {
		if ((attrs[col] != flags)
		 || (hilite && (col > hi_col))
#if OPT_ISO_COLORS
		 || ((flags & FG_COLOR) && (extract_fg(fb[col],attrs[col]) != fg))
		 || ((flags & BG_COLOR) && (extract_bg(fb[col]) != bg))
#endif
		 ) {
		   drawXtermText(screen, flags, gc, x, y,
			(char *) &chars[lastind], col - lastind);

		   x += (col - lastind) * FontWidth(screen);

		   lastind = col;

		   if (hilite && (col > hi_col))
			hilite = False;

		   flags = attrs[col];
		   if_OPT_ISO_COLORS(screen,{
		        fg = extract_fg(fb[col], flags);
		        bg = extract_bg(fb[col]);
		   })
	   	   gc = updatedXtermGC(screen, flags, fg, bg, hilite);
	   	   gc_changes |= (flags & (FG_COLOR|BG_COLOR));
		}

		if(chars[col] == 0)
			chars[col] = ' ';
	   }

	   drawXtermText(screen, flags, gc, x, y,
		(char *) &chars[lastind], col - lastind);
	}

	/*
	 * If we're in color mode, reset the various GC's to the current
	 * screen foreground and background so that other functions (e.g.,
	 * ClearRight) will get the correct colors.
	 */
	if_OPT_ISO_COLORS(screen,{
	    if (gc_changes & FG_COLOR)
		SGR_Foreground(term->cur_foreground);
	    if (gc_changes & BG_COLOR)
		SGR_Background(term->cur_background);
	})
}

void
ClearBufRows (screen, first, last)
/*
   Sets the rows first though last of the buffer of screen to spaces.
   Requires first <= last; first, last are rows of screen->buf.
 */
register TScreen *screen;
register int first, last;
{
	ScrnBuf	buf = screen->buf;
	int	len = screen->max_col + 1;
	register int row;
	register int flags = TERM_COLOR_FLAGS;

	for (row = first; row <= last; row++) {
	    bzero (BUF_CHARS(buf, row), len);
	    memset(BUF_ATTRS(buf, row), flags, len);
	    if_OPT_ISO_COLORS(screen,{
		memset(BUF_COLOR(buf, row), xtermColorPair(), len);
	    })
	}
}

/*
  Resizes screen:
  1. If new window would have fractional characters, sets window size so as to
  discard fractional characters and returns -1.
  Minimum screen size is 1 X 1.
  Note that this causes another ExposeWindow event.
  2. Enlarges screen->buf if necessary.  New space is appended to the bottom
  and to the right
  3. Reduces  screen->buf if necessary.  Old space is removed from the bottom
  and from the right
  4. Cursor is positioned as closely to its former position as possible
  5. Sets screen->max_row and screen->max_col to reflect new size
  6. Maintains the inner border (and clears the border on the screen).
  7. Clears origin mode and sets scrolling region to be entire screen.
  8. Returns 0
  */
int
ScreenResize (screen, width, height, flags)
    register TScreen *screen;
    int width, height;
    unsigned *flags;
{
	int rows, cols;
	int border = 2 * screen->border;
	int move_down_by;
#if defined(sun) && !defined(SVR4)
#ifdef TIOCSSIZE
	struct ttysize ts;
#endif	/* TIOCSSIZE */
#else	/* sun */
#ifdef TIOCSWINSZ
	struct winsize ws;
#endif	/* TIOCSWINSZ */
#endif	/* sun */
	Window tw = TextWindow (screen);

	/* clear the right and bottom internal border because of NorthWest
	   gravity might have left junk on the right and bottom edges */
	if (width >= FullWidth(screen)) {
		XClearArea (screen->display, tw,
			    FullWidth(screen), 0,             /* right edge */
			    0, height,                /* from top to bottom */
			    False);
	}
	if (height >= FullHeight(screen)) {
		XClearArea (screen->display, tw, 
		    	0, FullHeight(screen),	                  /* bottom */
		    	width, 0,                  /* all across the bottom */
		    	False);
	}

	/* round so that it is unlikely the screen will change size on  */
	/* small mouse movements.					*/
	rows = (height + FontHeight(screen) / 2 - border) /
	 FontHeight(screen);
	cols = (width + FontWidth(screen) / 2 - border - Scrollbar(screen)) /
	 FontWidth(screen);
	if (rows < 1) rows = 1;
	if (cols < 1) cols = 1;

	/* update buffers if the screen has changed size */
	if (screen->max_row != rows - 1 || screen->max_col != cols - 1) {
		register int savelines = screen->scrollWidget ?
		 screen->savelines : 0;
		int delta_rows = rows - (screen->max_row + 1);
		
		if(screen->cursor_state)
			HideCursor();
		if ( screen->alternate
		     && term->misc.resizeGravity == SouthWestGravity )
		    /* swap buffer pointers back to make all this hair work */
		    SwitchBufPtrs(screen);
		if (screen->altbuf) 
		    (void) Reallocate(&screen->altbuf, &screen->abuf_address,
			 rows, cols, screen->max_row + 1, screen->max_col + 1);
		move_down_by = Reallocate(&screen->allbuf,
					  &screen->sbuf_address,
					  rows + savelines, cols,
					  screen->max_row + 1 + savelines,
					  screen->max_col + 1);
		screen->buf = &screen->allbuf[MAX_PTRS * savelines];

		screen->max_row += delta_rows;
		screen->max_col = cols - 1;

		if (term->misc.resizeGravity == SouthWestGravity) {
		    screen->savedlines -= move_down_by;
		    if (screen->savedlines < 0)
			screen->savedlines = 0;
		    if (screen->savedlines > screen->savelines)
			screen->savedlines = screen->savelines;
		    if (screen->topline < -screen->savedlines)
			screen->topline = -screen->savedlines;
		    screen->cur_row += move_down_by;
		    screen->cursor_row += move_down_by;
		    ScrollSelection(screen, move_down_by);

		    if (screen->alternate)
			SwitchBufPtrs(screen); /* put the pointers back */
		}
	
		/* adjust scrolling region */
		screen->top_marg = 0;
		screen->bot_marg = screen->max_row;
		*flags &= ~ORIGIN;

		if (screen->cur_row > screen->max_row)
			screen->cur_row = screen->max_row;
		if (screen->cur_col > screen->max_col)
			screen->cur_col = screen->max_col;
	
		screen->fullVwin.height = height - border;
		screen->fullVwin.width = width - border - screen->fullVwin.scrollbar;

	} else if(FullHeight(screen) == height && FullWidth(screen) == width)
	 	return(0);	/* nothing has changed at all */

	if(screen->scrollWidget)
		ResizeScrollBar(screen->scrollWidget, -1, -1, height);
	
	screen->fullVwin.fullheight = height;
	screen->fullVwin.fullwidth = width;
	ResizeSelection (screen, rows, cols);

#ifndef NO_ACTIVE_ICON
	if (screen->iconVwin.window) {
	    XWindowChanges changes;
	    screen->iconVwin.width =
		(screen->max_col + 1) * screen->iconVwin.f_width;

	    screen->iconVwin.height =
		(screen->max_row + 1) * screen->iconVwin.f_height;

	    changes.width = screen->iconVwin.fullwidth =
		screen->iconVwin.width + 2 * screen->border;
		
	    changes.height = screen->iconVwin.fullheight =
		screen->iconVwin.height + 2 * screen->border;

	    XConfigureWindow(XtDisplay(term), screen->iconVwin.window,
			     CWWidth|CWHeight,&changes);
	}
#endif /* NO_ACTIVE_ICON */

#if defined(sun) && !defined(SVR4)
#ifdef TIOCSSIZE
	/* Set tty's idea of window size */
	ts.ts_lines = rows;
	ts.ts_cols = cols;
	ioctl (screen->respond, TIOCSSIZE, &ts);
#ifdef SIGWINCH
	if(screen->pid > 1) {
		int	pgrp;
		
		if (ioctl (screen->respond, TIOCGPGRP, &pgrp) != -1)
			kill_process_group(pgrp, SIGWINCH);
	}
#endif	/* SIGWINCH */
#endif	/* TIOCSSIZE */
#else	/* sun */
#ifdef TIOCSWINSZ
	/* Set tty's idea of window size */
	ws.ws_row = rows;
	ws.ws_col = cols;
	ws.ws_xpixel = width;
	ws.ws_ypixel = height;
	ioctl (screen->respond, TIOCSWINSZ, (char *)&ws);
#ifdef notdef	/* change to SIGWINCH if this doesn't work for you */
	if(screen->pid > 1) {
		int	pgrp;
		
		if (ioctl (screen->respond, TIOCGPGRP, &pgrp) != -1)
		    kill_process_group(pgrp, SIGWINCH);
	}
#endif	/* SIGWINCH */
#endif	/* TIOCSWINSZ */
#endif	/* sun */
	return (0);
}

/*
 * Sets the attributes from the row, col, to row, col + length according to
 * mask and value. The bits in the attribute byte specified by the mask are
 * set to the corresponding bits in the value byte. If length would carry us
 * over the end of the line, it stops at the end of the line.
 */
void
ScrnSetAttributes(screen, row, col, mask, value, length)
TScreen *screen;
int row, col;
unsigned mask, value;
register int length;		/* length of string */
{
	register Char *attrs;
	register int avail  = screen->max_col - col + 1;

	if (length > avail)
	    length = avail;
	if (length <= 0)
		return;
	attrs = SCRN_BUF_ATTRS(screen, row) + col;
	value &= mask;	/* make sure we only change the bits allowed by mask*/
	while(length-- > 0) {
		*attrs &= ~mask;	/* clear the bits */
		*attrs |= value;	/* copy in the new values */
		attrs++;
	}
}

/*
 * Gets the attributes from the row, col, to row, col + length into the
 * supplied array, which is assumed to be big enough.  If length would carry us
 * over the end of the line, it stops at the end of the line. Returns
 * the number of bytes of attributes (<= length)
 */
int
ScrnGetAttributes(screen, row, col, str, length)
TScreen *screen;
int row, col;
Char *str;
register int length;		/* length of string */
{
	register Char *attrs;
	register int avail  = screen->max_col - col + 1;
	int ret;

	if (length > avail)
	    length = avail;
	if (length <= 0)
		return 0;
	ret = length;
	attrs = SCRN_BUF_ATTRS(screen, row) + col;
	while(length-- > 0) {
		*str++ = *attrs++;
	}
	return ret;
}

Bool
non_blank_line(sb, row, col, len)
ScrnBuf sb;
register int row, col, len;
{
	register int	i;
	register Char *ptr = BUF_CHARS(sb, row);

	for (i = col; i < len; i++) {
		if (ptr[i])
			return True;
	}
	return False;
}
