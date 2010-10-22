/* $XTermId: button.c,v 1.390 2010/10/22 00:53:57 tom Exp $ */

/*
 * Copyright 1999-2009,2010 by Thomas E. Dickey
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

/*
button.c	Handles button events in the terminal emulator.
		does cut/paste operations, change modes via menu,
		passes button events through to some applications.
				J. Gettys.
*/

#include <xterm.h>

#include <stdio.h>
#include <assert.h>

#include <X11/Xatom.h>
#include <X11/Xmu/Atoms.h>
#include <X11/Xmu/StdSel.h>

#include <xutf8.h>
#include <fontutils.h>

#include <data.h>
#include <error.h>
#include <menu.h>
#include <xcharmouse.h>
#include <charclass.h>
#include <xstrings.h>

#if OPT_SELECT_REGEX
#ifdef HAVE_PCREPOSIX_H
#include <pcreposix.h>
#else /* POSIX regex.h */
#include <sys/types.h>
#include <regex.h>
#endif
#endif

#if OPT_WIDE_CHARS
#include <ctype.h>
#include <wcwidth.h>
#else
#define CharacterClass(value) \
	charClass[value & ((sizeof(charClass)/sizeof(charClass[0]))-1)]
#endif

    /*
     * We'll generally map rows to indices when doing selection.
     * Simplify that with a macro.
     *
     * Note that ROW2INX() is safe to use with auto increment/decrement for
     * the row expression since that is evaluated once.
     */
#define GET_LINEDATA(screen, row) \
	getLineData(screen, ROW2INX(screen, row))

    /*
     * We reserve shift modifier for cut/paste operations.  In principle we
     * can pass through control and meta modifiers, but in practice, the
     * popup menu uses control, and the window manager is likely to use meta,
     * so those events are not delivered to SendMousePosition.
     */
#define OurModifiers (ShiftMask | ControlMask | Mod1Mask)
#define AllModifiers (ShiftMask | LockMask | ControlMask | Mod1Mask | \
		      Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask)

#define BtnModifiers(event) (event->state & OurModifiers)
#define KeyModifiers(event) (event->xbutton.state & OurModifiers)

#define IsBtnEvent(event) ((event)->type == ButtonPress || (event)->type == ButtonRelease)

#define KeyState(x) (((int) ((x) & (ShiftMask|ControlMask))) \
			  + (((x) & Mod1Mask) ? 2 : 0))
    /* adds together the bits:
       shift key -> 1
       meta key  -> 2
       control key -> 4 */

#define	Coordinate(s,c)	((c)->row * MaxCols(s) + (c)->col)

static const CELL zeroCELL =
{0, 0};

#if OPT_DEC_LOCATOR
static Bool SendLocatorPosition(XtermWidget xw, XButtonEvent * event);
static void CheckLocatorPosition(XtermWidget xw, XButtonEvent * event);
#endif /* OPT_DEC_LOCATOR */

/* Multi-click handling */
#if OPT_READLINE
static Time lastButtonDownTime = 0;
static int ExtendingSelection = 0;
static Time lastButton3UpTime = 0;
static Time lastButton3DoubleDownTime = 0;
static CELL lastButton3;	/* At the release time */
#endif /* OPT_READLINE */

static Char *SaveText(TScreen * screen, int row, int scol, int ecol,
		      Char * lp, int *eol);
static int Length(TScreen * screen, int row, int scol, int ecol);
static void ComputeSelect(XtermWidget xw, CELL * startc, CELL * endc, Bool extend);
static void EditorButton(XtermWidget xw, XButtonEvent * event);
static void EndExtend(XtermWidget w, XEvent * event, String * params, Cardinal
		      num_params, Bool use_cursor_loc);
static void ExtendExtend(XtermWidget xw, const CELL * cell);
static void PointToCELL(TScreen * screen, int y, int x, CELL * cell);
static void ReHiliteText(XtermWidget xw, CELL * first, CELL * last);
static void SaltTextAway(XtermWidget xw, CELL * cellc, CELL * cell,
			 String * params, Cardinal num_params);
static void SelectSet(XtermWidget xw, XEvent * event, String * params, Cardinal num_params);
static void SelectionReceived PROTO_XT_SEL_CB_ARGS;
static void StartSelect(XtermWidget xw, const CELL * cell);
static void TrackDown(XtermWidget xw, XButtonEvent * event);
static void TrackText(XtermWidget xw, const CELL * first, const CELL * last);
static void _OwnSelection(XtermWidget xw, String * selections, Cardinal count);
static void do_select_end(XtermWidget xw, XEvent * event, String * params,
			  Cardinal *num_params, Bool use_cursor_loc);

#define MOUSE_LIMIT (255 - 32)

/* Send SET_EXT_SIZE_MOUSE to enable offsets up to EXT_MOUSE_LIMIT */
#define EXT_MOUSE_LIMIT (2047 - 32)
#define EXT_MOUSE_START (127 - 32)

static unsigned
EmitMousePosition(TScreen * screen, Char line[], unsigned count, int value)
{
    int mouse_limit = (screen->ext_mode_mouse
		       ? EXT_MOUSE_LIMIT
		       : MOUSE_LIMIT);

    /*
     * Add pointer position to key sequence
     *
     * In extended mode we encode large positions as two-byte UTF-8.
     *
     * NOTE: historically, it was possible to emit 256, which became
     * zero by truncation to 8 bits. While this was arguably a bug,
     * it's also somewhat useful as a past-end marker. We preserve
     * this behavior for both normal and extended mouse modes.
     */
    if (value == mouse_limit) {
	line[count++] = CharOf(0);
    } else if (!screen->ext_mode_mouse || value < EXT_MOUSE_START) {
	line[count++] = CharOf(' ' + value + 1);
    } else {
	value += ' ' + 1;
	line[count++] = CharOf(0xC0 + (value >> 6));
	line[count++] = CharOf(0x80 + (value & 0x3F));
    }
    return count;
}

Bool
SendMousePosition(XtermWidget xw, XEvent * event)
{
    TScreen *screen = TScreenOf(xw);
    XButtonEvent *my_event = (XButtonEvent *) event;
    Bool result = False;

    switch (screen->send_mouse_pos) {
    case MOUSE_OFF:
	/* If send_mouse_pos mode isn't on, we shouldn't be here */
	break;

    case BTN_EVENT_MOUSE:
    case ANY_EVENT_MOUSE:
	if (KeyModifiers(event) == 0 || KeyModifiers(event) == ControlMask) {
	    /* xterm extension for motion reporting. June 1998 */
	    /* EditorButton() will distinguish between the modes */
	    switch (event->type) {
	    case MotionNotify:
		my_event->button = 0;
		/* FALLTHRU */
	    case ButtonPress:
		/* FALLTHRU */
	    case ButtonRelease:
		EditorButton(xw, my_event);
		result = True;
		break;
	    }
	}
	break;

    default:
	/* Make sure the event is an appropriate type */
	if (IsBtnEvent(event)) {
	    switch (screen->send_mouse_pos) {
	    case X10_MOUSE:	/* X10 compatibility sequences */

		if (BtnModifiers(my_event) == 0) {
		    if (my_event->type == ButtonPress)
			EditorButton(xw, my_event);
		    result = True;
		}
		break;

	    case VT200_HIGHLIGHT_MOUSE:	/* DEC vt200 hilite tracking */
		if (my_event->type == ButtonPress &&
		    BtnModifiers(my_event) == 0 &&
		    my_event->button == Button1) {
		    TrackDown(xw, my_event);
		    result = True;
		} else if (BtnModifiers(my_event) == 0
			   || BtnModifiers(my_event) == ControlMask) {
		    EditorButton(xw, my_event);
		    result = True;
		}
		break;

	    case VT200_MOUSE:	/* DEC vt200 compatible */
		if (BtnModifiers(my_event) == 0
		    || BtnModifiers(my_event) == ControlMask) {
		    EditorButton(xw, my_event);
		    result = True;
		}
		break;

#if OPT_DEC_LOCATOR
	    case DEC_LOCATOR:
		result = SendLocatorPosition(xw, my_event);
		break;
#endif /* OPT_DEC_LOCATOR */
	    }
	}
    }
    return result;
}

#if OPT_DEC_LOCATOR

#define	LocatorCoords( row, col, x, y, oor )			\
    if( screen->locator_pixels ) {				\
	(oor)=False; (row) = (y)+1; (col) = (x)+1;		\
	/* Limit to screen dimensions */			\
	if ((row) < 1) (row) = 1,(oor)=True;			\
	else if ((row) > screen->border*2+Height(screen))	\
	    (row) = screen->border*2+Height(screen),(oor)=True;	\
	if ((col) < 1) (col) = 1,(oor)=True;			\
	else if ((col) > OriginX(screen)*2+Width(screen))	\
	    (col) = OriginX(screen)*2+Width(screen),(oor)=True;	\
    } else {							\
	(oor)=False;						\
	/* Compute character position of mouse pointer */	\
	(row) = ((y) - screen->border) / FontHeight(screen);	\
	(col) = ((x) - OriginX(screen)) / FontWidth(screen);	\
	/* Limit to screen dimensions */			\
	if ((row) < 0) (row) = 0,(oor)=True;			\
	else if ((row) > screen->max_row)			\
	    (row) = screen->max_row,(oor)=True;			\
	if ((col) < 0) (col) = 0,(oor)=True;			\
	else if ((col) > screen->max_col)			\
	    (col) = screen->max_col,(oor)=True;			\
	(row)++; (col)++;					\
    }

static Bool
SendLocatorPosition(XtermWidget xw, XButtonEvent * event)
{
    ANSI reply;
    TScreen *screen = TScreenOf(xw);
    int row, col;
    Bool oor;
    int button;
    unsigned state;

    /* Make sure the event is an appropriate type */
    if ((!IsBtnEvent(event) &&
	 !screen->loc_filter) ||
	(BtnModifiers(event) != 0 && BtnModifiers(event) != ControlMask))
	return (False);

    if ((event->type == ButtonPress &&
	 !(screen->locator_events & LOC_BTNS_DN)) ||
	(event->type == ButtonRelease &&
	 !(screen->locator_events & LOC_BTNS_UP)))
	return (True);

    if (event->type == MotionNotify) {
	CheckLocatorPosition(xw, event);
	return (True);
    }

    /* get button # */
    button = (int) event->button - 1;

    LocatorCoords(row, col, event->x, event->y, oor);

    /*
     * DECterm mouse:
     *
     * ESCAPE '[' event ; mask ; row ; column '&' 'w'
     */
    memset(&reply, 0, sizeof(reply));
    reply.a_type = ANSI_CSI;

    if (oor) {
	reply.a_nparam = 1;
	reply.a_param[0] = 0;	/* Event - 0 = locator unavailable */
	reply.a_inters = '&';
	reply.a_final = 'w';
	unparseseq(xw, &reply);

	if (screen->locator_reset) {
	    MotionOff(screen, xw);
	    screen->send_mouse_pos = MOUSE_OFF;
	}
	return (True);
    }

    /*
     * event:
     *        1       no buttons
     *        2       left button down
     *        3       left button up
     *        4       middle button down
     *        5       middle button up
     *        6       right button down
     *        7       right button up
     *        8       M4 down
     *        9       M4 up
     */
    reply.a_nparam = 4;
    switch (event->type) {
    case ButtonPress:
	reply.a_param[0] = (ParmType) (2 + (button << 1));
	break;
    case ButtonRelease:
	reply.a_param[0] = (ParmType) (3 + (button << 1));
	break;
    default:
	return (True);
    }
    /*
     * mask:
     * bit 7   bit 6   bit 5   bit 4   bit 3   bit 2       bit 1         bit 0
     *                                 M4 down left down   middle down   right down
     *
     * Notice that Button1 (left) and Button3 (right) are swapped in the mask.
     * Also, mask should be the state after the button press/release,
     * X provides the state not including the button press/release.
     */
    state = (event->state
	     & (Button1Mask | Button2Mask | Button3Mask | Button4Mask)) >> 8;
    /* update mask to "after" state */
    state ^= ((unsigned) (1 << button));
    /* swap Button1 & Button3 */
    state = ((state & (unsigned) ~(4 | 1))
	     | ((state & 1) ? 4 : 0)
	     | ((state & 4) ? 1 : 0));

    reply.a_param[1] = (ParmType) state;
    reply.a_param[2] = (ParmType) row;
    reply.a_param[3] = (ParmType) col;
    reply.a_inters = '&';
    reply.a_final = 'w';

    unparseseq(xw, &reply);

    if (screen->locator_reset) {
	MotionOff(screen, xw);
	screen->send_mouse_pos = MOUSE_OFF;
    }

    /*
     * DECterm turns the Locator off if a button is pressed while a filter rectangle
     * is active. This might be a bug, but I don't know, so I'll emulate it anyways.
     */
    if (screen->loc_filter) {
	screen->send_mouse_pos = MOUSE_OFF;
	screen->loc_filter = False;
	screen->locator_events = 0;
	MotionOff(screen, xw);
    }

    return (True);
}

/*
 * mask:
 * bit 7   bit 6   bit 5   bit 4   bit 3   bit 2       bit 1         bit 0
 *                                 M4 down left down   middle down   right down
 *
 * Button1 (left) and Button3 (right) are swapped in the mask relative to X.
 */
#define	ButtonState(state, mask)	\
{ (state) = (int) (((mask) & (Button1Mask | Button2Mask | Button3Mask | Button4Mask)) >> 8);	\
  /* swap Button1 & Button3 */								\
  (state) = ((state) & ~(4|1)) | (((state)&1)?4:0) | (((state)&4)?1:0);			\
}

void
GetLocatorPosition(XtermWidget xw)
{
    ANSI reply;
    TScreen *screen = TScreenOf(xw);
    Window root, child;
    int rx, ry, x, y;
    unsigned int mask;
    int row = 0, col = 0;
    Bool oor = False;
    Bool ret = False;
    int state;

    /*
     * DECterm turns the Locator off if the position is requested while a filter rectangle
     * is active.  This might be a bug, but I don't know, so I'll emulate it anyways.
     */
    if (screen->loc_filter) {
	screen->send_mouse_pos = MOUSE_OFF;
	screen->loc_filter = False;
	screen->locator_events = 0;
	MotionOff(screen, xw);
    }

    memset(&reply, 0, sizeof(reply));
    reply.a_type = ANSI_CSI;

    if (screen->send_mouse_pos == DEC_LOCATOR) {
	ret = XQueryPointer(screen->display, VWindow(screen), &root,
			    &child, &rx, &ry, &x, &y, &mask);
	if (ret) {
	    LocatorCoords(row, col, x, y, oor);
	}
    }
    if (ret == False || oor) {
	reply.a_nparam = 1;
	reply.a_param[0] = 0;	/* Event - 0 = locator unavailable */
	reply.a_inters = '&';
	reply.a_final = 'w';
	unparseseq(xw, &reply);

	if (screen->locator_reset) {
	    MotionOff(screen, xw);
	    screen->send_mouse_pos = MOUSE_OFF;
	}
	return;
    }

    ButtonState(state, mask);

    reply.a_nparam = 4;
    reply.a_param[0] = 1;	/* Event - 1 = response to locator request */
    reply.a_param[1] = (ParmType) state;
    reply.a_param[2] = (ParmType) row;
    reply.a_param[3] = (ParmType) col;
    reply.a_inters = '&';
    reply.a_final = 'w';
    unparseseq(xw, &reply);

    if (screen->locator_reset) {
	MotionOff(screen, xw);
	screen->send_mouse_pos = MOUSE_OFF;
    }
}

