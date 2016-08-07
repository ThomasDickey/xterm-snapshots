/* $Xorg: menu.h,v 1.4 2001/02/09 02:06:03 xorgcvs Exp $ */
/*

Copyright 1999-2001, 2002 by Thomas E. Dickey

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of the above listed
copyright holder(s) not be used in advertising or publicity pertaining
to distribution of the software without specific, written prior
permission.

THE ABOVE LISTED COPYRIGHT HOLDER(S) DISCLAIM ALL WARRANTIES WITH REGARD
TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS, IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT HOLDER(S) BE
LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


Copyright 1989  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/
/* $XFree86: xc/programs/xterm/menu.h,v 3.29 2002/08/12 00:36:33 dickey Exp $ */

#ifndef included_menu_h
#define included_menu_h

#ifdef HAVE_CONFIG_H
#include <xtermcfg.h>
#endif

#include <X11/Intrinsic.h>
#include <proto.h>

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
extern void HandleCursorBlink      PROTO_XT_ACTIONS_ARGS;
extern void HandleDeleteIsDEL      PROTO_XT_ACTIONS_ARGS;
extern void HandleFontBoxChars     PROTO_XT_ACTIONS_ARGS;
extern void HandleFontDoublesize   PROTO_XT_ACTIONS_ARGS;
extern void HandleFontLoading      PROTO_XT_ACTIONS_ARGS;
extern void HandleHardReset        PROTO_XT_ACTIONS_ARGS;
extern void HandleHpFunctionKeys   PROTO_XT_ACTIONS_ARGS;
extern void HandleJumpscroll       PROTO_XT_ACTIONS_ARGS;
extern void HandleLogging          PROTO_XT_ACTIONS_ARGS;
extern void HandleMarginBell       PROTO_XT_ACTIONS_ARGS;
extern void HandleMetaEsc          PROTO_XT_ACTIONS_ARGS;
extern void HandleNumLock          PROTO_XT_ACTIONS_ARGS;
extern void HandleOldFunctionKeys  PROTO_XT_ACTIONS_ARGS;
extern void HandlePopupMenu        PROTO_XT_ACTIONS_ARGS;
extern void HandlePrintControlMode PROTO_XT_ACTIONS_ARGS;
extern void HandlePrintScreen      PROTO_XT_ACTIONS_ARGS;
extern void HandleQuit             PROTO_XT_ACTIONS_ARGS;
extern void HandleRedraw           PROTO_XT_ACTIONS_ARGS;
extern void HandleReverseVideo     PROTO_XT_ACTIONS_ARGS;
extern void HandleReverseWrap      PROTO_XT_ACTIONS_ARGS;
extern void HandleScoFunctionKeys  PROTO_XT_ACTIONS_ARGS;
extern void HandleScrollKey        PROTO_XT_ACTIONS_ARGS;
extern void HandleScrollTtyOutput  PROTO_XT_ACTIONS_ARGS;
extern void HandleScrollbar        PROTO_XT_ACTIONS_ARGS;
extern void HandleSendSignal       PROTO_XT_ACTIONS_ARGS;
extern void HandleSetTekText       PROTO_XT_ACTIONS_ARGS;
extern void HandleSetTerminalType  PROTO_XT_ACTIONS_ARGS;
extern void HandleSetVisualBell    PROTO_XT_ACTIONS_ARGS;
extern void HandleSetPopOnBell     PROTO_XT_ACTIONS_ARGS;
extern void HandleSoftReset        PROTO_XT_ACTIONS_ARGS;
extern void HandleSunFunctionKeys  PROTO_XT_ACTIONS_ARGS;
extern void HandleSunKeyboard      PROTO_XT_ACTIONS_ARGS;
extern void HandleTekCopy          PROTO_XT_ACTIONS_ARGS;
extern void HandleTekPage          PROTO_XT_ACTIONS_ARGS;
extern void HandleTekReset         PROTO_XT_ACTIONS_ARGS;
extern void HandleTiteInhibit      PROTO_XT_ACTIONS_ARGS;
extern void HandleVisibility       PROTO_XT_ACTIONS_ARGS;

