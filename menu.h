/* $XConsortium: menu.h /main/27 1996/12/01 23:47:03 swick $ */
/* $XFree86: xc/programs/xterm/menu.h,v 3.4 1997/01/08 20:52:30 dawes Exp $ */
/*

Copyright (c) 1989  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
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
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.

*/

#ifndef included_menu_h
#define included_menu_h

#include "proto.h"

typedef struct _MenuEntry {
    char *name;
    void (*function) PROTO_XT_CALLBACK_ARGS;
    Widget widget;
} MenuEntry;

extern MenuEntry mainMenuEntries[], vtMenuEntries[], tekMenuEntries[];
extern MenuEntry fontMenuEntries[];
extern Arg menuArgs[];

extern void Handle8BitControl      PROTO_XT_ACTIONS_ARGS;
extern void HandleAllow132         PROTO_XT_ACTIONS_ARGS;
extern void HandleAllowSends       PROTO_XT_ACTIONS_ARGS;
extern void HandleAltScreen        PROTO_XT_ACTIONS_ARGS;
extern void HandleAppCursor        PROTO_XT_ACTIONS_ARGS;
extern void HandleAppKeypad        PROTO_XT_ACTIONS_ARGS;
extern void HandleAutoLineFeed     PROTO_XT_ACTIONS_ARGS;
extern void HandleAutoWrap         PROTO_XT_ACTIONS_ARGS;
extern void HandleClearSavedLines  PROTO_XT_ACTIONS_ARGS;
extern void HandleCreateMenu       PROTO_XT_ACTIONS_ARGS;
extern void HandleCursesEmul       PROTO_XT_ACTIONS_ARGS;
extern void HandleHardReset        PROTO_XT_ACTIONS_ARGS;
extern void HandleJumpscroll       PROTO_XT_ACTIONS_ARGS;
extern void HandleMarginBell       PROTO_XT_ACTIONS_ARGS;
extern void HandlePopupMenu        PROTO_XT_ACTIONS_ARGS;
extern void HandleQuit             PROTO_XT_ACTIONS_ARGS;
extern void HandleRedraw           PROTO_XT_ACTIONS_ARGS;
extern void HandleReverseVideo     PROTO_XT_ACTIONS_ARGS;
extern void HandleReverseWrap      PROTO_XT_ACTIONS_ARGS;
extern void HandleScrollKey        PROTO_XT_ACTIONS_ARGS;
extern void HandleScrollTtyOutput  PROTO_XT_ACTIONS_ARGS;
extern void HandleScrollbar        PROTO_XT_ACTIONS_ARGS;
extern void HandleSendSignal       PROTO_XT_ACTIONS_ARGS;
extern void HandleSetTekText       PROTO_XT_ACTIONS_ARGS;
extern void HandleSetTerminalType  PROTO_XT_ACTIONS_ARGS;
extern void HandleSetVisualBell    PROTO_XT_ACTIONS_ARGS;
extern void HandleSoftReset        PROTO_XT_ACTIONS_ARGS;
extern void HandleSunFunctionKeys  PROTO_XT_ACTIONS_ARGS;
extern void HandleTekCopy          PROTO_XT_ACTIONS_ARGS;
extern void HandleTekPage          PROTO_XT_ACTIONS_ARGS;
extern void HandleTekReset         PROTO_XT_ACTIONS_ARGS;
extern void HandleVisibility       PROTO_XT_ACTIONS_ARGS;

#ifdef ALLOWLOGGING
extern void HandleLogging          PROTO_XT_ACTIONS_ARGS;
#endif

extern void DoSecureKeyboard PROTO((Time tp));

/*
 * The following definitions MUST match the order of entries given in 
 * the mainMenuEntries, vtMenuEntries, and tekMenuEntries arrays in menu.c.
 */

/*
 * items in primary menu
 */