void
InitLocatorFilter(XtermWidget xw)
{
    ANSI reply;
    TScreen *screen = TScreenOf(xw);
    Window root, child;
    int rx, ry, x, y;
    unsigned int mask;
    int row = 0, col = 0;
    Bool oor = 0;
    Bool ret;
    int state;

    ret = XQueryPointer(screen->display, VWindow(screen),
			&root, &child, &rx, &ry, &x, &y, &mask);
    if (ret) {
	LocatorCoords(row, col, x, y, oor);
    }
    if (ret == False || oor) {
	/* Locator is unavailable */

	if (screen->loc_filter_top != LOC_FILTER_POS ||
	    screen->loc_filter_left != LOC_FILTER_POS ||
	    screen->loc_filter_bottom != LOC_FILTER_POS ||
	    screen->loc_filter_right != LOC_FILTER_POS) {
	    /*
	     * If any explicit coordinates were received,
	     * report immediately with no coordinates.
	     */
	    memset(&reply, 0, sizeof(reply));
	    reply.a_type = ANSI_CSI;
	    reply.a_nparam = 1;
	    reply.a_param[0] = 0;	/* Event - 0 = locator unavailable */
	    reply.a_inters = '&';
	    reply.a_final = 'w';
	    unparseseq(xw, &reply);

	    if (screen->locator_reset) {
		MotionOff(screen, xw);
		screen->send_mouse_pos = MOUSE_OFF;
	    }
	} else {
	    /*
	     * No explicit coordinates were received, and the pointer is
	     * unavailable.  Report when the pointer re-enters the window.
	     */
	    screen->loc_filter = True;
	    MotionOn(screen, xw);
	}
	return;
    }

    /*
     * Adjust rectangle coordinates:
     *  1. Replace "LOC_FILTER_POS" with current coordinates
     *  2. Limit coordinates to screen size
     *  3. make sure top and left are less than bottom and right, resp.
     */
    if (screen->locator_pixels) {
	rx = OriginX(screen) * 2 + Width(screen);
	ry = screen->border * 2 + Height(screen);
    } else {
	rx = screen->max_col;
	ry = screen->max_row;
    }

#define	Adjust( coord, def, max )				\
	if( (coord) == LOC_FILTER_POS )	(coord) = (def);	\
	else if ((coord) < 1)		(coord) = 1;		\
	else if ((coord) > (max))	(coord) = (max)

    Adjust(screen->loc_filter_top, row, ry);
    Adjust(screen->loc_filter_left, col, rx);
    Adjust(screen->loc_filter_bottom, row, ry);
    Adjust(screen->loc_filter_right, col, rx);

    if (screen->loc_filter_top > screen->loc_filter_bottom) {
	ry = screen->loc_filter_top;
	screen->loc_filter_top = screen->loc_filter_bottom;
	screen->loc_filter_bottom = ry;
    }

    if (screen->loc_filter_left > screen->loc_filter_right) {
	rx = screen->loc_filter_left;
	screen->loc_filter_left = screen->loc_filter_right;
	screen->loc_filter_right = rx;
    }

    if ((col < screen->loc_filter_left) ||
	(col > screen->loc_filter_right) ||
	(row < screen->loc_filter_top) ||
	(row > screen->loc_filter_bottom)) {
	/* Pointer is already outside the rectangle - report immediately */
	ButtonState(state, mask);

	memset(&reply, 0, sizeof(reply));
	reply.a_type = ANSI_CSI;
	reply.a_nparam = 4;
	reply.a_param[0] = 10;	/* Event - 10 = locator outside filter */
	reply.a_param[1] = (ParmType) state;
	reply.a_param[2] = (ParmType) row;
	reply.a_param[3] = (ParmType) col;
	reply.a_inters = '&';
	reply.a_final = 'w';
	unparseseq(xw, &reply);

	if (screen->locator_reset) {
	    MotionOff(screen, xw);
	    screen->send_mouse_pos = MOUSE_OFF;
	}
	return;
    }

    /*
     * Rectangle is set up.  Allow pointer tracking
     * to detect if the mouse leaves the rectangle.
     */
    screen->loc_filter = True;
    MotionOn(screen, xw);
}

static void
CheckLocatorPosition(XtermWidget xw, XButtonEvent * event)
{
    ANSI reply;
    TScreen *screen = TScreenOf(xw);
    int row, col;
    Bool oor;
    int state;

    LocatorCoords(row, col, event->x, event->y, oor);

    /*
     * Send report if the pointer left the filter rectangle, if
     * the pointer left the window, or if the filter rectangle
     * had no coordinates and the pointer re-entered the window.
     */
    if (oor || (screen->loc_filter_top == LOC_FILTER_POS) ||
	(col < screen->loc_filter_left) ||
	(col > screen->loc_filter_right) ||
	(row < screen->loc_filter_top) ||
	(row > screen->loc_filter_bottom)) {
	/* Filter triggered - disable it */
	screen->loc_filter = False;
	MotionOff(screen, xw);

	memset(&reply, 0, sizeof(reply));
	reply.a_type = ANSI_CSI;
	if (oor) {
	    reply.a_nparam = 1;
	    reply.a_param[0] = 0;	/* Event - 0 = locator unavailable */
	} else {
	    ButtonState(state, event->state);

	    reply.a_nparam = 4;
	    reply.a_param[0] = 10;	/* Event - 10 = locator outside filter */
	    reply.a_param[1] = (ParmType) state;
	    reply.a_param[2] = (ParmType) row;
	    reply.a_param[3] = (ParmType) col;
	}

	reply.a_inters = '&';
	reply.a_final = 'w';
	unparseseq(xw, &reply);

	if (screen->locator_reset) {
	    MotionOff(screen, xw);
	    screen->send_mouse_pos = MOUSE_OFF;
	}
    }
}
#endif /* OPT_DEC_LOCATOR */

#if OPT_READLINE
static int
isClick1_clean(TScreen * screen, XButtonEvent * event)
{
    int delta;

    if (!IsBtnEvent(event)
    /* Disable on Shift-Click-1, including the application-mouse modes */
	|| (BtnModifiers(event) & ShiftMask)
	|| (screen->send_mouse_pos != MOUSE_OFF)	/* Kinda duplicate... */
	||ExtendingSelection)	/* Was moved */
	return 0;

    if (event->type != ButtonRelease)
	return 0;

    if (lastButtonDownTime == (Time) 0) {
	/* first time or once in a blue moon */
	delta = screen->multiClickTime + 1;
    } else if (event->time > lastButtonDownTime) {
	/* most of the time */
	delta = (int) (event->time - lastButtonDownTime);
    } else {
	/* time has rolled over since lastButtonUpTime */
	delta = (int) ((((Time) ~ 0) - lastButtonDownTime) + event->time);
    }

    return delta <= screen->multiClickTime;
}

static int
isDoubleClick3(TScreen * screen, XButtonEvent * event)
{
    int delta;

    if (event->type != ButtonRelease
	|| (BtnModifiers(event) & ShiftMask)
	|| event->button != Button3) {
	lastButton3UpTime = 0;	/* Disable the cached info */
	return 0;
    }
    /* Process Btn3Release. */
    if (lastButton3DoubleDownTime == (Time) 0) {
	/* No previous click or once in a blue moon */
	delta = screen->multiClickTime + 1;
    } else if (event->time > lastButton3DoubleDownTime) {
	/* most of the time */
	delta = (int) (event->time - lastButton3DoubleDownTime);
    } else {
	/* time has rolled over since lastButton3DoubleDownTime */
	delta = (int) ((((Time) ~ 0) - lastButton3DoubleDownTime) + event->time);
    }
    if (delta <= screen->multiClickTime) {
	/* Double click */
	CELL cell;

	/* Cannot check ExtendingSelection, since mouse-3 always sets it */
	PointToCELL(screen, event->y, event->x, &cell);
	if (isSameCELL(&cell, &lastButton3)) {
	    lastButton3DoubleDownTime = 0;	/* Disable the third click */
	    return 1;
	}
    }
    /* Not a double click, memorize for future check. */
    lastButton3UpTime = event->time;
    PointToCELL(screen, event->y, event->x, &lastButton3);
    return 0;
}

static int
CheckSecondPress3(TScreen * screen, XEvent * event)
{
    int delta;

    if (event->type != ButtonPress
	|| (KeyModifiers(event) & ShiftMask)
	|| event->xbutton.button != Button3) {
	lastButton3DoubleDownTime = 0;	/* Disable the cached info */
	return 0;
    }
    /* Process Btn3Press. */
    if (lastButton3UpTime == (Time) 0) {
	/* No previous click or once in a blue moon */
	delta = screen->multiClickTime + 1;
    } else if (event->xbutton.time > lastButton3UpTime) {
	/* most of the time */
	delta = (int) (event->xbutton.time - lastButton3UpTime);
    } else {
	/* time has rolled over since lastButton3UpTime */
	delta = (int) ((((Time) ~ 0) - lastButton3UpTime) + event->xbutton.time);
    }
    if (delta <= screen->multiClickTime) {
	CELL cell;

	PointToCELL(screen, event->xbutton.y, event->xbutton.x, &cell);
	if (isSameCELL(&cell, &lastButton3)) {
	    /* A candidate for a double-click */
	    lastButton3DoubleDownTime = event->xbutton.time;
	    PointToCELL(screen, event->xbutton.y, event->xbutton.x, &lastButton3);
	    return 1;
	}
	lastButton3UpTime = 0;	/* Disable the info about the previous click */
    }
    /* Either too long, or moved, disable. */
    lastButton3DoubleDownTime = 0;
    return 0;
}

static int
rowOnCurrentLine(TScreen * screen,
		 int line,
		 int *deltap)	/* must be XButtonEvent */
{
    int result = 1;
    int l1, l2;

    *deltap = 0;
    if (line != screen->cur_row) {
	if (line < screen->cur_row)
	    l1 = line, l2 = screen->cur_row;
	else
	    l2 = line, l1 = screen->cur_row;
	l1--;
	while (++l1 < l2) {
	    LineData *ld = GET_LINEDATA(screen, l1);
	    if (!LineTstWrapped(ld)) {
		result = 0;
		break;
	    }
	}
	if (result) {
	    /* Everything is on one "wrapped line" now */
	    *deltap = line - screen->cur_row;
	}
    }
    return result;
}

static int
eventRow(TScreen * screen, XEvent * event)	/* must be XButtonEvent */
{
    return (event->xbutton.y - screen->border) / FontHeight(screen);
}

static int
eventColBetween(TScreen * screen, XEvent * event)	/* must be XButtonEvent */
{
    /* Correct by half a width - we are acting on a boundary, not on a cell. */
    return ((event->xbutton.x - OriginX(screen) + (FontWidth(screen) - 1) / 2)
	    / FontWidth(screen));
}

static int
ReadLineMovePoint(TScreen * screen, int col, int ldelta)
{
    Char line[6];
    unsigned count = 0;

    col += ldelta * MaxCols(screen) - screen->cur_col;
    if (col == 0)
	return 0;
    if (screen->control_eight_bits) {
	line[count++] = ANSI_CSI;
    } else {
	line[count++] = ANSI_ESC;
	line[count++] = '[';	/* XXX maybe sometimes O is better? */
    }
    line[count] = CharOf(col > 0 ? 'C' : 'D');
    if (col < 0)
	col = -col;
    while (col--)
	v_write(screen->respond, line, 3);
    return 1;
}

static int
ReadLineDelete(TScreen * screen, CELL * cell1, CELL * cell2)
{
    int del;

    del = (cell2->col - cell1->col) + ((cell2->row - cell1->row) * MaxCols(screen));
    if (del <= 0)		/* Just in case... */
	return 0;
    while (del--)
	v_write(screen->respond, (const Char *) "\177", 1);
    return 1;
}

static void
readlineExtend(TScreen * screen, XEvent * event)
{
    int ldelta1, ldelta2;

    if (IsBtnEvent(event)) {
	XButtonEvent *my_event = (XButtonEvent *) event;
	if (isClick1_clean(screen, my_event)
	    && SCREEN_FLAG(screen, click1_moves)
	    && rowOnCurrentLine(screen, eventRow(screen, event), &ldelta1)) {
	    ReadLineMovePoint(screen, eventColBetween(screen, event), ldelta1);
	}
	if (isDoubleClick3(screen, my_event)
	    && SCREEN_FLAG(screen, dclick3_deletes)
	    && rowOnCurrentLine(screen, screen->startSel.row, &ldelta1)
	    && rowOnCurrentLine(screen, screen->endSel.row, &ldelta2)) {
	    ReadLineMovePoint(screen, screen->endSel.col, ldelta2);
	    ReadLineDelete(screen, &screen->startSel, &(screen->endSel));
	}
    }
}

#endif /* OPT_READLINE */

/* ^XM-G<line+' '><col+' '> */
void
DiredButton(Widget w,
	    XEvent * event,	/* must be XButtonEvent */
	    String * params GCC_UNUSED,		/* selections */
	    Cardinal *num_params GCC_UNUSED)
{
    XtermWidget xw;

    if ((xw = getXtermWidget(w)) != 0) {
	TScreen *screen = TScreenOf(xw);
	Char Line[6];
	unsigned line, col;

	if (IsBtnEvent(event)
	    && (event->xbutton.y >= screen->border)
	    && (event->xbutton.x >= OriginX(screen))) {
	    line = (unsigned) ((event->xbutton.y - screen->border)
			       / FontHeight(screen));
	    col = (unsigned) ((event->xbutton.x - OriginX(screen))
			      / FontWidth(screen));
	    Line[0] = CONTROL('X');
	    Line[1] = ANSI_ESC;
	    Line[2] = 'G';
	    Line[3] = CharOf(' ' + col);
	    Line[4] = CharOf(' ' + line);
	    v_write(screen->respond, Line, 5);
	}
    }
}

#if OPT_READLINE
void
ReadLineButton(Widget w,
	       XEvent * event,	/* must be XButtonEvent */
	       String * params GCC_UNUSED,	/* selections */
	       Cardinal *num_params GCC_UNUSED)
{
    XtermWidget xw;

    if ((xw = getXtermWidget(w)) != 0) {
	TScreen *screen = TScreenOf(xw);
	Char Line[6];
	int line, col, ldelta = 0;

	if (!IsBtnEvent(event)
	    || (screen->send_mouse_pos != MOUSE_OFF) || ExtendingSelection)
	    goto finish;
	if (event->type == ButtonRelease) {
	    int delta;

	    if (lastButtonDownTime == (Time) 0) {
		/* first time and once in a blue moon */
		delta = screen->multiClickTime + 1;
	    } else if (event->xbutton.time > lastButtonDownTime) {
		/* most of the time */
		delta = (int) (event->xbutton.time - lastButtonDownTime);
	    } else {
		/* time has rolled over since lastButtonUpTime */
		delta = (int) ((((Time) ~ 0) - lastButtonDownTime) + event->xbutton.time);
	    }
	    if (delta > screen->multiClickTime)
		goto finish;	/* All this work for this... */
	}
	line = (event->xbutton.y - screen->border) / FontHeight(screen);
	if (!rowOnCurrentLine(screen, line, &ldelta))
	    goto finish;
	/* Correct by half a width - we are acting on a boundary, not on a cell. */
	col = (event->xbutton.x - OriginX(screen) + (FontWidth(screen) - 1)
	       / 2)
	    / FontWidth(screen) - screen->cur_col + ldelta * MaxCols(screen);
	if (col == 0)
	    goto finish;
	Line[0] = ANSI_ESC;
	/* XXX: sometimes it is better to send '['? */
	Line[1] = 'O';
	Line[2] = CharOf(col > 0 ? 'C' : 'D');
	if (col < 0)
	    col = -col;
	while (col--)
	    v_write(screen->respond, Line, 3);
      finish:
	if (event->type == ButtonRelease)
	    do_select_end(xw, event, params, num_params, False);
    }
}
#endif /* OPT_READLINE */

/* repeats <ESC>n or <ESC>p */
void
ViButton(Widget w,
	 XEvent * event,	/* must be XButtonEvent */
	 String * params GCC_UNUSED,	/* selections */
	 Cardinal *num_params GCC_UNUSED)
{
    XtermWidget xw;

    if ((xw = getXtermWidget(w)) != 0) {
	TScreen *screen = TScreenOf(xw);
	int pty = screen->respond;
	Char Line[6];
	int line;

	if (IsBtnEvent(event)) {

	    line = screen->cur_row -
		((event->xbutton.y - screen->border) / FontHeight(screen));
	    if (line != 0) {
		Line[0] = ANSI_ESC;	/* force an exit from insert-mode */
		v_write(pty, Line, 1);

		if (line < 0) {
		    line = -line;
		    Line[0] = CONTROL('n');
		} else {
		    Line[0] = CONTROL('p');
		}
		while (--line >= 0)
		    v_write(pty, Line, 1);
	    }
	}
    }
}

/*
 * This function handles button-motion events
 */
