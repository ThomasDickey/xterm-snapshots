/*
 *	$XConsortium: input.c /main/21 1996/04/17 15:54:23 kaleb $
 *	$XFree86: xc/programs/xterm/input.c,v 3.50 2000/12/07 02:40:00 dickey Exp $
 */

/*
 * Copyright 1999-2000 by Thomas E. Dickey
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

/* input.c */

#include <xterm.h>

#include <X11/keysym.h>

#ifdef VMS
#include <X11/keysymdef.h>
#endif

#ifdef HAVE_X11_DECKEYSYM_H
#include <X11/DECkeysym.h>
#endif

#ifdef HAVE_X11_SUNKEYSYM_H
#include <X11/Sunkeysym.h>
#endif

#include <X11/Xutil.h>
#include <X11/StringDefs.h>
#include <ctype.h>

#include <data.h>
#include <fontutils.h>
#include <keysym2ucs.h>

/*                       0123456789 abc def0123456789abdef0123456789abcdef0123456789abcd */
static char *kypd_num = " XXXXXXXX\tXXX\rXXXxxxxXXXXXXXXXXXXXXXXXXXXX*+,-./0123456789XX=";

/*                       0123456789abcdef0123456789abdef0123456789abcdef0123456789abcd */
static char *kypd_apl = " ABCDEFGHIJKLMNOPQRSTUVWXYZ??????abcdefghijklmnopqrstuvwxyzXX";

static char *curfinal = "HDACB  FE";

static int decfuncvalue (KeySym keycode);
static int sunfuncvalue (KeySym keycode);
#if OPT_HP_FUNC_KEYS
static int hpfuncvalue (KeySym keycode);
#endif
#if OPT_SCO_FUNC_KEYS
static int scofuncvalue (KeySym keycode);
#endif

#if OPT_TRACE
static char *
ModifierName(unsigned modifier)
{
    char *s = "";
    if (modifier & ShiftMask)
	s = " Shift";
    else if (modifier & LockMask)
	s = " Lock";
    else if (modifier & ControlMask)
	s = " Control";
    else if (modifier & Mod1Mask)
	s = " Mod1";
    else if (modifier & Mod2Mask)
	s = " Mod2";
    else if (modifier & Mod3Mask)
	s = " Mod3";
    else if (modifier & Mod4Mask)
	s = " Mod4";
    else if (modifier & Mod5Mask)
	s = " Mod5";
    return s;
}
#endif

static void
AdjustAfterInput (register TScreen *screen)
{
	if(screen->scrollkey && screen->topline != 0)
		WindowScroll(screen, 0);
	if(screen->marginbell) {
		int col = screen->max_col - screen->nmarginbell;
		if(screen->bellarmed >= 0) {
			if(screen->bellarmed == screen->cur_row) {
			    if(screen->cur_col >= col) {
				Bell(XkbBI_MarginBell,0);
				screen->bellarmed = -1;
			    }
			} else
			    screen->bellarmed =
				screen->cur_col < col ? screen->cur_row : -1;
		} else if(screen->cur_col < col)
			screen->bellarmed = screen->cur_row;
	}
}

/* returns true if the key is on the editing keypad */
static Boolean
IsEditFunctionKey(KeySym keysym)
{
	switch (keysym) {
	case XK_Prior:
	case XK_Next:
	case XK_Insert:
	case XK_Find:
	case XK_Select:
#ifdef DXK_Remove
	case DXK_Remove:
#endif
#ifdef XK_KP_Delete
	case XK_KP_Delete:
	case XK_KP_Insert:
#endif
#ifdef XK_ISO_Left_Tab
	case XK_ISO_Left_Tab:
#endif
		return True;
	default:
		return False;
	}
}

#if OPT_SUNPC_KBD
/*
 * If we have told xterm that our keyboard is really a Sun/PC keyboard, this is
 * enough to make a reasonable approximation to DEC vt220 numeric and editing
 * keypads.
 */
static KeySym
TranslateFromSUNPC(KeySym keysym)
{
	static struct {
		KeySym before, after;
	} table[] = {
#ifdef DXK_Remove
		{ XK_Delete,       DXK_Remove },
#endif
		{ XK_Home,         XK_Find },
		{ XK_End,          XK_Select },
#ifdef XK_KP_Home
		{ XK_Delete,       XK_KP_Decimal },
		{ XK_KP_Delete,    XK_KP_Decimal },
		{ XK_KP_Insert,    XK_KP_0 },
		{ XK_KP_End,       XK_KP_1 },
		{ XK_KP_Down,      XK_KP_2 },
		{ XK_KP_Next,      XK_KP_3 },
		{ XK_KP_Left,      XK_KP_4 },
		{ XK_KP_Begin,     XK_KP_5 },
		{ XK_KP_Right,     XK_KP_6 },
		{ XK_KP_Home,      XK_KP_7 },
		{ XK_KP_Up,        XK_KP_8 },
		{ XK_KP_Prior,     XK_KP_9 },
#endif
	};
	unsigned n;

	for (n = 0; n < sizeof(table)/sizeof(table[0]); n++) {
		if (table[n].before == keysym) {
			TRACE(("...Input keypad before was %#04lx\n", keysym));
			keysym = table[n].after;
			TRACE(("...Input keypad changed to %#04lx\n", keysym));
			break;
		}
	}
	return keysym;
}
#endif

