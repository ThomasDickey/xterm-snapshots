/* $XTermId: button.c,v 1.672 2025/05/15 23:19:08 tom Exp $ */

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
#include <ctype.h>
#include <assert.h>

#include <X11/Xatom.h>
#include <X11/Xmu/Atoms.h>
#include <X11/Xmu/StdSel.h>

#include <xutf8.h>
#include <fontutils.h>

#include <data.h>
#include <error.h>
#include <menu.h>
#include <charclass.h>
#include <xstrings.h>
#include <xterm_io.h>

#if OPT_SELECT_REGEX
#if defined(HAVE_PCRE2POSIX_H)
#include <pcre2posix.h>

/* pcre2 used to provide its "POSIX" entrypoints using the same names as the
 * standard ones in the C runtime, but that never worked because the linker
 * would use the C runtime.  Debian patched the library to fix this symbol
 * conflict, but overlooked the header file, and Debian's patch was made
 * obsolete when pcre2 was changed early in 2019 to provide different names.
 *
 * Here is a workaround to make the older version of Debian's package work.
 */
#if !defined(PCRE2regcomp) && defined(HAVE_PCRE2REGCOMP)

#undef regcomp
#undef regexec
#undef regfree

#ifdef __cplusplus
extern "C" {
#endif
    PCRE2POSIX_EXP_DECL int PCRE2regcomp(regex_t *, const char *, int);
    PCRE2POSIX_EXP_DECL int PCRE2regexec(const regex_t *, const char *, size_t,
					 regmatch_t *, int);
    PCRE2POSIX_EXP_DECL void PCRE2regfree(regex_t *);
#ifdef __cplusplus
}				/* extern "C" */
#endif
#define regcomp(r,s,n)          PCRE2regcomp(r,s,n)
#define regexec(r,s,n,m,x)      PCRE2regexec(r,s,n,m,x)
#define regfree(r)              PCRE2regfree(r)
#endif
/* end workaround... */
#elif defined(HAVE_PCREPOSIX_H)
#include <pcreposix.h>
#else /* POSIX regex.h */
#include <sys/types.h>
#include <regex.h>
#endif
#endif /* OPT_SELECT_REGEX */

#ifdef HAVE_X11_TRANSLATEI_H
#include <X11/ConvertI.h>
#include <X11/TranslateI.h>
#else
extern String _XtPrintXlations(Widget w,
			       XtTranslations xlations,
			       Widget accelWidget,
			       _XtBoolean includeRHS);
#endif

#define PRIMARY_NAME    "PRIMARY"
#define CLIPBOARD_NAME  "CLIPBOARD"
#define SECONDARY_NAME  "SECONDARY"

#define AtomToSelection(d,n) \
		 (((n) == XA_CLIPBOARD(d)) \
		  ? CLIPBOARD_CODE \
		  : (((n) == XA_SECONDARY) \
		     ? SECONDARY_CODE \
		     : PRIMARY_CODE))

#define isSelectionCode(n) ((n) >= PRIMARY_CODE)
#define CutBufferToCode(n) ((n) +  MAX_SELECTION_CODES)
#define okSelectionCode(n) (isSelectionCode(n) ? (n) : PRIMARY_CODE)

#if OPT_WIDE_CHARS
#include <ctype.h>
#include <wcwidth.h>
#else
#define CharacterClass(value) \
	charClass[(value) & (int)((sizeof(charClass)/sizeof(charClass[0]))-1)]
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

#define MaxMouseBtn  5

#define IsBtnEvent(event) ((event)->type == ButtonPress || (event)->type == ButtonRelease)
#define IsKeyEvent(event) ((event)->type == KeyPress    || (event)->type == KeyRelease)

#define	Coordinate(s,c)	((c)->row * MaxCols(s) + (c)->col)

static const CELL zeroCELL =
{0, 0};

#if OPT_DEC_LOCATOR
static Bool SendLocatorPosition(XtermWidget xw, XButtonEvent *event);
static void CheckLocatorPosition(XtermWidget xw, XButtonEvent *event);
#endif /* OPT_DEC_LOCATOR */

/* Multi-click handling */
#if OPT_READLINE
static Time lastButtonDownTime = 0;
static int ExtendingSelection = 0;
static Time lastButton3UpTime = 0;
static Time lastButton3DoubleDownTime = 0;
static CELL lastButton3;	/* At the release time */
#endif /* OPT_READLINE */

static Char *SaveText(TScreen *screen, int row, int scol, int ecol,
		      Char *lp, int *eol);
static int Length(TScreen *screen, int row, int scol, int ecol);
static void ComputeSelect(XtermWidget xw, const CELL *startc, const CELL *endc,
			  Bool extend, Bool normal);
static void EditorButton(XtermWidget xw, XButtonEvent *event);
static void EndExtend(XtermWidget w, XEvent *event, String *params, Cardinal
		      num_params, Bool use_cursor_loc);
static void ExtendExtend(XtermWidget xw, const CELL *cell);
static void PointToCELL(TScreen *screen, int y, int x, CELL *cell);
static void ReHiliteText(XtermWidget xw, const CELL *first, const CELL *last);
static void SaltTextAway(XtermWidget xw, int which, const CELL *cellc, const CELL *cell);
static void SelectSet(XtermWidget xw, XEvent *event, String *params, Cardinal num_params);
static void SelectionReceived PROTO_XT_SEL_CB_ARGS;
static void StartSelect(XtermWidget xw, const CELL *cell);
static void TrackDown(XtermWidget xw, XButtonEvent *event);
static void TrackText(XtermWidget xw, const CELL *first, const CELL *last);
static void UnHiliteText(XtermWidget xw);
static void _OwnSelection(XtermWidget xw, String *selections, Cardinal count);
static void do_select_end(XtermWidget xw, XEvent *event, String *params,
			  Cardinal *num_params, Bool use_cursor_loc);

#define MOUSE_LIMIT (255 - 32)

/* Send SET_EXT_SIZE_MOUSE to enable offsets up to EXT_MOUSE_LIMIT */
#define EXT_MOUSE_LIMIT (2047 - 32)
#define EXT_MOUSE_START (127 - 32)

static int
MouseLimit(TScreen *screen)
{
    int mouse_limit;

    switch (screen->extend_coords) {
    default:
	mouse_limit = MOUSE_LIMIT;
	break;
    case SET_EXT_MODE_MOUSE:
	mouse_limit = EXT_MOUSE_LIMIT;
	break;
    case SET_SGR_EXT_MODE_MOUSE:
    case SET_URXVT_EXT_MODE_MOUSE:
    case SET_PIXEL_POSITION_MOUSE:
	mouse_limit = -1;
	break;
    }
    return mouse_limit;
}

static unsigned
EmitMousePosition(TScreen *screen, Char line[], unsigned count, int value)
{
    int mouse_limit = MouseLimit(screen);

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
    switch (screen->extend_coords) {
    default:
	if (value == mouse_limit) {
	    line[count++] = CharOf(0);
	} else {
	    line[count++] = CharOf(' ' + value + 1);
	}
	break;
    case SET_EXT_MODE_MOUSE:
	if (value == mouse_limit) {
	    line[count++] = CharOf(0);
	} else if (value < EXT_MOUSE_START) {
	    line[count++] = CharOf(' ' + value + 1);
	} else {
	    value += ' ' + 1;
	    line[count++] = CharOf(0xC0 + (value >> 6));
	    line[count++] = CharOf(0x80 + (value & 0x3F));
	}
	break;
    case SET_SGR_EXT_MODE_MOUSE:
    case SET_URXVT_EXT_MODE_MOUSE:
    case SET_PIXEL_POSITION_MOUSE:
	count += (unsigned) sprintf((char *) line + count, "%d", value + 1);
	break;
    }
    return count;
}

static unsigned
EmitMousePositionSeparator(TScreen *screen, Char line[], unsigned count)
{
    switch (screen->extend_coords) {
    case SET_SGR_EXT_MODE_MOUSE:
    case SET_URXVT_EXT_MODE_MOUSE:
    case SET_PIXEL_POSITION_MOUSE:
	line[count++] = ';';
	break;
    }
    return count;
}

enum {
    scanMods,
    scanKey,
    scanColon,
    scanFunc,
    scanArgs
};

#if OPT_TRACE > 1
static const char *
visibleScan(int mode)
{
    const char *result = "?";
#define DATA(name) case name: result = #name; break
    switch (mode) {
	DATA(scanMods);
	DATA(scanKey);
	DATA(scanColon);
	DATA(scanFunc);
	DATA(scanArgs);
    }
#undef DATA
    return result;
}
#endif

#define L_BRACK '<'
#define R_BRACK '>'
#define L_PAREN '('
#define R_PAREN ')'

static char *
scanTrans(char *source, int *this_is, int *next_is, unsigned *first, unsigned *last)
{
    char *target = source;

    *first = *last = 0;
    if (IsEmpty(target)) {
	target = NULL;
    } else {
	do {
	    char ch;
	    while (IsSpace(*target))
		target++;
	    *first = (unsigned) (target - source);
	    switch (*this_is = *next_is) {
	    case scanMods:
		while ((ch = *target)) {
		    if (IsSpace(ch)) {
			break;
		    } else if (ch == L_BRACK) {
			*next_is = scanKey;
			break;
		    } else if (ch == ':') {
			*next_is = scanColon;
			break;
		    } else if (ch == '~' && target != source) {
			break;
		    }
		    target++;
		}
		break;
	    case scanKey:
		while ((ch = *target)) {
		    if (IsSpace(ch)) {
			break;
		    } else if (ch == ':') {
			*next_is = scanColon;
			break;
		    }
		    target++;
		    if (ch == R_BRACK)
			break;
		}
		break;
	    case scanColon:
		*next_is = scanFunc;
		target++;
		break;
	    case scanFunc:
		while ((ch = *target)) {
		    if (IsSpace(ch)) {
			break;
		    } else if (ch == L_PAREN) {
			*next_is = scanArgs;
			break;
		    }
		    target++;
		}
		break;
	    case scanArgs:
		while ((ch = *target)) {
		    if (ch == R_PAREN) {
			target++;
			*next_is = scanFunc;
			break;
		    }
		    target++;
		}
		break;
	    }
	    *last = (unsigned) (target - source);
	    if (*target == '\n') {
		*next_is = scanMods;
		target++;
	    }
	} while (*first == *last);
    }
    return target;
}

void
xtermButtonInit(XtermWidget xw)
{
    Widget w = (Widget) xw;
    XErrorHandler save = XSetErrorHandler(ignore_x11_error);
    XtTranslations xlations;
    Widget xcelerat;
    String result;

    XtVaGetValues(w,
		  XtNtranslations, &xlations,
		  XtNaccelerators, &xcelerat,
		  (XtPointer) 0);
    result = _XtPrintXlations(w, xlations, xcelerat, True);
    if (result) {
	static const char *table[] =
	{
	    "insert-selection",
	    "select-end",
	    "select-extend",
	    "select-start",
	    "start-extend",
	};
	char *data = x_strdup(result);
	char *next;
	int state = scanMods;
	int state2 = scanMods;
	unsigned first;
	unsigned last;
	int have_button = -1;
	Bool want_button = False;
	Bool have_shift = False;
	unsigned allowed = 0;
	unsigned disallow = 0;

	TRACE(("xtermButtonInit length %ld\n", (long) strlen(result)));
	xw->keyboard.print_translations = data;
	while ((next = scanTrans(data, &state, &state2, &first, &last)) != NULL) {
	    unsigned len = (last - first);
	    TRACE2(("parse %s:%d..%d '%.*s'\n",
		    visibleScan(state), first, last,
		    len, data + first));
	    if (state == scanMods) {
		if (len > 1 && data[first] == '~') {
		    len--;
		    first++;
		}
		if (len == 7 && !x_strncasecmp(data + first, "button", len - 1)) {
		    have_button = data[first + 6] - '0';
		} else if (len == 5 && !x_strncasecmp(data + first, "shift", len)) {
		    have_shift = True;
		}
	    } else if (state == scanKey) {
		if (!x_strncasecmp(data + first, "<buttonpress>", len) ||
		    !x_strncasecmp(data + first, "<buttonrelease>", len)) {
		    want_button = True;
		} else if (want_button) {
		    have_button = data[first] - '0';
		    want_button = False;
		}
	    } else if (state == scanFunc && have_button > 0) {
		Cardinal n;
		unsigned bmask = 1U << (have_button - 1);
		for (n = 0; n < XtNumber(table); ++n) {
		    if (!x_strncasecmp(table[n], data + first, len)) {
			TRACE(("...button %d: %s%s\n",
			       have_button, table[n],
			       have_shift ? " (disallow)" : ""));
			if (have_shift)
			    disallow |= bmask;
			else
			    allowed |= bmask;
			break;
		    }
		}
	    }
	    if (state2 == scanMods && state >= scanColon) {
		have_button = -1;
		want_button = False;
		have_shift = False;
	    }
	    state = state2;
	    data = next;
	}
	XFree((char *) result);
	xw->keyboard.shift_buttons = allowed & ~disallow;
#if OPT_TRACE
	if (xw->keyboard.shift_buttons) {
	    int button = 0;
	    unsigned mask = xw->keyboard.shift_buttons;
	    TRACE(("...Buttons used for selection that can be overridden:"));
	    while (mask != 0) {
		++button;
		if ((mask & 1) != 0)
		    TRACE((" %d", button));
		mask >>= 1;
	    }
	    TRACE(("\n"));
	} else {
	    TRACE(("...No buttons used with selection can be overridden\n"));
	}
#endif
    }
    XSetErrorHandler(save);
}

/*
 * Shift and control are regular X11 modifiers, but meta is not:
 * + X10 (which had no xmodmap utility) had a meta mask, but X11 did not.
 * + X11R1 introduced xmodmap, along with the current set of modifier masks.
 *   The meta key has been assumed to be mod1 since X11R1.
 *   The initial xterm logic in X11 was different, but gave the same result.
 * + X11R2 modified xterm was to eliminate the X10 table which provided part of
 *   the meta logic.
 * + X11R3 modified Xt, making Meta_L and Meta_R assignable via xmodmap, and
 *   equating Alt with Meta.  Neither Alt/Meta are modifiers, but Alt is more
 *   likely to be on the keyboard.  This release also added keymap tables for
 *   the server; Meta was used frequently in HP keymaps, which were the most
 *   extensive set of keymaps.
 * + X11R4 mentions Meta in the ICCCM, stating that if Meta_L or Meta_R are
 *   found in the keysyms for a given modifier, that the client should use
 *   that modifier.
 *
 * This function follows the ICCCM, picking the modifier which contains the
 * Meta_L/Meta_R keysyms (if available), falling back to the Alt_L/Alt_R
 * (as per X11R3), and ultimately to mod1 (per X11R1).
 */
static unsigned
MetaMask(XtermWidget xw)
{
#if OPT_NUM_LOCK
    unsigned meta = xw->work.meta_mods;
    if (meta == 0)
	meta = xw->work.alt_mods;
    if (meta == 0)
	meta = Mod1Mask;
#else
    unsigned meta = Mod1Mask;
    (void) xw;
#endif
    return meta;
}

/*
 * Returns a mask of the modifiers we may use for modifying the mouse protocol
 * response strings.
 */
static unsigned
OurModifiers(XtermWidget xw)
{
    return (ShiftMask
	    | ControlMask
	    | MetaMask(xw));
}

/*
 * The actual check for the shift-mask, to see if it should tell xterm to
 * override mouse-protocol in favor of select/paste actions depends upon
 * whether the shiftEscape resource is set to true/always vs false/never.
 */
static Boolean
ShiftOverride(XtermWidget xw, unsigned state, int button)
{
    unsigned check = (state & OurModifiers(xw));
    Boolean result = False;

    if (check & ShiftMask) {
	if (xw->keyboard.shift_escape == ssFalse ||
	    xw->keyboard.shift_escape == ssNever) {
	    result = True;
	} else if (xw->keyboard.shift_escape == ssTrue) {
	    /*
	     * Check if the button is one that we found does not directly use
	     * the shift-modifier in its bindings to select/copy actions.
	     */
	    if (button > 0 && button <= MaxMouseBtn) {
		if (xw->keyboard.shift_buttons & (1U << (button - 1))) {
		    result = True;
		}
	    } else {
		result = True;	/* unlikely, and we don't care */
	    }
	}
    }
    TRACE2(("ShiftOverride ( %#x -> %#x ) %d\n", state, check, result));
    return result;
}