/*ARGSUSED*/
void
HandleSelectExtend(Widget w,
		   XEvent * event,	/* must be XMotionEvent */
		   String * params GCC_UNUSED,
		   Cardinal *num_params GCC_UNUSED)
{
    XtermWidget xw;

    if ((xw = getXtermWidget(w)) != 0) {
	TScreen *screen = TScreenOf(xw);
	CELL cell;

	screen->selection_time = event->xmotion.time;
	switch (screen->eventMode) {
	    /* If not in one of the DEC mouse-reporting modes */
	case LEFTEXTENSION:
	case RIGHTEXTENSION:
	    PointToCELL(screen, event->xmotion.y, event->xmotion.x, &cell);
	    ExtendExtend(xw, &cell);
	    break;

	    /* If in motion reporting mode, send mouse position to
	       character process as a key sequence \E[M... */
	case NORMAL:
	    /* will get here if send_mouse_pos != MOUSE_OFF */
	    if (screen->send_mouse_pos == BTN_EVENT_MOUSE
		|| screen->send_mouse_pos == ANY_EVENT_MOUSE) {
		(void) SendMousePosition(xw, event);
	    }
	    break;
	}
    }
}

void
HandleKeyboardSelectExtend(Widget w,
			   XEvent * event GCC_UNUSED,	/* must be XButtonEvent */
			   String * params GCC_UNUSED,
			   Cardinal *num_params GCC_UNUSED)
{
    XtermWidget xw;

    if ((xw = getXtermWidget(w)) != 0) {
	TScreen *screen = TScreenOf(xw);
	ExtendExtend(xw, &screen->cursorp);
    }
}

static void
do_select_end(XtermWidget xw,
	      XEvent * event,	/* must be XButtonEvent */
	      String * params,	/* selections */
	      Cardinal *num_params,
	      Bool use_cursor_loc)
{
    TScreen *screen = TScreenOf(xw);

    screen->selection_time = event->xbutton.time;
    switch (screen->eventMode) {
    case NORMAL:
	(void) SendMousePosition(xw, event);
	break;
    case LEFTEXTENSION:
    case RIGHTEXTENSION:
	EndExtend(xw, event, params, *num_params, use_cursor_loc);
#if OPT_READLINE
	readlineExtend(screen, event);
#endif /* OPT_READLINE */
	break;
    }
}

void
HandleSelectEnd(Widget w,
		XEvent * event,	/* must be XButtonEvent */
		String * params,	/* selections */
		Cardinal *num_params)
{
    XtermWidget xw;

    if ((xw = getXtermWidget(w)) != 0) {
	do_select_end(xw, event, params, num_params, False);
    }
}

void
HandleKeyboardSelectEnd(Widget w,
			XEvent * event,		/* must be XButtonEvent */
			String * params,	/* selections */
			Cardinal *num_params)
{
    XtermWidget xw;

    if ((xw = getXtermWidget(w)) != 0) {
	do_select_end(xw, event, params, num_params, True);
    }
}

/*
 * Like "select-end" (optionally copies the selection to the given targets),
 * but also sets the modes so that releasing the mouse button or moving the
 * mouse does not alter the selection.
 */
static void
do_select_stop(XtermWidget xw,
	       XEvent * event,	/* must be XButtonEvent */
	       String * params,	/* selections */
	       Cardinal *num_params,
	       Bool use_cursor_loc)
{
    TScreen *screen = TScreenOf(xw);

    screen->selection_time = event->xbutton.time;
    switch (screen->eventMode) {
    case NORMAL:
	(void) SendMousePosition(xw, event);
	break;
    case LEFTEXTENSION:
    case RIGHTEXTENSION:
	EndExtend(xw, event, params, *num_params, use_cursor_loc);
	break;
    }
}

void
HandleSelectStop(Widget w,
		 XEvent * event,	/* must be XButtonEvent */
		 String * params,	/* selections */
		 Cardinal *num_params)
{
    XtermWidget xw;

    if ((xw = getXtermWidget(w)) != 0) {
	do_select_stop(xw, event, params, num_params, False);
    }
}

void
HandleKeyboardSelectStop(Widget w,
			 XEvent * event,	/* must be XButtonEvent */
			 String * params,	/* selections */
			 Cardinal *num_params)
{
    XtermWidget xw;

    if ((xw = getXtermWidget(w)) != 0) {
	do_select_stop(xw, event, params, num_params, True);
    }
}

struct _SelectionList {
    String *params;
    Cardinal count;
    Atom *targets;
    Time time;
};

static unsigned
DECtoASCII(unsigned ch)
{
    if (xtermIsDecGraphic(ch)) {
	ch = CharOf("###########+++++##-##++++|######"[ch]);
	/*           01234567890123456789012345678901 */
    }
    return ch;
}

#if OPT_WIDE_CHARS
static Cardinal
addXtermChar(Char ** buffer, Cardinal *used, Cardinal offset, unsigned value)
{
    if (offset + 1 >= *used) {
	*used = 1 + (2 * (offset + 1));
	allocXtermChars(buffer, *used);
    }
    (*buffer)[offset++] = (Char) value;
    return offset;
}
#define AddChar(buffer, used, offset, value) \
	offset = addXtermChar(buffer, used, offset, (unsigned) value)

/*
 * Convert a UTF-8 string to Latin-1, replacing non Latin-1 characters by `#',
 * or ASCII/Latin-1 equivalents for special cases.
 */
static Char *
UTF8toLatin1(TScreen * screen, Char * s, unsigned long len, unsigned long *result)
{
    static Char *buffer;
    static Cardinal used;

    Cardinal offset = 0;

    const Char *p;

    if (len != 0) {
	PtyData data;

	fakePtyData(&data, s, s + len);
	while (decodeUtf8(&data)) {
	    Bool fails = False;
	    Bool extra = False;
	    IChar value = skipPtyData(&data);
	    if (value == UCS_REPL) {
		fails = True;
	    } else if (value < 256) {
		AddChar(&buffer, &used, offset, CharOf(value));
	    } else {
		unsigned eqv = ucs2dec(value);
		if (xtermIsDecGraphic(eqv)) {
		    AddChar(&buffer, &used, offset, DECtoASCII(eqv));
		} else {
		    eqv = AsciiEquivs(value);
		    if (eqv == value) {
			fails = True;
		    } else {
			AddChar(&buffer, &used, offset, eqv);
		    }
		    if (isWide((wchar_t) value))
			extra = True;
		}
	    }

	    /*
	     * If we're not able to plug in a single-byte result, insert the
	     * defaultString (which normally is a single "#", but could be
	     * whatever the user wants).
	     */
	    if (fails) {
		for (p = (const Char *) screen->default_string; *p != '\0'; ++p) {
		    AddChar(&buffer, &used, offset, *p);
		}
	    }
	    if (extra)
		AddChar(&buffer, &used, offset, ' ');
	}
	AddChar(&buffer, &used, offset, '\0');
	*result = (unsigned long) (offset - 1);
    } else {
	*result = 0;
    }
    return buffer;
}

int
xtermUtf8ToTextList(XtermWidget xw,
		    XTextProperty * text_prop,
		    char ***text_list,
		    int *text_list_count)
{
    TScreen *screen = TScreenOf(xw);
    Display *dpy = screen->display;
    int rc = -1;

    if (text_prop->format == 8
	&& (rc = Xutf8TextPropertyToTextList(dpy, text_prop,
					     text_list,
					     text_list_count)) >= 0) {
	if (*text_list != NULL && *text_list_count != 0) {
	    int i;
	    Char *data;
	    char **new_text_list, *tmp;
	    unsigned long size, new_size;

	    TRACE(("xtermUtf8ToTextList size %d\n", *text_list_count));

	    /*
	     * XLib StringList actually uses only two pointers, one for the
	     * list itself, and one for the data.  Pointer to the data is the
	     * first element of the list, the rest (if any) list elements point
	     * to the same memory block as the first element
	     */
	    new_size = 0;
	    for (i = 0; i < *text_list_count; ++i) {
		data = (Char *) (*text_list)[i];
		size = strlen((*text_list)[i]) + 1;
		(void) UTF8toLatin1(screen, data, size, &size);
		new_size += size + 1;
	    }
	    new_text_list =
		(char **) XtMalloc((Cardinal) sizeof(char *) * (unsigned) *text_list_count);
	    new_text_list[0] = tmp = XtMalloc((Cardinal) new_size);
	    for (i = 0; i < (*text_list_count); ++i) {
		data = (Char *) (*text_list)[i];
		size = strlen((*text_list)[i]) + 1;
		data = UTF8toLatin1(screen, data, size, &size);
		memcpy(tmp, data, size + 1);
		new_text_list[i] = tmp;
		tmp += size + 1;
	    }
	    XFreeStringList((*text_list));
	    *text_list = new_text_list;
	} else {
	    rc = -1;
	}
    }
    return rc;
}
#endif /* OPT_WIDE_CHARS */

static char *
parseItem(char *value, char *nextc)
{
    char *nextp = value;
    while (*nextp != '\0' && *nextp != ',') {
	*nextp = x_toupper(*nextp);
	++nextp;
    }
    *nextc = *nextp;
    *nextp = '\0';
    x_strtrim(value);

    return nextp;
}

/*
 * All of the wanted strings are unique in the first character, so we can
 * use simple abbreviations.
 */
static Bool
sameItem(const char *actual, const char *wanted)
{
    Bool result = False;
    size_t have = strlen(actual);
    size_t need = strlen(wanted);

    if (have != 0 && have <= need) {
	if (!strncmp(actual, wanted, have)) {
	    TRACE(("...matched \"%s\"\n", wanted));
	    result = True;
	}
    }

    return result;
}

/*
 * Handle the eightBitSelectTypes or utf8SelectTypes resource values.
 */
static Bool
overrideTargets(Widget w, String value, Atom ** resultp)
{
    Bool override = False;
    XtermWidget xw;

    if ((xw = getXtermWidget(w)) != 0) {
	TScreen *screen = TScreenOf(xw);

	if (!IsEmpty(value)) {
	    char *copied = x_strdup(value);
	    if (copied != 0) {
		Atom *result = 0;
		Cardinal count = 1;
		int n;

		TRACE(("decoding SelectTypes \"%s\"\n", value));
		for (n = 0; copied[n] != '\0'; ++n) {
		    if (copied[n] == ',')
			++count;
		}
		result = (Atom *) XtMalloc(((2 * count) + 1)
					   * (Cardinal) sizeof(Atom));
		if (result == NULL) {
		    TRACE(("Couldn't allocate selection types\n"));
		} else {
		    char nextc = '?';
		    char *listp = (char *) copied;
		    count = 0;
		    do {
			char *nextp = parseItem(listp, &nextc);
			size_t len = strlen(listp);

			if (len == 0) {
			    ;
			}
#if OPT_WIDE_CHARS
			else if (sameItem(listp, "UTF8")) {
			    result[count++] = XA_UTF8_STRING(XtDisplay(w));
			}
#endif
			else if (sameItem(listp, "I18N")) {
			    if (screen->i18nSelections) {
				result[count++] = XA_TEXT(XtDisplay(w));
				result[count++] = XA_COMPOUND_TEXT(XtDisplay(w));
			    }
			} else if (sameItem(listp, "TEXT")) {
			    result[count++] = XA_TEXT(XtDisplay(w));
			} else if (sameItem(listp, "COMPOUND_TEXT")) {
			    result[count++] = XA_COMPOUND_TEXT(XtDisplay(w));
			} else if (sameItem(listp, "STRING")) {
			    result[count++] = XA_STRING;
			}
			*nextp++ = nextc;
			listp = nextp;
		    } while (nextc != '\0');
		    if (count) {
			result[count] = None;
			override = True;
			*resultp = result;
		    } else {
			XtFree((char *) result);
		    }
		}
	    } else {
		TRACE(("Couldn't allocate copy of selection types\n"));
	    }
	}
    }
    return override;
}

#if OPT_WIDE_CHARS
static Atom *
allocUtf8Targets(Widget w, TScreen * screen)
{
    Atom **resultp = &(screen->selection_targets_utf8);

    if (*resultp == 0) {
	Atom *result;

	if (!overrideTargets(w, screen->utf8_select_types, &result)) {
	    result = (Atom *) XtMalloc((Cardinal) (5 * sizeof(Atom)));
	    if (result == NULL) {
		TRACE(("Couldn't allocate utf-8 selection targets\n"));
	    } else {
		int n = 0;

		result[n++] = XA_UTF8_STRING(XtDisplay(w));
#ifdef X_HAVE_UTF8_STRING
		if (screen->i18nSelections) {
		    result[n++] = XA_TEXT(XtDisplay(w));
		    result[n++] = XA_COMPOUND_TEXT(XtDisplay(w));
		}
#endif
		result[n++] = XA_STRING;
		result[n] = None;
	    }
	}

	*resultp = result;
    }

    return *resultp;
}
#endif

static Atom *
alloc8bitTargets(Widget w, TScreen * screen)
{
    Atom **resultp = &(screen->selection_targets_8bit);

    if (*resultp == 0) {
	Atom *result = 0;

	if (!overrideTargets(w, screen->eightbit_select_types, &result)) {
	    result = (Atom *) XtMalloc((Cardinal) (5 * sizeof(Atom)));
	    if (result == NULL) {
		TRACE(("Couldn't allocate 8bit selection targets\n"));
	    } else {
		int n = 0;

#ifdef X_HAVE_UTF8_STRING
		result[n++] = XA_UTF8_STRING(XtDisplay(w));
#endif
		if (screen->i18nSelections) {
		    result[n++] = XA_TEXT(XtDisplay(w));
		    result[n++] = XA_COMPOUND_TEXT(XtDisplay(w));
		}
		result[n++] = XA_STRING;
		result[n] = None;
	    }
	}

	*resultp = result;
    }

    return *resultp;
}

static Atom *
_SelectionTargets(Widget w)
{
    Atom *result;
    TScreen *screen;
    XtermWidget xw;

    if ((xw = getXtermWidget(w)) == 0) {
	result = NULL;
    } else {
	screen = TScreenOf(xw);

#if OPT_WIDE_CHARS
	if (screen->wide_chars) {
	    result = allocUtf8Targets(w, screen);
	} else
#endif
	{
	    /* not screen->wide_chars */
	    result = alloc8bitTargets(w, screen);
	}
    }

    return result;
}

#define isSELECT(value) (!strcmp(value, "SELECT"))

static void
UnmapSelections(XtermWidget xw)
{
    TScreen *screen = TScreenOf(xw);
    Cardinal n;

    if (screen->mappedSelect) {
	for (n = 0; screen->mappedSelect[n] != 0; ++n)
	    free((void *) screen->mappedSelect[n]);
	free(screen->mappedSelect);
	screen->mappedSelect = 0;
    }
}

/*
 * xterm generally uses the primary selection.  Some applications prefer
 * (or are limited to) the clipboard.  Since the translations resource is
 * complicated, users seldom change the way it affects selection.  But it
 * is simple to remap the choice between primary and clipboard before the
 * call to XmuInternStrings().
 */
static String *
MapSelections(XtermWidget xw, String * params, Cardinal num_params)
{
    String *result = params;

    if (num_params > 0) {
	Cardinal j;
	Boolean map = False;

	for (j = 0; j < num_params; ++j) {
	    TRACE(("param[%d]:%s\n", j, params[j]));
	    if (isSELECT(params[j])) {
		map = True;
		break;
	    }
	}
	if (map) {
	    TScreen *screen = TScreenOf(xw);
	    const char *mapTo = (screen->selectToClipboard
				 ? "CLIPBOARD"
				 : "PRIMARY");

	    UnmapSelections(xw);
	    if ((result = TypeMallocN(String, num_params + 1)) != 0) {
		result[num_params] = 0;
		for (j = 0; j < num_params; ++j) {
		    result[j] = x_strdup((isSELECT(params[j])
					  ? mapTo
					  : params[j]));
		    if (result[j] == 0) {
			UnmapSelections(xw);
			result = 0;
			break;
		    }
		}
		screen->mappedSelect = result;
	    }
	}
    }
    return result;
}

/*
 * Lookup the cut-buffer number, which will be in the range 0-7.
 * If it is not a cut-buffer, it is the primary selection (-1).
 */