/*
 * Modifiers other than shift, control and numlock should be reserved for the
 * user.  We use the first two explicitly to support VT220 keyboard, and the
 * third is used implicitly in keyboard configuration to make the keypad work.
 */
#define isModified(event) \
    (event->state & \
	(Mod1Mask | Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask ))

#define VT52_KEYPAD \
	if_OPT_VT52_MODE(screen,{ \
		reply.a_type = ESC; \
		reply.a_pintro = '?'; \
		})

#define VT52_CURSOR_KEYS \
	if_OPT_VT52_MODE(screen,{ \
		reply.a_type = ESC; \
		})

#define MODIFIER_PARM \
	if (modify_parm > 1) { \
	    reply.a_param[(int) reply.a_nparam] = modify_parm; \
	    reply.a_nparam += 1; \
	}

#if OPT_WIDE_CHARS
/* Convert a Unicode value c into a UTF-8 sequence in strbuf */
int
convertFromUTF8(unsigned long c, Char *strbuf)
{
	int nbytes = 0;

	if (c < 0x80) {
		strbuf[nbytes++] = c;
	} else if (c < 0x800) {
		strbuf[nbytes++] = 0xc0 | (c >> 6);
		strbuf[nbytes++] = 0x80 | (c & 0x3f);
	} else if (c < 0x10000) {
		strbuf[nbytes++] = 0xe0 |  (c >> 12);
		strbuf[nbytes++] = 0x80 | ((c >>  6) & 0x3f);
		strbuf[nbytes++] = 0x80 | ( c        & 0x3f);
	} else if (c < 0x200000) {
		strbuf[nbytes++] = 0xf0 |  (c >> 18);
		strbuf[nbytes++] = 0x80 | ((c >> 12) & 0x3f);
		strbuf[nbytes++] = 0x80 | ((c >>  6) & 0x3f);
		strbuf[nbytes++] = 0x80 | ( c        & 0x3f);
	} else if (c < 0x4000000) {
		strbuf[nbytes++] = 0xf8 |  (c >> 24);
		strbuf[nbytes++] = 0x80 | ((c >> 18) & 0x3f);
		strbuf[nbytes++] = 0x80 | ((c >> 12) & 0x3f);
		strbuf[nbytes++] = 0x80 | ((c >>  6) & 0x3f);
		strbuf[nbytes++] = 0x80 | ( c        & 0x3f);
	} else if (c < UCS_LIMIT) {
		strbuf[nbytes++] = 0xfe |  (c >> 30);
		strbuf[nbytes++] = 0x80 | ((c >> 24) & 0x3f);
		strbuf[nbytes++] = 0x80 | ((c >> 18) & 0x3f);
		strbuf[nbytes++] = 0x80 | ((c >> 12) & 0x3f);
		strbuf[nbytes++] = 0x80 | ((c >> 6)  & 0x3f);
		strbuf[nbytes++] = 0x80 | ( c        & 0x3f);
	} else
		return convertFromUTF8(UCS_REPL, strbuf);

	return nbytes;
}
#endif /* OPT_WIDE_CHARS */

/*
 * Determine if we use the \E[3~ sequence for Delete, or the legacy ^?.  We
 * maintain the delete_is_del value as 3 states:  unspecified(2), true and
 * false.  If unspecified, it is handled differently according to whether the
 * legacy keybord support is enabled, or if xterm emulates a VT220.
 *
 * Once the user (or application) has specified delete_is_del via resource
 * settting, popup menu or escape sequence, it overrides the keyboard type
 * rather than the reverse.
 */
Boolean
xtermDeleteIsDEL(void)
{
	TScreen *screen = &term->screen;
	Boolean result = True;

	if (term->keyboard.type == keyboardIsDefault
	 || term->keyboard.type == keyboardIsVT220)
		result = (screen->delete_is_del == True);

	if (term->keyboard.type == keyboardIsLegacy)
		result = (screen->delete_is_del != False);

	TRACE(("xtermDeleteIsDEL(%d/%d) = %d\n",
		term->keyboard.type,
		screen->delete_is_del,
		result));

	return result;
}