extern void DoSecureKeyboard (Time tp);
extern void SetupMenus (Widget shell, Widget *forms, Widget *menus);

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
    mainMenu_redraw,
    mainMenu_line1,
#ifdef ALLOWLOGGING
    mainMenu_logging,
#endif
    mainMenu_print,
    mainMenu_print_redir,
    mainMenu_line2,
    mainMenu_8bit_ctrl,
    mainMenu_backarrow,
#if OPT_NUM_LOCK
    mainMenu_num_lock,
    mainMenu_meta_esc,
#endif
    mainMenu_delete_del,
    mainMenu_old_fkeys,
#if OPT_HP_FUNC_KEYS
    mainMenu_hp_fkeys,
#endif
#if OPT_SCO_FUNC_KEYS
    mainMenu_sco_fkeys,
#endif
    mainMenu_sun_fkeys,
#if OPT_SUNPC_KBD
    mainMenu_sun_kbd,
#endif
    mainMenu_line3,
    mainMenu_suspend,
    mainMenu_continue,
    mainMenu_interrupt,
    mainMenu_hangup,
    mainMenu_terminate,
    mainMenu_kill,
    mainMenu_line4,
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
    vtMenu_poponbell,
    vtMenu_marginbell,
#if OPT_BLINK_CURS
    vtMenu_cursorblink,
#endif
    vtMenu_titeInhibit,
#ifndef NO_ACTIVE_ICON
    vtMenu_activeicon,
#endif /* NO_ACTIVE_ICON */
    vtMenu_line1,
    vtMenu_softreset,
    vtMenu_hardreset,
    vtMenu_clearsavedlines,
    vtMenu_line2,
#if OPT_TEK4014
    vtMenu_tekshow,
    vtMenu_tekmode,
    vtMenu_vthide,
#endif
    vtMenu_altscreen,
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
/* number of non-line items down to here should match NMENUFONTS in ptyx.h */
#if OPT_DEC_CHRSET || OPT_BOX_CHARS || OPT_DEC_SOFTFONT
    fontMenu_line1,
#if OPT_BOX_CHARS
    fontMenu_font_boxchars,
#endif
#if OPT_DEC_CHRSET
    fontMenu_font_doublesize,
#endif
#if OPT_DEC_SOFTFONT
    fontMenu_font_loadable,
#endif
#endif
    fontMenu_LAST
} fontMenuIndices;


/*
 * items in tek4014 mode menu
 */
#if OPT_TEK4014
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
#endif


/*
 * macros for updating menus
 */

#define update_menu_item(w,mi,val) UpdateMenuItem(mi,val)
extern void UpdateMenuItem(Widget mi, XtArgVal val);

#define set_sensitivity(w,mi,val) SetItemSensitivity(mi,val)
extern void SetItemSensitivity(Widget mi, XtArgVal val);


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
#else
#define update_logging() /*nothing*/
#endif

#define update_print_redir() \
  update_menu_item (term->screen.mainMenu, \
		    mainMenuEntries[mainMenu_print_redir].widget, \
		    term->screen.printer_controlmode)

#define update_8bit_control() \
  update_menu_item (term->screen.mainMenu, \
		    mainMenuEntries[mainMenu_8bit_ctrl].widget, \
		    term->screen.control_eight_bits)

#define update_decbkm() \
  update_menu_item (term->screen.mainMenu, \
		    mainMenuEntries[mainMenu_backarrow].widget, \
		    term->keyboard.flags & MODE_DECBKM)

#if OPT_NUM_LOCK
#define update_num_lock() \
  update_menu_item (term->screen.mainMenu, \
		    mainMenuEntries[mainMenu_num_lock].widget, \
		    term->misc.real_NumLock)
#define update_meta_esc() \
  update_menu_item (term->screen.mainMenu, \
		    mainMenuEntries[mainMenu_meta_esc].widget, \
		    term->screen.meta_sends_esc)