static int
CutBuffer(Atom code)
{
    int cutbuffer;
    switch ((unsigned) code) {
    case XA_CUT_BUFFER0:
	cutbuffer = 0;
	break;
    case XA_CUT_BUFFER1:
	cutbuffer = 1;
	break;
    case XA_CUT_BUFFER2:
	cutbuffer = 2;
	break;
    case XA_CUT_BUFFER3:
	cutbuffer = 3;
	break;
    case XA_CUT_BUFFER4:
	cutbuffer = 4;
	break;
    case XA_CUT_BUFFER5:
	cutbuffer = 5;
	break;
    case XA_CUT_BUFFER6:
	cutbuffer = 6;
	break;
    case XA_CUT_BUFFER7:
	cutbuffer = 7;
	break;
    default:
	cutbuffer = -1;
	break;
    }
    return cutbuffer;
}

#if OPT_PASTE64
static void
FinishPaste64(XtermWidget xw)
{
    TScreen *screen = TScreenOf(xw);

    TRACE(("FinishPaste64(%d)\n", screen->base64_paste));
    if (screen->base64_paste) {
	screen->base64_paste = 0;
	unparseputc1(xw, screen->base64_final);
	unparse_end(xw);
    }
}
#endif

#if !OPT_PASTE64
static
#endif
void
xtermGetSelection(Widget w,
		  Time ev_time,
		  String * params,	/* selections in precedence order */
		  Cardinal num_params,
		  Atom * targets)
{
    Atom selection;
    int cutbuffer;
    Atom target;

    XtermWidget xw;

    if (num_params == 0)
	return;
    if ((xw = getXtermWidget(w)) == 0)
	return;

    TRACE(("xtermGetSelection num_params %d\n", num_params));
    params = MapSelections(xw, params, num_params);

    XmuInternStrings(XtDisplay(w), params, (Cardinal) 1, &selection);
    cutbuffer = CutBuffer(selection);

    TRACE(("Cutbuffer: %d, target: %s\n", cutbuffer,
	   (targets
	    ? visibleSelectionTarget(XtDisplay(w), targets[0])
	    : "None")));

    if (cutbuffer >= 0) {
	int inbytes;
	unsigned long nbytes;
	int fmt8 = 8;
	Atom type = XA_STRING;
	char *line;

	/* 'line' is freed in SelectionReceived */
	line = XFetchBuffer(XtDisplay(w), &inbytes, cutbuffer);
	nbytes = (unsigned long) inbytes;

	if (nbytes > 0)
	    SelectionReceived(w, NULL, &selection, &type, (XtPointer) line,
			      &nbytes, &fmt8);
	else if (num_params > 1) {
	    xtermGetSelection(w, ev_time, params + 1, num_params - 1, NULL);
	}
#if OPT_PASTE64
	else {
	    FinishPaste64(xw);
	}
#endif
	return;
    } else {
	struct _SelectionList *list;

	if (targets == NULL || targets[0] == None) {
	    targets = _SelectionTargets(w);
	}

	if (targets != 0) {
	    target = targets[0];

	    if (targets[1] == None) {	/* last target in list */
		params++;
		num_params--;
		targets = _SelectionTargets(w);
	    } else {
		targets = &(targets[1]);
	    }

	    if (num_params) {
		/* 'list' is freed in SelectionReceived */
		list = XtNew(struct _SelectionList);
		if (list != 0) {
		    list->params = params;
		    list->count = num_params;
		    list->targets = targets;
		    list->time = ev_time;
		}
	    } else {
		list = NULL;
	    }

	    XtGetSelectionValue(w, selection,
				target,
				SelectionReceived,
				(XtPointer) list, ev_time);
	}
    }
}

#if OPT_TRACE && OPT_WIDE_CHARS
static void
GettingSelection(Display * dpy, Atom type, Char * line, unsigned long len)
{
    Char *cp;
    char *name;

    name = XGetAtomName(dpy, type);

    TRACE(("Getting %s (%ld)\n", name, (long int) type));
    for (cp = line; cp < line + len; cp++) {
	TRACE(("[%d:%lu]", (int) (cp + 1 - line), len));
	if (isprint(*cp)) {
	    TRACE(("%c\n", *cp));
	} else {
	    TRACE(("\\x%02x\n", *cp));
	}
    }
}
#else
#define GettingSelection(dpy,type,line,len)	/* nothing */
#endif

#ifdef VMS
#  define tty_vwrite(pty,lag,l)		tt_write(lag,l)
#else /* !( VMS ) */
#  define tty_vwrite(pty,lag,l)		v_write(pty,lag,l)
#endif /* defined VMS */

#if OPT_PASTE64
/* Return base64 code character given 6-bit number */
static const char base64_code[] = "\
ABCDEFGHIJKLMNOPQRSTUVWXYZ\
abcdefghijklmnopqrstuvwxyz\
0123456789+/";
static void
base64_flush(TScreen * screen)
{
    Char x;
    switch (screen->base64_count) {
    case 0:
	break;
    case 2:
	x = CharOf(base64_code[screen->base64_accu << 4]);
	tty_vwrite(screen->respond, &x, 1);
	break;
    case 4:
	x = CharOf(base64_code[screen->base64_accu << 2]);
	tty_vwrite(screen->respond, &x, 1);
	break;
    }
    if (screen->base64_pad & 3)
	tty_vwrite(screen->respond,
		   (const Char *) "===",
		   (unsigned) (4 - (screen->base64_pad & 3)));
    screen->base64_count = 0;
    screen->base64_accu = 0;
    screen->base64_pad = 0;
}
#endif /* OPT_PASTE64 */

static void
_qWriteSelectionData(TScreen * screen, Char * lag, unsigned length)
{
#if OPT_PASTE64
    if (screen->base64_paste) {
	/* Send data as base64 */
	Char *p = lag;
	Char buf[64];
	unsigned x = 0;
	while (length--) {
	    switch (screen->base64_count) {
	    case 0:
		buf[x++] = CharOf(base64_code[*p >> 2]);
		screen->base64_accu = (unsigned) (*p & 0x3);
		screen->base64_count = 2;
		++p;
		break;
	    case 2:
		buf[x++] = CharOf(base64_code[(screen->base64_accu << 4) +
					      (*p >> 4)]);
		screen->base64_accu = (unsigned) (*p & 0xF);
		screen->base64_count = 4;
		++p;
		break;
	    case 4:
		buf[x++] = CharOf(base64_code[(screen->base64_accu << 2) +
					      (*p >> 6)]);
		buf[x++] = CharOf(base64_code[*p & 0x3F]);
		screen->base64_accu = 0;
		screen->base64_count = 0;
		++p;
		break;
	    }
	    if (x >= 63) {
		/* Write 63 or 64 characters */
		screen->base64_pad += x;
		tty_vwrite(screen->respond, buf, x);
		x = 0;
	    }
	}
	if (x != 0) {
	    screen->base64_pad += x;
	    tty_vwrite(screen->respond, buf, x);
	}
    } else
#endif /* OPT_PASTE64 */
#if OPT_READLINE
    if (SCREEN_FLAG(screen, paste_quotes)) {
	while (length--) {
	    tty_vwrite(screen->respond, (const Char *) "\026", 1);	/* Control-V */
	    tty_vwrite(screen->respond, lag++, 1);
	}
    } else
#endif
	tty_vwrite(screen->respond, lag, length);
}

static void
_WriteSelectionData(TScreen * screen, Char * line, size_t length)
{
    /* Write data to pty a line at a time. */
    /* Doing this one line at a time may no longer be necessary
       because v_write has been re-written. */

    Char *lag, *end;

    /* in the VMS version, if tt_pasting isn't set to True then qio
       reads aren't blocked and an infinite loop is entered, where the
       pasted text shows up as new input, goes in again, shows up
       again, ad nauseum. */
#ifdef VMS
    tt_pasting = True;
#endif

    end = &line[length];
    lag = line;

#if OPT_PASTE64
    if (screen->base64_paste) {
	_qWriteSelectionData(screen, lag, (unsigned) (end - lag));
	base64_flush(screen);
    } else
#endif
    {
	if (!SCREEN_FLAG(screen, paste_literal_nl)) {
	    Char *cp;
	    for (cp = line; cp != end; cp++) {
		if (*cp == '\n') {
		    *cp = '\r';
		    _qWriteSelectionData(screen, lag, (unsigned) (cp - lag + 1));
		    lag = cp + 1;
		}
	    }
	}

	if (lag != end) {
	    _qWriteSelectionData(screen, lag, (unsigned) (end - lag));
	}
    }
#ifdef VMS
    tt_pasting = False;
    tt_start_read();		/* reenable reads or a character may be lost */
#endif
}

#if OPT_READLINE
static void
_WriteKey(TScreen * screen, const Char * in)
{
    Char line[16];
    unsigned count = 0;
    size_t length = strlen((const char *) in);

    if (screen->control_eight_bits) {
	line[count++] = ANSI_CSI;
    } else {
	line[count++] = ANSI_ESC;
	line[count++] = '[';
    }
    while (length--)
	line[count++] = *in++;
    line[count++] = '~';
    tty_vwrite(screen->respond, line, count);
}
#endif /* OPT_READLINE */

/* SelectionReceived: stuff received selection text into pty */

/* ARGSUSED */
static void
SelectionReceived(Widget w,
		  XtPointer client_data,
		  Atom * selection GCC_UNUSED,
		  Atom * type,
		  XtPointer value,
		  unsigned long *length,
		  int *format)
{
    char **text_list = NULL;
    int text_list_count;
    XTextProperty text_prop;
    TScreen *screen;
    Display *dpy;
#if OPT_TRACE && OPT_WIDE_CHARS
    Char *line = (Char *) value;
#endif

    XtermWidget xw;

    if ((xw = getXtermWidget(w)) == 0)
	return;

    screen = TScreenOf(xw);
    dpy = XtDisplay(w);

    if (*type == 0		/*XT_CONVERT_FAIL */
	|| *length == 0
	|| value == NULL)
	goto fail;

    text_prop.value = (unsigned char *) value;
    text_prop.encoding = *type;
    text_prop.format = *format;
    text_prop.nitems = *length;

    TRACE(("SelectionReceived %s format %d, nitems %ld\n",
	   visibleSelectionTarget(dpy, text_prop.encoding),
	   text_prop.format,
	   text_prop.nitems));

#if OPT_WIDE_CHARS
    if (screen->wide_chars) {
	if (*type == XA_UTF8_STRING(dpy) ||
	    *type == XA_STRING ||
	    *type == XA_COMPOUND_TEXT(dpy)) {
	    GettingSelection(dpy, *type, line, *length);
	    if (Xutf8TextPropertyToTextList(dpy, &text_prop,
					    &text_list,
					    &text_list_count) < 0) {
		TRACE(("Conversion failed\n"));
		text_list = NULL;
	    }
	}
    } else
#endif /* OPT_WIDE_CHARS */
    {
	/* Convert the selection to locale's multibyte encoding. */

	if (*type == XA_UTF8_STRING(dpy) ||
	    *type == XA_STRING ||
	    *type == XA_COMPOUND_TEXT(dpy)) {
	    Status rc;

	    GettingSelection(dpy, *type, line, *length);

#if OPT_WIDE_CHARS
	    if (*type == XA_UTF8_STRING(dpy) &&
		!(screen->wide_chars || screen->c1_printable)) {
		rc = xtermUtf8ToTextList(xw, &text_prop,
					 &text_list, &text_list_count);
	    } else
#endif
	    if (*type == XA_STRING && screen->brokenSelections) {
		rc = XTextPropertyToStringList(&text_prop,
					       &text_list, &text_list_count);
	    } else {
		rc = XmbTextPropertyToTextList(dpy, &text_prop,
					       &text_list,
					       &text_list_count);
	    }
	    if (rc < 0) {
		TRACE(("Conversion failed\n"));
		text_list = NULL;
	    }
	}
    }

    if (text_list != NULL && text_list_count != 0) {
	int i;

#if OPT_PASTE64
	if (screen->base64_paste) {
	    ;
	} else
#endif
#if OPT_READLINE
	if (SCREEN_FLAG(screen, paste_brackets)) {
	    _WriteKey(screen, (const Char *) "200");
	}
#endif
	for (i = 0; i < text_list_count; i++) {
	    size_t len = strlen(text_list[i]);
	    _WriteSelectionData(screen, (Char *) text_list[i], len);
	}
#if OPT_PASTE64
	if (screen->base64_paste) {
	    FinishPaste64(xw);
	} else
#endif
#if OPT_READLINE
	if (SCREEN_FLAG(screen, paste_brackets)) {
	    _WriteKey(screen, (const Char *) "201");
	}
#endif
	XFreeStringList(text_list);
    } else
	goto fail;

    XtFree((char *) client_data);
    XtFree((char *) value);

    return;

  fail:
    if (client_data != 0) {
	struct _SelectionList *list = (struct _SelectionList *) client_data;

	TRACE(("SelectionReceived ->xtermGetSelection\n"));
	xtermGetSelection(w, list->time,
			  list->params, list->count, list->targets);
	XtFree((char *) client_data);
#if OPT_PASTE64
    } else {
	FinishPaste64(xw);
#endif
    }
    return;
}

void
HandleInsertSelection(Widget w,
		      XEvent * event,	/* assumed to be XButtonEvent* */
		      String * params,	/* selections in precedence order */
		      Cardinal *num_params)
{
    XtermWidget xw;

    if ((xw = getXtermWidget(w)) != 0) {
	if (!SendMousePosition(xw, event)) {
#if OPT_READLINE
	    int ldelta;
	    TScreen *screen = TScreenOf(xw);
	    if (IsBtnEvent(event)
	    /* Disable on Shift-mouse, including the application-mouse modes */
		&& !(KeyModifiers(event) & ShiftMask)
		&& (screen->send_mouse_pos == MOUSE_OFF)
		&& SCREEN_FLAG(screen, paste_moves)
		&& rowOnCurrentLine(screen, eventRow(screen, event), &ldelta))
		ReadLineMovePoint(screen, eventColBetween(screen, event), ldelta);
#endif /* OPT_READLINE */

	    xtermGetSelection(w, event->xbutton.time, params, *num_params, NULL);
	}
    }
}

static SelectUnit
EvalSelectUnit(XtermWidget xw,
	       Time buttonDownTime,
	       SelectUnit defaultUnit,
	       unsigned int button)
{
    TScreen *screen = TScreenOf(xw);
    SelectUnit result;
    int delta;

    if (button != screen->lastButton) {
	delta = screen->multiClickTime + 1;
    } else if (screen->lastButtonUpTime == (Time) 0) {
	/* first time and once in a blue moon */
	delta = screen->multiClickTime + 1;
    } else if (buttonDownTime > screen->lastButtonUpTime) {
	/* most of the time */
	delta = (int) (buttonDownTime - screen->lastButtonUpTime);
    } else {
	/* time has rolled over since lastButtonUpTime */
	delta = (int) ((((Time) ~ 0) - screen->lastButtonUpTime) + buttonDownTime);
    }

    if (delta > screen->multiClickTime) {
	screen->numberOfClicks = 1;
	result = defaultUnit;
    } else {
	result = screen->selectMap[screen->numberOfClicks % screen->maxClicks];
	screen->numberOfClicks += 1;
    }
    TRACE(("EvalSelectUnit(%d) = %d\n", screen->numberOfClicks, result));
    return result;
}

static void
do_select_start(XtermWidget xw,
		XEvent * event,	/* must be XButtonEvent* */
		CELL * cell)
{
    TScreen *screen = TScreenOf(xw);

    if (SendMousePosition(xw, event))
	return;
    screen->selectUnit = EvalSelectUnit(xw,
					event->xbutton.time,
					Select_CHAR,
					event->xbutton.button);
    screen->replyToEmacs = False;

#if OPT_READLINE
    lastButtonDownTime = event->xbutton.time;
#endif

    StartSelect(xw, cell);
}

/* ARGSUSED */
void
HandleSelectStart(Widget w,
		  XEvent * event,	/* must be XButtonEvent* */
		  String * params GCC_UNUSED,
		  Cardinal *num_params GCC_UNUSED)
{
    XtermWidget xw;

    if ((xw = getXtermWidget(w)) != 0) {
	TScreen *screen = TScreenOf(xw);
	CELL cell;

	screen->firstValidRow = 0;
	screen->lastValidRow = screen->max_row;
	PointToCELL(screen, event->xbutton.y, event->xbutton.x, &cell);

#if OPT_READLINE
	ExtendingSelection = 0;
#endif

	do_select_start(xw, event, &cell);
    }
}