void
Input (
	register TKeyboard *keyboard,
	register TScreen *screen,
	register XKeyEvent *event,
	Bool eightbit)
{

#define STRBUFSIZE 500

	char strbuf[STRBUFSIZE];
	register Char *string;
	register int key = FALSE;
	int	pty	= screen->respond;
	int	nbytes;
	KeySym  keysym = 0;
	ANSI	reply;
	int	dec_code;
	short	modify_parm = 0;
	int	keypad_mode = ((keyboard->flags & MODE_DECKPAM) != 0);
#if OPT_WIDE_CHARS
	long    ucs;
#endif

	/* Ignore characters typed at the keyboard */
	if (keyboard->flags & MODE_KAM)
		return;

#if OPT_TCAP_QUERY
	if (screen->tc_query >= 0) {
		keysym = screen->tc_query;
		nbytes = 0;
		strbuf[0] = 0;
	}
	else
#endif
#if OPT_I18N_SUPPORT
	if (screen->xic
#if OPT_WIDE_CHARS
	 && !screen->utf8_mode
#endif
	 ) {
	    Status status_return;
	    nbytes = XmbLookupString (screen->xic, event,
				      strbuf, sizeof(strbuf),
				      &keysym, &status_return);
	}
	else
#endif
	{
	    static XComposeStatus compose_status = {NULL, 0};
	    nbytes = XLookupString (event, strbuf, sizeof(strbuf),
				    &keysym, &compose_status);
	}

#if OPT_WIDE_CHARS
	/*
	 * FIXME:  As long as Xlib does not provide proper UTF-8 conversion via
	 * XLookupString(), we have to generate them here.  Once Xlib is fully
	 * UTF-8 capable, this code here should be removed again.
	 */
	if (screen->utf8_mode) {
		ucs = -1;
		if (nbytes == 1) {
		/* Take ISO 8859-1 character delivered by XLookupString() */
			ucs = (unsigned char) strbuf[0];
		} else if (!nbytes &&
			   ((keysym >= 0x100 && keysym <= 0xf000) ||
			    (keysym & 0xff000000U) == 0x01000000))
			ucs = keysym2ucs(keysym);
		else
			ucs = -2;
		if (ucs == -1)
			nbytes = 0;
		if (ucs >= 0)
			nbytes = convertFromUTF8(ucs, (Char *)strbuf);
	}
#endif

	string = (Char *)&strbuf[0];
	reply.a_pintro = 0;
	reply.a_final = 0;
	reply.a_nparam = 0;
	reply.a_inters = 0;

	TRACE(("Input keysym %#04lx, %d:'%.*s'%s%s%s%s%s%s%s%s%s%s%s%s\n",
		keysym,
		nbytes,
		nbytes > 0 ? nbytes : 1,
		nbytes > 0 ? strbuf : "",
		ModifierName(event->state & ShiftMask),
		ModifierName(event->state & LockMask),
		ModifierName(event->state & ControlMask),
		ModifierName(event->state & Mod1Mask),
		ModifierName(event->state & Mod2Mask),
		ModifierName(event->state & Mod3Mask),
		ModifierName(event->state & Mod4Mask),
		ModifierName(event->state & Mod5Mask),
		eightbit ? " 8bit" : " 7bit",
		IsFunctionKey(keysym)     ? " FKey"     : "",
		IsMiscFunctionKey(keysym) ? " MiscFKey" : "",
		IsEditFunctionKey(keysym) ? " EditFkey" : ""));

#if OPT_SUNPC_KBD
	/*
	 * DEC keyboards don't have keypad(+), but do have keypad(,) instead.
	 * Other (Sun, PC) keyboards commonly have keypad(+), but no keypad(,)
	 * - it's a pain for users to work around.
	 */
	if (term->keyboard.type != keyboardIsSun
	 && (event->state & ShiftMask) == 0
	 && term->keyboard.type == keyboardIsVT220
	 && keysym == XK_KP_Add) {
		keysym = XK_KP_Separator;
		TRACE(("...Input keypad(+), change keysym to %#04lx\n", keysym));
	}
#endif

	/*
	 * The keyboard tables may give us different keypad codes according to
	 * whether NumLock is pressed.  Use this check to simplify the process
	 * of determining whether we generate an escape sequence for a keypad
	 * key, or force it to the value kypd_num[].  There is no fixed
	 * modifier for this feature, so we assume that it is the one assigned
	 * to the NumLock key.
	 *
	 * This check used to try to return the contents of strbuf, but that
	 * does not work properly when a control modifier is given (trash is
	 * returned in the buffer in some cases -- perhaps an X bug).
	 */
#if OPT_NUM_LOCK
	if (nbytes == 1
	 && IsKeypadKey(keysym)
	 && term->misc.real_NumLock
	 && ((term->misc.num_lock == 0)
	  || (term->misc.num_lock & event->state) != 0)) {
		keypad_mode = 0;
		TRACE(("...Input num_lock, force keypad_mode off\n"));
	}
#endif

	/*
	 * If we are in the normal (possibly Sun/PC) keyboard state, allow
	 * modifiers to add a parameter to the function-key control sequences.
	 */
	if (event->state != 0
	 && !(IsKeypadKey(keysym) && keypad_mode)
#if OPT_SUNPC_KBD
	 && term->keyboard.type != keyboardIsVT220
#endif
#if OPT_VT52_MODE
	 && screen->ansi_level != 0
#endif
	) {
/*
* Modifier codes:
*	None		1
*	Shift		2 = 1(None)+1(Shift)
*	Alt		3 = 1(None)+2(Alt)
*	Alt+Shift	4 = 1(None)+1(Shift)+2(Alt)
*	Ctrl		5 = 1(None)+4(Ctrl)
*	Ctrl+Shift	6 = 1(None)+1(Shift)+4(Ctrl)
*	Ctrl+Alt	7 = 1(None)+2(Alt)+4(Ctrl)
*	Ctrl+Alt+Shift	8 = 1(None)+1(Shift)+2(Alt)+4(Ctrl)
*/
#define	UNMOD	1
#define	SHIFT	1
#define	ALT	2
#define	CTRL	4
#define	META	8
	    modify_parm = UNMOD;
	    if (event->state & ShiftMask) {
		modify_parm += SHIFT;
	    }
	    if (event->state & ControlMask) {
		modify_parm += CTRL;
	    }
#if OPT_NUM_LOCK
	    if ((term->misc.alwaysUseMods
	      || term->misc.real_NumLock)
	     && ((event->state & term->misc.alt_left) != 0
	      || (event->state & term->misc.alt_right)) != 0) {
		modify_parm += ALT;
	    }
	    if (term->misc.alwaysUseMods
	     && ((event->state & term->misc.meta_left) != 0
	      || (event->state & term->misc.meta_right)) != 0) {
		modify_parm += META;
	    }
#endif
	    TRACE(("...ModifierParm %d\n", modify_parm));
	}

#if OPT_SHIFT_KEYS
	if (term->misc.shift_keys
	 && (event->state & ShiftMask) != 0) {
		switch (keysym) {
		case XK_KP_Add:
		    HandleLargerFont((Widget)0, (XEvent *)0, (String *)0, (Cardinal *)0);
		    return;
		case XK_KP_Subtract:
		    HandleSmallerFont((Widget)0, (XEvent *)0, (String *)0, (Cardinal *)0);
		    return;
		default:
		    break;
		}
	}
#endif

	/* VT300 & up: backarrow toggle */
	if ((nbytes == 1)
	 && (((term->keyboard.flags & MODE_DECBKM) == 0)
	   ^ ((event->state & ControlMask) != 0))
	 && (keysym == XK_BackSpace)) {
		strbuf[0] = '\177';
		TRACE(("...Input backarrow changed to %d\n", *strbuf));
	}

#if OPT_SUNPC_KBD
	/* make an DEC editing-keypad from a Sun or PC editing-keypad */
	if (term->keyboard.type == keyboardIsVT220
	 && (keysym != XK_Delete || !xtermDeleteIsDEL()))
		keysym = TranslateFromSUNPC(keysym);
	else
#endif
	{
#ifdef XK_KP_Home
	if (keysym >= XK_KP_Home && keysym <= XK_KP_Begin) {
		TRACE(("...Input keypad before was %#04lx\n", keysym));
		keysym += XK_Home - XK_KP_Home;
		TRACE(("...Input keypad changed to %#04lx\n", keysym));
	}
#endif
	}

#if OPT_HP_FUNC_KEYS
	if (term->keyboard.type == keyboardIsHP
	 && (reply.a_final = hpfuncvalue (keysym)) != 0) {
		reply.a_type = ESC;
		MODIFIER_PARM;
		unparseseq(&reply, pty);
	} else
#endif
#if OPT_SCO_FUNC_KEYS
	if (term->keyboard.type == keyboardIsSCO
	 && (reply.a_final = scofuncvalue (keysym)) != 0) {
		reply.a_type = CSI;
		MODIFIER_PARM;
		unparseseq(&reply, pty);
	} else
#endif
	if (IsPFKey(keysym)) {
		reply.a_type = SS3;
		reply.a_final = keysym-XK_KP_F1+'P';
		VT52_CURSOR_KEYS
		MODIFIER_PARM;
		unparseseq(&reply, pty);
		key = TRUE;
#if 0	/* OPT_SUNPC_KBD should suppress - but only for vt220 compatibility */
	} else if (term->keyboard.type == keyboardIsVT220
		&& screen->ansi_level <= 1
		&& IsEditFunctionKey(keysym)) {
		key = FALSE;	/* ignore editing-keypad in vt100 mode */
#endif
	} else if (IsCursorKey(keysym) &&
		keysym != XK_Prior && keysym != XK_Next) {
		if (keyboard->flags & MODE_DECCKM) {
			reply.a_type = SS3;
			reply.a_final = curfinal[keysym-XK_Home];
			VT52_CURSOR_KEYS
			MODIFIER_PARM;
			unparseseq(&reply, pty);
		} else {
			reply.a_type = CSI;
			if_OPT_VT52_MODE(screen,{ reply.a_type = ESC; })
			reply.a_final = curfinal[keysym-XK_Home];
			MODIFIER_PARM;
			unparseseq(&reply, pty);
		}
		key = TRUE;
	 } else if (IsFunctionKey(keysym)
		|| IsMiscFunctionKey(keysym)
		|| IsEditFunctionKey(keysym)
#ifdef SunXK_F36
		|| keysym == SunXK_F36
		|| keysym == SunXK_F37
#endif
		|| (keysym == XK_Delete
		 && ((modify_parm > 1)
		  || !xtermDeleteIsDEL()))) {
#if OPT_SUNPC_KBD
		if (term->keyboard.type == keyboardIsVT220) {
			if ((event->state & ControlMask)
			 && (keysym >= XK_F1 && keysym <= XK_F12))
				keysym += term->misc.ctrl_fkeys;
		}
#endif

		dec_code = decfuncvalue(keysym);
		if ((event->state & ShiftMask)
#if OPT_SUNPC_KBD
		 && term->keyboard.type == keyboardIsVT220
#endif
		 && ((string = (Char *)udk_lookup(dec_code, &nbytes)) != 0)) {
			while (nbytes-- > 0)
				unparseputc(*string++, pty);
		}
#if OPT_VT52_MODE
		/*
		 * Interpret F1-F4 as PF1-PF4 for VT52, VT100
		 */
		else if (term->keyboard.type != keyboardIsSun
		 && term->keyboard.type != keyboardIsLegacy
		 && (dec_code >= 11 && dec_code <= 14))
		{
			reply.a_type = SS3;
			VT52_CURSOR_KEYS
			reply.a_final = A2E(dec_code - 11 + E2A('P')) ;
			MODIFIER_PARM;
			unparseseq(&reply, pty);
		}
#endif
		else {
			reply.a_type = CSI;
			reply.a_nparam = 1;
			reply.a_final = 0;
			MODIFIER_PARM;
			if (term->keyboard.type == keyboardIsSun ) {
				reply.a_param[0] = sunfuncvalue (keysym);
				reply.a_final = 'z';
#ifdef XK_ISO_Left_Tab
			} else if (keysym == XK_ISO_Left_Tab) {
				reply.a_nparam = 0;
				reply.a_final = 'Z';
#endif
			} else {
				reply.a_param[0] = dec_code;
				reply.a_final = '~';
			}
			if (reply.a_final != 0
			 && (reply.a_nparam == 0 || reply.a_param[0] >= 0))
				unparseseq(&reply, pty);
		}
		key = TRUE;
	} else if (IsKeypadKey(keysym)) {
		if (keypad_mode) {
			reply.a_type  = SS3;
			reply.a_final = kypd_apl[keysym-XK_KP_Space];
			VT52_KEYPAD
			MODIFIER_PARM;
			unparseseq(&reply, pty);
		} else {
			unparseputc(kypd_num[keysym-XK_KP_Space], pty);
		}
		key = TRUE;
	} else if (nbytes > 0) {
#if OPT_TEK4014
		if(screen->TekGIN) {
			TekEnqMouse(*string++);
			TekGINoff();
			nbytes--;
		}
#endif
		if (nbytes == 1) {
#if OPT_NUM_LOCK
			/*
			 * Send ESC if we have a META modifier and
			 * metaSendsEcape is true.  Like eightBitInput, except
			 * that it is not associated with terminal settings.
			 */
			if (eightbit
			 && screen->meta_sends_esc
			 && ((event->state & term->misc.meta_left) != 0
			  || (event->state & term->misc.meta_right)) != 0) {
				TRACE(("...input-char is modified by META\n"));
				eightbit = False;
				unparseputc (ESC, pty);  /* escape */
			}
#endif
			if (eightbit && screen->input_eight_bits) {
				if (CharOf(*string) < 128) {
					TRACE(("...input shift from %d to %d\n",
						CharOf(*string),
						CharOf(*string) | 0x80));
					*string |= 0x80;
				}
				eightbit = False;
			}
			/* VT220 & up: National Replacement Characters */
			if ((term->flags & NATIONAL) != 0) {
				int cmp = xtermCharSetIn(CharOf(*string), screen->keyboard_dialect[0]);
				TRACE(("...input NRC %d, %s %d\n",
					CharOf(*string),
					(CharOf(*string) == cmp)
						? "unchanged"
						: "changed to",
					CharOf(cmp)));
				*string = cmp;
			} else if (eightbit) {
				unparseputc (ESC, pty);  /* escape */
			} else if (*string == '?'
			    && (event->state & ControlMask) != 0) {
				*string = 127;
			}
		}
		while (nbytes-- > 0)
			unparseputc(*string++, pty);
		key = TRUE;
	}
	if(key && !TEK4014_ACTIVE(screen))
		AdjustAfterInput(screen);
#ifdef ENABLE_PRINT
	if (keysym == XK_F2) TekPrint();
#endif
	return;
}

