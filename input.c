/*
 *	$XConsortium: input.c /main/21 1996/04/17 15:54:23 kaleb $
 *	$XFree86: xc/programs/xterm/input.c,v 3.28 1999/03/14 03:22:36 dawes Exp $
 */

/*
 * Copyright 1999 by Thomas E. Dickey <dickey@clark.net>
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
#if HAVE_X11_DECKEYSYM_H
#include <X11/DECkeysym.h>
#endif

#include <X11/Xutil.h>

#include <data.h>
#include <fontutils.h>

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
			keysym = table[n].after;
			TRACE(("...Input keypad changed to %#04lx\n", keysym))
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

void
Input (
	register TKeyboard *keyboard,
	register TScreen *screen,
	register XKeyEvent *event,
	Bool eightbit)
{

#define STRBUFSIZE 500

	char strbuf[STRBUFSIZE];
	register char *string;
	register int key = FALSE;
	int	pty	= screen->respond;
	int	nbytes;
	KeySym  keysym = 0;
	ANSI	reply;
	int	dec_code;

	/* Ignore characters typed at the keyboard */
	if (keyboard->flags & MODE_KAM)
		return;

#if OPT_I18N_SUPPORT
	if (screen->xic) {
	    Status status_return;
	    nbytes = XmbLookupString (screen->xic, event, strbuf, STRBUFSIZE,
				      &keysym, &status_return);
	}
	else
#endif
	{
	    static XComposeStatus compose_status = {NULL, 0};
	    nbytes = XLookupString (event, strbuf, STRBUFSIZE,
				    &keysym, &compose_status);
	}

	string = &strbuf[0];
	reply.a_pintro = 0;
	reply.a_final = 0;
	reply.a_nparam = 0;
	reply.a_inters = 0;

	TRACE(("Input keysym %#04lx, %d:'%.*s'%s%s%s%s%s%s%s%s\n",
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
		ModifierName(event->state & Mod5Mask)))

#if OPT_SUNPC_KBD
	/*
	 * DEC keyboards don't have keypad(+), but do have keypad(,) instead. 
	 * Other (Sun, PC) keyboards commonly have keypad(+), but no keypad(,)
	 * - it's a pain for users to work around.
	 */
	if (!sunFunctionKeys
	 && sunKeyboard
	 && keysym == XK_KP_Add)
		keysym = XK_KP_Separator;
#endif

	/*
	 * The keyboard tables may give us different keypad codes according to
	 * whether NumLock is pressed.  Use this check to simplify the process
	 * of determining whether we generate an escape sequence for a keypad
	 * key, or use the string returned by the keyboard tables.  There is no
	 * fixed modifier for this feature, so we assume that it is the one
	 * assigned to the NumLock key.
	 */
#if OPT_NUM_LOCK
	if (nbytes == 1
	 && IsKeypadKey(keysym)
	 && term->misc.real_NumLock
	 && (term->misc.num_lock & event->state) != 0) {
		keysym = *string;
		TRACE(("...Input num_lock, change keysym to %#04lx\n", keysym))
	}
#endif

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

	/* VT220 & up: National Replacement Characters */
	if ((nbytes == 1)
	 && (term->flags & NATIONAL)) {
		keysym = xtermCharSetIn(keysym, screen->keyboard_dialect[0]);
		if (keysym < 128) {
			strbuf[0] = keysym;
			TRACE(("...input NRC changed to %d\n", *strbuf))
		}
	}

	/*
	 * VT220 & up:  users expect that the Delete key on the editing keypad
	 * should be mapped to \E[3~.  However, we won't get there unless it is
	 * treated as a keypad key, which XK_Delete is not.  This presumes that
	 * we have a backarrow key to supply a DEL character, which is still
	 * needed in a number of applications.
	 */
#ifdef XK_KP_Delete
	if (keysym == XK_Delete) {
		keysym = XK_KP_Delete;
		TRACE(("...Input delete changed to %#04lx\n", keysym))
	}
#endif

	/* VT300 & up: backarrow toggle */
	if ((nbytes == 1)
	 && (((term->keyboard.flags & MODE_DECBKM) == 0)
	   ^ ((event->state & ControlMask) != 0))
	 && (keysym == XK_BackSpace)) {
		strbuf[0] = '\177';
		TRACE(("...Input backarrow changed to %d\n", *strbuf))
	}

#if OPT_SUNPC_KBD
	/* make an DEC editing-keypad from a Sun or PC editing-keypad */
	if (sunKeyboard)
		keysym = TranslateFromSUNPC(keysym);
	else
#endif
	{
#ifdef XK_KP_Home
	if (keysym >= XK_KP_Home && keysym <= XK_KP_Begin) {
		keysym += XK_Home - XK_KP_Home;
		TRACE(("...Input keypad changed to %#04lx\n", keysym))
	}
#endif
	}

