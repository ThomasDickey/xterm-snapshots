/*
 *	$XConsortium: input.c /main/21 1996/04/17 15:54:23 kaleb $
 *	$XFree86: xc/programs/xterm/input.c,v 3.18 1998/03/27 23:24:01 hohndel Exp $
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

/* input.c */

#ifdef HAVE_CONFIG_H
#include <xtermcfg.h>
#endif

#include "ptyx.h"		/* gets Xt headers, too */

#include <X11/keysym.h>
#if HAVE_X11_DECKEYSYM_H
#include <X11/DECkeysym.h>
#endif

#include <X11/Xutil.h>
#include <stdio.h>

#include "xterm.h"
#include "data.h"

static char *kypd_num = " XXXXXXXX\tXXX\rXXXxxxxXXXXXXXXXXXXXXXXXXXXX*+,-./0123456789XXX=";
static char *kypd_apl = " ABCDEFGHIJKLMNOPQRSTUVWXYZ??????abcdefghijklmnopqrstuvwxyzXXX";
static char *cur = "HDACB  FE";

static int decfuncvalue PROTO((KeySym keycode));
static int sunfuncvalue PROTO((KeySym keycode));
static void AdjustAfterInput PROTO((TScreen *screen));

static void
AdjustAfterInput (screen)
register TScreen *screen;
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

void
Input (keyboard, screen, event, eightbit)
    register TKeyboard	*keyboard;
    register TScreen	*screen;
    register XKeyEvent *event;
    Bool eightbit;
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

	/* VT300 & up: backarrow toggle */
	if ((nbytes == 1)
	 && !(term->keyboard.flags & MODE_DECBKM)
	 && (keysym == XK_BackSpace)) {
		keysym = XK_Delete;
		strbuf[0] = '\177';
	}

#ifdef XK_KP_Home
	if (keysym >= XK_KP_Home && keysym <= XK_KP_Begin) {
	    keysym += XK_Home - XK_KP_Home;
	}
#endif

#define VT52_KEYPAD \
	if_OPT_VT52_MODE(screen,{ \
		reply.a_type = ESC; \
		reply.a_pintro = '?'; \
		})

#define VT52_CURSOR_KEYS \
	if_OPT_VT52_MODE(screen,{ \
		reply.a_type = ESC; \
		})

#if OPT_SUNPC_KBD
	/* make an DEC editing-keypad from a Sun or PC editing-keypad */
	if (sunKeyboard) {
		switch (keysym) {
		case XK_Delete:
#ifdef DXK_Remove
			keysym = DXK_Remove;
#endif
			break;
		case XK_Home:
			keysym = XK_Find;
			break;
		case XK_End:
			keysym = XK_Select;
			break;
		}
	}
#endif

	if (IsPFKey(keysym)) {
		reply.a_type = SS3;
		reply.a_final = keysym-XK_KP_F1+'P';
		VT52_CURSOR_KEYS
		unparseseq(&reply, pty);
		key = TRUE;
        } else if (IsCursorKey(keysym) &&
        	keysym != XK_Prior && keysym != XK_Next) {
       		if (keyboard->flags & MODE_DECCKM) {
			reply.a_type = SS3;
			reply.a_final = cur[keysym-XK_Home];
			VT52_CURSOR_KEYS
			unparseseq(&reply, pty);
		} else {
			reply.a_type = CSI;
			if_OPT_VT52_MODE(screen,{ reply.a_type = ESC; })
			reply.a_final = cur[keysym-XK_Home];
			unparseseq(&reply, pty);
		}
		key = TRUE;
	 } else if (IsFunctionKey(keysym) || IsMiscFunctionKey(keysym)
	 	|| keysym == XK_Prior
		|| keysym == XK_Next
#ifdef DXK_Remove
		|| keysym == DXK_Remove
#endif
#ifdef XK_KP_Delete
		|| keysym == XK_KP_Delete
		|| keysym == XK_KP_Insert
#endif
		) {
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
		else if (screen->ansi_level <= 1
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
#if OPT_SUNPC_KBD
		/*
		 * DEC keyboards don't have keypad(+), but do have keypad(,)
		 * instead.  Other (Sun, PC) keyboards commonly have keypad(+),
		 * but no keypad(,) - it's a pain for users to work around.
		 */
		if (!sunFunctionKeys
		 && sunKeyboard
		 && keysym == XK_KP_Add)
			keysym = XK_KP_Separator;
#endif
	  	if ((keyboard->flags & MODE_DECKPAM) != 0) {
			reply.a_type  = SS3;
			reply.a_final = kypd_apl[keysym-XK_KP_Space];
			VT52_KEYPAD
			unparseseq(&reply, pty);
		} else
			unparseputc(kypd_num[keysym-XK_KP_Space], pty);
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
StringInput (screen, string, nbytes)
    register TScreen	*screen;
    register char *string;
    size_t nbytes;
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
static int decfuncvalue (keycode)
	KeySym  keycode;
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


static int sunfuncvalue (keycode)
	KeySym  keycode;
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