void
StringInput ( register TScreen *screen, Char *string, size_t nbytes)
{
	int	pty	= screen->respond;

	TRACE(("InputString (%s,%d)\n", visibleChars(PAIRED_CHARS(string,0), nbytes), nbytes));
#if OPT_TEK4014
	if(nbytes && screen->TekGIN) {
		TekEnqMouse(*string++);
		TekGINoff();
		nbytes--;
	}
#endif
	while (nbytes-- != 0)
		unparseputc(*string++, pty);
	if (!TEK4014_ACTIVE(screen))
		AdjustAfterInput(screen);
}

/* These definitions are DEC-style (e.g., vt320) */
static int
decfuncvalue (KeySym keycode)
{
	switch (keycode) {
		case XK_F1:	return(11);
		case XK_F2:	return(12);
		case XK_F3:	return(13);
		case XK_F4:	return(14);
		case XK_F5:	return(15);
		case XK_F6:	return(17);
		case XK_F7:	return(18);
		case XK_F8:	return(19);
		case XK_F9:	return(20);
		case XK_F10:	return(21);
		case XK_F11:	return(23);
		case XK_F12:	return(24);
		case XK_F13:	return(25);
		case XK_F14:	return(26);
		case XK_F15:	return(28);
		case XK_Help:	return(28);
		case XK_F16:	return(29);
		case XK_Menu:	return(29);
		case XK_F17:	return(31);
		case XK_F18:	return(32);
		case XK_F19:	return(33);
		case XK_F20:	return(34);
#ifdef SunXK_F36
		case SunXK_F36:	return(57);
		case SunXK_F37:	return(58);
#endif

		case XK_Find :	return(1);
		case XK_Insert:	return(2);
		case XK_Delete:	return(3);
#ifdef XK_KP_Insert
		case XK_KP_Insert: return(2);
		case XK_KP_Delete: return(3);
#endif
#ifdef DXK_Remove
		case DXK_Remove: return(3);
#endif
		case XK_Select:	return(4);
		case XK_Prior:	return(5);
		case XK_Next:	return(6);
#ifdef XK_ISO_Left_Tab
		case XK_ISO_Left_Tab: return('Z');
#endif
		default:	return(-1);
	}
}