/* ARGSUSED */
void
HandleKeyboardSelectStart(Widget w,
			  XEvent * event,	/* must be XButtonEvent* */
			  String * params GCC_UNUSED,
			  Cardinal *num_params GCC_UNUSED)
{
    XtermWidget xw;

    if ((xw = getXtermWidget(w)) != 0) {
	TScreen *screen = TScreenOf(xw);
	do_select_start(xw, event, &screen->cursorp);
    }
}

static void
TrackDown(XtermWidget xw, XButtonEvent * event)
{
    TScreen *screen = TScreenOf(xw);
    CELL cell;

    screen->selectUnit = EvalSelectUnit(xw,
					event->time,
					Select_CHAR,
					event->button);
    if (screen->numberOfClicks > 1) {
	PointToCELL(screen, event->y, event->x, &cell);
	screen->replyToEmacs = True;
	StartSelect(xw, &cell);
    } else {
	screen->waitingForTrackInfo = True;
	EditorButton(xw, event);
    }
}

#define boundsCheck(x)	if (x < 0) \
			    x = 0; \
			else if (x >= screen->max_row) \
			    x = screen->max_row

void
TrackMouse(XtermWidget xw,
	   int func,
	   CELL * start,
	   int firstrow,
	   int lastrow)
{
    TScreen *screen = TScreenOf(xw);

    if (screen->waitingForTrackInfo) {	/* if Timed, ignore */
	screen->waitingForTrackInfo = False;

	if (func != 0) {
	    CELL first = *start;

	    boundsCheck(first.row);
	    boundsCheck(firstrow);
	    boundsCheck(lastrow);
	    screen->firstValidRow = firstrow;
	    screen->lastValidRow = lastrow;
	    screen->replyToEmacs = True;
	    StartSelect(xw, &first);
	}
    }
}

static void
StartSelect(XtermWidget xw, const CELL * cell)
{
    TScreen *screen = TScreenOf(xw);

    TRACE(("StartSelect row=%d, col=%d\n", cell->row, cell->col));
    if (screen->cursor_state)
	HideCursor();
    if (screen->numberOfClicks == 1) {
	/* set start of selection */
	screen->rawPos = *cell;
    }
    /* else use old values in rawPos */
    screen->saveStartR = screen->startExt = screen->rawPos;
    screen->saveEndR = screen->endExt = screen->rawPos;
    if (Coordinate(screen, cell) < Coordinate(screen, &(screen->rawPos))) {
	screen->eventMode = LEFTEXTENSION;
	screen->startExt = *cell;
    } else {
	screen->eventMode = RIGHTEXTENSION;
	screen->endExt = *cell;
    }
    ComputeSelect(xw, &(screen->startExt), &(screen->endExt), False);
}

static void
EndExtend(XtermWidget xw,
	  XEvent * event,	/* must be XButtonEvent */
	  String * params,	/* selections */
	  Cardinal num_params,
	  Bool use_cursor_loc)
{
    CELL cell;
    unsigned count;
    TScreen *screen = TScreenOf(xw);
    Char line[20];

    if (use_cursor_loc) {
	cell = screen->cursorp;
    } else {
	PointToCELL(screen, event->xbutton.y, event->xbutton.x, &cell);
    }
    ExtendExtend(xw, &cell);
    screen->lastButtonUpTime = event->xbutton.time;
    screen->lastButton = event->xbutton.button;
    if (!isSameCELL(&(screen->startSel), &(screen->endSel))) {
	if (screen->replyToEmacs) {
	    count = 0;
	    if (screen->control_eight_bits) {
		line[count++] = ANSI_CSI;
	    } else {
		line[count++] = ANSI_ESC;
		line[count++] = '[';
	    }
	    if (isSameCELL(&(screen->rawPos), &(screen->startSel))
		&& isSameCELL(&cell, &(screen->endSel))) {
		/* Use short-form emacs select */
		line[count++] = 't';
		count = EmitMousePosition(screen, line, count, screen->endSel.col);
		count = EmitMousePosition(screen, line, count, screen->endSel.row);
	    } else {
		/* long-form, specify everything */
		line[count++] = 'T';
		count = EmitMousePosition(screen, line, count, screen->startSel.col);
		count = EmitMousePosition(screen, line, count, screen->startSel.row);
		count = EmitMousePosition(screen, line, count, screen->endSel.col);
		count = EmitMousePosition(screen, line, count, screen->endSel.row);
		count = EmitMousePosition(screen, line, count, cell.col);
		count = EmitMousePosition(screen, line, count, cell.row);
	    }
	    v_write(screen->respond, line, count);
	    TrackText(xw, &zeroCELL, &zeroCELL);
	}
    }
    SelectSet(xw, event, params, num_params);
    screen->eventMode = NORMAL;
}

void
HandleSelectSet(Widget w,
		XEvent * event,
		String * params,
		Cardinal *num_params)
{
    XtermWidget xw;

    if ((xw = getXtermWidget(w)) != 0) {
	SelectSet(xw, event, params, *num_params);
    }
}

/* ARGSUSED */
static void
SelectSet(XtermWidget xw,
	  XEvent * event GCC_UNUSED,
	  String * params,
	  Cardinal num_params)
{
    TScreen *screen = TScreenOf(xw);

    TRACE(("SelectSet\n"));
    /* Only do select stuff if non-null select */
    if (!isSameCELL(&(screen->startSel), &(screen->endSel))) {
	SaltTextAway(xw, &(screen->startSel), &(screen->endSel), params, num_params);
    } else {
	DisownSelection(xw);
    }
}

#define Abs(x)		((x) < 0 ? -(x) : (x))

/* ARGSUSED */
static void
do_start_extend(XtermWidget xw,
		XEvent * event,	/* must be XButtonEvent* */
		String * params GCC_UNUSED,
		Cardinal *num_params GCC_UNUSED,
		Bool use_cursor_loc)
{
    TScreen *screen = TScreenOf(xw);
    int coord;
    CELL cell;

    if (SendMousePosition(xw, event))
	return;

    screen->firstValidRow = 0;
    screen->lastValidRow = screen->max_row;
#if OPT_READLINE
    if ((KeyModifiers(event) & ShiftMask)
	|| event->xbutton.button != Button3
	|| !(SCREEN_FLAG(screen, dclick3_deletes)))
#endif
	screen->selectUnit = EvalSelectUnit(xw,
					    event->xbutton.time,
					    screen->selectUnit,
					    event->xbutton.button);
    screen->replyToEmacs = False;

#if OPT_READLINE
    CheckSecondPress3(screen, event);
#endif

    if (screen->numberOfClicks == 1
	|| (SCREEN_FLAG(screen, dclick3_deletes)	/* Dclick special */
	    &&!(KeyModifiers(event) & ShiftMask))) {
	/* Save existing selection so we can reestablish it if the guy
	   extends past the other end of the selection */
	screen->saveStartR = screen->startExt = screen->startRaw;
	screen->saveEndR = screen->endExt = screen->endRaw;
    } else {
	/* He just needed the selection mode changed, use old values. */
	screen->startExt = screen->startRaw = screen->saveStartR;
	screen->endExt = screen->endRaw = screen->saveEndR;
    }
    if (use_cursor_loc) {
	cell = screen->cursorp;
    } else {
	PointToCELL(screen, event->xbutton.y, event->xbutton.x, &cell);
    }
    coord = Coordinate(screen, &cell);

    if (Abs(coord - Coordinate(screen, &(screen->startSel)))
	< Abs(coord - Coordinate(screen, &(screen->endSel)))
	|| coord < Coordinate(screen, &(screen->startSel))) {
	/* point is close to left side of selection */
	screen->eventMode = LEFTEXTENSION;
	screen->startExt = cell;
    } else {
	/* point is close to left side of selection */
	screen->eventMode = RIGHTEXTENSION;
	screen->endExt = cell;
    }
    ComputeSelect(xw, &(screen->startExt), &(screen->endExt), True);

#if OPT_READLINE
    if (!isSameCELL(&(screen->startSel), &(screen->endSel)))
	ExtendingSelection = 1;
#endif
}

static void
ExtendExtend(XtermWidget xw, const CELL * cell)
{
    TScreen *screen = TScreenOf(xw);
    int coord = Coordinate(screen, cell);

    TRACE(("ExtendExtend row=%d, col=%d\n", cell->row, cell->col));
    if (screen->eventMode == LEFTEXTENSION
	&& ((coord + (screen->selectUnit != Select_CHAR))
	    > Coordinate(screen, &(screen->endSel)))) {
	/* Whoops, he's changed his mind.  Do RIGHTEXTENSION */
	screen->eventMode = RIGHTEXTENSION;
	screen->startExt = screen->saveStartR;
    } else if (screen->eventMode == RIGHTEXTENSION
	       && coord < Coordinate(screen, &(screen->startSel))) {
	/* Whoops, he's changed his mind.  Do LEFTEXTENSION */
	screen->eventMode = LEFTEXTENSION;
	screen->endExt = screen->saveEndR;
    }
    if (screen->eventMode == LEFTEXTENSION) {
	screen->startExt = *cell;
    } else {
	screen->endExt = *cell;
    }
    ComputeSelect(xw, &(screen->startExt), &(screen->endExt), False);

#if OPT_READLINE
    if (!isSameCELL(&(screen->startSel), &(screen->endSel)))
	ExtendingSelection = 1;
#endif
}

void
HandleStartExtend(Widget w,
		  XEvent * event,	/* must be XButtonEvent* */
		  String * params,	/* unused */
		  Cardinal *num_params)		/* unused */
{
    XtermWidget xw;

    if ((xw = getXtermWidget(w)) != 0) {
	do_start_extend(xw, event, params, num_params, False);
    }
}

void
HandleKeyboardStartExtend(Widget w,
			  XEvent * event,	/* must be XButtonEvent* */
			  String * params,	/* unused */
			  Cardinal *num_params)		/* unused */
{
    XtermWidget xw;

    if ((xw = getXtermWidget(w)) != 0) {
	do_start_extend(xw, event, params, num_params, True);
    }
}

void
ScrollSelection(TScreen * screen, int amount, Bool always)
{
    int minrow = INX2ROW(screen, -screen->savedlines);
    int maxrow = INX2ROW(screen, screen->max_row);
    int maxcol = screen->max_col;

#define scroll_update_one(cell) \
	(cell)->row += amount; \
	if ((cell)->row < minrow) { \
	    (cell)->row = minrow; \
	    (cell)->col = 0; \
	} \
	if ((cell)->row > maxrow) { \
	    (cell)->row = maxrow; \
	    (cell)->col = maxcol; \
	}

    scroll_update_one(&(screen->startRaw));
    scroll_update_one(&(screen->endRaw));
    scroll_update_one(&(screen->startSel));
    scroll_update_one(&(screen->endSel));

    scroll_update_one(&(screen->rawPos));

    /*
     * If we are told to scroll the selection but it lies outside the scrolling
     * margins, then that could cause the selection to move (bad).  It is not
     * simple to fix, because this function is called both for the scrollbar
     * actions as well as application scrolling.  The 'always' flag is set in
     * the former case.  The rest of the logic handles the latter.
     */
    if (ScrnHaveSelection(screen)) {
	int adjust;

	adjust = ROW2INX(screen, screen->startH.row);
	if (always
	    || !ScrnHaveLineMargins(screen)
	    || ScrnIsLineInMargins(screen, adjust)) {
	    scroll_update_one(&screen->startH);
	}
	adjust = ROW2INX(screen, screen->endH.row);
	if (always
	    || !ScrnHaveLineMargins(screen)
	    || ScrnIsLineInMargins(screen, adjust)) {
	    scroll_update_one(&screen->endH);
	}
    }

    screen->startHCoord = Coordinate(screen, &screen->startH);
    screen->endHCoord = Coordinate(screen, &screen->endH);
}

/*ARGSUSED*/
void
ResizeSelection(TScreen * screen GCC_UNUSED, int rows, int cols)
{
    rows--;			/* decr to get 0-max */
    cols--;

    if (screen->startRaw.row > rows)
	screen->startRaw.row = rows;
    if (screen->startSel.row > rows)
	screen->startSel.row = rows;
    if (screen->endRaw.row > rows)
	screen->endRaw.row = rows;
    if (screen->endSel.row > rows)
	screen->endSel.row = rows;
    if (screen->rawPos.row > rows)
	screen->rawPos.row = rows;

    if (screen->startRaw.col > cols)
	screen->startRaw.col = cols;
    if (screen->startSel.col > cols)
	screen->startSel.col = cols;
    if (screen->endRaw.col > cols)
	screen->endRaw.col = cols;
    if (screen->endSel.col > cols)
	screen->endSel.col = cols;
    if (screen->rawPos.col > cols)
	screen->rawPos.col = cols;
}

#if OPT_WIDE_CHARS
Bool
iswide(int i)
{
    return (i == HIDDEN_CHAR) || (WideCells(i) == 2);
}

#define isWideCell(row, col) iswide((int)XTERM_CELL(row, col))
#endif

static void
PointToCELL(TScreen * screen,
	    int y,
	    int x,
	    CELL * cell)
/* Convert pixel coordinates to character coordinates.
   Rows are clipped between firstValidRow and lastValidRow.
   Columns are clipped between to be 0 or greater, but are not clipped to some
       maximum value. */
{
    cell->row = (y - screen->border) / FontHeight(screen);
    if (cell->row < screen->firstValidRow)
	cell->row = screen->firstValidRow;
    else if (cell->row > screen->lastValidRow)
	cell->row = screen->lastValidRow;
    cell->col = (x - OriginX(screen)) / FontWidth(screen);
    if (cell->col < 0)
	cell->col = 0;
    else if (cell->col > MaxCols(screen)) {
	cell->col = MaxCols(screen);
    }
#if OPT_WIDE_CHARS
    /*
     * If we got a click on the right half of a doublewidth character,
     * pretend it happened on the left half.
     */
    if (cell->col > 0
	&& isWideCell(cell->row, cell->col - 1)
	&& (XTERM_CELL(cell->row, cell->col) == HIDDEN_CHAR)) {
	cell->col -= 1;
    }
#endif
}

/*
 * Find the last column at which text was drawn on the given row.
 */
static int
LastTextCol(TScreen * screen, LineData * ld, int row)
{
    int i = -1;
    Char *ch;

    if (ld != 0) {
	if (okScrnRow(screen, row)) {
	    for (i = screen->max_col,
		 ch = ld->attribs + i;
		 i >= 0 && !(*ch & CHARDRAWN);
		 ch--, i--) {
		;
	    }
#if OPT_DEC_CHRSET
	    if (CSET_DOUBLE(GetLineDblCS(ld))) {
		i *= 2;
	    }
#endif
	}
    }
    return (i);
}