#else
#define update_num_lock() /*nothing*/
#define update_meta_esc() /*nothing*/
#endif

#define update_sun_fkeys() \
  update_menu_item (term->screen.mainMenu, \
		    mainMenuEntries[mainMenu_sun_fkeys].widget, \
		    term->keyboard.type == keyboardIsSun)

#define update_old_fkeys() \
  update_menu_item (term->screen.mainMenu, \
		    mainMenuEntries[mainMenu_old_fkeys].widget, \
		    term->keyboard.type == keyboardIsLegacy)

#define update_delete_del() \
  update_menu_item (term->screen.mainMenu, \
		    mainMenuEntries[mainMenu_delete_del].widget, \
		    xtermDeleteIsDEL())

#if OPT_SUNPC_KBD
#define update_sun_kbd() \
  update_menu_item (term->screen.mainMenu, \
		    mainMenuEntries[mainMenu_sun_kbd].widget, \
		    term->keyboard.type == keyboardIsVT220)
#endif

#if OPT_HP_FUNC_KEYS
#define update_hp_fkeys() \
  update_menu_item (term->screen.mainMenu, \
		    mainMenuEntries[mainMenu_hp_fkeys].widget, \
		    term->keyboard.type == keyboardIsHP)
#else
#define update_hp_fkeys() /*nothing*/
#endif

#if OPT_SCO_FUNC_KEYS
#define update_sco_fkeys() \
  update_menu_item (term->screen.mainMenu, \
		    mainMenuEntries[mainMenu_sco_fkeys].widget, \
		    term->keyboard.type == keyboardIsSCO)
#else
#define update_sco_fkeys() /*nothing*/
#endif

#define update_scrollbar() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_scrollbar].widget, \
		    ScrollbarWidth(&term->screen))

#define update_jumpscroll() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_jumpscroll].widget, \
		    term->screen.jumpscroll)

#define update_reversevideo() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_reversevideo].widget, \
		    (term->misc.re_verse))

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

#define update_poponbell() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_poponbell].widget, \
		    term->screen.poponbell)

#define update_marginbell() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_marginbell].widget, \
		    term->screen.marginbell)

#if OPT_BLINK_CURS
#define update_cursorblink() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_cursorblink].widget, \
		    term->screen.cursor_blink)
#else
#define update_cursorblink() /* nothing */
#endif

#define update_altscreen() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_altscreen].widget, \
		    term->screen.alternate)

#define update_titeInhibit() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_titeInhibit].widget, \
		    !(term->misc.titeInhibit))

#ifndef NO_ACTIVE_ICON
#define update_activeicon() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_activeicon].widget, \
		    term->misc.active_icon)
#endif /* NO_ACTIVE_ICON */

#if OPT_DEC_CHRSET
#define update_font_doublesize() \
  update_menu_item (term->screen.fontMenu, \
		    fontMenuEntries[fontMenu_font_doublesize].widget, \
		    term->screen.font_doublesize)
#else
#define update_font_doublesize() /* nothing */
#endif

#if OPT_BOX_CHARS
#define update_font_boxchars() \
  update_menu_item (term->screen.fontMenu, \
		    fontMenuEntries[fontMenu_font_boxchars].widget, \
		    term->screen.force_box_chars)
#else
#define update_font_boxchars() /* nothing */
#endif

#if OPT_DEC_SOFTFONT
#define update_font_loadable() \
  update_menu_item (term->screen.fontMenu, \
		    fontMenuEntries[fontMenu_font_loadable].widget, \
		    term->misc.font_loadable)
#else
#define update_font_loadable() /* nothing */
#endif

#if OPT_TEK4014
#define update_tekshow() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_tekshow].widget, \
		    term->screen.Tshow)

#define update_vttekmode() { \
    update_menu_item (term->screen.vtMenu, \
		      vtMenuEntries[vtMenu_tekmode].widget, \
		      term->screen.TekEmu); \
    update_menu_item (term->screen.tekMenu, \
		      tekMenuEntries[tekMenu_vtmode].widget, \
		      !term->screen.TekEmu); }

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