#define mainMenu_securekbd  ( 0)
#define mainMenu_allowsends ( 1)
#ifdef ALLOWLOGGING
#define mainMenu_logging    ( 2)
#define mainMenu_fix1       ( 0)
#else
#define mainMenu_fix1       (-1)
#endif
#define mainMenu_redraw     ( 3 + mainMenu_fix1)
#define mainMenu_line1      ( 4 + mainMenu_fix1)
#define mainMenu_8bit_ctrl  ( 5 + mainMenu_fix1)
#define mainMenu_sun_fkeys  ( 6 + mainMenu_fix1)
#define mainMenu_line2      ( 7 + mainMenu_fix1)
#define mainMenu_suspend    ( 8 + mainMenu_fix1)
#define mainMenu_continue   ( 9 + mainMenu_fix1)
#define mainMenu_interrupt  (10 + mainMenu_fix1)
#define mainMenu_hangup     (11 + mainMenu_fix1)
#define mainMenu_terminate  (12 + mainMenu_fix1)
#define mainMenu_kill       (13 + mainMenu_fix1)
#define mainMenu_line3      (14 + mainMenu_fix1)
#define mainMenu_quit       (15 + mainMenu_fix1)


/*
 * items in vt100 mode menu
 */
#define vtMenu_scrollbar 0
#define vtMenu_jumpscroll 1
#define vtMenu_reversevideo 2
#define vtMenu_autowrap 3
#define vtMenu_reversewrap 4
#define vtMenu_autolinefeed 5
#define vtMenu_appcursor 6
#define vtMenu_appkeypad 7
#define vtMenu_scrollkey 8
#define vtMenu_scrollttyoutput 9
#define vtMenu_allow132 10
#define vtMenu_cursesemul 11
#define vtMenu_visualbell 12
#define vtMenu_marginbell 13
#define vtMenu_altscreen 14
#ifndef NO_ACTIVE_ICON
#define vtMenu_activeicon 15
#endif /* NO_ACTIVE_ICON */
#define vtMenu_line1 16
#define vtMenu_softreset 17
#define vtMenu_hardreset 18
#define vtMenu_clearsavedlines 19
#define vtMenu_line2 20
#define vtMenu_tekshow 21
#define vtMenu_tekmode 22
#define vtMenu_vthide 23

/*
 * items in vt100 font menu
 */
#define fontMenu_fontdefault 0
#define fontMenu_font1 1
#define fontMenu_font2 2
#define fontMenu_font3 3
#define fontMenu_font4 4
#define fontMenu_font5 5
#define fontMenu_font6 6
#define fontMenu_lastBuiltin fontMenu_font6
#define fontMenu_fontescape 7
#define fontMenu_fontsel 8
/* number of non-line items should match NMENUFONTS in ptyx.h */


/*
 * items in tek4014 mode menu
 */
#define tekMenu_tektextlarge 0
#define tekMenu_tektext2 1
#define tekMenu_tektext3 2
#define tekMenu_tektextsmall 3
#define tekMenu_line1 4
#define tekMenu_tekpage 5
#define tekMenu_tekreset 6
#define tekMenu_tekcopy 7
#define tekMenu_line2 8
#define tekMenu_vtshow 9
#define tekMenu_vtmode 10
#define tekMenu_tekhide 11


/*
 * macros for updating menus
 */

#define update_menu_item(w,mi,val) { if (mi) { \
    menuArgs[0].value = (XtArgVal) ((val) ? term->screen.menu_item_bitmap \
				          : None); \
    XtSetValues (mi, menuArgs, (Cardinal) 1); }}


#define set_sensitivity(w,mi,val) { if (mi) { \
    menuArgs[1].value = (XtArgVal) (val); \
    XtSetValues (mi, menuArgs+1, (Cardinal) 1);  }}



/*
 * there should be one of each of the following for each checkable item
 */


#define update_securekbd() \
  update_menu_item (term->screen.mainMenu, \
		    mainMenuEntries[mainMenu_securekbd].widget, \
		    term->screen.grabbedKbd)

#define update_allowsends() \
  update_menu_item (term->screen.mainMenu, \
		    mainMenuEntries[mainMenu_allowsends].widget, \
		    term->screen.allowSendEvents)