#if !OPT_WIDE_CHARS
/*
** double click table for cut and paste in 8 bits
**
** This table is divided in four parts :
**
**	- control characters	[0,0x1f] U [0x80,0x9f]
**	- separators		[0x20,0x3f] U [0xa0,0xb9]
**	- binding characters	[0x40,0x7f] U [0xc0,0xff]
**	- exceptions
*/
/* *INDENT-OFF* */
static int charClass[256] =
{
/* NUL  SOH  STX  ETX  EOT  ENQ  ACK  BEL */
    32,  1,    1,   1,   1,   1,   1,   1,
/*  BS   HT   NL   VT   NP   CR   SO   SI */
     1,  32,   1,   1,   1,   1,   1,   1,
/* DLE  DC1  DC2  DC3  DC4  NAK  SYN  ETB */
     1,   1,   1,   1,   1,   1,   1,   1,
/* CAN   EM  SUB  ESC   FS   GS   RS   US */
     1,   1,   1,   1,   1,   1,   1,   1,
/*  SP    !    "    #    $    %    &    ' */
    32,  33,  34,  35,  36,  37,  38,  39,
/*   (    )    *    +    ,    -    .    / */
    40,  41,  42,  43,  44,  45,  46,  47,
/*   0    1    2    3    4    5    6    7 */
    48,  48,  48,  48,  48,  48,  48,  48,
/*   8    9    :    ;    <    =    >    ? */
    48,  48,  58,  59,  60,  61,  62,  63,
/*   @    A    B    C    D    E    F    G */
    64,  48,  48,  48,  48,  48,  48,  48,
/*   H    I    J    K    L    M    N    O */
    48,  48,  48,  48,  48,  48,  48,  48,
/*   P    Q    R    S    T    U    V    W */
    48,  48,  48,  48,  48,  48,  48,  48,
/*   X    Y    Z    [    \    ]    ^    _ */
    48,  48,  48,  91,  92,  93,  94,  48,
/*   `    a    b    c    d    e    f    g */
    96,  48,  48,  48,  48,  48,  48,  48,
/*   h    i    j    k    l    m    n    o */
    48,  48,  48,  48,  48,  48,  48,  48,
/*   p    q    r    s    t    u    v    w */
    48,  48,  48,  48,  48,  48,  48,  48,
/*   x    y    z    {    |    }    ~  DEL */
    48,  48,  48, 123, 124, 125, 126,   1,
/* x80  x81  x82  x83  IND  NEL  SSA  ESA */
    1,    1,   1,   1,   1,   1,   1,   1,
/* HTS  HTJ  VTS  PLD  PLU   RI  SS2  SS3 */
    1,    1,   1,   1,   1,   1,   1,   1,
/* DCS  PU1  PU2  STS  CCH   MW  SPA  EPA */
    1,    1,   1,   1,   1,   1,   1,   1,
/* x98  x99  x9A  CSI   ST  OSC   PM  APC */
    1,    1,   1,   1,   1,   1,   1,   1,
/*   -    i   c/    L   ox   Y-    |   So */
    160, 161, 162, 163, 164, 165, 166, 167,
/*  ..   c0   ip   <<    _        R0    - */
    168, 169, 170, 171, 172, 173, 174, 175,
/*   o   +-    2    3    '    u   q|    . */
    176, 177, 178, 179, 180, 181, 182, 183,
/*   ,    1    2   >>  1/4  1/2  3/4    ? */
    184, 185, 186, 187, 188, 189, 190, 191,
/*  A`   A'   A^   A~   A:   Ao   AE   C, */
     48,  48,  48,  48,  48,  48,  48,  48,
/*  E`   E'   E^   E:   I`   I'   I^   I: */
     48,  48,  48,  48,  48,  48,  48,  48,
/*  D-   N~   O`   O'   O^   O~   O:    X */
     48,  48,  48,  48,  48,  48,  48, 215,
/*  O/   U`   U'   U^   U:   Y'    P    B */
     48,  48,  48,  48,  48,  48,  48,  48,
/*  a`   a'   a^   a~   a:   ao   ae   c, */
     48,  48,  48,  48,  48,  48,  48,  48,
/*  e`   e'   e^   e:    i`  i'   i^   i: */
     48,  48,  48,  48,  48,  48,  48,  48,
/*   d   n~   o`   o'   o^   o~   o:   -: */
     48,  48,  48,  48,  48,  48,  48, 247,
/*  o/   u`   u'   u^   u:   y'    P   y: */
     48,  48,  48,  48,  48,  48,  48,  48};
/* *INDENT-ON* */

int
SetCharacterClassRange(int low,	/* in range of [0..255] */
		       int high,
		       int value)	/* arbitrary */
{

    if (low < 0 || high > 255 || high < low)
	return (-1);

    for (; low <= high; low++)
	charClass[low] = value;

    return (0);
}
#endif

static int
class_of(LineData * ld, CELL * cell)
{
    CELL temp = *cell;

#if OPT_DEC_CHRSET
    if (CSET_DOUBLE(GetLineDblCS(ld))) {
	temp.col /= 2;
    }
#endif

    assert(temp.col < ld->lineSize);
    return CharacterClass((int) (ld->charData[temp.col]));
}

#if OPT_WIDE_CHARS
#define CClassSelects(name, cclass) \
	 (CClassOf(name) == cclass \
	 || XTERM_CELL(screen->name.row, screen->name.col) == HIDDEN_CHAR)
#else
#define CClassSelects(name, cclass) \
	 (class_of(ld.name, &((screen->name))) == cclass)
#endif

#define CClassOf(name) class_of(ld.name, &((screen->name)))

/*
 * If the given column is past the end of text on the given row, bump to the
 * beginning of the next line.
 */
static Boolean
okPosition(TScreen * screen,
	   LineData ** ld,
	   CELL * cell)
{
    Boolean result = True;

    if (cell->row > screen->max_row) {
	result = False;
    } else if (cell->col > (LastTextCol(screen, *ld, cell->row) + 1)) {
	if (cell->row < screen->max_row) {
	    cell->col = 0;
	    *ld = GET_LINEDATA(screen, ++cell->row);
	    result = False;
	}
    }
    return result;
}

static void
trimLastLine(TScreen * screen,
	     LineData ** ld,
	     CELL * last)
{
    if (screen->cutNewline && last->row < screen->max_row) {
	last->col = 0;
	*ld = GET_LINEDATA(screen, ++last->row);
    } else {
	last->col = LastTextCol(screen, *ld, last->row) + 1;
    }
}

#if OPT_SELECT_REGEX
/*
 * Returns the first row of a wrapped line.
 */
static int
firstRowOfLine(TScreen * screen, int row, Bool visible)
{
    LineData *ld = 0;
    int limit = visible ? 0 : -screen->savedlines;

    while (row > limit &&
	   (ld = GET_LINEDATA(screen, row - 1)) != 0 &&
	   LineTstWrapped(ld)) {
	--row;
    }
    return row;
}

/*
 * Returns the last row of a wrapped line.
 */
static int
lastRowOfLine(TScreen * screen, int row)
{
    LineData *ld;

    while (row < screen->max_row &&
	   (ld = GET_LINEDATA(screen, row)) != 0 &&
	   LineTstWrapped(ld)) {
	++row;
    }
    return row;
}

/*
 * Returns the number of cells on the range of rows.
 */
static unsigned
lengthOfLines(TScreen * screen, int firstRow, int lastRow)
{
    unsigned length = 0;
    int n;

    for (n = firstRow; n <= lastRow; ++n) {
	LineData *ld = GET_LINEDATA(screen, n);
	int value = LastTextCol(screen, ld, n);
	if (value >= 0)
	    length += (unsigned) (value + 1);
    }
    return length;
}

/*
 * Make a copy of the wrapped-line which corresponds to the given row as a
 * string of bytes.  Construct an index for the columns from the beginning of
 * the line.
 */
static char *
make_indexed_text(TScreen * screen, int row, unsigned length, int *indexed)
{
    Char *result = 0;
    size_t need = (length + 1);

    /*
     * Get a quick upper bound to the number of bytes needed, if the whole
     * string were UTF-8.
     */
    if_OPT_WIDE_CHARS(screen, {
	need *= ((screen->lineExtra + 1) * 6);
    });

    if ((result = TypeCallocN(Char, need + 1)) != 0) {
	LineData *ld = GET_LINEDATA(screen, row);
	unsigned used = 0;
	Char *last = result;

	do {
	    int col = 0;
	    int limit = LastTextCol(screen, ld, row);

	    while (col <= limit) {
		Char *next = last;
		unsigned data = ld->charData[col];

		assert(col < ld->lineSize);
		/* some internal points may not be drawn */
		if (data == 0)
		    data = ' ';

		if_WIDE_OR_NARROW(screen, {
		    next = convertToUTF8(last, data);
		}
		, {
		    *next++ = CharOf(data);
		});

		if_OPT_WIDE_CHARS(screen, {
		    size_t off;
		    for_each_combData(off, ld) {
			data = ld->combData[off][col];
			if (data == 0)
			    break;
			next = convertToUTF8(next, data);
		    }
		});

		indexed[used] = (int) (last - result);
		*next = 0;
		/* TRACE(("index[%d.%d] %d:%s\n", row, used, indexed[used], last)); */
		last = next;
		++used;
		++col;
		indexed[used] = (int) (next - result);
	    }
	} while (used < length &&
		 LineTstWrapped(ld) &&
		 (ld = GET_LINEDATA(screen, ++row)) != 0 &&
		 row < screen->max_row);
    }
    /* TRACE(("result:%s\n", result)); */
    return (char *) result;
}

/*
 * Find the column given an offset into the character string by using the
 * index constructed in make_indexed_text().
 */
static int
indexToCol(int *indexed, int len, int off)
{
    int col = 0;
    while (indexed[col] < len) {
	if (indexed[col] >= off)
	    break;
	++col;
    }
    return col;
}

/*
 * Given a row number, and a column offset from that (which may be wrapped),
 * set the cell to the actual row/column values.
 */
static void
columnToCell(TScreen * screen, int row, int col, CELL * cell)
{
    while (row < screen->max_row) {
	LineData *ld = GET_LINEDATA(screen, row);
	int last = LastTextCol(screen, ld, row);

	/* TRACE(("last(%d) = %d, have %d\n", row, last, col)); */
	if (col <= last) {
	    break;
	}
	/*
	 * Stop if the current row does not wrap (does not continue the current
	 * line).
	 */
	if (!LineTstWrapped(ld)) {
	    col = last + 1;
	    break;
	}
	col -= (last + 1);
	++row;
    }
    if (col < 0)
	col = 0;
    cell->row = row;
    cell->col = col;
}

/*
 * Given a cell, find the corresponding column offset.
 */
static int
cellToColumn(TScreen * screen, CELL * cell)
{
    LineData *ld = 0;
    int col = cell->col;
    int row = firstRowOfLine(screen, cell->row, False);
    while (row < cell->row) {
	ld = GET_LINEDATA(screen, row);
	col += LastTextCol(screen, ld, row++);
    }
#if OPT_DEC_CHRSET
    if (ld == 0)
	ld = GET_LINEDATA(screen, row);
    if (CSET_DOUBLE(GetLineDblCS(ld)))
	col /= 2;
#endif
    return col;
}

static void
do_select_regex(TScreen * screen, CELL * startc, CELL * endc)
{
    LineData *ld = GET_LINEDATA(screen, startc->row);
    int inx = ((screen->numberOfClicks - 1) % screen->maxClicks);
    char *expr = screen->selectExpr[inx];
    regex_t preg;
    regmatch_t match;
    char *search;
    int *indexed;

    TRACE(("Select_REGEX:%s\n", NonNull(expr)));
    if (okPosition(screen, &ld, startc) && expr != 0) {
	if (regcomp(&preg, expr, REG_EXTENDED) == 0) {
	    int firstRow = firstRowOfLine(screen, startc->row, True);
	    int lastRow = lastRowOfLine(screen, firstRow);
	    unsigned size = lengthOfLines(screen, firstRow, lastRow);
	    int actual = cellToColumn(screen, startc);

	    TRACE(("regcomp ok rows %d..%d bytes %d\n",
		   firstRow, lastRow, size));

	    if ((indexed = TypeCallocN(int, size + 1)) != 0) {
		if ((search = make_indexed_text(screen,
						firstRow,
						size,
						indexed)) != 0) {
		    int len = (int) strlen(search);
		    int col;
		    int best_col = -1;
		    int best_len = -1;

		    for (col = 0; indexed[col] < len; ++col) {
			if (regexec(&preg,
				    search + indexed[col],
				    (size_t) 1, &match, 0) == 0) {
			    int start_inx = match.rm_so + indexed[col];
			    int finis_inx = match.rm_eo + indexed[col];
			    int start_col = indexToCol(indexed, len, start_inx);
			    int finis_col = indexToCol(indexed, len, finis_inx);

			    if (start_col <= actual &&
				actual < finis_col) {
				int test = finis_col - start_col;
				if (best_len < test) {
				    best_len = test;
				    best_col = start_col;
				    TRACE(("match column %d len %d\n",
					   best_col,
					   best_len));
				}
			    }
			}
		    }
		    if (best_col >= 0) {
			int best_nxt = best_col + best_len;
			columnToCell(screen, firstRow, best_col, startc);
			columnToCell(screen, firstRow, best_nxt, endc);
			TRACE(("search::%s\n", search));
			TRACE(("indexed:%d..%d -> %d..%d\n",
			       best_col, best_nxt,
			       indexed[best_col],
			       indexed[best_nxt]));
			TRACE(("matched:%d:%s\n",
			       indexed[best_nxt] + 1 -
			       indexed[best_col],
			       visibleChars((Char *) (search + indexed[best_col]),
					    (unsigned) (indexed[best_nxt] +
							1 -
							indexed[best_col]))));
		    }
		    free(search);
		}
		free(indexed);
#if OPT_DEC_CHRSET
		if ((ld = GET_LINEDATA(screen, startc->row)) != 0) {
		    if (CSET_DOUBLE(GetLineDblCS(ld)))
			startc->col *= 2;
		}
		if ((ld = GET_LINEDATA(screen, endc->row)) != 0) {
		    if (CSET_DOUBLE(GetLineDblCS(ld)))
			endc->col *= 2;
		}
#endif
	    }
	    regfree(&preg);
	}
    }
}
#endif /* OPT_SELECT_REGEX */

#define InitRow(name) \
	ld.name = GET_LINEDATA(screen, screen->name.row)

#define NextRow(name) \
	ld.name = GET_LINEDATA(screen, ++screen->name.row)

#define PrevRow(name) \
	ld.name = GET_LINEDATA(screen, --screen->name.row)

#define MoreRows(name) \
	(screen->name.row < screen->max_row)

#define isPrevWrapped(name) \
	(screen->name.row > 0 \
	   && (ltmp = GET_LINEDATA(screen, screen->name.row - 1)) != 0 \
	   && LineTstWrapped(ltmp))

/*
 * sets startSel endSel
 * ensuring that they have legal values
 */