#if OPT_HP_FUNC_KEYS
static int
hpfuncvalue (KeySym  keycode)
{
	switch (keycode) {
		case XK_Up:		return('A');
		case XK_Down:		return('B');
		case XK_Right:		return('C');
		case XK_Left:		return('D');
		case XK_End:		return('F');
		case XK_Clear:		return('J');
		case XK_Delete:		return('P');
		case XK_Insert:		return('Q');
		case XK_Next:		return('S');
		case XK_Prior:		return('T');
		case XK_Home:		return('h');
		case XK_F1:		return('p');
		case XK_F2:		return('q');
		case XK_F3:		return('r');
		case XK_F4:		return('s');
		case XK_F5:		return('t');
		case XK_F6:		return('u');
		case XK_F7:		return('v');
		case XK_F8:		return('w');
#ifdef XK_KP_Insert
		case XK_KP_Delete:	return('P');
		case XK_KP_Insert:	return('Q');
#endif
#ifdef DXK_Remove
		case DXK_Remove:	return('P');
#endif
		case XK_Select:		return('F');
		case XK_Find:		return('h');
		default:		return 0;
	}
}
#endif

#if OPT_SCO_FUNC_KEYS
static int
scofuncvalue (KeySym  keycode)
{
	switch (keycode) {
		case XK_Up:		return('A');
		case XK_Down:		return('B');
		case XK_Right:		return('C');
		case XK_Left:		return('D');
		case XK_End:		return('F');
		case XK_Insert:		return('L');
		case XK_Next:		return('G');
		case XK_Prior:		return('I');
		case XK_Home:		return('H');
		case XK_F1:		return('M');
		case XK_F2:		return('N');
		case XK_F3:		return('O');
		case XK_F4:		return('P');
		case XK_F5:		return('Q');
		case XK_F6:		return('R');
		case XK_F7:		return('S');
		case XK_F8:		return('T');
		case XK_F9:		return('U');
		case XK_F10:		return('V');
		case XK_F11:		return('W');
		case XK_F12:		return('X');
		case XK_F13:		return('Y');
		case XK_F15:		return('a');
		case XK_F16:		return('b');
		case XK_F17:		return('c');
		case XK_F18:		return('d');
		case XK_F19:		return('e');
		case XK_F20:		return('f');
#if defined(XK_F21)
		case XK_F21:		return('g');
		case XK_F22:		return('h');
		case XK_F23:		return('i');
		case XK_F24:		return('j');
		case XK_F25:		return('k');
		case XK_F26:		return('l');
		case XK_F27:		return('m');
		case XK_F28:		return('n');
		case XK_F29:		return('o');
		case XK_F30:		return('p');
		case XK_F31:		return('q');
		case XK_F32:		return('r');
		case XK_F33:		return('s');
		case XK_F34:		return('t');
		case XK_F35:		return('u');
#endif
#ifdef XK_KP_Insert
		case XK_KP_Insert:	return('L');
#endif
		default:		return 0;
	}
}
#endif