/*
 * Normally xterm treats the shift-modifier specially when the mouse protocol
 * is active.  The translations resource binds otherwise unmodified button
 * for these mouse-related events:
 *
 *         ~Meta <Btn1Down>:select-start() \n\
 *       ~Meta <Btn1Motion>:select-extend() \n\
 *     ~Ctrl ~Meta <Btn2Up>:insert-selection(SELECT, CUT_BUFFER0) \n\
 *   ~Ctrl ~Meta <Btn3Down>:start-extend() \n\
 *       ~Meta <Btn3Motion>:select-extend() \n\
 *                  <BtnUp>:select-end(SELECT, CUT_BUFFER0) \n\
 *
 * There is no API in the X libraries which would tell us if a given mouse
 * button is bound to one of these actions.  These functions make the choice
 * configurable.
 */
static Bool
InterpretButton(XtermWidget xw, XButtonEvent *event)
{
    Bool result = False;

    if (ShiftOverride(xw, event->state, (int) event->button)) {
	TRACE(("...shift-button #%d overrides mouse-protocol\n", event->button));
	result = True;
    }
    return result;
}

#define Button1Index 8		/* X.h should have done this */

static int
MotionButton(unsigned state)
{
    unsigned bmask = state >> Button1Index;
    int result = 1;

    if (bmask != 0) {
	while (!(bmask & 1)) {
	    ++result;
	    bmask >>= 1;
	}
    }
    return result;
}

static Bool
InterpretEvent(XtermWidget xw, XEvent *event)
{
    Bool result = False;	/* if not a button, is motion */

    if (IsBtnEvent(event)) {
	result = InterpretButton(xw, (XButtonEvent *) event);
    } else if (event->type == MotionNotify) {
	unsigned state = event->xmotion.state;
	int button = MotionButton(state);

	if (ShiftOverride(xw, state, button)) {
	    TRACE(("...shift-motion #%d (%d,%d) overrides mouse-protocol\n",
		   button,
		   event->xmotion.y,
		   event->xmotion.x));
	    result = True;
	}
    }
    return result;
}

#define OverrideEvent(event)  InterpretEvent(xw, event)
#define OverrideButton(event) InterpretButton(xw, event)

/*
 * Returns true if we handled the event here, and nothing more is needed.
 */