static void
ComputeSelect(XtermWidget xw,
	      CELL * startc,
	      CELL * endc,
	      Bool extend)
{
    TScreen *screen = TScreenOf(xw);

    int length;
    int cclass;
    CELL first = *startc;
    CELL last = *endc;
    Boolean ignored = False;

    struct {
	LineData *startSel;
	LineData *endSel;
    } ld;
    LineData *ltmp;

    TRACE(("ComputeSelect(startRow=%d, startCol=%d, endRow=%d, endCol=%d, %sextend)\n",
	   first.row, first.col,
	   last.row, last.col,
	   extend ? "" : "no"));

#if OPT_WIDE_CHARS
    if (first.col > 1
	&& isWideCell(first.row, first.col - 1)
	&& XTERM_CELL(first.row, first.col - 0) == HIDDEN_CHAR) {
	TRACE(("Adjusting start. Changing downwards from %i.\n", first.col));
	first.col -= 1;
	if (last.col == (first.col + 1))
	    last.col--;
    }

    if (last.col > 1
	&& isWideCell(last.row, last.col - 1)
	&& XTERM_CELL(last.row, last.col) == HIDDEN_CHAR) {
	last.col += 1;
    }
#endif

    if (Coordinate(screen, &first) <= Coordinate(screen, &last)) {
	screen->startSel = screen->startRaw = first;
	screen->endSel = screen->endRaw = last;
    } else {			/* Swap them */
	screen->startSel = screen->startRaw = last;
	screen->endSel = screen->endRaw = first;
    }

    InitRow(startSel);
    InitRow(endSel);

    switch (screen->selectUnit) {
    case Select_CHAR:
	(void) okPosition(screen, &(ld.startSel), &(screen->startSel));
	(void) okPosition(screen, &(ld.endSel), &(screen->endSel));
	break;

    case Select_WORD:
	TRACE(("Select_WORD\n"));
	if (okPosition(screen, &(ld.startSel), &(screen->startSel))) {
	    cclass = CClassOf(startSel);
	    do {
		--screen->startSel.col;
		if (screen->startSel.col < 0
		    && isPrevWrapped(startSel)) {
		    PrevRow(startSel);
		    screen->startSel.col = LastTextCol(screen, ld.startSel, screen->startSel.row);
		}
	    } while (screen->startSel.col >= 0
		     && CClassSelects(startSel, cclass));
	    ++screen->startSel.col;
	}
#if OPT_WIDE_CHARS
	if (screen->startSel.col
	    && XTERM_CELL(screen->startSel.row,
			  screen->startSel.col) == HIDDEN_CHAR)
	    screen->startSel.col++;
#endif

	if (okPosition(screen, &(ld.endSel), &(screen->endSel))) {
	    length = LastTextCol(screen, ld.endSel, screen->endSel.row);
	    cclass = CClassOf(endSel);
	    do {
		++screen->endSel.col;
		if (screen->endSel.col > length
		    && LineTstWrapped(ld.endSel)) {
		    if (!MoreRows(endSel))
			break;
		    screen->endSel.col = 0;
		    NextRow(endSel);
		    length = LastTextCol(screen, ld.endSel, screen->endSel.row);
		}
	    } while (screen->endSel.col <= length
		     && CClassSelects(endSel, cclass));
	    /* Word-select selects if pointing to any char in "word",
	     * especially note that it includes the last character in a word.
	     * So we do no --endSel.col and do special eol handling.
	     */
	    if (screen->endSel.col > length + 1
		&& MoreRows(endSel)) {
		screen->endSel.col = 0;
		NextRow(endSel);
	    }
	}
#if OPT_WIDE_CHARS
	if (screen->endSel.col
	    && XTERM_CELL(screen->endSel.row,
			  screen->endSel.col) == HIDDEN_CHAR)
	    screen->endSel.col++;
#endif

	screen->saveStartW = screen->startSel;
	break;

    case Select_LINE:
	TRACE(("Select_LINE\n"));
	while (LineTstWrapped(ld.endSel)
	       && MoreRows(endSel)) {
	    NextRow(endSel);
	}
	if (screen->cutToBeginningOfLine
	    || screen->startSel.row < screen->saveStartW.row) {
	    screen->startSel.col = 0;
	    while (isPrevWrapped(startSel)) {
		PrevRow(startSel);
	    }
	} else if (!extend) {
	    if ((first.row < screen->saveStartW.row)
		|| (isSameRow(&first, &(screen->saveStartW))
		    && first.col < screen->saveStartW.col)) {
		screen->startSel.col = 0;
		while (isPrevWrapped(startSel)) {
		    PrevRow(startSel);
		}
	    } else {
		screen->startSel = screen->saveStartW;
	    }
	}
	trimLastLine(screen, &(ld.endSel), &(screen->endSel));
	break;

    case Select_GROUP:		/* paragraph */
	TRACE(("Select_GROUP\n"));
	if (okPosition(screen, &(ld.startSel), &(screen->startSel))) {
	    /* scan backward for beginning of group */
	    while (screen->startSel.row > 0 &&
		   (LastTextCol(screen, ld.startSel, screen->startSel.row -
				1) > 0 ||
		    isPrevWrapped(startSel))) {
		PrevRow(startSel);
	    }
	    screen->startSel.col = 0;
	    /* scan forward for end of group */
	    while (MoreRows(endSel) &&
		   (LastTextCol(screen, ld.endSel, screen->endSel.row + 1) >
		    0 ||
		    LineTstWrapped(ld.endSel))) {
		NextRow(endSel);
	    }
	    trimLastLine(screen, &(ld.endSel), &(screen->endSel));
	}
	break;

    case Select_PAGE:		/* everything one can see */
	TRACE(("Select_PAGE\n"));
	screen->startSel.row = 0;
	screen->startSel.col = 0;
	screen->endSel.row = MaxRows(screen);
	screen->endSel.col = 0;
	break;

    case Select_ALL:		/* counts scrollback if in normal screen */
	TRACE(("Select_ALL\n"));
	screen->startSel.row = -screen->savedlines;
	screen->startSel.col = 0;
	screen->endSel.row = MaxRows(screen);
	screen->endSel.col = 0;
	break;

#if OPT_SELECT_REGEX
    case Select_REGEX:
	do_select_regex(screen, &(screen->startSel), &(screen->endSel));
	break;
#endif

    case NSELECTUNITS:		/* always ignore */
	ignored = True;
	break;
    }

    if (!ignored) {
	/* check boundaries */
	ScrollSelection(screen, 0, False);
	TrackText(xw, &(screen->startSel), &(screen->endSel));
    }

    return;
}

/* Guaranteed (first.row, first.col) <= (last.row, last.col) */
static void
TrackText(XtermWidget xw,
	  const CELL * firstp,
	  const CELL * lastp)
{
    TScreen *screen = TScreenOf(xw);
    int from, to;
    CELL old_start, old_end;
    CELL first = *firstp;
    CELL last = *lastp;

    TRACE(("TrackText(first=%d,%d, last=%d,%d)\n",
	   first.row, first.col, last.row, last.col));

    old_start = screen->startH;
    old_end = screen->endH;
    if (isSameCELL(&first, &old_start) &&
	isSameCELL(&last, &old_end))
	return;
    screen->startH = first;
    screen->endH = last;
    from = Coordinate(screen, &screen->startH);
    to = Coordinate(screen, &screen->endH);
    if (to <= screen->startHCoord || from > screen->endHCoord) {
	/* No overlap whatsoever between old and new hilite */
	ReHiliteText(xw, &old_start, &old_end);
	ReHiliteText(xw, &first, &last);
    } else {
	if (from < screen->startHCoord) {
	    /* Extend left end */
	    ReHiliteText(xw, &first, &old_start);
	} else if (from > screen->startHCoord) {
	    /* Shorten left end */
	    ReHiliteText(xw, &old_start, &first);
	}
	if (to > screen->endHCoord) {
	    /* Extend right end */
	    ReHiliteText(xw, &old_end, &last);
	} else if (to < screen->endHCoord) {
	    /* Shorten right end */
	    ReHiliteText(xw, &last, &old_end);
	}
    }
    screen->startHCoord = from;
    screen->endHCoord = to;
}

/* Guaranteed that (first->row, first->col) <= (last->row, last->col) */
static void
ReHiliteText(XtermWidget xw,
	     CELL * firstp,
	     CELL * lastp)
{
    TScreen *screen = TScreenOf(xw);
    int i;
    CELL first = *firstp;
    CELL last = *lastp;

    TRACE(("ReHiliteText from %d.%d to %d.%d\n",
	   first.row, first.col, last.row, last.col));

    if (first.row < 0)
	first.row = first.col = 0;
    else if (first.row > screen->max_row)
	return;			/* nothing to do, since last.row >= first.row */

    if (last.row < 0)
	return;			/* nothing to do, since first.row <= last.row */
    else if (last.row > screen->max_row) {
	last.row = screen->max_row;
	last.col = MaxCols(screen);
    }
    if (isSameCELL(&first, &last))
	return;

    if (!isSameRow(&first, &last)) {	/* do multiple rows */
	if ((i = screen->max_col - first.col + 1) > 0) {	/* first row */
	    ScrnRefresh(xw, first.row, first.col, 1, i, True);
	}
	if ((i = last.row - first.row - 1) > 0) {	/* middle rows */
	    ScrnRefresh(xw, first.row + 1, 0, i, MaxCols(screen), True);
	}
	if (last.col > 0 && last.row <= screen->max_row) {	/* last row */
	    ScrnRefresh(xw, last.row, 0, 1, last.col, True);
	}
    } else {			/* do single row */
	ScrnRefresh(xw, first.row, first.col, 1, last.col - first.col, True);
    }
}

/*
 * Guaranteed that (cellc->row, cellc->col) <= (cell->row, cell->col), and that both points are valid
 * (may have cell->row = screen->max_row+1, cell->col = 0).
 */
static void
SaltTextAway(XtermWidget xw,
	     CELL * cellc,
	     CELL * cell,
	     String * params,	/* selections */
	     Cardinal num_params)
{
    TScreen *screen = TScreenOf(xw);
    int i, j = 0;
    int eol;
    int tmp;
    Char *line;
    Char *lp;
    CELL first = *cellc;
    CELL last = *cell;

    if (isSameRow(&first, &last) && first.col > last.col) {
	EXCHANGE(first.col, last.col, tmp);
    }

    --last.col;
    /* first we need to know how long the string is before we can save it */

    if (isSameRow(&last, &first)) {
	j = Length(screen, first.row, first.col, last.col);
    } else {			/* two cases, cut is on same line, cut spans multiple lines */
	j += Length(screen, first.row, first.col, screen->max_col) + 1;
	for (i = first.row + 1; i < last.row; i++)
	    j += Length(screen, i, 0, screen->max_col) + 1;
	if (last.col >= 0)
	    j += Length(screen, last.row, 0, last.col);
    }

    /* UTF-8 may require more space */
    if_OPT_WIDE_CHARS(screen, {
	j *= 4;
    });

    /* now get some memory to save it in */

    if (screen->selection_size <= j) {
	if ((line = (Char *) malloc((size_t) j + 1)) == 0)
	    SysError(ERROR_BMALLOC2);
	XtFree((char *) screen->selection_data);
	screen->selection_data = line;
	screen->selection_size = j + 1;
    } else {
	line = screen->selection_data;
    }

    if ((line == 0)
	|| (j < 0))
	return;

    line[j] = '\0';		/* make sure it is null terminated */
    lp = line;			/* lp points to where to save the text */
    if (isSameRow(&last, &first)) {
	lp = SaveText(screen, last.row, first.col, last.col, lp, &eol);
    } else {
	lp = SaveText(screen, first.row, first.col, screen->max_col, lp, &eol);
	if (eol)
	    *lp++ = '\n';	/* put in newline at end of line */
	for (i = first.row + 1; i < last.row; i++) {
	    lp = SaveText(screen, i, 0, screen->max_col, lp, &eol);
	    if (eol)
		*lp++ = '\n';
	}
	if (last.col >= 0)
	    lp = SaveText(screen, last.row, 0, last.col, lp, &eol);
    }
    *lp = '\0';			/* make sure we have end marked */

    TRACE(("Salted TEXT:%d:%s\n", (int) (lp - line),
	   visibleChars(line, (unsigned) (lp - line))));

    screen->selection_length = (unsigned long) (lp - line);
    _OwnSelection(xw, params, num_params);
}

#if OPT_PASTE64
void
ClearSelectionBuffer(TScreen * screen)
{
    screen->selection_length = 0;
    screen->base64_count = 0;
}

static void
AppendStrToSelectionBuffer(TScreen * screen, Char * text, size_t len)
{
    if (len != 0) {
	int j = (int) (screen->selection_length + len);		/* New length */
	int k = j + (j >> 2) + 80;	/* New size if we grow buffer: grow by ~50% */
	if (j + 1 >= screen->selection_size) {
	    if (!screen->selection_length) {
		/* New buffer */
		Char *line;
		if ((line = (Char *) malloc((size_t) k)) == 0)
		    SysError(ERROR_BMALLOC2);
		XtFree((char *) screen->selection_data);
		screen->selection_data = line;
	    } else {
		/* Realloc buffer */
		screen->selection_data = (Char *)
		    realloc(screen->selection_data,
			    (size_t) k);
		if (screen->selection_data == 0)
		    SysError(ERROR_BMALLOC2);
	    }
	    screen->selection_size = k;
	}
	if (screen->selection_data != 0) {
	    memcpy(screen->selection_data + screen->selection_length, text, len);
	    screen->selection_length += len;
	    screen->selection_data[screen->selection_length] = 0;
	}
    }
}

void
AppendToSelectionBuffer(TScreen * screen, unsigned c)
{
    unsigned six;
    Char ch;

    /* Decode base64 character */
    if (c >= 'A' && c <= 'Z')
	six = c - 'A';
    else if (c >= 'a' && c <= 'z')
	six = c - 'a' + 26;
    else if (c >= '0' && c <= '9')
	six = c - '0' + 52;
    else if (c == '+')
	six = 62;
    else if (c == '/')
	six = 63;
    else
	return;

    /* Accumulate bytes */
    switch (screen->base64_count) {
    case 0:
	screen->base64_accu = six;
	screen->base64_count = 6;
	break;

    case 2:
	ch = CharOf((screen->base64_accu << 6) + six);
	screen->base64_count = 0;
	AppendStrToSelectionBuffer(screen, &ch, (size_t) 1);
	break;

    case 4:
	ch = CharOf((screen->base64_accu << 4) + (six >> 2));
	screen->base64_accu = (six & 0x3);
	screen->base64_count = 2;
	AppendStrToSelectionBuffer(screen, &ch, (size_t) 1);
	break;

    case 6:
	ch = CharOf((screen->base64_accu << 2) + (six >> 4));
	screen->base64_accu = (six & 0xF);
	screen->base64_count = 4;
	AppendStrToSelectionBuffer(screen, &ch, (size_t) 1);
	break;
    }
}

void
CompleteSelection(XtermWidget xw, String * args, Cardinal len)
{
    TScreen *screen = TScreenOf(xw);

    screen->base64_count = 0;
    screen->base64_accu = 0;
    _OwnSelection(xw, args, len);
}
#endif /* OPT_PASTE64 */

static Bool
_ConvertSelectionHelper(Widget w,
			Atom * type,
			XtPointer *value,
			unsigned long *length,
			int *format,
			int (*conversion_function) (Display *,
						    char **, int,
						    XICCEncodingStyle,
						    XTextProperty *),
			XICCEncodingStyle conversion_style)
{
    XtermWidget xw;

    if ((xw = getXtermWidget(w)) != 0) {
	TScreen *screen = TScreenOf(xw);
	Display *dpy = XtDisplay(w);
	XTextProperty textprop;
	char *the_data = (char *) screen->selection_data;

	if (conversion_function(dpy, &the_data, 1,
				conversion_style,
				&textprop) >= Success) {
	    *value = (XtPointer) textprop.value;
	    *length = textprop.nitems;
	    *type = textprop.encoding;
	    *format = textprop.format;
	    return True;
	}
    }
    return False;
}

static Boolean
SaveConvertedLength(XtPointer *target, unsigned long source)
{
    Boolean result = False;

    *target = XtMalloc(4);
    if (*target != 0) {
	result = True;
	if (sizeof(unsigned long) == 4) {
	    *(unsigned long *) *target = source;
	} else if (sizeof(unsigned) == 4) {
	    *(unsigned *) *target = (unsigned) source;
	} else if (sizeof(unsigned short) == 4) {
	    *(unsigned short *) *target = (unsigned short) source;
	} else {
	    /* FIXME - does this depend on byte-order? */
	    unsigned long temp = source;
	    memcpy((char *) *target,
		   ((char *) &temp) + sizeof(temp) - 4,
		   (size_t) 4);
	}
    }
    return result;
}