static int
sunfuncvalue (KeySym  keycode)
{
	switch (keycode) {
		case XK_F1:	return(224);
		case XK_F2:	return(225);
		case XK_F3:	return(226);
		case XK_F4:	return(227);
		case XK_F5:	return(228);
		case XK_F6:	return(229);
		case XK_F7:	return(230);
		case XK_F8:	return(231);
		case XK_F9:	return(232);
		case XK_F10:	return(233);
		case XK_F11:	return(192);
		case XK_F12:	return(193);
		case XK_F13:	return(194);
		case XK_F14:	return(195);	/* kund */
		case XK_F15:	return(196);
		case XK_Help:	return(196);	/* khlp */
		case XK_F16:	return(197);	/* kcpy */
		case XK_Menu:	return(197);
		case XK_F17:	return(198);
		case XK_F18:	return(199);
		case XK_F19:	return(200);	/* kfnd */
		case XK_F20:	return(201);

		case XK_R1:	return(208);	/* kf31 */
		case XK_R2:	return(209);	/* kf32 */
		case XK_R3:	return(210);	/* kf33 */
		case XK_R4:	return(211);	/* kf34 */
		case XK_R5:	return(212);	/* kf35 */
		case XK_R6:	return(213);	/* kf36 */
		case XK_R7:	return(214);	/* kf37 */
		case XK_R8:	return(215);	/* kf38 */
		case XK_R9:	return(216);	/* kf39=kpp */
		case XK_R10:	return(217);	/* kf40 */
		case XK_R11:	return(218);	/* kf41=kb2 */
		case XK_R12:	return(219);	/* kf42 */
		case XK_R13:	return(220);	/* kf43=kend */
		case XK_R14:	return(221);	/* kf44 */
		case XK_R15:	return(222);	/* kf45 */
#ifdef SunXK_F36
		case SunXK_F36:	return(234);
		case SunXK_F37:	return(235);
#endif

		case XK_Find :	return(1);
		case XK_Insert:	return(2);	/* kich1 */
		case XK_Delete:	return(3);
#ifdef XK_KP_Insert
		case XK_KP_Insert: return(2);
		case XK_KP_Delete: return(3);
#endif
#ifdef DXK_Remove
		case DXK_Remove: return(3);
#endif
		case XK_Select:	return(4);
		case XK_Prior:	return(5);
		case XK_Next:	return(6);
		default:	return(-1);
	}
}

