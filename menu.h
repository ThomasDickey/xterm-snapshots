/* $XConsortium: menu.h /main/27 1996/12/01 23:47:03 swick $ */
/* $XFree86: xc/programs/xterm/menu.h,v 3.8 1997/12/28 21:28:43 hohndel Exp $ */
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

extern MenuEntry mainMenuEntries[], vtMenuEntries[];
extern MenuEntry fontMenuEntries[];
#if OPT_TEK4014
extern MenuEntry tekMenuEntries[];
#endif

extern Arg menuArgs[];

extern void Handle8BitControl      PROTO_XT_ACTIONS_ARGS;
extern void HandleAllow132         PROTO_XT_ACTIONS_ARGS;
extern void HandleAllowSends       PROTO_XT_ACTIONS_ARGS;
extern void HandleAltScreen        PROTO_XT_ACTIONS_ARGS;
extern void HandleAppCursor        PROTO_XT_ACTIONS_ARGS;
extern void HandleAppKeypad        PROTO_XT_ACTIONS_ARGS;
extern void HandleAutoLineFeed     PROTO_XT_ACTIONS_ARGS;
extern void HandleAutoWrap         PROTO_XT_ACTIONS_ARGS;
extern void HandleBackarrow        PROTO_XT_ACTIONS_ARGS;
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
extern void HandleSunKeyboard      PROTO_XT_ACTIONS_ARGS;
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
typedef enum {
    mainMenu_securekbd,
    mainMenu_allowsends,
#ifdef ALLOWLOGGING
    mainMenu_logging,
#endif
    mainMenu_redraw,
    mainMenu_line1,
    mainMenu_8bit_ctrl,
    mainMenu_backarrow,
    mainMenu_sun_fkeys,
#if OPT_SUNPC_KBD
    mainMenu_sun_kbd,
#endif
    mainMenu_line2,
    mainMenu_suspend,
    mainMenu_continue,
    mainMenu_interrupt,
    mainMenu_hangup,
    mainMenu_terminate,
    mainMenu_kill,
    mainMenu_line3,
    mainMenu_quit,
    mainMenu_LAST
} mainMenuIndices;


/*
 * items in vt100 mode menu
 */
typedef enum {
    vtMenu_scrollbar,
    vtMenu_jumpscroll,
    vtMenu_reversevideo,
    vtMenu_autowrap,
    vtMenu_reversewrap,
    vtMenu_autolinefeed,
    vtMenu_appcursor,
    vtMenu_appkeypad,
    vtMenu_scrollkey,
    vtMenu_scrollttyoutput,
    vtMenu_allow132,
    vtMenu_cursesemul,
    vtMenu_visualbell,
    vtMenu_marginbell,
    vtMenu_altscreen,
#ifndef NO_ACTIVE_ICON
    vtMenu_activeicon,
#endif /* NO_ACTIVE_ICON */
    vtMenu_line1,
    vtMenu_softreset,
    vtMenu_hardreset,
    vtMenu_clearsavedlines,
    vtMenu_line2,
    vtMenu_tekshow,
    vtMenu_tekmode,
    vtMenu_vthide,
    vtMenu_LAST
} vtMenuIndices;

/*
 * items in vt100 font menu
 */
typedef enum {
    fontMenu_fontdefault,
    fontMenu_font1,
    fontMenu_font2,
    fontMenu_font3,
    fontMenu_font4,
    fontMenu_font5,
    fontMenu_font6,
#define fontMenu_lastBuiltin fontMenu_font6
    fontMenu_fontescape,
    fontMenu_fontsel,
    fontMenu_LAST
} fontMenuIndices;
/* number of non-line items should match NMENUFONTS in ptyx.h */


/*
 * items in tek4014 mode menu
 */
typedef enum {
    tekMenu_tektextlarge,
    tekMenu_tektext2,
    tekMenu_tektext3,
    tekMenu_tektextsmall,
    tekMenu_line1,
    tekMenu_tekpage,
    tekMenu_tekreset,
    tekMenu_tekcopy,
    tekMenu_line2,
    tekMenu_vtshow,
    tekMenu_vtmode,
    tekMenu_tekhide,
    tekMenu_LAST
} tekMenuIndices;


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

#define update_decbkm() \
  update_menu_item (term->screen.mainMenu, \
		    mainMenuEntries[mainMenu_backarrow].widget, \
		    term->keyboard.flags & MODE_DECBKM)

#define update_sun_fkeys() \
  update_menu_item (term->screen.mainMenu, \
		    mainMenuEntries[mainMenu_sun_fkeys].widget, \
		    sunFunctionKeys)

#if OPT_SUNPC_KBD
#define update_sun_kbd() \
  update_menu_item (term->screen.mainMenu, \
		    mainMenuEntries[mainMenu_sun_kbd].widget, \
		    sunKeyboard)
#endif

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

#if OPT_TEK4014
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
#else
#define update_tekshow() /*nothing*/
#define update_vttekmode() /*nothing*/
#define update_vtshow() /*nothing*/
#define set_vthide_sensitivity() /*nothing*/
#define set_tekhide_sensitivity() /*nothing*/
#endif


/*
 * macros for mapping font size to tekMenu placement
 */
#define FS2MI(n) (n)			/* font_size_to_menu_item */
#define MI2FS(n) (n)			/* menu_item_to_font_size */

#if OPT_TEK4014
#define set_tekfont_menu_item(n,val) \
  update_menu_item (term->screen.tekMenu, \
		    tekMenuEntries[FS2MI(n)].widget, \
		    (val))
#else
#define set_tekfont_menu_item(n,val) /*nothing*/
#endif

#define set_menu_font(val) \
  update_menu_item (term->screen.fontMenu, \
		    fontMenuEntries[term->screen.menu_font_number].widget, \
		    (val))
#endif/*included_menu_h*/