static Boolean
ConvertSelection(Widget w,
		 Atom * selection,
		 Atom * target,
		 Atom * type,
		 XtPointer *value,
		 unsigned long *length,
		 int *format)
{
    Display *dpy = XtDisplay(w);
    TScreen *screen;
    Bool result = False;

    XtermWidget xw;

    if ((xw = getXtermWidget(w)) == 0)
	return False;

    screen = TScreenOf(xw);

    if (screen->selection_data == NULL)
	return False;		/* can this happen? */

    TRACE(("ConvertSelection %s\n",
	   visibleSelectionTarget(dpy, *target)));

    if (*target == XA_TARGETS(dpy)) {
	Atom *allocP;
	Atom *targetP;
	Atom *std_targets;
	XPointer std_return = 0;
	unsigned long std_length;

	if (XmuConvertStandardSelection(w, screen->selection_time, selection,
					target, type, &std_return,
					&std_length, format)) {
	    Atom *my_targets = _SelectionTargets(w);

	    TRACE(("XmuConvertStandardSelection - success\n"));
	    std_targets = (Atom *) (std_return);
	    *length = std_length + 6;

	    targetP = (Atom *) XtMalloc((Cardinal) (sizeof(Atom) * (*length)));
	    allocP = targetP;

	    *value = (XtPointer) targetP;

	    while (*my_targets != None) {
		*targetP++ = *my_targets++;
	    }
	    *targetP++ = XA_LENGTH(dpy);
	    *targetP++ = XA_LIST_LENGTH(dpy);

	    *length = std_length + (unsigned long) (targetP - allocP);

	    memcpy(targetP, std_targets, sizeof(Atom) * std_length);
	    XtFree((char *) std_targets);
	    *type = XA_ATOM;
	    *format = 32;
	    result = True;
	} else {
	    TRACE(("XmuConvertStandardSelection - failed\n"));
	}
    }
#if OPT_WIDE_CHARS
    else if (screen->wide_chars && *target == XA_STRING) {
	result =
	    _ConvertSelectionHelper(w,
				    type, value, length, format,
				    Xutf8TextListToTextProperty,
				    XStringStyle);
	TRACE(("...Xutf8TextListToTextProperty:%d\n", result));
    } else if (screen->wide_chars && *target == XA_UTF8_STRING(dpy)) {
	result =
	    _ConvertSelectionHelper(w,
				    type, value, length, format,
				    Xutf8TextListToTextProperty,
				    XUTF8StringStyle);
	TRACE(("...Xutf8TextListToTextProperty:%d\n", result));
    } else if (screen->wide_chars && *target == XA_TEXT(dpy)) {
	result =
	    _ConvertSelectionHelper(w,
				    type, value, length, format,
				    Xutf8TextListToTextProperty,
				    XStdICCTextStyle);
	TRACE(("...Xutf8TextListToTextProperty:%d\n", result));
    } else if (screen->wide_chars && *target == XA_COMPOUND_TEXT(dpy)) {
	result =
	    _ConvertSelectionHelper(w,
				    type, value, length, format,
				    Xutf8TextListToTextProperty,
				    XCompoundTextStyle);
	TRACE(("...Xutf8TextListToTextProperty:%d\n", result));
    }
#endif

    else if (*target == XA_STRING) {	/* not wide_chars */
	/* We can only reach this point if the selection requestor
	   requested STRING before any of TEXT, COMPOUND_TEXT or
	   UTF8_STRING.  We therefore assume that the requestor is not
	   properly internationalised, and dump raw eight-bit data
	   with no conversion into the selection.  Yes, this breaks
	   the ICCCM in non-Latin-1 locales. */
	*type = XA_STRING;
	*value = (XtPointer) screen->selection_data;
	*length = screen->selection_length;
	*format = 8;
	result = True;
	TRACE(("...raw 8-bit data:%d\n", result));
    } else if (*target == XA_TEXT(dpy)) {	/* not wide_chars */
	result =
	    _ConvertSelectionHelper(w,
				    type, value, length, format,
				    XmbTextListToTextProperty,
				    XStdICCTextStyle);
	TRACE(("...XmbTextListToTextProperty(StdICC):%d\n", result));
    } else if (*target == XA_COMPOUND_TEXT(dpy)) {	/* not wide_chars */
	result =
	    _ConvertSelectionHelper(w,
				    type, value, length, format,
				    XmbTextListToTextProperty,
				    XCompoundTextStyle);
	TRACE(("...XmbTextListToTextProperty(Compound):%d\n", result));
    }
#ifdef X_HAVE_UTF8_STRING
    else if (*target == XA_UTF8_STRING(dpy)) {	/* not wide_chars */
	result =
	    _ConvertSelectionHelper(w,
				    type, value, length, format,
				    XmbTextListToTextProperty,
				    XUTF8StringStyle);
	TRACE(("...XmbTextListToTextProperty(UTF8):%d\n", result));
    }
#endif
    else if (*target == XA_LIST_LENGTH(dpy)) {
	result = SaveConvertedLength(value, (unsigned long) 1);
	*type = XA_INTEGER;
	*length = 1;
	*format = 32;
	TRACE(("...list of values:%d\n", result));
    } else if (*target == XA_LENGTH(dpy)) {
	/* This value is wrong if we have UTF-8 text */
	result = SaveConvertedLength(value, screen->selection_length);
	*type = XA_INTEGER;
	*length = 1;
	*format = 32;
	TRACE(("...list of values:%d\n", result));
    } else if (XmuConvertStandardSelection(w,
					   screen->selection_time, selection,
					   target, type, (XPointer *) value,
					   length, format)) {
	result = True;
	TRACE(("...XmuConvertStandardSelection:%d\n", result));
    }

    /* else */
    return (Boolean) result;
}

static void
LoseSelection(Widget w, Atom * selection)
{
    TScreen *screen;
    Atom *atomP;
    Cardinal i;

    XtermWidget xw;

    if ((xw = getXtermWidget(w)) == 0)
	return;

    screen = TScreenOf(xw);
    for (i = 0, atomP = screen->selection_atoms;
	 i < screen->selection_count; i++, atomP++) {
	if (*selection == *atomP)
	    *atomP = (Atom) 0;
	if (CutBuffer(*atomP) >= 0) {
	    *atomP = (Atom) 0;
	}
    }

    for (i = screen->selection_count; i; i--) {
	if (screen->selection_atoms[i - 1] != 0)
	    break;
    }
    screen->selection_count = i;

    for (i = 0, atomP = screen->selection_atoms;
	 i < screen->selection_count; i++, atomP++) {
	if (*atomP == (Atom) 0) {
	    *atomP = screen->selection_atoms[--screen->selection_count];
	}
    }

    if (screen->selection_count == 0)
	TrackText(xw, &zeroCELL, &zeroCELL);
}

/* ARGSUSED */
static void
SelectionDone(Widget w GCC_UNUSED,
	      Atom * selection GCC_UNUSED,
	      Atom * target GCC_UNUSED)
{
    /* empty proc so Intrinsics know we want to keep storage */
}

static void
_OwnSelection(XtermWidget xw,
	      String * selections,
	      Cardinal count)
{
    TScreen *screen = TScreenOf(xw);
    Atom *atoms = screen->selection_atoms;
    Cardinal i;
    Bool have_selection = False;

    if (count == 0)
	return;
    if (screen->selection_length == 0)
	return;

    TRACE(("_OwnSelection count %d\n", count));
    selections = MapSelections(xw, selections, count);

    if (count > screen->sel_atoms_size) {
	XtFree((char *) atoms);
	atoms = (Atom *) XtMalloc((Cardinal) (count * sizeof(Atom)));
	screen->selection_atoms = atoms;
	screen->sel_atoms_size = count;
    }
    XmuInternStrings(XtDisplay((Widget) xw), selections, count, atoms);
    for (i = 0; i < count; i++) {
	int cutbuffer = CutBuffer(atoms[i]);
	if (cutbuffer >= 0) {
	    unsigned long limit =
	    (unsigned long) (4 * XMaxRequestSize(XtDisplay((Widget) xw)) - 32);
	    if (screen->selection_length > limit) {
		TRACE(("selection too big (%lu bytes), not storing in CUT_BUFFER%d\n",
		       screen->selection_length, cutbuffer));
		fprintf(stderr,
			"%s: selection too big (%lu bytes), not storing in CUT_BUFFER%d\n",
			xterm_name, screen->selection_length, cutbuffer);
	    } else {
		/* This used to just use the UTF-8 data, which was totally
		 * broken as not even the corresponding paste code in Xterm
		 * understood this!  So now it converts to Latin1 first.
		 *   Robert Brady, 2000-09-05
		 */
		unsigned long length = screen->selection_length;
		Char *data = screen->selection_data;
		if_OPT_WIDE_CHARS((screen), {
		    data = UTF8toLatin1(screen, data, length, &length);
		});
		TRACE(("XStoreBuffer(%d)\n", cutbuffer));
		XStoreBuffer(XtDisplay((Widget) xw),
			     (char *) data,
			     (int) length,
			     cutbuffer);
	    }
	} else if (!screen->replyToEmacs) {
	    have_selection |=
		XtOwnSelection((Widget) xw, atoms[i],
			       screen->selection_time,
			       ConvertSelection, LoseSelection, SelectionDone);
	}
    }
    if (!screen->replyToEmacs)
	screen->selection_count = count;
    if (!have_selection)
	TrackText(xw, &zeroCELL, &zeroCELL);
}

static void
ResetSelectionState(TScreen * screen)
{
    screen->selection_count = 0;
    screen->startH = zeroCELL;
    screen->endH = zeroCELL;
}

void
DisownSelection(XtermWidget xw)
{
    TScreen *screen = TScreenOf(xw);
    Atom *atoms = screen->selection_atoms;
    Cardinal count = screen->selection_count;
    Cardinal i;

    TRACE(("DisownSelection count %d, start %d.%d, end %d.%d\n",
	   count,
	   screen->startH.row,
	   screen->startH.col,
	   screen->endH.row,
	   screen->endH.col));

    for (i = 0; i < count; i++) {
	int cutbuffer = CutBuffer(atoms[i]);
	if (cutbuffer < 0) {
	    XtDisownSelection((Widget) xw, atoms[i],
			      screen->selection_time);
	}
    }
    /*
     * If none of the callbacks via XtDisownSelection() reset highlighting
     * do it now.
     */
    if (ScrnHaveSelection(screen)) {
	/* save data which will be reset */
	CELL first = screen->startH;
	CELL last = screen->endH;

	ResetSelectionState(screen);
	ReHiliteText(xw, &first, &last);
    } else {
	ResetSelectionState(screen);
    }
}

void
UnhiliteSelection(XtermWidget xw)
{
    TScreen *screen = TScreenOf(xw);

    if (ScrnHaveSelection(screen)) {
	CELL first = screen->startH;
	CELL last = screen->endH;

	screen->startH = zeroCELL;
	screen->endH = zeroCELL;
	ReHiliteText(xw, &first, &last);
    }
}

/* returns number of chars in line from scol to ecol out */
/* ARGSUSED */
static int
Length(TScreen * screen,
       int row,
       int scol,
       int ecol)
{
    LineData *ld = GET_LINEDATA(screen, row);
    int lastcol = LastTextCol(screen, ld, row);

    if (ecol > lastcol)
	ecol = lastcol;
    return (ecol - scol + 1);
}

/* copies text into line, preallocated */
static Char *
SaveText(TScreen * screen,
	 int row,
	 int scol,
	 int ecol,
	 Char * lp,		/* pointer to where to put the text */
	 int *eol)
{
    LineData *ld;
    int i = 0;
    unsigned c;
    Char *result = lp;
#if OPT_WIDE_CHARS
    unsigned previous = 0;
#endif

    ld = GET_LINEDATA(screen, row);
    i = Length(screen, row, scol, ecol);
    ecol = scol + i;
#if OPT_DEC_CHRSET
    if (CSET_DOUBLE(GetLineDblCS(ld))) {
	scol = (scol + 0) / 2;
	ecol = (ecol + 1) / 2;
    }
#endif
    *eol = !LineTstWrapped(ld);
    for (i = scol; i < ecol; i++) {
	assert(i < ld->lineSize);
	c = E2A(ld->charData[i]);
#if OPT_WIDE_CHARS
	/* We want to strip out every occurrence of HIDDEN_CHAR AFTER a
	 * wide character.
	 */
	if (c == HIDDEN_CHAR && isWide((int) previous)) {
	    previous = c;
	    /* Combining characters attached to double-width characters
	       are in memory attached to the HIDDEN_CHAR */
	    if_OPT_WIDE_CHARS(screen, {
		if (screen->utf8_mode != uFalse) {
		    unsigned ch;
		    size_t off;
		    for_each_combData(off, ld) {
			ch = ld->combData[off][i];
			if (ch == 0)
			    break;
			lp = convertToUTF8(lp, ch);
		    }
		}
	    });
	    continue;
	}
	previous = c;
	if (screen->utf8_mode != uFalse) {
	    lp = convertToUTF8(lp, (c != 0) ? c : ' ');
	    if_OPT_WIDE_CHARS(screen, {
		unsigned ch;
		size_t off;
		for_each_combData(off, ld) {
		    ch = ld->combData[off][i];
		    if (ch == 0)
			break;
		    lp = convertToUTF8(lp, ch);
		}
	    });
	} else
#endif
	{
	    if (c == 0) {
		c = E2A(' ');
	    } else if (c < E2A(' ')) {
		c = DECtoASCII(c);
	    } else if (c == 0x7f) {
		c = 0x5f;
	    }
	    *lp++ = CharOf(A2E(c));
	}
	if (c != E2A(' '))
	    result = lp;
    }

    /*
     * If requested, trim trailing blanks from selected lines.  Do not do this
     * if the line is wrapped.
     */
    if (!*eol || !screen->trim_selection)
	result = lp;

    return (result);
}

/* 32 + following 7-bit word:

   1:0  Button no: 0, 1, 2.  3=release.
     2  shift
     3  meta
     4  ctrl
     5  set for motion notify
     6  set for wheel
*/

/* Position: 32 - 255. */

static Char
BtnCode(XButtonEvent * event, int button)
{
    int result = (int) (32 + (KeyState(event->state) << 2));

    if (button < 0 || button > 5) {
	result += 3;
    } else {
	if (button > 3)
	    result += (64 - 4);
	if (event->type == MotionNotify)
	    result += 32;
	result += button;
    }
    return CharOf(result);
}

static void
EditorButton(XtermWidget xw, XButtonEvent * event)
{
    TScreen *screen = TScreenOf(xw);
    int pty = screen->respond;
    int mouse_limit = screen->ext_mode_mouse ? EXT_MOUSE_LIMIT : MOUSE_LIMIT;
    Char line[10];
    int row, col;
    int button;
    unsigned count = 0;
    Boolean changed = True;

    /* If button event, get button # adjusted for DEC compatibility */
    button = (int) (event->button - 1);
    if (button >= 3)
	button++;

    /* Compute character position of mouse pointer */
    row = (event->y - screen->border) / FontHeight(screen);
    col = (event->x - OriginX(screen)) / FontWidth(screen);

    /* Limit to screen dimensions */
    if (row < 0)
	row = 0;
    else if (row > screen->max_row)
	row = screen->max_row;

    if (col < 0)
	col = 0;
    else if (col > screen->max_col)
	col = screen->max_col;

    /* Limit to representable mouse dimensions */
    if (row > mouse_limit)
	row = mouse_limit;
    if (col > mouse_limit)
	col = mouse_limit;

    /* Build key sequence starting with \E[M */
    if (screen->control_eight_bits) {
	line[count++] = ANSI_CSI;
    } else {
	line[count++] = ANSI_ESC;
	line[count++] = '[';
    }
#if OPT_SCO_FUNC_KEYS
    if (xw->keyboard.type == keyboardIsSCO) {
	/*
	 * SCO function key F1 is \E[M, which would conflict with xterm's
	 * normal kmous.
	 */
	line[count++] = '>';
    }
#endif
    line[count++] = 'M';

    /* Add event code to key sequence */
    if (screen->send_mouse_pos == X10_MOUSE) {
	line[count++] = CharOf(' ' + button);
    } else {
	/* Button-Motion events */
	switch (event->type) {
	case ButtonPress:
	    line[count++] = BtnCode(event, screen->mouse_button = button);
	    break;
	case ButtonRelease:
	    /*
	     * Wheel mouse interface generates release-events for buttons
	     * 4 and 5, coded here as 3 and 4 respectively.  We change the
	     * release for buttons 1..3 to a -1.
	     */
	    if (button < 3)
		button = -1;
	    line[count++] = BtnCode(event, screen->mouse_button = button);
	    break;
	case MotionNotify:
	    /* BTN_EVENT_MOUSE and ANY_EVENT_MOUSE modes send motion
	     * events only if character cell has changed.
	     */
	    if ((row == screen->mouse_row)
		&& (col == screen->mouse_col)) {
		changed = False;
	    } else {
		line[count++] = BtnCode(event, screen->mouse_button);
	    }
	    break;
	default:
	    changed = False;
	    break;
	}
    }

    if (changed) {
	screen->mouse_row = row;
	screen->mouse_col = col;

	TRACE(("mouse at %d,%d button+mask = %#x\n", row, col, line[count - 1]));

	/* Add pointer position to key sequence */
	count = EmitMousePosition(screen, line, count, col);
	count = EmitMousePosition(screen, line, count, row);

	/* Transmit key sequence to process running under xterm */
	v_write(pty, line, count);
    }
    return;
}

#if OPT_FOCUS_EVENT
void
SendFocusButton(XtermWidget xw, XFocusChangeEvent * event)
{
    TScreen *screen = TScreenOf(xw);

    if (screen->send_focus_pos) {
	ANSI reply;

	memset(&reply, 0, sizeof(reply));
	reply.a_type = ANSI_CSI;

#if OPT_SCO_FUNC_KEYS
	if (xw->keyboard.type == keyboardIsSCO) {
	    reply.a_pintro = '>';
	}
#endif
	reply.a_final = CharOf((event->type == FocusIn) ? 'I' : 'O');
	unparseseq(xw, &reply);
    }
    return;
}
#endif /* OPT_FOCUS_EVENT */