#if OPT_NUM_LOCK
/*
 * Note that this can only retrieve translations that are given as resource
 * values; the default translations in charproc.c for example are not
 * retrievable by any interface to X.
 *
 * Also:  We can retrieve only the most-specified translation resource.  For
 * example, if the resource file specifies both "*translations" and
 * "XTerm*translations", we see only the latter.
 */
static Bool
TranslationsUseKeyword(Widget w, const char *keyword)
{
    static String data;
    static XtResource key_resources[] = {
	{ XtNtranslations, XtCTranslations, XtRString,
	      sizeof(data), 0, XtRString, (XtPointer)NULL}
    };
    Bool result = False;

    XtGetSubresources( w, (XtPointer)&data, "vt100", "VT100",
		   key_resources, XtNumber(key_resources), NULL, (Cardinal)0 );

    if (data != 0) {
	char *p = data;
	int state = 0;
	int now = ' ', prv;
	while (*p != 0) {
	    prv = now;
	    now = char2lower(*p++);
	    if (now == ':'
	     || now == '!') {
		state = -1;
	    } else if (now == '\n') {
		state = 0;
	    } else if (state >= 0) {
		if (isgraph(now)
		 && now == keyword[state]) {
		    if ((state != 0
		      || !isalnum(prv))
		     && ((keyword[++state] == 0)
		      && !isalnum(CharOf(*p)))) {
			result = True;
			break;
		    }
		} else {
		    state = 0;
		}
	    }
	}
    }
    return result;
}

#define SaveMask(name)	term->misc.name = mask;\
			TRACE(("%s mask %#lx is%s modifier\n", \
				#name, \
				term->misc.name, \
				ModifierName(term->misc.name)));
/*
 * Determine which modifier mask (if any) applies to the Num_Lock keysym.
 *
 * Also, determine which modifiers are associated with the ALT keys, so we can
 * send that information as a parameter for special keys in Sun/PC keyboard
 * mode.  However, if the ALT modifier is used in translations, we do not want
 * to confuse things by sending the parameter.
 */
void
VTInitModifiers(void)
{
    int i, j, k;
    Display *dpy = XtDisplay(term);
    XModifierKeymap *keymap = XGetModifierMapping(dpy);
    unsigned long mask;

    if (keymap != 0) {

	TRACE(("VTInitModifiers\n"));
	for (i = k = 0, mask = 1; i < 8; i++, mask <<= 1) {
	    for (j = 0; j < keymap->max_keypermod; j++) {
		KeyCode code = keymap->modifiermap[k];
		if (code != 0) {
		    KeySym keysym = XKeycodeToKeysym(dpy,code,0);
		    if (keysym == XK_Num_Lock) {
			SaveMask(num_lock);
		    } else if (keysym == XK_Alt_L) {
			SaveMask(alt_left);
		    } else if (keysym == XK_Alt_R) {
			SaveMask(alt_right);
		    } else if (keysym == XK_Meta_L) {
			SaveMask(meta_left);
		    } else if (keysym == XK_Meta_R) {
			SaveMask(meta_right);
		    }
		}
		k++;
	    }
	}

	/* Don't disable any mods if "alwaysUseMods" is true. */
	if (!term->misc.alwaysUseMods) {
	    /*
	     * If the Alt modifier is used in translations, we would rather not
	     * use it to modify function-keys when NumLock is active.
	     */
	    if ((term->misc.alt_left != 0
	      || term->misc.alt_right != 0)
	     && (TranslationsUseKeyword(toplevel, "alt")
	      || TranslationsUseKeyword((Widget)term, "alt"))) {
		TRACE(("ALT is used as a modifier in translations (ignore mask)\n"));
		term->misc.alt_left = 0;
		term->misc.alt_right = 0;
	    }

	    /*
	     * If the Meta modifier is used in translations, we would rather not
	     * use it to modify function-keys.
	     */
	    if ((term->misc.meta_left != 0
	      || term->misc.meta_right != 0)
	     && (TranslationsUseKeyword(toplevel, "meta")
	      || TranslationsUseKeyword((Widget)term, "meta"))) {
		TRACE(("META is used as a modifier in translations\n"));
		term->misc.meta_trans = True;
	    }
	}

	XFreeModifiermap(keymap);
    }
}
#endif /* OPT_NUM_LOCK */

#if OPT_TCAP_QUERY
	static int
hex2int(int c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}

/*
 * Parse the termcap/terminfo name from the string, returning a positive number
 * (the keysym) if found, otherwise -1.  Update the string pointer.
 * Returns the (shift, control) state in *state.
 */