#ifdef ALLOWLOGGING
#define update_logging() \
  update_menu_item (term->screen.mainMenu, \
		    mainMenuEntries[mainMenu_logging].widget, \
		    term->screen.logging)
#endif

#define update_8bit_control() \
  update_menu_item (term->screen.mainMenu, \
		    mainMenuEntries[mainMenu_8bit_ctrl].widget, \
		    term->screen.control_eight_bits)

#define update_sun_fkeys() \
  update_menu_item (term->screen.mainMenu, \
		    mainMenuEntries[mainMenu_sun_fkeys].widget, \
		    sunFunctionKeys)

#define update_scrollbar() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_scrollbar].widget, \
		    Scrollbar(&term->screen))

#define update_jumpscroll() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_jumpscroll].widget, \
		    term->screen.jumpscroll)

#define update_reversevideo() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_reversevideo].widget, \
		    (term->flags & REVERSE_VIDEO))

#define update_autowrap() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_autowrap].widget, \
		    (term->flags & WRAPAROUND))

#define update_reversewrap() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_reversewrap].widget, \
		    (term->flags & REVERSEWRAP))

#define update_autolinefeed() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_autolinefeed].widget, \
		    (term->flags & LINEFEED))

#define update_appcursor() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_appcursor].widget, \
		    (term->keyboard.flags & MODE_DECCKM))

#define update_appkeypad() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_appkeypad].widget, \
		    (term->keyboard.flags & MODE_DECKPAM))

#define update_scrollkey() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_scrollkey].widget,  \
		    term->screen.scrollkey)

#define update_scrollttyoutput() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_scrollttyoutput].widget, \
		    term->screen.scrollttyoutput)

#define update_allow132() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_allow132].widget, \
		    term->screen.c132)
  
#define update_cursesemul() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_cursesemul].widget, \
		    term->screen.curses)

#define update_visualbell() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_visualbell].widget, \
		    term->screen.visualbell)

#define update_marginbell() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_marginbell].widget, \
		    term->screen.marginbell)

#define update_altscreen() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_altscreen].widget, \
		    term->screen.alternate)

#ifndef NO_ACTIVE_ICON
#define update_activeicon() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_activeicon].widget, \
		    term->misc.active_icon)
#endif /* NO_ACTIVE_ICON */

#define update_tekshow() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_tekshow].widget, \
		    term->screen.Tshow)

#define update_vttekmode() { \
    update_menu_item (term->screen.vtMenu, \
		      vtMenuEntries[vtMenu_tekmode].widget, \
		      term->screen.TekEmu) \
    update_menu_item (term->screen.tekMenu, \
		      tekMenuEntries[tekMenu_vtmode].widget, \
		      !term->screen.TekEmu) }

#define update_vtshow() \
  update_menu_item (term->screen.tekMenu, \
		    tekMenuEntries[tekMenu_vtshow].widget, \
		    term->screen.Vshow)


#define set_vthide_sensitivity() \
  set_sensitivity (term->screen.vtMenu, \
		   vtMenuEntries[vtMenu_vthide].widget, \
		   term->screen.Tshow)

#define set_tekhide_sensitivity() \
  set_sensitivity (term->screen.tekMenu, \
		   tekMenuEntries[tekMenu_tekhide].widget, \
		   term->screen.Vshow)

#define set_altscreen_sensitivity(val) \
  set_sensitivity (term->screen.vtMenu,\
		   vtMenuEntries[vtMenu_altscreen].widget, (val))


/*
 * macros for mapping font size to tekMenu placement
 */
#define FS2MI(n) (n)			/* font_size_to_menu_item */
#define MI2FS(n) (n)			/* menu_item_to_font_size */

#define set_tekfont_menu_item(n,val) \
  update_menu_item (term->screen.tekMenu, \
		    tekMenuEntries[FS2MI(n)].widget, \
		    (val))

#define set_menu_font(val) \
  update_menu_item (term->screen.fontMenu, \
		    fontMenuEntries[term->screen.menu_font_number].widget, \
		    (val))
#endif/*included_menu_h*/