Bool
SendMousePosition(XtermWidget xw, XEvent *event)
{
    XButtonEvent *my_event = (XButtonEvent *) event;
    Bool result = False;

    switch (okSendMousePos(xw)) {
    case MOUSE_OFF:
	/* If send_mouse_pos mode isn't on, we shouldn't be here */
	break;

    case BTN_EVENT_MOUSE:
    case ANY_EVENT_MOUSE:
	if (!OverrideEvent(event)) {
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

    case X10_MOUSE:		/* X10 compatibility sequences */
	if (IsBtnEvent(event)) {
	    if (!OverrideButton(my_event)) {
		if (my_event->type == ButtonPress)
		    EditorButton(xw, my_event);
		result = True;
	    }
	}
	break;

    case VT200_HIGHLIGHT_MOUSE:	/* DEC vt200 hilite tracking */
	if (IsBtnEvent(event)) {
	    if (!OverrideButton(my_event)) {
		if (my_event->type == ButtonPress &&
		    my_event->button == Button1) {
		    TrackDown(xw, my_event);
		} else {
		    EditorButton(xw, my_event);
		}
		result = True;
	    }
	}
	break;

    case VT200_MOUSE:		/* DEC vt200 compatible */
	if (IsBtnEvent(event)) {
	    if (!OverrideButton(my_event)) {
		EditorButton(xw, my_event);
		result = True;
	    }
	}
	break;

    case DEC_LOCATOR:
#if OPT_DEC_LOCATOR
	if (IsBtnEvent(event) || event->type == MotionNotify) {
	    result = SendLocatorPosition(xw, my_event);
	}
#endif /* OPT_DEC_LOCATOR */
	break;
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
SendLocatorPosition(XtermWidget xw, XButtonEvent *event)
{
    ANSI reply;
    TScreen *screen = TScreenOf(xw);
    int row, col;
    Bool oor;
    int button;
    unsigned state;

    /* Make sure the event is an appropriate type */
    if (IsBtnEvent(event)) {
	if (OverrideButton(event))
	    return (False);
    } else {
	if (!screen->loc_filter)
	    return (False);
    }

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
     * bit7   bit6   bit5   bit4   bit3     bit2       bit1         bit0
     *                             M4 down  left down  middle down  right down
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
     * DECterm turns the Locator off if a button is pressed while a filter
     * rectangle is active.  This might be a bug, but I don't know, so I'll
     * emulate it anyway.
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
{ int stemp = (int) (((mask) & (Button1Mask | Button2Mask | Button3Mask | Button4Mask)) >> 8);	\
  /* swap Button1 & Button3 */								\
  (state) = (stemp & ~(4|1)) | ((stemp & 1) ? 4 : 0) | ((stemp & 4) ? 1 : 0);			\
}

void
GetLocatorPosition(XtermWidget xw)
{
    ANSI reply;
    TScreen *screen = TScreenOf(xw);
    Window root, child;
    int rx, ry, x, y;
    unsigned int mask = 0;
    int row = 0, col = 0;
    Bool oor = False;
    Bool ret = False;
    int state;

    /*
     * DECterm turns the Locator off if the position is requested while a
     * filter rectangle is active.  This might be a bug, but I don't know, so
     * I'll emulate it anyways.
     */
    if (screen->loc_filter) {
	screen->send_mouse_pos = MOUSE_OFF;
	screen->loc_filter = False;
	screen->locator_events = 0;
	MotionOff(screen, xw);
    }

    memset(&reply, 0, sizeof(reply));
    reply.a_type = ANSI_CSI;

    if (okSendMousePos(xw) == DEC_LOCATOR) {
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
	int state;

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
CheckLocatorPosition(XtermWidget xw, XButtonEvent *event)
{
    ANSI reply;
    TScreen *screen = TScreenOf(xw);
    int row, col;
    Bool oor;

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
	    int state;

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
isClick1_clean(XtermWidget xw, XButtonEvent *event)
{
    TScreen *screen = TScreenOf(xw);
    int delta;

    /* Disable on Shift-Click-1, including the application-mouse modes */
    if (OverrideButton(event)
	|| (okSendMousePos(xw) != MOUSE_OFF)
	|| ExtendingSelection)	/* Was moved */
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
isDoubleClick3(XtermWidget xw, TScreen *screen, XButtonEvent *event)
{
    int delta;

    if (event->type != ButtonRelease
	|| OverrideButton(event)
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
CheckSecondPress3(XtermWidget xw, TScreen *screen, XEvent *event)
{
    int delta;

    if (event->type != ButtonPress
	|| OverrideEvent(event)
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
rowOnCurrentLine(TScreen *screen,
		 int line,
		 int *deltap)	/* must be XButtonEvent */
{
    int result = 1;

    *deltap = 0;

    if (line != screen->cur_row) {
	int l1, l2;

	if (line < screen->cur_row) {
	    l1 = line;
	    l2 = screen->cur_row;
	} else {
	    l2 = line;
	    l1 = screen->cur_row;
	}
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
eventRow(TScreen *screen, XEvent *event)	/* must be XButtonEvent */
{
    return (event->xbutton.y - screen->border) / FontHeight(screen);
}

static int
eventColBetween(TScreen *screen, XEvent *event)		/* must be XButtonEvent */
{
    /* Correct by half a width - we are acting on a boundary, not on a cell. */
    return ((event->xbutton.x - OriginX(screen) + (FontWidth(screen) - 1) / 2)
	    / FontWidth(screen));
}

static int
ReadLineMovePoint(XtermWidget xw, int col, int ldelta)
{
    TScreen *screen = TScreenOf(xw);
    Char line[6];
    unsigned count = 0;

    col += ldelta * MaxCols(screen) - screen->cur_col;
    if (col == 0)
	return 0;
    if (screen->control_eight_bits) {
	line[count++] = ANSI_CSI;
    } else {
	line[count++] = ANSI_ESC;
	line[count++] = (xw->keyboard.flags & MODE_DECCKM) ? 'O' : '[';
    }
    line[count] = CharOf(col > 0 ? 'C' : 'D');
    if (col < 0)
	col = -col;
    while (col--)
	v_write(screen->respond, line, (size_t) 3);
    return 1;
}

static int
ReadLineDelete(TScreen *screen, CELL *cell1, CELL *cell2)
{
    int del;
    Char erases[2];

    erases[0] = (Char) get_tty_erase(screen->respond, XTERM_ERASE, "pty");
    erases[1] = 0;

    del = (cell2->col - cell1->col) + ((cell2->row - cell1->row) * MaxCols(screen));
    if (del <= 0)		/* Just in case... */
	return 0;
    while (del--)
	v_write(screen->respond, erases, (size_t) 1);
    return 1;
}

static void
readlineExtend(XtermWidget xw, XEvent *event)
{
    TScreen *screen = TScreenOf(xw);
    int ldelta1, ldelta2;

    if (IsBtnEvent(event)) {
	XButtonEvent *my_event = (XButtonEvent *) event;
	if (isClick1_clean(xw, my_event)
	    && SCREEN_FLAG(screen, click1_moves)
	    && rowOnCurrentLine(screen, eventRow(screen, event), &ldelta1)) {
	    ReadLineMovePoint(xw, eventColBetween(screen, event), ldelta1);
	}
	if (isDoubleClick3(xw, screen, my_event)
	    && SCREEN_FLAG(screen, dclick3_deletes)
	    && rowOnCurrentLine(screen, screen->startSel.row, &ldelta1)
	    && rowOnCurrentLine(screen, screen->endSel.row, &ldelta2)) {
	    ReadLineMovePoint(xw, screen->endSel.col, ldelta2);
	    ReadLineDelete(screen, &screen->startSel, &(screen->endSel));
	}
    }
}
#endif /* OPT_READLINE */

/* ^XM-G<line+' '><col+' '> */
void
DiredButton(Widget w,
	    XEvent *event,	/* must be XButtonEvent */
	    String *params GCC_UNUSED,	/* selections */
	    Cardinal *num_params GCC_UNUSED)
{
    XtermWidget xw;

    if ((xw = getXtermWidget(w)) != NULL) {
	TScreen *screen = TScreenOf(xw);

	if (IsBtnEvent(event)
	    && (event->xbutton.y >= screen->border)
	    && (event->xbutton.x >= OriginX(screen))) {
	    Char Line[6];
	    unsigned line, col;

	    line = (unsigned) ((event->xbutton.y - screen->border)
			       / FontHeight(screen));
	    col = (unsigned) ((event->xbutton.x - OriginX(screen))
			      / FontWidth(screen));
	    Line[0] = CONTROL('X');
	    Line[1] = ANSI_ESC;
	    Line[2] = 'G';
	    Line[3] = CharOf(' ' + col);
	    Line[4] = CharOf(' ' + line);
	    v_write(screen->respond, Line, (size_t) 5);
	}
    }
}

#if OPT_READLINE
void
ReadLineButton(Widget w,
	       XEvent *event,	/* must be XButtonEvent */
	       String *params,	/* selections */
	       Cardinal *num_params)
{
    XtermWidget xw;

    if ((xw = getXtermWidget(w)) != NULL) {
	TScreen *screen = TScreenOf(xw);
	Char Line[6];
	int line, col, ldelta = 0;

	if (!IsBtnEvent(event)
	    || (okSendMousePos(xw) != MOUSE_OFF) || ExtendingSelection)
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
	Line[1] = (xw->keyboard.flags & MODE_DECCKM) ? 'O' : '[';
	Line[2] = CharOf(col > 0 ? 'C' : 'D');
	if (col < 0)
	    col = -col;
	while (col--)
	    v_write(screen->respond, Line, (size_t) 3);
      finish:
	if (event->type == ButtonRelease)
	    do_select_end(xw, event, params, num_params, False);
    }
}
#endif /* OPT_READLINE */

/* repeats <ESC>n or <ESC>p */
void
ViButton(Widget w,
	 XEvent *event,		/* must be XButtonEvent */
	 String *params GCC_UNUSED,	/* selections */
	 Cardinal *num_params GCC_UNUSED)
{
    XtermWidget xw;

    if ((xw = getXtermWidget(w)) != NULL) {
	TScreen *screen = TScreenOf(xw);
	int pty = screen->respond;

	if (IsBtnEvent(event)) {
	    int line;

	    line = screen->cur_row -
		((event->xbutton.y - screen->border) / FontHeight(screen));

	    if (line != 0) {
		Char Line[6];

		Line[0] = ANSI_ESC;	/* force an exit from insert-mode */
		v_write(pty, Line, (size_t) 1);

		if (line < 0) {
		    line = -line;
		    Line[0] = CONTROL('n');
		} else {
		    Line[0] = CONTROL('p');
		}
		while (--line >= 0)
		    v_write(pty, Line, (size_t) 1);
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
		   XEvent *event,	/* must be XMotionEvent */
		   String *params GCC_UNUSED,
		   Cardinal *num_params GCC_UNUSED)
{
    XtermWidget xw;

    if ((xw = getXtermWidget(w)) != NULL) {
	TScreen *screen = TScreenOf(xw);
	CELL cell;

	TRACE_EVENT("HandleSelectExtend", event, params, num_params);

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
	    if (okSendMousePos(xw) == BTN_EVENT_MOUSE
		|| okSendMousePos(xw) == ANY_EVENT_MOUSE) {
		(void) SendMousePosition(xw, event);
	    }
	    break;
	}
    }
}

void
HandleKeyboardSelectExtend(Widget w,
			   XEvent *event GCC_UNUSED,	/* must be XButtonEvent */
			   String *params GCC_UNUSED,
			   Cardinal *num_params GCC_UNUSED)
{
    XtermWidget xw;

    if ((xw = getXtermWidget(w)) != NULL) {
	TScreen *screen = TScreenOf(xw);

	TRACE_EVENT("HandleKeyboardSelectExtend", event, params, num_params);
	ExtendExtend(xw, &screen->cursorp);
    }
}

static void
do_select_end(XtermWidget xw,
	      XEvent *event,	/* must be XButtonEvent */
	      String *params,	/* selections */
	      Cardinal *num_params,
	      Bool use_cursor_loc)
{
    TScreen *screen = TScreenOf(xw);

    screen->selection_time = event->xbutton.time;

    TRACE(("do_select_end %s @%ld\n",
	   visibleEventMode(screen->eventMode),
	   screen->selection_time));

    switch (screen->eventMode) {
    case NORMAL:
	(void) SendMousePosition(xw, event);
	break;
    case LEFTEXTENSION:
    case RIGHTEXTENSION:
	EndExtend(xw, event, params, *num_params, use_cursor_loc);
#if OPT_READLINE
	readlineExtend(xw, event);
#endif /* OPT_READLINE */
	break;
    }
}

void
HandleSelectEnd(Widget w,
		XEvent *event,	/* must be XButtonEvent */
		String *params,	/* selections */
		Cardinal *num_params)
{
    XtermWidget xw;

    if ((xw = getXtermWidget(w)) != NULL) {
	TRACE(("HandleSelectEnd\n"));
	do_select_end(xw, event, params, num_params, False);
    }
}

void
HandleKeyboardSelectEnd(Widget w,
			XEvent *event,	/* must be XButtonEvent */
			String *params,		/* selections */
			Cardinal *num_params)
{
    XtermWidget xw;

    if ((xw = getXtermWidget(w)) != NULL) {
	TRACE(("HandleKeyboardSelectEnd\n"));
	do_select_end(xw, event, params, num_params, True);
    }
}

void
HandlePointerMotion(Widget w,
		    XEvent *event,
		    String *params,	/* selections */
		    Cardinal *num_params)
{
    XtermWidget xw;

    (void) params;
    (void) num_params;
    if ((xw = getXtermWidget(w)) != NULL) {
	TRACE(("HandlePointerMotion\n"));
	if (event->type == MotionNotify)
	    (void) SendMousePosition(xw, event);
    }
}

void
HandlePointerButton(Widget w,
		    XEvent *event,
		    String *params,	/* selections */
		    Cardinal *num_params)
{
    XtermWidget xw;

    (void) params;
    (void) num_params;
    if ((xw = getXtermWidget(w)) != NULL) {
	TRACE(("HandlePointerButton\n"));
	if (IsBtnEvent(event))
	    (void) SendMousePosition(xw, event);
    }
}

/*
 * Copy the selection data to the given target(s).
 */
void
HandleCopySelection(Widget w,
		    XEvent *event,
		    String *params,	/* list of targets */
		    Cardinal *num_params)
{
    XtermWidget xw;

    if ((xw = getXtermWidget(w)) != NULL) {
	TRACE_EVENT("HandleCopySelection", event, params, num_params);
	SelectSet(xw, event, params, *num_params);
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
    } else {
	ch = '?';		/* DEC Technical has no mapping */
    }
    return ch;
}

#if OPT_WIDE_CHARS
static Cardinal
addXtermChar(Char **buffer, Cardinal *used, Cardinal offset, unsigned value)
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
UTF8toLatin1(TScreen *screen, Char *s, unsigned long len, unsigned long *result)
{
    static Char *buffer;
    static Cardinal used;

    Cardinal offset = 0;

    if (len != 0) {
	PtyData data;
	Boolean save_vt100 = screen->vt100_graphics;

	fakePtyData(&data, s, s + len);
	screen->vt100_graphics = False;		/* temporary override */
	while (decodeUtf8(screen, &data)) {
	    Bool fails = False;
	    Bool extra = False;
	    IChar value;
	    skipPtyData(&data, value);
	    if (is_UCS_SPECIAL(value)) {
		fails = True;
	    } else if (value < 256) {
		AddChar(&buffer, &used, offset, CharOf(value));
	    } else {
		unsigned eqv = ucs2dec(screen, value);
		if (xtermIsInternalCs(eqv)) {
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
		const Char *p;

		for (p = (const Char *) screen->default_string; *p != '\0'; ++p) {
		    AddChar(&buffer, &used, offset, *p);
		}
	    }
	    if (extra)
		AddChar(&buffer, &used, offset, ' ');
	}
	AddChar(&buffer, &used, offset, '\0');
	screen->vt100_graphics = save_vt100;
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
	    new_text_list = TypeXtMallocN(char *, *text_list_count);
	    new_text_list[0] = tmp = XtMalloc((Cardinal) new_size);
	    for (i = 0; i < (*text_list_count); ++i) {
		data = (Char *) (*text_list)[i];
		size = strlen((*text_list)[i]) + 1;
		if ((data = UTF8toLatin1(screen, data, size, &size)) != NULL) {
		    memcpy(tmp, data, size + 1);
		    new_text_list[i] = tmp;
		    tmp += size + 1;
		}
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
overrideTargets(Widget w, String value, Atom **resultp)
{
    Bool override = False;
    XtermWidget xw;

    if ((xw = getXtermWidget(w)) != NULL) {
	TScreen *screen = TScreenOf(xw);

	if (!IsEmpty(value)) {
	    char *copied = x_strdup(value);
	    if (copied != NULL) {
		Atom *result = NULL;
		Cardinal count = 1;
		int n;

		TRACE(("decoding SelectTypes \"%s\"\n", value));
		for (n = 0; copied[n] != '\0'; ++n) {
		    if (copied[n] == ',')
			++count;
		}
		result = TypeXtMallocN(Atom, (2 * count) + 1);
		if (result == NULL) {
		    TRACE(("Couldn't allocate selection types\n"));
		} else {
		    char nextc = '?';
		    char *listp = (char *) copied;
		    count = 0;
		    do {
			char *nextp = parseItem(listp, &nextc);
			char *item = x_strtrim(listp);
			size_t len = (item ? strlen(item) : 0);

			if (len == 0) {
			    /* EMPTY */ ;
			}
#if OPT_WIDE_CHARS
			else if (sameItem(item, "UTF8")) {
			    result[count++] = XA_UTF8_STRING(XtDisplay(w));
			}
#endif
			else if (sameItem(item, "I18N")) {
			    if (screen->i18nSelections) {
				result[count++] = XA_TEXT(XtDisplay(w));
				result[count++] = XA_COMPOUND_TEXT(XtDisplay(w));
			    }
			} else if (sameItem(item, "TEXT")) {
			    result[count++] = XA_TEXT(XtDisplay(w));
			} else if (sameItem(item, "COMPOUND_TEXT")) {
			    result[count++] = XA_COMPOUND_TEXT(XtDisplay(w));
			} else if (sameItem(item, "STRING")) {
			    result[count++] = XA_STRING;
			}
			*nextp++ = nextc;
			listp = nextp;
			free(item);
		    } while (nextc != '\0');
		    if (count) {
			result[count] = None;
			override = True;
			*resultp = result;
		    } else {
			XtFree((char *) result);
		    }
		}
		free(copied);
	    } else {
		TRACE(("Couldn't allocate copy of selection types\n"));
	    }
	}
    }
    return override;
}

#if OPT_WIDE_CHARS
static Atom *
allocUtf8Targets(Widget w, TScreen *screen)
{
    Atom **resultp = &(screen->selection_targets_utf8);

    if (*resultp == NULL) {
	Atom *result;

	if (!overrideTargets(w, screen->utf8_select_types, &result)) {
	    result = TypeXtMallocN(Atom, 5);
	    if (result == NULL) {
		TRACE(("Couldn't allocate utf-8 selection targets\n"));
	    } else {
		int n = 0;

		if (XSupportsLocale()) {
		    result[n++] = XA_UTF8_STRING(XtDisplay(w));
#ifdef X_HAVE_UTF8_STRING
		    if (screen->i18nSelections) {
			result[n++] = XA_TEXT(XtDisplay(w));
			result[n++] = XA_COMPOUND_TEXT(XtDisplay(w));
		    }
#endif
		}
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
alloc8bitTargets(Widget w, TScreen *screen)
{
    Atom **resultp = &(screen->selection_targets_8bit);

    if (*resultp == NULL) {
	Atom *result = NULL;

	if (!overrideTargets(w, screen->eightbit_select_types, &result)) {
	    result = TypeXtMallocN(Atom, 5);
	    if (result == NULL) {
		TRACE(("Couldn't allocate 8bit selection targets\n"));
	    } else {
		int n = 0;

		if (XSupportsLocale()) {
#ifdef X_HAVE_UTF8_STRING
		    result[n++] = XA_UTF8_STRING(XtDisplay(w));
#endif
		    if (screen->i18nSelections) {
			result[n++] = XA_TEXT(XtDisplay(w));
			result[n++] = XA_COMPOUND_TEXT(XtDisplay(w));
		    }
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
    XtermWidget xw;

    if ((xw = getXtermWidget(w)) == NULL) {
	result = NULL;
    } else {
	TScreen *screen = TScreenOf(xw);

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

#define isSELECT(value) (!strcmp(NonNull(value), "SELECT"))

static int
DefaultSelection(TScreen *screen)
{
    return (screen->selectToClipboard ? 1 : 0);
}

static int
TargetToSelection(TScreen *screen, String name)
{
    int result = -1;
    int cutb;

    if (isSELECT(name)) {
	result = DefaultSelection(screen);
    } else if (!strcmp(name, PRIMARY_NAME)) {
	result = PRIMARY_CODE;
    } else if (!strcmp(name, CLIPBOARD_NAME)) {
	result = CLIPBOARD_CODE;
    } else if (!strcmp(name, SECONDARY_NAME)) {
	result = SECONDARY_CODE;
    } else if (sscanf(name, "CUT_BUFFER%d", &cutb) == 1) {
	if (cutb >= 0 && cutb < MAX_CUT_BUFFER) {
	    result = CutBufferToCode(cutb);
	} else {
	    xtermWarning("unexpected cut-buffer code: %d\n", cutb);
	}
    } else {
	xtermWarning("unexpected selection target: %s\n", name);
    }
    TRACE2(("TargetToSelection(%s) ->%d\n", name, result));
    return result;
}

void
UnmapSelections(XtermWidget xw)
{
    TScreen *screen = TScreenOf(xw);

    FreeAndNull(screen->mappedSelect);
}

/*
 * xterm generally uses the primary selection.  Some applications prefer
 * (or are limited to) the clipboard.  Since the translations resource is
 * complicated, users seldom change the way it affects selection.  But it
 * is simple to remap the choice between primary and clipboard before the
 * call to XmuInternStrings().
 */
static String *
MapSelections(XtermWidget xw, String *params, Cardinal num_params)
{
    String *result = params;

    if (params != NULL && num_params > 0) {
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
				 ? CLIPBOARD_NAME
				 : PRIMARY_NAME);

	    UnmapSelections(xw);
	    if ((result = TypeMallocN(String, num_params + 1)) != NULL) {
		result[num_params] = NULL;
		for (j = 0; j < num_params; ++j) {
		    result[j] = (String) (isSELECT(params[j])
					  ? mapTo
					  : params[j]);
		    if (result[j] == NULL) {
			UnmapSelections(xw);
			FreeAndNull(result);
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
 * If it is not a cut-buffer, it is a type of selection, e.g., primary.
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
    TRACE2(("CutBuffer(%d) = %d\n", (int) code, cutbuffer));
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
		  String *params,	/* selections in precedence order */
		  Cardinal num_params,
		  Atom *targets)
{
    Atom selection;
    int cutbuffer;
    Atom target;

    XtermWidget xw;

    if (num_params == 0)
	return;
    if ((xw = getXtermWidget(w)) == NULL)
	return;

    TRACE(("xtermGetSelection num_params %d @%ld\n", num_params, ev_time));
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

	if (nbytes > 0) {
	    SelectionReceived(w, NULL, &selection, &type, (XtPointer) line,
			      &nbytes, &fmt8);
	} else if (num_params > 1) {
	    xtermGetSelection(w, ev_time, params + 1, num_params - 1, NULL);
	}
#if OPT_PASTE64
	else {
	    FinishPaste64(xw);
	}
#endif
    } else {

	if (targets == NULL || targets[0] == None) {
	    targets = _SelectionTargets(w);
	}

	if (targets != NULL) {
	    struct _SelectionList *list;

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
		list = TypeXtMalloc(struct _SelectionList);
		if (list != NULL) {
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
GettingSelection(Display *dpy, Atom type, Char *line, unsigned long len)
{
    Char *cp;
    const char *name = TraceAtomName(dpy, type);

    TRACE(("Getting %s (type=%ld, length=%ld)\n", name, (long int) type, len));
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

#define tty_vwrite(pty,lag,l)		v_write(pty,lag,(size_t) l)

#if OPT_PASTE64
/* Return base64 code character given 6-bit number */
static const char base64_code[] = "\
ABCDEFGHIJKLMNOPQRSTUVWXYZ\
abcdefghijklmnopqrstuvwxyz\
0123456789+/";
static void
base64_flush(TScreen *screen)
{
    Char x;

    TRACE(("base64_flush count %d, pad %d (%d)\n",
	   screen->base64_count,
	   screen->base64_pad,
	   screen->base64_pad & 3));

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
    if (screen->base64_pad & 3) {
	tty_vwrite(screen->respond,
		   (const Char *) "===",
		   (unsigned) (3 - (screen->base64_pad & 3)));
    }
    screen->base64_count = 0;
    screen->base64_accu = 0;
    screen->base64_pad = 0;
}
#endif /* OPT_PASTE64 */

/*
 * Translate ISO-8859-1 or UTF-8 data to NRCS.
 */
static void
ToNational(XtermWidget xw, Char *buffer, size_t *length)
{
    TScreen *screen = TScreenOf(xw);
    DECNRCM_codes gsetL = screen->gsets[screen->curgl];
    DECNRCM_codes gsetR = screen->gsets[screen->curgr];

#if OPT_WIDE_CHARS
    if ((screen->utf8_nrc_mode | screen->utf8_mode) != uFalse) {
	Char *p;
	PtyData *data = TypeXtMallocX(PtyData, *length);

	memset(data, 0, sizeof(*data));
	data->next = data->buffer;
	data->last = data->buffer + *length;
	memcpy(data->buffer, buffer, *length);
	p = buffer;
	while (data->next < data->last) {
	    unsigned chr, out, gl, gr;

	    if (!decodeUtf8(screen, data)) {
		data->utf_size = 1;
		data->utf_data = data->next[0];
	    }
	    data->next += data->utf_size;
	    chr = data->utf_data;
	    out = chr;
	    if ((gl = xtermCharSetIn(xw, chr, gsetL)) != chr) {
		out = gl;
	    } else if ((gr = xtermCharSetIn(xw, chr, gsetR)) != chr) {
		out = gr;
	    }
	    *p++ = (Char) ((out < 256) ? out : ' ');
	}
	*length = (size_t) (p - buffer);
	free(data);
    } else
#endif
    {
	Char *p;

	for (p = buffer; (size_t) (p - buffer) < *length; ++p) {
	    unsigned gl, gr;
	    unsigned chr = *p;
	    unsigned out = chr;
	    if ((gl = xtermCharSetIn(xw, chr, gsetL)) != chr) {
		out = gl;
	    } else if ((gr = xtermCharSetIn(xw, chr, gsetR)) != chr) {
		out = gr;
	    }
	    *p = (Char) out;
	}
    }
}

static void
_qWriteSelectionData(XtermWidget xw, Char *lag, size_t length)
{
    TScreen *screen = TScreenOf(xw);

    /*
     * If we are pasting into a window which is using NRCS, we want to map
     * the text from the normal encoding (ISO-8859-1 or UTF-8) into the coding
     * that an application would use to write characters with NRCS.
     *
     * TODO: handle conversion from UTF-8, and adjust length.  This can be done
     * in the same buffer because the target is always 8-bit.
     */
    if ((xw->flags & NATIONAL) && (length != 0)) {
	ToNational(xw, lag, &length);
    }
#if OPT_PASTE64
    if (screen->base64_paste) {
	/* Send data as base64 */
	Char *p = lag;
	Char buf[64];
	unsigned x = 0;

	TRACE(("convert to base64 %lu:%s\n",
	       (unsigned long) length,
	       visibleChars(p, length)));

	/*
	 * Handle the case where the selection is from _this_ xterm, which
	 * puts part of the reply in the buffer before the selection callback
	 * happens.
	 */
	if (screen->base64_paste && screen->unparse_len) {
	    unparse_end(xw);
	}
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
		TRACE(("writing base64 interim %s\n", visibleChars(buf, x)));
		tty_vwrite(screen->respond, buf, x);
		x = 0;
	    }
	}
	if (x != 0) {
	    screen->base64_pad += x;
	    TRACE(("writing base64 finish %s\n", visibleChars(buf, x)));
	    tty_vwrite(screen->respond, buf, x);
	}
    } else
#endif /* OPT_PASTE64 */
#if OPT_READLINE
    if (SCREEN_FLAG(screen, paste_quotes)) {
	Char quote[2];
	quote[0] = (Char) get_tty_lnext(screen->respond, XTERM_LNEXT, "pty");
	quote[1] = 0;
	TRACE(("writing quoted selection data %s\n", visibleChars(lag, length)));
	while (length--) {
	    tty_vwrite(screen->respond, quote, 1);
	    tty_vwrite(screen->respond, lag++, 1);
	}
    } else
#endif
    {
	TRACE(("writing selection data %s\n", visibleChars(lag, length)));
	tty_vwrite(screen->respond, lag, length);
    }
}

static void
_WriteSelectionData(XtermWidget xw, Char *line, size_t length)
{
#if OPT_PASTE64 || OPT_READLINE
    TScreen *screen = TScreenOf(xw);
#endif

#if OPT_PASTE64
    if (screen->base64_paste) {
	_qWriteSelectionData(xw, line, length);
	base64_flush(screen);
    } else
#endif
    {
	if (!SCREEN_FLAG(screen, paste_literal_nl)) {
	    size_t n;
	    for (n = 0; n < length; ++n) {
		if (line[n] == '\n') {
		    line[n] = '\r';
		}
	    }
	}

	_qWriteSelectionData(xw, line, length);
    }
}

#if OPT_PASTE64 || OPT_READLINE
static void
_WriteKey(TScreen *screen, const Char *in)
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

/*
 * Unless enabled by the user, strip control characters other than formatting.
 */
static size_t
removeControls(XtermWidget xw, char *value)
{
    TScreen *screen = TScreenOf(xw);
    size_t dst = 0;

    if (screen->allowPasteControls) {
	dst = strlen(value);
    } else {
	size_t src = 0;
	Boolean *disallowed = screen->disallow_paste_ops;
	TERMIO_STRUCT data;
	char current_chars[epLAST];

	if (disallowed[epSTTY] && ttyGetAttr(screen->respond, &data) == 0) {
	    int n;
	    int disabled = xtermDisabledChar();

	    TRACE(("disallow(STTY):"));
	    memcpy(current_chars, disallowed, sizeof(current_chars));

	    for (n = 0; n < NCCS; ++n) {
		PasteControls nc = (data.c_cc[n] < 32
				    ? data.c_cc[n]
				    : (data.c_cc[n] == 127
				       ? epDEL
				       : epLAST));
		if (nc == epNUL || nc == epLAST)
		    continue;
		if (CharOf(data.c_cc[n]) == CharOf(disabled))
		    continue;
		if ((n == VMIN || n == VTIME) && !(data.c_lflag & ICANON))
		    continue;
		switch (n) {
		    /* POSIX */
		case VEOF:
		case VEOL:
		case VERASE:
		case VINTR:
		case VKILL:
		case VQUIT:
		case VSTART:
		case VSTOP:
		case VSUSP:
		    /* system-dependent */
#ifdef VDISCARD
		case VDISCARD:
#endif
#ifdef VDSUSP
		case VDSUSP:
#endif
#ifdef VEOL2
		case VEOL2:
#endif
#ifdef VLNEXT
		case VLNEXT:
#endif
#ifdef VREPRINT
		case VREPRINT:
#endif
#ifdef VSTATUS
		case VSTATUS:
#endif
#ifdef VSWTC
		case VSWTC:	/* System V SWTCH */
#endif
#ifdef VWERASE
		case VWERASE:
#endif
		    break;
		default:
		    continue;
		}
		if (nc != epLAST) {
		    TRACE((" \\%03o", data.c_cc[n]));
		    current_chars[nc] = 1;
		}
	    }
	    TRACE(("\n"));
	    disallowed = current_chars;
	}
	while ((value[dst] = value[src]) != '\0') {
	    int ch = CharOf(value[src++]);

#define ReplacePaste(n) \
	    if (disallowed[n]) \
		value[dst] = ' '

	    if (ch < 32) {
		ReplacePaste(epC0);
		ReplacePaste(ch);
		++dst;
	    } else if (ch == ANSI_DEL) {
		ReplacePaste(epDEL);
		++dst;
	    }
#if OPT_WIDE_CHARS
	    else if (screen->utf8_inparse || screen->utf8_nrc_mode)
		++dst;
#endif
#if OPT_C1_PRINT || OPT_WIDE_CHARS
	    else if (screen->c1_printable)
		++dst;
#endif
	    else if (ch >= 128 && ch < 160)
		continue;
	    else
		++dst;
	}
    }
    return dst;
}

#if OPT_SELECTION_OPS
static void
beginInternalSelect(XtermWidget xw)
{
    TScreen *screen = TScreenOf(xw);
    InternalSelect *mydata = &(screen->internal_select);

    (void) mydata;
    /* override flags so that SelectionReceived only updates a buffer */
#if OPT_PASTE64
    mydata->base64_paste = screen->base64_paste;
    screen->base64_paste = 0;
#endif
#if OPT_PASTE64 || OPT_READLINE
    mydata->paste_brackets = screen->paste_brackets;
    SCREEN_FLAG_unset(screen, paste_brackets);
#endif
}

static void
finishInternalSelect(XtermWidget xw)
{
    TScreen *screen = TScreenOf(xw);
    InternalSelect *mydata = &(screen->internal_select);

    (void) mydata;
#if OPT_PASTE64
    screen->base64_paste = mydata->base64_paste;
#endif
#if OPT_PASTE64 || OPT_READLINE
    screen->paste_brackets = mydata->paste_brackets;
#endif
}

#else
#define finishInternalSelect(xw)	/* nothing */
#endif /* OPT_SELECTION_OPS */

/* SelectionReceived: stuff received selection text into pty */

/* ARGSUSED */
static void
SelectionReceived(Widget w,
		  XtPointer client_data,
		  Atom *selection GCC_UNUSED,
		  Atom *type,
		  XtPointer value,
		  unsigned long *length,
		  int *format)
{
    char **text_list = NULL;
    int text_list_count = 0;
    XTextProperty text_prop;
    TScreen *screen;
    Display *dpy;
#if OPT_TRACE && OPT_WIDE_CHARS
    Char *line = (Char *) value;
#endif

    XtermWidget xw;

    if ((xw = getXtermWidget(w)) == NULL)
	return;

    screen = TScreenOf(xw);
    dpy = XtDisplay(w);

    if (*type == 0		/*XT_CONVERT_FAIL */
	|| *length == 0
	|| value == NULL) {
	TRACE(("...no data to convert\n"));
	goto fail;
    }

    text_prop.value = (unsigned char *) value;
    text_prop.encoding = *type;
    text_prop.format = *format;
    text_prop.nitems = *length;

    TRACE(("SelectionReceived %s %s format %d, nitems %ld\n",
	   TraceAtomName(screen->display, *selection),
	   visibleSelectionTarget(dpy, text_prop.encoding),
	   text_prop.format,
	   text_prop.nitems));

#if OPT_WIDE_CHARS
    if (XSupportsLocale() && screen->wide_chars) {
	if (*type == XA_UTF8_STRING(dpy) ||
	    *type == XA_STRING ||
	    *type == XA_COMPOUND_TEXT(dpy)) {
	    GettingSelection(dpy, *type, line, *length);
	    if (Xutf8TextPropertyToTextList(dpy, &text_prop,
					    &text_list,
					    &text_list_count) < 0) {
		TRACE(("default Xutf8 Conversion failed\n"));
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
	    if (*type == XA_STRING && (!XSupportsLocale() || screen->brokenSelections)) {
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
	    /* EMPTY */ ;
	} else
#endif
#if OPT_PASTE64 || OPT_READLINE
	if (SCREEN_FLAG(screen, paste_brackets) && !screen->selectToBuffer) {
	    _WriteKey(screen, (const Char *) "200");
	}
#endif
	for (i = 0; i < text_list_count; i++) {
	    size_t len = removeControls(xw, text_list[i]);

	    if (screen->selectToBuffer) {
		InternalSelect *mydata = &(screen->internal_select);
		if (!mydata->done) {
		    size_t have = (mydata->buffer
				   ? strlen(mydata->buffer)
				   : 0);
		    size_t need = have + len + 1;
		    char *buffer = realloc(mydata->buffer, need);

		    if (buffer != NULL) {
			strcpy(buffer + have, text_list[i]);
			mydata->buffer = buffer;
		    }
		    TRACE(("FormatSelect %d.%d .. %d.%d %s\n",
			   screen->startSel.row,
			   screen->startSel.col,
			   screen->endSel.row,
			   screen->endSel.col,
			   mydata->buffer));
		    mydata->format_select(w, mydata->format, mydata->buffer,
					  &(screen->startSel),
					  &(screen->endSel));
		    mydata->done = True;
		}

	    } else {
		_WriteSelectionData(xw, (Char *) text_list[i], len);
	    }
	}
#if OPT_PASTE64
	if (screen->base64_paste) {
	    FinishPaste64(xw);
	} else
#endif
#if OPT_PASTE64 || OPT_READLINE
	if (SCREEN_FLAG(screen, paste_brackets) && !screen->selectToBuffer) {
	    _WriteKey(screen, (const Char *) "201");
	}
#endif
	if (screen->selectToBuffer) {
	    InternalSelect *mydata = &(screen->internal_select);
	    finishInternalSelect(xw);
	    if (mydata->done) {
		free(mydata->format);
		free(mydata->buffer);
		memset(mydata, 0, sizeof(*mydata));
	    }
	    screen->selectToBuffer = False;
	}
	XFreeStringList(text_list);
    } else {
	TRACE(("...empty text-list\n"));
	goto fail;
    }

    XtFree((char *) client_data);
    XtFree((char *) value);

    return;

  fail:
    if (client_data != NULL) {
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
		      XEvent *event,	/* assumed to be XButtonEvent* */
		      String *params,	/* selections in precedence order */
		      Cardinal *num_params)
{
    XtermWidget xw;

    if ((xw = getXtermWidget(w)) != NULL) {
	TRACE_EVENT("HandleInsertSelection", event, params, num_params);
	if (!SendMousePosition(xw, event)) {
#if OPT_READLINE
	    int ldelta;
	    TScreen *screen = TScreenOf(xw);
	    if (IsBtnEvent(event)
		&& !OverrideEvent(event)
		&& (okSendMousePos(xw) == MOUSE_OFF)
		&& SCREEN_FLAG(screen, paste_moves)
		&& rowOnCurrentLine(screen, eventRow(screen, event), &ldelta))
		ReadLineMovePoint(xw, eventColBetween(screen, event), ldelta);
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

#if OPT_BLOCK_SELECT
    if (screen->blockSelecting
	|| screen->blockSelecting != screen->lastSelectWasBlock) {
	/* No word, line, paragraph selecting when block selecting
	   or when our last click was a block select */
	screen->numberOfClicks = 1;
	result = defaultUnit;
    } else
#endif
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
		XEvent *event,	/* must be XButtonEvent* */
		CELL *cell)
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
		  XEvent *event,	/* must be XButtonEvent* */
		  String *params GCC_UNUSED,
		  Cardinal *num_params GCC_UNUSED)
{
    XtermWidget xw;

    if ((xw = getXtermWidget(w)) != NULL) {
	TScreen *screen = TScreenOf(xw);
	CELL cell;

	TRACE_EVENT("HandleSelectStart", event, params, num_params);
	screen->firstValidRow = 0;
	screen->lastValidRow = screen->max_row;
	PointToCELL(screen, event->xbutton.y, event->xbutton.x, &cell);

#if OPT_READLINE
	ExtendingSelection = 0;
#endif

#if OPT_BLOCK_SELECT
	screen->blockSelecting =
	    (*num_params >= 1 && !strcmp(params[0], "block")) ? 1 : 0;
#endif

	do_select_start(xw, event, &cell);
    }
}

/* ARGSUSED */
void
HandleKeyboardSelectStart(Widget w,
			  XEvent *event,	/* must be XButtonEvent* */
			  String *params GCC_UNUSED,
			  Cardinal *num_params GCC_UNUSED)
{
    XtermWidget xw;

    if ((xw = getXtermWidget(w)) != NULL) {
	TScreen *screen = TScreenOf(xw);

	TRACE_EVENT("HandleKeyboardSelectStart", event, params, num_params);
	do_select_start(xw, event, &screen->cursorp);
    }
}

static void
TrackDown(XtermWidget xw, XButtonEvent *event)
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
	   const CELL *start,
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
StartSelect(XtermWidget xw, const CELL *cell)
{
    TScreen *screen = TScreenOf(xw);

    TRACE(("StartSelect row=%d, col=%d\n", cell->row, cell->col));
    if (screen->cursor_state)
	HideCursor(xw);
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
    ComputeSelect(xw, &(screen->startExt), &(screen->endExt), False, True);
}

static void
EndExtend(XtermWidget xw,
	  XEvent *event,	/* must be XButtonEvent */
	  String *params,	/* selections */
	  Cardinal num_params,
	  Bool use_cursor_loc)
{
    CELL cell;
    TScreen *screen = TScreenOf(xw);

    TRACE_EVENT("EndExtend", event, params, &num_params);
    if (use_cursor_loc) {
	cell = screen->cursorp;
    } else {
	PointToCELL(screen, event->xbutton.y, event->xbutton.x, &cell);
    }
    ExtendExtend(xw, &cell);

    screen->lastButtonUpTime = event->xbutton.time;
    screen->lastButton = event->xbutton.button;
#if OPT_BLOCK_SELECT
    screen->lastSelectWasBlock = screen->blockSelecting;
#endif

    if (!isSameCELL(&(screen->startSel), &(screen->endSel))) {
	if (screen->replyToEmacs) {
	    Char line[64];
	    unsigned count = 0;

	    if (screen->control_eight_bits) {
		line[count++] = ANSI_CSI;
	    } else {
		line[count++] = ANSI_ESC;
		line[count++] = '[';
	    }
	    if (isSameCELL(&(screen->rawPos), &(screen->startSel))
		&& isSameCELL(&cell, &(screen->endSel))) {
		/* Use short-form emacs select */

		switch (screen->extend_coords) {
		case 0:
		case SET_EXT_MODE_MOUSE:
		    line[count++] = 't';
		    break;
		case SET_SGR_EXT_MODE_MOUSE:
		case SET_PIXEL_POSITION_MOUSE:
		    line[count++] = '<';
		    break;
		}

		count = EmitMousePosition(screen, line, count, screen->endSel.col);
		count = EmitMousePositionSeparator(screen, line, count);
		count = EmitMousePosition(screen, line, count, screen->endSel.row);

		switch (screen->extend_coords) {
		case SET_SGR_EXT_MODE_MOUSE:
		case SET_URXVT_EXT_MODE_MOUSE:
		case SET_PIXEL_POSITION_MOUSE:
		    line[count++] = 't';
		    break;
		}
	    } else {
		/* long-form, specify everything */

		switch (screen->extend_coords) {
		case 0:
		case SET_EXT_MODE_MOUSE:
		    line[count++] = 'T';
		    break;
		case SET_SGR_EXT_MODE_MOUSE:
		case SET_PIXEL_POSITION_MOUSE:
		    line[count++] = '<';
		    break;
		}

		count = EmitMousePosition(screen, line, count, screen->startSel.col);
		count = EmitMousePositionSeparator(screen, line, count);
		count = EmitMousePosition(screen, line, count, screen->startSel.row);
		count = EmitMousePositionSeparator(screen, line, count);
		count = EmitMousePosition(screen, line, count, screen->endSel.col);
		count = EmitMousePositionSeparator(screen, line, count);
		count = EmitMousePosition(screen, line, count, screen->endSel.row);
		count = EmitMousePositionSeparator(screen, line, count);
		count = EmitMousePosition(screen, line, count, cell.col);
		count = EmitMousePositionSeparator(screen, line, count);
		count = EmitMousePosition(screen, line, count, cell.row);

		switch (screen->extend_coords) {
		case SET_SGR_EXT_MODE_MOUSE:
		case SET_URXVT_EXT_MODE_MOUSE:
		case SET_PIXEL_POSITION_MOUSE:
		    line[count++] = 'T';
		    break;
		}
	    }
	    v_write(screen->respond, line, (size_t) count);
	    UnHiliteText(xw);
	}
    }
    SelectSet(xw, event, params, num_params);
    screen->eventMode = NORMAL;
}

void
HandleSelectSet(Widget w,
		XEvent *event,
		String *params,
		Cardinal *num_params)
{
    XtermWidget xw;

    if ((xw = getXtermWidget(w)) != NULL) {
	TRACE_EVENT("HandleSelectSet", event, params, num_params);
	SelectSet(xw, event, params, *num_params);
    }
}

/* ARGSUSED */
static void
SelectSet(XtermWidget xw,
	  XEvent *event GCC_UNUSED,
	  String *params,
	  Cardinal num_params)
{
    TScreen *screen = TScreenOf(xw);

    TRACE(("SelectSet\n"));
    /* Only do select stuff if non-null select */
    if (!isSameCELL(&(screen->startSel), &(screen->endSel))) {
	Cardinal n;
	for (n = 0; n < num_params; ++n) {
	    SaltTextAway(xw,
			 TargetToSelection(screen, params[n]),
			 &(screen->startSel), &(screen->endSel));
	}
	_OwnSelection(xw, params, num_params);
    } else {
	ScrnDisownSelection(xw);
    }
}

#define Abs(x)		((x) < 0 ? -(x) : (x))

/* ARGSUSED */
static void
do_start_extend(XtermWidget xw,
		XEvent *event,	/* must be XButtonEvent* */
		String *params GCC_UNUSED,
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
    if (OverrideEvent(event)
	|| event->xbutton.button != Button3
	|| !(SCREEN_FLAG(screen, dclick3_deletes)))
#endif
	screen->selectUnit = EvalSelectUnit(xw,
					    event->xbutton.time,
					    screen->selectUnit,
					    event->xbutton.button);
    screen->replyToEmacs = False;

#if OPT_READLINE
    CheckSecondPress3(xw, screen, event);
#endif

    if (screen->numberOfClicks == 1
	|| (SCREEN_FLAG(screen, dclick3_deletes)
	    && !OverrideEvent(event))) {
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
    ComputeSelect(xw, &(screen->startExt), &(screen->endExt), True, True);

#if OPT_READLINE
    if (!isSameCELL(&(screen->startSel), &(screen->endSel)))
	ExtendingSelection = 1;
#endif
}

static void
ExtendExtend(XtermWidget xw, const CELL *cell)
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
    ComputeSelect(xw, &(screen->startExt), &(screen->endExt), False, True);

#if OPT_READLINE
    if (!isSameCELL(&(screen->startSel), &(screen->endSel)))
	ExtendingSelection = 1;
#endif
}

void
HandleStartExtend(Widget w,
		  XEvent *event,	/* must be XButtonEvent* */
		  String *params,	/* unused */
		  Cardinal *num_params)		/* unused */
{
    XtermWidget xw;

    if ((xw = getXtermWidget(w)) != NULL) {
	TRACE_EVENT("HandleStartExtend", event, params, num_params);
	do_start_extend(xw, event, params, num_params, False);
    }
}

void
HandleKeyboardStartExtend(Widget w,
			  XEvent *event,	/* must be XButtonEvent* */
			  String *params,	/* unused */
			  Cardinal *num_params)		/* unused */
{
    XtermWidget xw;

    if ((xw = getXtermWidget(w)) != NULL) {
	TRACE_EVENT("HandleKeyboardStartExtend", event, params, num_params);
	do_start_extend(xw, event, params, num_params, True);
    }
}

void
ScrollSelection(TScreen *screen, int amount, Bool always)
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
	    || !ScrnHaveRowMargins(screen)
	    || ScrnIsRowInMargins(screen, adjust)) {
	    scroll_update_one(&screen->startH);
	}
	adjust = ROW2INX(screen, screen->endH.row);
	if (always
	    || !ScrnHaveRowMargins(screen)
	    || ScrnIsRowInMargins(screen, adjust)) {
	    scroll_update_one(&screen->endH);
	}
    }

    screen->startHCoord = Coordinate(screen, &screen->startH);
    screen->endHCoord = Coordinate(screen, &screen->endH);
}

/*ARGSUSED*/
void
ResizeSelection(TScreen *screen, int rows, int cols)
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
#define isWideCell(row, col) isWideFrg((int)XTERM_CELL(row, col))
#endif

static void
PointToCELL(TScreen *screen,
	    int y,
	    int x,
	    CELL *cell)
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
LastTextCol(TScreen *screen, CLineData *ld, int row)
{
    int i = -1;

    if (ld != NULL) {
	if (okScrnRow(screen, row)) {
	    const IAttr *ch;
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
/*  BS   HT   NL   VT   FF   CR   SO   SI */
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
class_of(LineData *ld, const CELL *cell)
{
    CELL temp = *cell;
    int result = 0;

#if OPT_DEC_CHRSET
    if (CSET_DOUBLE(GetLineDblCS(ld))) {
	temp.col /= 2;
    }
#endif
    if (temp.col < (int) ld->lineSize)
	result = CharacterClass((int) (ld->charData[temp.col]));
    return result;
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

#if OPT_REPORT_CCLASS
static int
show_cclass_range(int lo, int hi)
{
    int cclass = CharacterClass(lo);
    int ident = (cclass == lo);
    int more = 0;
    if (ident) {
	int ch;
	for (ch = lo + 1; ch <= hi; ch++) {
	    if (CharacterClass(ch) != ch) {
		ident = 0;
		break;
	    }
	}
	if (ident && (hi < 255)) {
	    ch = hi + 1;
	    if (CharacterClass(ch) == ch) {
		if (ch >= 255 || CharacterClass(ch + 1) != ch) {
		    more = 1;
		}
	    }
	}
    }
    if (!more) {
	if (lo == hi) {
	    printf("\t%d", lo);
	} else {
	    printf("\t%d-%d", lo, hi);
	}
	if (!ident)
	    printf(":%d", cclass);
	if (hi < 255)
	    printf(", \\");
	printf("\n");
    }
    return !more;
}

void
report_char_class(XtermWidget xw)
{
    /* simple table, to match documentation */
    static const char charnames[] =
    "NUL\0" "SOH\0" "STX\0" "ETX\0" "EOT\0" "ENQ\0" "ACK\0" "BEL\0"
    " BS\0" " HT\0" " NL\0" " VT\0" " NP\0" " CR\0" " SO\0" " SI\0"
    "DLE\0" "DC1\0" "DC2\0" "DC3\0" "DC4\0" "NAK\0" "SYN\0" "ETB\0"
    "CAN\0" " EM\0" "SUB\0" "ESC\0" " FS\0" " GS\0" " RS\0" " US\0"
    " SP\0" "  !\0" "  \"\0" "  #\0" "  $\0" "  %\0" "  &\0" "  '\0"
    "  (\0" "  )\0" "  *\0" "  +\0" "  ,\0" "  -\0" "  .\0" "  /\0"
    "  0\0" "  1\0" "  2\0" "  3\0" "  4\0" "  5\0" "  6\0" "  7\0"
    "  8\0" "  9\0" "  :\0" "  ;\0" "  <\0" "  =\0" "  >\0" "  ?\0"
    "  @\0" "  A\0" "  B\0" "  C\0" "  D\0" "  E\0" "  F\0" "  G\0"
    "  H\0" "  I\0" "  J\0" "  K\0" "  L\0" "  M\0" "  N\0" "  O\0"
    "  P\0" "  Q\0" "  R\0" "  S\0" "  T\0" "  U\0" "  V\0" "  W\0"
    "  X\0" "  Y\0" "  Z\0" "  [\0" "  \\\0" "  ]\0" "  ^\0" "  _\0"
    "  `\0" "  a\0" "  b\0" "  c\0" "  d\0" "  e\0" "  f\0" "  g\0"
    "  h\0" "  i\0" "  j\0" "  k\0" "  l\0" "  m\0" "  n\0" "  o\0"
    "  p\0" "  q\0" "  r\0" "  s\0" "  t\0" "  u\0" "  v\0" "  w\0"
    "  x\0" "  y\0" "  z\0" "  {\0" "  |\0" "  }\0" "  ~\0" "DEL\0"
    "x80\0" "x81\0" "x82\0" "x83\0" "IND\0" "NEL\0" "SSA\0" "ESA\0"
    "HTS\0" "HTJ\0" "VTS\0" "PLD\0" "PLU\0" " RI\0" "SS2\0" "SS3\0"
    "DCS\0" "PU1\0" "PU2\0" "STS\0" "CCH\0" " MW\0" "SPA\0" "EPA\0"
    "x98\0" "x99\0" "x9A\0" "CSI\0" " ST\0" "OSC\0" " PM\0" "APC\0"
    "  -\0" "  i\0" " c/\0" "  L\0" " ox\0" " Y-\0" "  |\0" " So\0"
    " ..\0" " c0\0" " ip\0" " <<\0" "  _\0" "   \0" " R0\0" "  -\0"
    "  o\0" " +-\0" "  2\0" "  3\0" "  '\0" "  u\0" " q|\0" "  .\0"
    "  ,\0" "  1\0" "  2\0" " >>\0" "1/4\0" "1/2\0" "3/4\0" "  ?\0"
    " A`\0" " A'\0" " A^\0" " A~\0" " A:\0" " Ao\0" " AE\0" " C,\0"
    " E`\0" " E'\0" " E^\0" " E:\0" " I`\0" " I'\0" " I^\0" " I:\0"
    " D-\0" " N~\0" " O`\0" " O'\0" " O^\0" " O~\0" " O:\0" "  X\0"
    " O/\0" " U`\0" " U'\0" " U^\0" " U:\0" " Y'\0" "  P\0" "  B\0"
    " a`\0" " a'\0" " a^\0" " a~\0" " a:\0" " ao\0" " ae\0" " c,\0"
    " e`\0" " e'\0" " e^\0" " e:\0" " i`\0" " i'\0" " i^\0" " i:\0"
    "  d\0" " n~\0" " o`\0" " o'\0" " o^\0" " o~\0" " o:\0" " -:\0"
    " o/\0" " u`\0" " u'\0" " u^\0" " u:\0" " y'\0" "  P\0" " y:\0";
    int ch, dh;
    int class_p;

    (void) xw;

    printf("static int charClass[256] = {\n");
    for (ch = 0; ch < 256; ++ch) {
	const char *s = charnames + (ch * 4);
	if ((ch & 7) == 0)
	    printf("/*");
	printf(" %s ", s);
	if (((ch + 1) & 7) == 0) {
	    printf("*/\n  ");
	    for (dh = ch - 7; dh <= ch; ++dh) {
		printf(" %3d%s", CharacterClass(dh), dh == 255 ? "};" : ",");
	    }
	    printf("\n");
	}
    }

    /* print the table as if it were the charClass resource */
    printf("\n");
    printf("The table is equivalent to this \"charClass\" resource:\n");
    class_p = CharacterClass(dh = 0);
    for (ch = 0; ch < 256; ++ch) {
	int class_c = CharacterClass(ch);
	if (class_c != class_p) {
	    if (show_cclass_range(dh, ch - 1)) {
		dh = ch;
		class_p = class_c;
	    }
	}
    }
    if (dh < 255) {
	show_cclass_range(dh, 255);
    }

    if_OPT_WIDE_CHARS(TScreenOf(xw), {
	/* if this is a wide-character configuration, print all intervals */
	report_wide_char_class();
    });
}
#endif

/*
 * If the given column is past the end of text on the given row, bump to the
 * beginning of the next line.
 */
static Boolean
okPosition(TScreen *screen,
	   LineData **ld,
	   CELL *cell)
{
    Boolean result = True;

    assert(ld != NULL);
    assert(*ld != NULL);

    if (*ld == NULL) {
	result = False;
	TRACE(("okPosition LineData is null!\n"));
    } else if (cell->row > screen->max_row) {
	result = False;
	TRACE(("okPosition cell row %d > screen max %d\n", cell->row, screen->max_row));
    } else if (cell->col > (LastTextCol(screen, *ld, cell->row) + 1)) {
	TRACE(("okPosition cell col %d > screen max %d\n", cell->col,
	       (LastTextCol(screen, *ld, cell->row) + 1)));
	if (cell->row < screen->max_row) {
	    TRACE(("okPosition cell row %d < screen max %d\n", cell->row, screen->max_row));
	    cell->col = 0;
	    *ld = GET_LINEDATA(screen, ++cell->row);
	    result = False;
	}
    }
    return result;
}

static void
trimLastLine(TScreen *screen,
	     LineData **ld,
	     CELL *last)
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
firstRowOfLine(TScreen *screen, int row, Bool visible)
{
    LineData *ld = NULL;
    int limit = visible ? 0 : -screen->savedlines;

    while (row > limit &&
	   (ld = GET_LINEDATA(screen, row - 1)) != NULL &&
	   LineTstWrapped(ld)) {
	--row;
    }
    return row;
}

/*
 * Returns the last row of a wrapped line.
 */
static int
lastRowOfLine(TScreen *screen, int row)
{
    LineData *ld;

    while (row < screen->max_row &&
	   (ld = GET_LINEDATA(screen, row)) != NULL &&
	   LineTstWrapped(ld)) {
	++row;
    }
    return row;
}

/*
 * Returns the number of cells on the range of rows.
 */
static unsigned
lengthOfLines(TScreen *screen, int firstRow, int lastRow)
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
make_indexed_text(TScreen *screen, int row, unsigned length, int *indexed)
{
    Char *result = NULL;
    size_t need = (length + 1);

    /*
     * Get a quick upper bound to the number of bytes needed, if the whole
     * string were UTF-8.
     */
    if_OPT_WIDE_CHARS(screen, {
	need *= ((screen->lineExtra + 1) * 6);
    });

    if ((result = TypeCallocN(Char, need + 1)) != NULL) {
	LineData *ld = GET_LINEDATA(screen, row);
	unsigned used = 0;
	Char *last = result;

	do {
	    int col = 0;
	    int limit = LastTextCol(screen, ld, row);

	    while (col <= limit) {
		Char *next = last;
		unsigned data = ld->charData[col];

		assert(col < (int) ld->lineSize);
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
		 (ld = GET_LINEDATA(screen, ++row)) != NULL &&
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
indexToCol(const int *indexed, int len, int off)
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
columnToCell(TScreen *screen, int row, int col, CELL *cell)
{
    while (row < screen->max_row) {
	CLineData *ld = GET_LINEDATA(screen, row);
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
cellToColumn(TScreen *screen, CELL *cell)
{
    CLineData *ld = NULL;
    int col = cell->col;
    int row = firstRowOfLine(screen, cell->row, False);
    while (row < cell->row) {
	int adj;
	if ((ld = GET_LINEDATA(screen, row)) == NULL)
	    break;
	if ((adj = LastTextCol(screen, ld, row++)) < 0)
	col += adj;
    }
#if OPT_DEC_CHRSET
    if (ld == NULL)
	ld = GET_LINEDATA(screen, row);
    if (CSET_DOUBLE(GetLineDblCS(ld)))
	col /= 2;
#endif
    return col;
}

static void
do_select_regex(TScreen *screen, CELL *startc, CELL *endc)
{
    LineData *ld = GET_LINEDATA(screen, startc->row);
    int inx = ((screen->numberOfClicks - 1) % screen->maxClicks);
    char *expr = screen->selectExpr[inx];
    regex_t preg;
    regmatch_t match;

    TRACE(("Select_REGEX[%d]:%s\n", inx, NonNull(expr)));
    if (okPosition(screen, &ld, startc) && expr != NULL) {
	if (regcomp(&preg, expr, REG_EXTENDED) == 0) {
	    int firstRow = firstRowOfLine(screen, startc->row, True);
	    int lastRow = lastRowOfLine(screen, firstRow);
	    unsigned size = lengthOfLines(screen, firstRow, lastRow);
	    int actual = cellToColumn(screen, startc);
	    int *indexed;

	    TRACE(("regcomp ok rows %d..%d bytes %d\n",
		   firstRow, lastRow, size));

	    if ((indexed = TypeCallocN(int, size + 1)) != NULL) {
		char *search;
		if ((search = make_indexed_text(screen,
						firstRow,
						size,
						indexed)) != NULL) {
		    int len = (int) strlen(search);
		    int col;
		    int offset;
		    int best_col = -1;
		    int best_len = -1;

		    startc->row = 0;
		    startc->col = 0;
		    endc->row = 0;
		    endc->col = 0;

		    for (col = 0; (offset = indexed[col]) < len; ++col) {
			if (regexec(&preg,
				    search + offset,
				    (size_t) 1, &match,
				    col ? REG_NOTBOL : 0) == 0) {
			    int start_inx = (int) (match.rm_so + offset);
			    int finis_inx = (int) (match.rm_eo + offset);
			    int start_col = indexToCol(indexed, len, start_inx);
			    int finis_col = indexToCol(indexed, len, finis_inx);

			    if (start_col <= actual &&
				actual <= finis_col) {
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
			       indexed[best_nxt] -
			       indexed[best_col],
			       visibleChars((Char *) (search + indexed[best_col]),
					    (unsigned) (indexed[best_nxt] -
							indexed[best_col]))));
		    }
		    free(search);
		}
		free(indexed);
#if OPT_DEC_CHRSET
		if ((ld = GET_LINEDATA(screen, startc->row)) != NULL) {
		    if (CSET_DOUBLE(GetLineDblCS(ld)))
			startc->col *= 2;
		}
		if ((ld = GET_LINEDATA(screen, endc->row)) != NULL) {
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
	   && (ltmp = GET_LINEDATA(screen, screen->name.row - 1)) != NULL \
	   && LineTstWrapped(ltmp))

/*
 * sets startSel endSel
 * ensuring that they have legal values
 */
static void
ComputeSelect(XtermWidget xw,
	      const CELL *startc,
	      const CELL *endc,
	      Bool extend,
	      Bool normal)
{
    TScreen *screen = TScreenOf(xw);

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
#if OPT_BLOCK_SELECT
	/* Allow block selecting past EOL */
	if (screen->blockSelecting)
	    break;
#endif
	(void) okPosition(screen, &(ld.startSel), &(screen->startSel));
	(void) okPosition(screen, &(ld.endSel), &(screen->endSel));
	break;

    case Select_WORD:
	TRACE(("Select_WORD\n"));
	if (okPosition(screen, &(ld.startSel), &(screen->startSel))) {
	    CELL mark;
	    cclass = CClassOf(startSel);
	    TRACE(("...starting with class %d\n", cclass));
	    do {
		mark = screen->startSel;
		--screen->startSel.col;
		if (screen->startSel.col < 0
		    && isPrevWrapped(startSel)) {
		    PrevRow(startSel);
		    screen->startSel.col = LastTextCol(screen, ld.startSel, screen->startSel.row);
		}
	    } while (screen->startSel.col >= 0
		     && CClassSelects(startSel, cclass));
	    if (normal)
		++screen->startSel.col;
	    else
		screen->startSel = mark;
	}
#if OPT_WIDE_CHARS
#define SkipHiddenCell(mark) \
	if (mark.col && XTERM_CELL(mark.row, mark.col) == HIDDEN_CHAR) \
	    mark.col++
#else
#define SkipHiddenCell(mark)	/* nothing */
#endif
	SkipHiddenCell(screen->startSel);

	if (!normal) {
	    screen->endSel = screen->startSel;
	    ld.endSel = ld.startSel;
	}

	if (okPosition(screen, &(ld.endSel), &(screen->endSel))) {
	    int length = LastTextCol(screen, ld.endSel, screen->endSel.row);
	    cclass = CClassOf(endSel);
	    TRACE(("...ending with class %d\n", cclass));
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
	    if (normal
		&& screen->endSel.col > length + 1
		&& MoreRows(endSel)) {
		screen->endSel.col = 0;
		NextRow(endSel);
	    }
	}
	SkipHiddenCell(screen->endSel);

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
	  const CELL *firstp,
	  const CELL *lastp)
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
    TRACE(("...previous(first=%d,%d, last=%d,%d)\n",
	   old_start.row, old_start.col,
	   old_end.row, old_end.col));
    if (isSameCELL(&first, &old_start) &&
	isSameCELL(&last, &old_end)) {
	return;
    }

    screen->startH = first;
    screen->endH = last;
    from = Coordinate(screen, &screen->startH);
    to = Coordinate(screen, &screen->endH);
    if (to <= screen->startHCoord || from > screen->endHCoord
#if OPT_BLOCK_SELECT
	|| screen->blockSelecting
	|| screen->blockSelecting != screen->lastSelectWasBlock
#endif
	) {
#if OPT_BLOCK_SELECT
	/* Either no overlap whatsoever between old and new hilite,
	   or we're in block select mode (or just came from it),
	   in which case there is no optimization possible. */
	if (screen->lastSelectWasBlock) {
	    /* If we just came from block select mode, we need to
	       unhighlight more aggressively. */
	    old_start.col = 0;
	    old_end.col = MaxCols(screen);
	}
#endif
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

static void
UnHiliteText(XtermWidget xw)
{
    TrackText(xw, &zeroCELL, &zeroCELL);
}

/* Guaranteed that (first->row, first->col) <= (last->row, last->col) */
static void
ReHiliteText(XtermWidget xw,
	     const CELL *firstp,
	     const CELL *lastp)
{
    TScreen *screen = TScreenOf(xw);
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

#if OPT_BLOCK_SELECT
    if (screen->blockSelecting) {
	/* In block select mode, there is no special case for the first or
	   last rows. Also, unlike normal selections, there can be
	   unselected text on both sides of the selection per row, so we
	   refresh all columns. */
	int row;
	for (row = first.row; row <= last.row; row++) {
	    ScrnRefresh(xw, row, 0, 1, MaxCols(screen), True);
	}
    } else
#endif
    if (!isSameRow(&first, &last)) {	/* do multiple rows */
	int i;
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
 * Guaranteed that (cellc->row, cellc->col) <= (cell->row, cell->col),
 * and that both points are valid
 * (may have cell->row = screen->max_row+1, cell->col = 0).
 */
static void
SaltTextAway(XtermWidget xw,
	     int which,
	     const CELL *cellc,
	     const CELL *cell)
{
    TScreen *screen = TScreenOf(xw);
    SelectedCells *scp;
    int i;
    int eol;
    int need = 0;
    size_t have = 0;
    Char *line;
    Char *lp;
    CELL first = *cellc;
    CELL last = *cell;

    if (which < 0 || which >= MAX_SELECTIONS) {
	TRACE(("SaltTextAway - which selection?\n"));
	return;
    }
    scp = &(screen->selected_cells[which]);

    TRACE(("SaltTextAway which=%d, first=%d,%d, last=%d,%d\n",
	   which, first.row, first.col, last.row, last.col));

    if (isSameRow(&first, &last) && first.col > last.col) {
	int tmp;
	EXCHANGE(first.col, last.col, tmp);
    }

    --last.col;
    /* first we need to know how long the string is before we can save it */

    if (isSameRow(&last, &first)) {
	need = Length(screen, first.row, first.col, last.col);
    } else {			/* two cases, cut is on same line, cut spans multiple lines */
	need += Length(screen, first.row, first.col, screen->max_col) + 1;
	for (i = first.row + 1; i < last.row; i++)
	    need += Length(screen, i, 0, screen->max_col) + 1;
	if (last.col >= 0)
	    need += Length(screen, last.row, 0, last.col);
    }

    /* UTF-8 may require more space */
    if_OPT_WIDE_CHARS(screen, {
	if (need > 0) {
	    if (screen->max_combining > 0)
		need += screen->max_combining;
	    need *= 6;
	}
    });

    /* now get some memory to save it in */
    if (need < 0)
	return;

    if (scp->data_limit <= (unsigned) need) {
	if ((line = (Char *) malloc((size_t) need + 1)) == NULL)
	    SysError(ERROR_BMALLOC2);
	free(scp->data_buffer);
	scp->data_buffer = line;
	scp->data_limit = (size_t) (need + 1);
    } else {
	line = scp->data_buffer;
    }

    if (line == NULL)
	return;

    line[need] = '\0';		/* make sure it is null terminated */
    lp = line;			/* lp points to where to save the text */
    if (isSameRow(&last, &first)) {
	lp = SaveText(screen, last.row, first.col, last.col, lp, &eol);
    }
#if OPT_BLOCK_SELECT
    else if (screen->blockSelecting) {
	/* In block select mode, find the left most column of the block.
	   This can be from either the start or the end of the selection. */
	int blockFirst, blockLast;
	if (first.col < last.col) {
	    blockFirst = first.col;
	    blockLast = last.col;
	} else {
	    blockFirst = last.col;
	    blockLast = first.col;
	}
	for (i = first.row; i <= last.row; i++) {
	    lp = SaveText(screen, i, blockFirst, blockLast, lp, &eol);
	    if (i < last.row || eol)
		*lp++ = '\n';
	}
    }
#endif
    else {
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

    have = (size_t) (lp - line);
    /*
     * Scanning the buffer twice is unnecessary.  Discard unwanted memory if
     * the estimate is too-far off.
     */
    if ((have * 2) < (size_t) need) {
	Char *next;
	scp->data_limit = have + 1;
	next = realloc(line, scp->data_limit);
	if (next == NULL) {
	    free(line);
	    scp->data_length = 0;
	    scp->data_limit = 0;
	}
	scp->data_buffer = next;
    }
    scp->data_length = have;

    TRACE(("Salted TEXT:%u:%s\n", (unsigned) have,
	   visibleChars(scp->data_buffer, (unsigned) have)));
}

#if OPT_PASTE64
void
ClearSelectionBuffer(TScreen *screen, String selection)
{
    int which = TargetToSelection(screen, selection);
    SelectedCells *scp = &(screen->selected_cells[okSelectionCode(which)]);
    FreeAndNull(scp->data_buffer);
    scp->data_limit = 0;
    scp->data_length = 0;
    screen->base64_count = 0;
}

static void
AppendStrToSelectionBuffer(SelectedCells * scp, Char *text, size_t len)
{
    if (len != 0) {
	size_t j = (scp->data_length + len);
	size_t k = j + (j >> 2) + 80;
	if (j + 1 >= scp->data_limit) {
	    Char *line;
	    if (!scp->data_length) {
		line = (Char *) malloc(k);
	    } else {
		line = (Char *) realloc(scp->data_buffer, k);
	    }
	    if (line == NULL)
		SysError(ERROR_BMALLOC2);
	    scp->data_buffer = line;
	    scp->data_limit = k;
	}
	if (scp->data_buffer != NULL) {
	    memcpy(scp->data_buffer + scp->data_length, text, len);
	    scp->data_length += len;
	    scp->data_buffer[scp->data_length] = 0;
	}
    }
}

void
AppendToSelectionBuffer(TScreen *screen, unsigned c, String selection)
{
    int which = TargetToSelection(screen, selection);
    SelectedCells *scp = &(screen->selected_cells[okSelectionCode(which)]);
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
	AppendStrToSelectionBuffer(scp, &ch, (size_t) 1);
	break;

    case 4:
	ch = CharOf((screen->base64_accu << 4) + (six >> 2));
	screen->base64_accu = (six & 0x3);
	screen->base64_count = 2;
	AppendStrToSelectionBuffer(scp, &ch, (size_t) 1);
	break;

    case 6:
	ch = CharOf((screen->base64_accu << 2) + (six >> 4));
	screen->base64_accu = (six & 0xF);
	screen->base64_count = 4;
	AppendStrToSelectionBuffer(scp, &ch, (size_t) 1);
	break;
    }
}

void
CompleteSelection(XtermWidget xw, String *args, Cardinal len)
{
    TScreen *screen = TScreenOf(xw);

    screen->base64_count = 0;
    screen->base64_accu = 0;
    _OwnSelection(xw, args, len);
}
#endif /* OPT_PASTE64 */

static Bool
_ConvertSelectionHelper(Widget w,
			SelectedCells * scp,
			Atom *type,
			XtPointer *value,
			unsigned long *length,
			int *format,
			int (*conversion_function) (Display *,
						    char **, int,
						    XICCEncodingStyle,
						    XTextProperty *),
			XICCEncodingStyle conversion_style)
{
    *value = NULL;
    *length = 0;
    *type = 0;
    *format = 0;

    if (getXtermWidget(w) != NULL) {
	Display *dpy = XtDisplay(w);
	XTextProperty textprop;
	int out_n = 0;
	char *result = NULL;
	char *the_data = (char *) scp->data_buffer;
	char *the_next;
	unsigned long remaining = scp->data_length;

	TRACE(("converting %ld:'%s'\n",
	       (long) scp->data_length,
	       visibleChars(scp->data_buffer, (unsigned) scp->data_length)));
	/*
	 * For most selections, we can convert in one pass.  It is possible
	 * that some applications contain embedded nulls, e.g., using xterm's
	 * paste64 feature.  For those cases, we will build up the result in
	 * parts.
	 */
	if (memchr(the_data, 0, scp->data_length) != NULL) {
	    TRACE(("selection contains embedded nulls\n"));
	    result = calloc(scp->data_length + 1, sizeof(char));
	}

      next_try:
	memset(&textprop, 0, sizeof(textprop));
	if (conversion_function(dpy, &the_data, 1,
				conversion_style,
				&textprop) >= Success) {
	    if ((result != NULL)
		&& (textprop.value != NULL)
		&& (textprop.format == 8)) {
		char *text_values = (char *) textprop.value;
		unsigned long in_n;

		if (out_n == 0) {
		    *value = result;
		    *type = textprop.encoding;
		    *format = textprop.format;
		}
		for (in_n = 0; in_n < textprop.nitems; ++in_n) {
		    result[out_n++] = text_values[in_n];
		}
		*length += textprop.nitems;
		if ((the_next = memchr(the_data, 0, remaining)) != NULL) {
		    unsigned long this_was = (unsigned long) (the_next - the_data);
		    this_was++;
		    the_data += this_was;
		    remaining -= this_was;
		    result[out_n++] = 0;
		    *length += 1;
		    if (remaining)
			goto next_try;
		}
		return True;
	    } else {
		free(result);
		*value = (XtPointer) textprop.value;
		*length = textprop.nitems;
		*type = textprop.encoding;
		*format = textprop.format;
		return True;
	    }
	}
	free(result);
    }
    return False;
}

static Boolean
SaveConvertedLength(XtPointer *target, unsigned long source)
{
    Boolean result = False;

    *target = XtMalloc(4);
    if (*target != NULL) {
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

#define keepClipboard(d,atom) ((screen->keepClipboard) && \
	 (atom == XA_CLIPBOARD(d)))

static Boolean
ConvertSelection(Widget w,
		 Atom *selection,
		 Atom *target,
		 Atom *type,
		 XtPointer *value,
		 unsigned long *length,
		 int *format)
{
    Display *dpy = XtDisplay(w);
    TScreen *screen;
    SelectedCells *scp;
    Bool result = False;

    Char *data;
    unsigned long data_length;

    XtermWidget xw;

    if ((xw = getXtermWidget(w)) == NULL)
	return False;

    screen = TScreenOf(xw);

    TRACE(("ConvertSelection %s -> %s\n",
	   TraceAtomName(screen->display, *selection),
	   visibleSelectionTarget(dpy, *target)));

    if (keepClipboard(dpy, *selection)) {
	TRACE(("asked for clipboard\n"));
	scp = &(screen->clipboard_data);
    } else {
	TRACE(("asked for selection\n"));
	scp = &(screen->selected_cells[AtomToSelection(dpy, *selection)]);
    }

    data = scp->data_buffer;
    data_length = scp->data_length;
    if (data == NULL) {
	TRACE(("...no selection-data\n"));
	return False;
    }

    if (*target == XA_TARGETS(dpy)) {
	Atom *targetP;
	XPointer std_return = NULL;
	unsigned long std_length;

	if (XmuConvertStandardSelection(w, screen->selection_time, selection,
					target, type, &std_return,
					&std_length, format)) {
	    Atom *my_targets = _SelectionTargets(w);
	    Atom *allocP;
	    Atom *std_targets;

	    TRACE(("XmuConvertStandardSelection - success\n"));
	    std_targets = (Atom *) (void *) (std_return);
	    *length = std_length + 6;

	    targetP = TypeXtMallocN(Atom, *length);
	    allocP = targetP;

	    *value = (XtPointer) targetP;

	    if (my_targets != NULL) {
		while (*my_targets != None) {
		    *targetP++ = *my_targets++;
		}
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
	    _ConvertSelectionHelper(w, scp,
				    type, value, length, format,
				    Xutf8TextListToTextProperty,
				    XStringStyle);
	TRACE(("...Xutf8TextListToTextProperty:%d\n", result));
    } else if (screen->wide_chars && *target == XA_UTF8_STRING(dpy)) {
	result =
	    _ConvertSelectionHelper(w, scp,
				    type, value, length, format,
				    Xutf8TextListToTextProperty,
				    XUTF8StringStyle);
	TRACE(("...Xutf8TextListToTextProperty:%d\n", result));
    } else if (screen->wide_chars && *target == XA_TEXT(dpy)) {
	result =
	    _ConvertSelectionHelper(w, scp,
				    type, value, length, format,
				    Xutf8TextListToTextProperty,
				    XStdICCTextStyle);
	TRACE(("...Xutf8TextListToTextProperty:%d\n", result));
    } else if (screen->wide_chars && *target == XA_COMPOUND_TEXT(dpy)) {
	result =
	    _ConvertSelectionHelper(w, scp,
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
	*value = (XtPointer) data;
	*length = data_length;
	*format = 8;
	result = True;
	TRACE(("...raw 8-bit data:%d\n", result));
    } else if (*target == XA_TEXT(dpy)) {	/* not wide_chars */
	result =
	    _ConvertSelectionHelper(w, scp,
				    type, value, length, format,
				    XmbTextListToTextProperty,
				    XStdICCTextStyle);
	TRACE(("...XmbTextListToTextProperty(StdICC):%d\n", result));
    } else if (*target == XA_COMPOUND_TEXT(dpy)) {	/* not wide_chars */
	result =
	    _ConvertSelectionHelper(w, scp,
				    type, value, length, format,
				    XmbTextListToTextProperty,
				    XCompoundTextStyle);
	TRACE(("...XmbTextListToTextProperty(Compound):%d\n", result));
    }
#ifdef X_HAVE_UTF8_STRING
    else if (*target == XA_UTF8_STRING(dpy)) {	/* not wide_chars */
	result =
	    _ConvertSelectionHelper(w, scp,
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
	result = SaveConvertedLength(value, scp->data_length);
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
LoseSelection(Widget w, Atom *selection)
{
    TScreen *screen;
    Atom *atomP;
    Cardinal i;

    XtermWidget xw;

    if ((xw = getXtermWidget(w)) == NULL)
	return;

    screen = TScreenOf(xw);
    TRACE(("LoseSelection %s\n", TraceAtomName(screen->display, *selection)));

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
	UnHiliteText(xw);
}

/* ARGSUSED */
static void
SelectionDone(Widget w GCC_UNUSED,
	      Atom *selection GCC_UNUSED,
	      Atom *target GCC_UNUSED)
{
    /* empty proc so Intrinsics know we want to keep storage */
    TRACE(("SelectionDone\n"));
}

static void
_OwnSelection(XtermWidget xw,
	      String *selections,
	      Cardinal count)
{
    TScreen *screen = TScreenOf(xw);
    Display *dpy = screen->display;
    Atom *atoms = screen->selection_atoms;
    Cardinal i;
    Bool have_selection = False;
    SelectedCells *scp;

    if (count == 0)
	return;

    TRACE(("_OwnSelection count %d\n", count));
    selections = MapSelections(xw, selections, count);

    if (count > screen->sel_atoms_size) {
	XtFree((char *) atoms);
	atoms = TypeXtMallocN(Atom, count);
	screen->selection_atoms = atoms;
	screen->sel_atoms_size = count;
    }
    XmuInternStrings(dpy, selections, count, atoms);
    for (i = 0; i < count; i++) {
	int cutbuffer = CutBuffer(atoms[i]);
	if (cutbuffer >= 0) {
	    unsigned long limit =
	    (unsigned long) (4 * XMaxRequestSize(dpy) - 32);
	    scp = &(screen->selected_cells[CutBufferToCode(cutbuffer)]);
	    if (scp->data_length > limit) {
		TRACE(("selection too big (%lu bytes), not storing in CUT_BUFFER%d\n",
		       (unsigned long) scp->data_length, cutbuffer));
		xtermWarning("selection too big (%lu bytes), not storing in CUT_BUFFER%d\n",
			     (unsigned long) scp->data_length, cutbuffer);
	    } else {
		/* This used to just use the UTF-8 data, which was totally
		 * broken as not even the corresponding paste code in xterm
		 * understood this!  So now it converts to Latin1 first.
		 *   Robert Brady, 2000-09-05
		 */
		unsigned long length = scp->data_length;
		Char *data = scp->data_buffer;
		if_OPT_WIDE_CHARS((screen), {
		    data = UTF8toLatin1(screen, data, length, &length);
		});
		TRACE(("XStoreBuffer(%d)\n", cutbuffer));
		XStoreBuffer(dpy,
			     (char *) data,
			     (int) length,
			     cutbuffer);
	    }
	} else {
	    int which = AtomToSelection(dpy, atoms[i]);
	    if (keepClipboard(dpy, atoms[i])) {
		Char *buf;
		SelectedCells *tcp = &(screen->clipboard_data);
		TRACE(("saving selection to clipboard buffer\n"));
		scp = &(screen->selected_cells[CLIPBOARD_CODE]);
		if ((buf = (Char *) malloc((size_t) scp->data_length)) == NULL) {
		    SysError(ERROR_BMALLOC2);
		} else {
		    free(tcp->data_buffer);
		    memcpy(buf, scp->data_buffer, scp->data_length);
		    tcp->data_buffer = buf;
		    tcp->data_limit = scp->data_length;
		    tcp->data_length = scp->data_length;
		}
	    }
	    scp = &(screen->selected_cells[which]);
	    if (scp->data_length == 0) {
		TRACE(("XtDisownSelection(%s, @%ld)\n",
		       TraceAtomName(screen->display, atoms[i]),
		       (long) screen->selection_time));
		XtDisownSelection((Widget) xw,
				  atoms[i],
				  screen->selection_time);
	    } else if (!screen->replyToEmacs && atoms[i] != 0) {
		TRACE(("XtOwnSelection(%s, @%ld)\n",
		       TraceAtomName(screen->display, atoms[i]),
		       (long) screen->selection_time));
		have_selection |=
		    XtOwnSelection((Widget) xw, atoms[i],
				   screen->selection_time,
				   ConvertSelection,
				   LoseSelection,
				   SelectionDone);
	    }
	}
	TRACE(("... _OwnSelection used length %lu value %s\n",
	       (unsigned long) scp->data_length,
	       visibleChars(scp->data_buffer,
			    (unsigned) scp->data_length)));
    }
    if (!screen->replyToEmacs)
	screen->selection_count = count;
    if (!have_selection)
	UnHiliteText(xw);
}

static void
ResetSelectionState(TScreen *screen)
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
Length(TScreen *screen,
       int row,
       int scol,
       int ecol)
{
    CLineData *ld = GET_LINEDATA(screen, row);
    const int lastcol = LastTextCol(screen, ld, row);

    if (ecol > lastcol)
	ecol = lastcol;
    return Max(0, (ecol - scol + 1));
}

/* copies text into line, preallocated */
static Char *
SaveText(TScreen *screen,
	 int row,
	 int scol,
	 int ecol,
	 Char *lp,		/* pointer to where to put the text */
	 int *eol)
{
    LineData *ld;
    int i = 0;
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
	unsigned c;
	if (i >= (int) ld->lineSize) {
	    /* The terminal was probably resized */
	    *lp++ = CharOf(' ');
	    continue;
	}
	c = ld->charData[i];
	if (ld->attribs[i] & INVISIBLE)
	    continue;
#if OPT_WIDE_CHARS
	/* We want to strip out every occurrence of HIDDEN_CHAR AFTER a
	 * wide character.
	 */
	if (c == HIDDEN_CHAR) {
	    if (isWide((int) previous)) {
		previous = c;
		/* Combining characters attached to double-width characters
		   are in memory attached to the HIDDEN_CHAR */
		if_OPT_WIDE_CHARS(screen, {
		    if ((screen->utf8_nrc_mode | screen->utf8_mode) != uFalse) {
			size_t off;
			for_each_combData(off, ld) {
			    unsigned ch = ld->combData[off][i];
			    if (ch == 0)
				break;
			    lp = convertToUTF8(lp, ch);
			}
		    }
		});
		continue;
	    } else {
		c = ' ';	/* should not happen, but just in case... */
	    }
	}
	previous = c;
	if ((screen->utf8_nrc_mode | screen->utf8_mode) != uFalse) {
	    lp = convertToUTF8(lp, (c != 0) ? c : ' ');
	    if_OPT_WIDE_CHARS(screen, {
		size_t off;
		for_each_combData(off, ld) {
		    unsigned ch = ld->combData[off][i];
		    if (ch == 0)
			break;
		    lp = convertToUTF8(lp, ch);
		}
	    });
	} else
#endif
	{
	    if (c == 0) {
		c = ' ';
	    } else if (c < ' ') {
		c = DECtoASCII(c);
	    } else if (c == 0x7f) {
		c = 0x5f;
	    }
	    *lp++ = CharOf(c);
	}
	if (c != ' ')
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

/*
 * This adds together the bits:
 *   shift key   -> 1
 *   meta key    -> 2
 *   control key -> 4
 */
static unsigned
KeyState(XtermWidget xw, unsigned x)
{
    return ((((x) & (ShiftMask | ControlMask)))
	    + (((x) & MetaMask(xw)) ? 2 : 0));
}

/* 32 + following 8-bit word:

   1:0  Button no: 0, 1, 2.  3=release.
     2  shift
     3  meta
     4  ctrl
     5  set for motion notify
     6  set for wheel (and button 6 and 7)
     7  set for buttons 8 to 11
*/

/* Position: 32 - 255. */
static int
BtnCode(XtermWidget xw, XButtonEvent *event, int button)
{
    int result = (int) (32 + (KeyState(xw, event->state) << 2));

    if (event->type == MotionNotify)
	result += 32;

    if (button < 0) {
	result += 3;
    } else {
	result += button & 3;
	if (button & 4)
	    result += 64;
	if (button & 8)
	    result += 128;
    }
    TRACE(("BtnCode button %d, %s state " FMT_MODIFIER_NAMES " ->%#x\n",
	   button,
	   visibleEventType(event->type),
	   ARG_MODIFIER_NAMES(event->state),
	   result));
    return result;
}

static unsigned
EmitButtonCode(XtermWidget xw,
	       Char *line,
	       unsigned count,
	       XButtonEvent *event,
	       int button)
{
    TScreen *screen = TScreenOf(xw);
    int value;

    if (okSendMousePos(xw) == X10_MOUSE) {
	value = CharOf(' ' + button);
    } else {
	value = BtnCode(xw, event, button);
    }

    switch (screen->extend_coords) {
    default:
	line[count++] = CharOf(value);
	break;
    case SET_SGR_EXT_MODE_MOUSE:
    case SET_PIXEL_POSITION_MOUSE:
	value -= 32;		/* encoding starts at zero */
	/* FALLTHRU */
    case SET_URXVT_EXT_MODE_MOUSE:
	count += (unsigned) sprintf((char *) line + count, "%d", value);
	break;
    case SET_EXT_MODE_MOUSE:
	if (value < 128) {
	    line[count++] = CharOf(value);
	} else {
	    line[count++] = CharOf(0xC0 + (value >> 6));
	    line[count++] = CharOf(0x80 + (value & 0x3F));
	}
	break;
    }
    return count;
}

static int
FirstBitN(int bits)
{
    int result = -1;
    if (bits > 0) {
	result = 0;
	while (!(bits & 1)) {
	    bits /= 2;
	    ++result;
	}
    }
    return result;
}

#define ButtonBit(button) ((button >= 0) ? (1 << (button)) : 0)

#define EMIT_BUTTON(button) EmitButtonCode(xw, line, count, event, button)

static void
EditorButton(XtermWidget xw, XButtonEvent *event)
{
    TScreen *screen = TScreenOf(xw);
    int pty = screen->respond;
    int mouse_limit = MouseLimit(screen);
    Char line[32];
    Char final = 'M';
    int row, col;
    int button;
    unsigned count = 0;
    Boolean changed = True;

    /* If button event, get button # adjusted for DEC compatibility */
    button = (int) (event->button - 1);
    if (button >= 3)
	button++;

    /* Ignore buttons that cannot be encoded */
    if (screen->send_mouse_pos == X10_MOUSE) {
	if (button > 3)
	    return;
    } else if (screen->extend_coords == SET_SGR_EXT_MODE_MOUSE
	       || screen->extend_coords == SET_URXVT_EXT_MODE_MOUSE
	       || screen->extend_coords == SET_PIXEL_POSITION_MOUSE) {
	if (button > 15) {
	    return;
	}
    } else {
	if (button > 11) {
	    return;
	}
    }

    if (screen->extend_coords == SET_PIXEL_POSITION_MOUSE) {
	row = event->y - OriginY(screen);
	col = event->x - OriginX(screen);
    } else {
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

	if (mouse_limit > 0) {
	    /* Limit to representable mouse dimensions */
	    if (row > mouse_limit)
		row = mouse_limit;
	    if (col > mouse_limit)
		col = mouse_limit;
	}
    }

    /* Build key sequence starting with \E[M */
    if (screen->control_eight_bits) {
	line[count++] = ANSI_CSI;
    } else {
	line[count++] = ANSI_ESC;
	line[count++] = '[';
    }
    switch (screen->extend_coords) {
    case 0:
    case SET_EXT_MODE_MOUSE:
#if OPT_SCO_FUNC_KEYS
	if (xw->keyboard.type == keyboardIsSCO) {
	    /*
	     * SCO function key F1 is \E[M, which would conflict with xterm's
	     * normal kmous.
	     */
	    line[count++] = '>';
	}
#endif
	line[count++] = final;
	break;
    case SET_SGR_EXT_MODE_MOUSE:
    case SET_PIXEL_POSITION_MOUSE:
	line[count++] = '<';
	break;
    }

    /* Add event code to key sequence */
    if (okSendMousePos(xw) == X10_MOUSE) {
	count = EMIT_BUTTON(button);
    } else {
	/* Button-Motion events */
	switch (event->type) {
	case ButtonPress:
	    screen->mouse_button |= ButtonBit(button);
	    count = EMIT_BUTTON(button);
	    break;
	case ButtonRelease:
	    /*
	     * The (vertical) wheel mouse interface generates release-events
	     * for buttons 4 and 5.
	     *
	     * The X10/X11 xterm protocol maps the release for buttons 1..3 to
	     * a -1, which will be later mapped into a "0" (some button was
	     * released),  At this point, buttons 1..3 are encoded 0..2 (the
	     * code 3 is unused).
	     *
	     * The SGR (extended) xterm mouse protocol keeps the button number
	     * and uses a "m" to indicate button release.
	     *
	     * The behavior for mice with more buttons is unclear, and may be
	     * revised -TD
	     */
	    screen->mouse_button &= ~ButtonBit(button);
	    if (button < 3 || button > 5) {
		switch (screen->extend_coords) {
		case SET_SGR_EXT_MODE_MOUSE:
		case SET_PIXEL_POSITION_MOUSE:
		    final = 'm';
		    break;
		default:
		    button = -1;
		    break;
		}
	    }
	    count = EMIT_BUTTON(button);
	    break;
	case MotionNotify:
	    /* BTN_EVENT_MOUSE and ANY_EVENT_MOUSE modes send motion
	     * events only if character cell has changed.
	     */
	    if ((row == screen->mouse_row)
		&& (col == screen->mouse_col)) {
		changed = False;
	    } else {
		count = EMIT_BUTTON(FirstBitN(screen->mouse_button));
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
	count = EmitMousePositionSeparator(screen, line, count);
	count = EmitMousePosition(screen, line, count, col);
	count = EmitMousePositionSeparator(screen, line, count);
	count = EmitMousePosition(screen, line, count, row);

	switch (screen->extend_coords) {
	case SET_SGR_EXT_MODE_MOUSE:
	case SET_URXVT_EXT_MODE_MOUSE:
	case SET_PIXEL_POSITION_MOUSE:
	    line[count++] = final;
	    break;
	}

	/* Transmit key sequence to process running under xterm */
	TRACE(("EditorButton -> %s\n", visibleChars(line, count)));
	v_write(pty, line, (size_t) count);
    }
    return;
}

/*
 * Check the current send_mouse_pos against allowed mouse-operations, returning
 * none if it is disallowed.
 */
XtermMouseModes
okSendMousePos(XtermWidget xw)
{
    TScreen *screen = TScreenOf(xw);
    XtermMouseModes result = (XtermMouseModes) screen->send_mouse_pos;

    switch ((int) result) {
    case MOUSE_OFF:
	break;
    case X10_MOUSE:
	if (!AllowMouseOps(xw, emX10))
	    result = MOUSE_OFF;
	break;
    case VT200_MOUSE:
	if (!AllowMouseOps(xw, emVT200Click))
	    result = MOUSE_OFF;
	break;
    case VT200_HIGHLIGHT_MOUSE:
	if (!AllowMouseOps(xw, emVT200Hilite))
	    result = MOUSE_OFF;
	break;
    case BTN_EVENT_MOUSE:
	if (!AllowMouseOps(xw, emAnyButton))
	    result = MOUSE_OFF;
	break;
    case ANY_EVENT_MOUSE:
	if (!AllowMouseOps(xw, emAnyEvent))
	    result = MOUSE_OFF;
	break;
    case DEC_LOCATOR:
	if (!AllowMouseOps(xw, emLocator))
	    result = MOUSE_OFF;
	break;
    }
    return result;
}

#if OPT_FOCUS_EVENT
/*
 * Check the current send_focus_pos against allowed mouse-operations, returning
 * none if it is disallowed.
 */
static int
okSendFocusPos(XtermWidget xw)
{
    TScreen *screen = TScreenOf(xw);
    int result = screen->send_focus_pos;

    if (!AllowMouseOps(xw, emFocusEvent)) {
	result = False;
    }
    return result;
}

void
SendFocusButton(XtermWidget xw, XFocusChangeEvent *event)
{
    if (okSendFocusPos(xw)) {
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

#if OPT_SELECTION_OPS
/*
 * Get the event-time, needed to process selections.
 */
static Time
getEventTime(XEvent *event)
{
    Time result;

    if (IsBtnEvent(event)) {
	result = ((XButtonEvent *) event)->time;
    } else if (IsKeyEvent(event)) {
	result = ((XKeyEvent *) event)->time;
    } else {
	result = 0;
    }

    return result;
}

/* obtain the selection string, passing the endpoints to caller's parameters */
static void
doSelectionFormat(XtermWidget xw,
		  Widget w,
		  XEvent *event,
		  String *params,
		  const Cardinal *num_params,
		  FormatSelect format_select)
{
    TScreen *screen = TScreenOf(xw);
    InternalSelect *mydata = &(screen->internal_select);

    memset(mydata, 0, sizeof(*mydata));
    mydata->format = x_strdup(params[0]);
    mydata->format_select = format_select;

    screen->selectToBuffer = True;
    beginInternalSelect(xw);

    xtermGetSelection(w, getEventTime(event), params + 1, *num_params - 1, NULL);

    if (screen->selectToBuffer)
	finishInternalSelect(xw);
}

/* obtain data from the screen, passing the endpoints to caller's parameters */
static char *
getDataFromScreen(XtermWidget xw, XEvent *event, String method, CELL *start, CELL *finish)
{
    TScreen *screen = TScreenOf(xw);

    CELL save_old_start = screen->startH;
    CELL save_old_end = screen->endH;

    CELL save_startSel = screen->startSel;
    CELL save_startRaw = screen->startRaw;
    CELL save_finishSel = screen->endSel;
    CELL save_finishRaw = screen->endRaw;

    int save_firstValidRow = screen->firstValidRow;
    int save_lastValidRow = screen->lastValidRow;

    const Cardinal noClick = 0;
    int save_numberOfClicks = screen->numberOfClicks;

    SelectUnit saveUnits = screen->selectUnit;
    SelectUnit saveMap = screen->selectMap[noClick];
#if OPT_SELECT_REGEX
    char *saveExpr = screen->selectExpr[noClick];
#endif
    SelectedCells *scp = &(screen->selected_cells[PRIMARY_CODE]);
    SelectedCells save_selection = *scp;

    char *result = NULL;

    TRACE(("getDataFromScreen %s\n", method));

    memset(scp, 0, sizeof(*scp));

    screen->numberOfClicks = 1;
    lookupSelectUnit(xw, noClick, method);
    screen->selectUnit = screen->selectMap[noClick];

    memset(start, 0, sizeof(*start));
    if (IsBtnEvent(event)) {
	XButtonEvent *btn_event = (XButtonEvent *) event;
	CELL cell;
	screen->firstValidRow = 0;
	screen->lastValidRow = screen->max_row;
	PointToCELL(screen, btn_event->y, btn_event->x, &cell);
	start->row = cell.row;
	start->col = cell.col;
	finish->row = cell.row;
	finish->col = screen->max_col;
    } else {
	start->row = screen->cur_row;
	start->col = screen->cur_col;
	finish->row = screen->cur_row;
	finish->col = screen->max_col;
    }

    ComputeSelect(xw, start, finish, False, False);
    SaltTextAway(xw,
		 TargetToSelection(screen, PRIMARY_NAME),
		 &(screen->startSel), &(screen->endSel));

    if (scp->data_limit && scp->data_buffer) {
	TRACE(("...getDataFromScreen selection-data %.*s\n",
	       (int) scp->data_limit,
	       scp->data_buffer));
	result = malloc(scp->data_limit + 1);
	if (result) {
	    memcpy(result, scp->data_buffer, scp->data_limit);
	    result[scp->data_limit] = 0;
	}
	free(scp->data_buffer);
	scp->data_limit = 0;
    }

    TRACE(("...getDataFromScreen restoring previous selection\n"));

    screen->startSel = save_startSel;
    screen->startRaw = save_startRaw;
    screen->endSel = save_finishSel;
    screen->endRaw = save_finishRaw;

    screen->firstValidRow = save_firstValidRow;
    screen->lastValidRow = save_lastValidRow;

    screen->numberOfClicks = save_numberOfClicks;
    screen->selectUnit = saveUnits;
    screen->selectMap[noClick] = saveMap;
#if OPT_SELECT_REGEX
    screen->selectExpr[noClick] = saveExpr;
#endif

    screen->selected_cells[0] = save_selection;

    TrackText(xw, &save_old_start, &save_old_end);

    TRACE(("...getDataFromScreen done\n"));
    return result;
}

#if OPT_EXEC_SELECTION
/*
 * Split-up the format before substituting data, to avoid quoting issues.
 * The resource mechanism has a limited ability to handle escapes.  We take
 * the result as if it were an sh-type string and parse it into a regular
 * argv array.
 */
static char **
tokenizeFormat(String format)
{
    char **result = NULL;

    format = x_skip_blanks(format);
    if (*format != '\0') {
	char *blob = x_strdup(format);
	int pass;

	for (pass = 0; pass < 2; ++pass) {
	    int used = 0;
	    int first = 1;
	    int escaped = 0;
	    int squoted = 0;
	    int dquoted = 0;
	    int n;
	    int argc = 0;

	    for (n = 0; format[n] != '\0'; ++n) {
		if (escaped) {
		    blob[used++] = format[n];
		    escaped = 0;
		} else if (format[n] == '"') {
		    if (!squoted) {
			if (!dquoted)
			    blob[used++] = format[n];
			dquoted = !dquoted;
		    }
		} else if (format[n] == '\'') {
		    if (!dquoted) {
			if (!squoted)
			    blob[used++] = format[n];
			squoted = !squoted;
		    }
		} else if (format[n] == '\\') {
		    blob[used++] = format[n];
		    escaped = 1;
		} else {
		    if (first) {
			first = 0;
			if (pass) {
			    result[argc] = &blob[n];
			}
			++argc;
		    }
		    if (isspace((Char) format[n])) {
			first = !isspace((Char) format[n + 1]);
			if (squoted || dquoted) {
			    blob[used++] = format[n];
			} else if (first) {
			    blob[used++] = '\0';
			}
		    } else {
			blob[used++] = format[n];
		    }
		}
	    }
	    blob[used] = '\0';
	    assert(strlen(blob) <= strlen(format));
	    if (!pass) {
		result = TypeCallocN(char *, argc + 1);
		if (result == NULL) {
		    free(blob);
		    break;
		}
	    }
	}
    }
#if OPT_TRACE
    if (result) {
	int n;
	TRACE(("tokenizeFormat %s\n", format));
	for (n = 0; result[n]; ++n) {
	    TRACE(("argv[%d] = %s\n", n, result[n]));
	}
    }
#endif

    return result;
}
#endif /* OPT_EXEC_SELECTION */

static void
formatVideoAttrs(XtermWidget xw, char *buffer, CELL *cell)
{
    TScreen *screen = TScreenOf(xw);
    LineData *ld = GET_LINEDATA(screen, cell->row);

    *buffer = '\0';
    if (ld != NULL && cell->col < (int) ld->lineSize) {
	IAttr attribs = ld->attribs[cell->col];
	const char *delim = "";

	if (attribs & INVERSE) {
	    buffer += sprintf(buffer, "7");
	    delim = ";";
	}
	if (attribs & UNDERLINE) {
	    buffer += sprintf(buffer, "%s4", delim);
	    delim = ";";
	}
	if (attribs & BOLD) {
	    buffer += sprintf(buffer, "%s1", delim);
	    delim = ";";
	}
	if (attribs & BLINK) {
	    buffer += sprintf(buffer, "%s5", delim);
	    delim = ";";
	}
#if OPT_ISO_COLORS
	if (attribs & FG_COLOR) {
	    Pixel fg = extract_fg(xw, ld->color[cell->col], attribs);
	    if (fg < 8) {
		fg += 30;
	    } else if (fg < 16) {
		fg += 90;
	    } else {
		buffer += sprintf(buffer, "%s38;5", delim);
		delim = ";";
	    }
	    buffer += sprintf(buffer, "%s%lu", delim, fg);
	    delim = ";";
	}
	if (attribs & BG_COLOR) {
	    Pixel bg = extract_bg(xw, ld->color[cell->col], attribs);
	    if (bg < 8) {
		bg += 40;
	    } else if (bg < 16) {
		bg += 100;
	    } else {
		buffer += sprintf(buffer, "%s48;5", delim);
		delim = ";";
	    }
	    (void) sprintf(buffer, "%s%lu", delim, bg);
	}
#endif
    }
}

static char *
formatStrlen(char *target, char *source, int freeit)
{
    if (source != NULL) {
	sprintf(target, "%u", (unsigned) strlen(source));
	if (freeit) {
	    free(source);
	}
    } else {
	strcpy(target, "0");
    }
    return target;
}

/* substitute data into format, reallocating the result */
static char *
expandFormat(XtermWidget xw,
	     const char *format,
	     char *data,
	     CELL *start,
	     CELL *finish)
{
    char *result = NULL;
    if (!IsEmpty(format)) {
	static char empty[1];
	int pass;
	int n;
	char numbers[80];

	if (data == NULL)
	    data = empty;

	for (pass = 0; pass < 2; ++pass) {
	    size_t need = 0;

	    for (n = 0; format[n] != '\0'; ++n) {

		if (format[n] == '%') {
		    char *value = NULL;

		    switch (format[++n]) {
		    case '%':
			if (pass) {
			    result[need] = format[n];
			}
			++need;
			break;
		    case 'P':
			sprintf(numbers, "%d;%d",
				TScreenOf(xw)->topline + start->row + 1,
				start->col + 1);
			value = numbers;
			break;
		    case 'p':
			sprintf(numbers, "%d;%d",
				TScreenOf(xw)->topline + finish->row + 1,
				finish->col + 1);
			value = numbers;
			break;
		    case 'R':
			value = formatStrlen(numbers, x_strrtrim(data), 1);
			break;
		    case 'r':
			value = x_strrtrim(data);
			break;
		    case 'S':
			value = formatStrlen(numbers, data, 0);
			break;
		    case 's':
			value = data;
			break;
		    case 'T':
			value = formatStrlen(numbers, x_strtrim(data), 1);
			break;
		    case 't':
			value = x_strtrim(data);
			break;
		    case 'V':
			formatVideoAttrs(xw, numbers, start);
			value = numbers;
			break;
		    case 'v':
			formatVideoAttrs(xw, numbers, finish);
			value = numbers;
			break;
		    default:
			if (pass) {
			    result[need] = format[n];
			}
			--n;
			++need;
			break;
		    }
		    if (value != NULL) {
			if (pass) {
			    strcpy(result + need, value);
			}
			need += strlen(value);
			if (value != numbers && value != data) {
			    free(value);
			}
		    }
		} else {
		    if (pass) {
			result[need] = format[n];
		    }
		    ++need;
		}
	    }
	    if (pass) {
		result[need] = '\0';
	    } else {
		++need;
		result = malloc(need);
		if (result == NULL) {
		    break;
		}
	    }
	}
    }
    TRACE(("expandFormat(%s) = %s\n", NonNull(format), NonNull(result)));
    return result;
}

#if OPT_EXEC_SELECTION
/* execute the command after forking.  The main process frees its data */
static void
executeCommand(pid_t pid, char **argv)
{
    (void) pid;
    if (argv != NULL && argv[0] != NULL) {
	char *child_cwd = ProcGetCWD(pid);

	if (fork() == 0) {
	    if (child_cwd) {
		IGNORE_RC(chdir(child_cwd));	/* We don't care if this fails */
	    }
	    execvp(argv[0], argv);
	    exit(EXIT_FAILURE);
	}
	free(child_cwd);
    }
}

static void
freeArgv(char *blob, char **argv)
{
    if (blob) {
	free(blob);
	if (argv) {
	    int n;
	    for (n = 0; argv[n]; ++n)
		free(argv[n]);
	    free(argv);
	}
    }
}

static void
reallyExecFormatted(Widget w, char *format, char *data, CELL *start, CELL *finish)
{
    XtermWidget xw;

    if ((xw = getXtermWidget(w)) != NULL) {
	char **argv;

	if ((argv = tokenizeFormat(format)) != NULL) {
	    char *blob = argv[0];
	    int argc;

	    for (argc = 0; argv[argc] != NULL; ++argc) {
		argv[argc] = expandFormat(xw, argv[argc], data, start, finish);
	    }
	    executeCommand(TScreenOf(xw)->pid, argv);
	    freeArgv(blob, argv);
	}
    }
}

void
HandleExecFormatted(Widget w,
		    XEvent *event,
		    String *params,	/* selections */
		    Cardinal *num_params)
{
    XtermWidget xw;

    TRACE_EVENT("HandleExecFormatted", event, params, num_params);
    if ((xw = getXtermWidget(w)) != NULL &&
	(*num_params > 1)) {
	doSelectionFormat(xw, w, event, params, num_params, reallyExecFormatted);
    }
}

void
HandleExecSelectable(Widget w,
		     XEvent *event,
		     String *params,	/* selections */
		     Cardinal *num_params)
{
    XtermWidget xw;

    if ((xw = getXtermWidget(w)) != NULL) {
	TRACE_EVENT("HandleExecSelectable", event, params, num_params);

	if (*num_params == 2) {
	    CELL start, finish;
	    char *data;
	    char **argv;

	    data = getDataFromScreen(xw, event, params[1], &start, &finish);
	    if (data != NULL) {
		if ((argv = tokenizeFormat(params[0])) != NULL) {
		    char *blob = argv[0];
		    int argc;

		    for (argc = 0; argv[argc] != NULL; ++argc) {
			argv[argc] = expandFormat(xw, argv[argc], data,
						  &start, &finish);
		    }
		    executeCommand(TScreenOf(xw)->pid, argv);
		    freeArgv(blob, argv);
		}
		free(data);
	    }
	}
    }
}
#endif /* OPT_EXEC_SELECTION */

static void
reallyInsertFormatted(Widget w, char *format, char *data, CELL *start, CELL *finish)
{
    XtermWidget xw;

    if ((xw = getXtermWidget(w)) != NULL) {
	char *exps;

	if ((exps = expandFormat(xw, format, data, start, finish)) != NULL) {
	    unparseputs(xw, exps);
	    unparse_end(xw);
	    free(exps);
	}
    }
}

void
HandleInsertFormatted(Widget w,
		      XEvent *event,
		      String *params,	/* selections */
		      Cardinal *num_params)
{
    XtermWidget xw;

    TRACE_EVENT("HandleInsertFormatted", event, params, num_params);
    if ((xw = getXtermWidget(w)) != NULL &&
	(*num_params > 1)) {
	doSelectionFormat(xw, w, event, params, num_params, reallyInsertFormatted);
    }
}

void
HandleInsertSelectable(Widget w,
		       XEvent *event,
		       String *params,	/* selections */
		       Cardinal *num_params)
{
    XtermWidget xw;

    if ((xw = getXtermWidget(w)) != NULL) {
	TRACE_EVENT("HandleInsertSelectable", event, params, num_params);

	if (*num_params == 2) {
	    CELL start, finish;
	    char *data;
	    char *temp = x_strdup(params[0]);

	    data = getDataFromScreen(xw, event, params[1], &start, &finish);
	    if (data != NULL) {
		char *exps = expandFormat(xw, temp, data, &start, &finish);
		if (exps != NULL) {
		    unparseputs(xw, exps);
		    unparse_end(xw);
		    free(exps);
		}
		free(data);
	    }
	    free(temp);
	}
    }
}
#endif /* OPT_SELECTION_OPS */