int
xtermcapKeycode(char *params, unsigned *state)
{
#define DATA(tc,ti,x,y) { tc, ti, x, y }
	static struct {
		char *tc;
		char *ti;
		int code;
		unsigned state;
	} table[] = {
		DATA(	"#2",	"kHOM",		XK_Home,	ShiftMask),
		DATA(	"#4",	"kLFT",		XK_Left,	ShiftMask),
		DATA(	"%1",	"khlp",		XK_Help,	0),
		DATA(	"%i",	"kRIT",		XK_Right,	ShiftMask),
		DATA(	"*6",	"kslt",		XK_Select,	0),
		DATA(	"*7",	"kEND",		XK_End,		ShiftMask),
		DATA(	"@0",	"kfnd",		XK_Find,	0),
		DATA(	"@7",	"kend",		XK_End,		0),
		DATA(	"F1",	"kf11",		XK_F11,		0),
		DATA(	"F2",	"kf12",		XK_F12,		0),
		DATA(	"F3",	"kf13",		XK_F13,		0),
		DATA(	"F4",	"kf14",		XK_F14,		0),
		DATA(	"F5",	"kf15",		XK_F15,		0),
		DATA(	"F6",	"kf16",		XK_F16,		0),
		DATA(	"F7",	"kf17",		XK_F17,		0),
		DATA(	"F8",	"kf18",		XK_F18,		0),
		DATA(	"F9",	"kf19",		XK_F19,		0),
		DATA(	"FA",	"kf20",		XK_F20,		0),
		DATA(	"FB",	"kf21",		XK_F21,		0),
		DATA(	"FC",	"kf22",		XK_F22,		0),
		DATA(	"FD",	"kf23",		XK_F23,		0),
		DATA(	"FE",	"kf24",		XK_F24,		0),
		DATA(	"FF",	"kf25",		XK_F25,		0),
		DATA(	"FG",	"kf26",		XK_F26,		0),
		DATA(	"FH",	"kf27",		XK_F27,		0),
		DATA(	"FI",	"kf28",		XK_F28,		0),
		DATA(	"FJ",	"kf29",		XK_F29,		0),
		DATA(	"FK",	"kf30",		XK_F30,		0),
		DATA(	"FL",	"kf31",		XK_F31,		0),
		DATA(	"FM",	"kf32",		XK_F32,		0),
		DATA(	"FN",	"kf33",		XK_F33,		0),
		DATA(	"FO",	"kf34",		XK_F34,		0),
		DATA(	"FP",	"kf35",		XK_F35,		0),
		DATA(	"FQ",	"kf36",		SunXK_F36,	0),
		DATA(	"FR",	"kf37",		SunXK_F37,	0),
		DATA(	"K1",	"ka1",		XK_KP_Home,	0),
		DATA(	"K4",	"kc1",		XK_KP_End,	0),
		DATA(	"k1",	"kf1",		XK_F1,		0),
		DATA(	"k2",	"kf2",		XK_F2,		0),
		DATA(	"k3",	"kf3",		XK_F3,		0),
		DATA(	"k4",	"kf4",		XK_F4,		0),
		DATA(	"k5",	"kf5",		XK_F5,		0),
		DATA(	"k6",	"kf6",		XK_F6,		0),
		DATA(	"k7",	"kf7",		XK_F7,		0),
		DATA(	"k8",	"kf8",		XK_F8,		0),
		DATA(	"k9",	"kf9",		XK_F9,		0),
		DATA(	"k;",	"kf10",		XK_F10,		0),
#ifdef XK_ISO_Left_Tab
		DATA(	"kB",	"kcbt",		XK_ISO_Left_Tab,0),
#endif
		DATA(	"kC",	"kclr",		XK_Clear,	0),
		DATA(	"kD",	"kdch1",	XK_Delete,	0),
		DATA(	"kI",	"kich1",	XK_Insert,	0),
		DATA(	"kN",	"knp",		XK_Next,	0),
		DATA(	"kP",	"kpp",		XK_Prior,	0),
		DATA(	"kb",	"kbs",		XK_BackSpace,	0),
		DATA(	"kd",	"kcud1",	XK_Down,	0),
		DATA(	"kh",	"khome",	XK_Home,	0),
		DATA(	"kl",	"kcub1",	XK_Left,	0),
		DATA(	"kr",	"kcuf1",	XK_Right,	0),
		DATA(	"ku",	"kcuu1",	XK_Up,		0),
# if OPT_ISO_COLORS
		/* XK_COLORS is a fake code. */
		DATA(	"Co",	"colors",	XK_COLORS,	0),
# endif
	};
	Cardinal n;
	unsigned len = 0;
	int code = -1;
#define MAX_TNAME_LEN 6
	char name[MAX_TNAME_LEN];
	char *p;

	/* Convert hex encoded name to ascii */
	for (p = params; hex2int(p[0]) >= 0 && hex2int(p[1]) >= 0; p += 2) {
		if (len == MAX_TNAME_LEN - 1)
			return -1;
		name[len++] = (hex2int(p[0]) << 4) + hex2int(p[1]);
	}
	if (*p)
		return -1;
	name[len] = 0;
	for (n = 0; n < XtNumber(table); n++) {
		if (!strcmp(table[n].ti, name) || !strcmp(table[n].tc, name)) {
			code = table[n].code;
			*state = table[n].state;
			break;
		}
	}
	return code;
}
#endif