#if OPT_HP_FUNC_KEYS
	if (hpFunctionKeys
	 && (reply.a_final = hpfuncvalue (keysym)) != 0) {
		reply.a_type = ESC;
		unparseseq(&reply, pty);
	} else
#endif
	if (IsPFKey(keysym)) {
		reply.a_type = SS3;
		reply.a_final = keysym-XK_KP_F1+'P';
		VT52_CURSOR_KEYS
		unparseseq(&reply, pty);
		key = TRUE;
#if 0	/* OPT_SUNPC_KBD should suppress - but only for vt220 compatibility */
	} else if (sunKeyboard
	 	&& screen->old_fkeys == False
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
			unparseseq(&reply, pty);
		} else {
			reply.a_type = CSI;
			if_OPT_VT52_MODE(screen,{ reply.a_type = ESC; })
			reply.a_final = curfinal[keysym-XK_Home];
			unparseseq(&reply, pty);
		}
		key = TRUE;
	 } else if (IsFunctionKey(keysym)
		|| IsMiscFunctionKey(keysym)
		|| IsEditFunctionKey(keysym)) {
#if OPT_SUNPC_KBD
		if ((event->state & ControlMask)
		 && sunKeyboard
		 && (keysym >= XK_F1 && keysym <= XK_F12))
			keysym += 12;
#endif

		dec_code = decfuncvalue(keysym);
		if ((event->state & ShiftMask)
		 && ((string = udk_lookup(dec_code, &nbytes)) != 0)) {
			while (nbytes-- > 0)
				unparseputc(*string++, pty);
		}
#if OPT_VT52_MODE
		/*
		 * Interpret F1-F4 as PF1-PF4 for VT52, VT100
		 */
		else if (!sunFunctionKeys
		 && screen->old_fkeys == False
		 && (dec_code >= 11 && dec_code <= 14))
		{
			reply.a_type = SS3;
			VT52_CURSOR_KEYS
			reply.a_final = dec_code - 11 + 'P';
			unparseseq(&reply, pty);
		}
#endif
		else {
			reply.a_type = CSI;
			reply.a_nparam = 1;
			if (sunFunctionKeys) {
				reply.a_param[0] = sunfuncvalue (keysym);
				reply.a_final = 'z';
			} else {
				reply.a_param[0] = dec_code;
				reply.a_final = '~';
			}
			if (reply.a_param[0] > 0)
				unparseseq(&reply, pty);
		}
		key = TRUE;
	} else if (IsKeypadKey(keysym)) {
		if ((keyboard->flags & MODE_DECKPAM) != 0) {
			reply.a_type  = SS3;
			reply.a_final = kypd_apl[keysym-XK_KP_Space];
			VT52_KEYPAD
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
		if ((nbytes == 1) && eightbit) {
		    if (screen->input_eight_bits)
		      *string |= 0x80;	/* turn on eighth bit */
		    else
		      unparseputc (ESC, pty);  /* escape */
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
StringInput ( register TScreen *screen, register char *string, size_t nbytes)
{
	int	pty	= screen->respond;

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
		case XK_F14:	return(195);
		case XK_F15:	return(196);
		case XK_Help:	return(196);
		case XK_F16:	return(197);
		case XK_Menu:	return(197);
		case XK_F17:	return(198);
		case XK_F18:	return(199);
		case XK_F19:	return(200);
		case XK_F20:	return(201);

		case XK_R1:	return(208);
		case XK_R2:	return(209);
		case XK_R3:	return(210);
		case XK_R4:	return(211);
		case XK_R5:	return(212);
		case XK_R6:	return(213);
		case XK_R7:	return(214);
		case XK_R8:	return(215);
		case XK_R9:	return(216);
		case XK_R10:	return(217);
		case XK_R11:	return(218);
		case XK_R12:	return(219);
		case XK_R13:	return(220);
		case XK_R14:	return(221);
		case XK_R15:	return(222);

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
		default:	return(-1);
	}
}

#if OPT_NUM_LOCK
/*
 * Determine which modifier mask (if any) applies to the Num_Lock keysym.
 */
void
VTInitModifiers(void)
{
    int i, j, k;
    Display *dpy = XtDisplay(term);
    XModifierKeymap *keymap = XGetModifierMapping(dpy);

    if (keymap != 0) {

	TRACE(("VTInitModifiers\n"))
	for (i = k = 0; i < 8; i++) {
	    for (j = 0; j < keymap->max_keypermod; j++) {
		KeyCode code = keymap->modifiermap[k];
		if (code != 0) {
		    KeySym keysym = XKeycodeToKeysym(dpy,code,0);
		    if (keysym == XK_Num_Lock) {
			term->misc.num_lock = (1<<i);
			TRACE(("numlock mask %#lx is%s modifier\n",
				term->misc.num_lock,
				ModifierName(term->misc.num_lock)))
		    }
		}
		k++;
	    }
	}

	XFreeModifiermap(keymap);
    }
}
#endif /* OPT_NUM_LOCK */
