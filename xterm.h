/* $XFree86: xc/programs/xterm/xterm.h,v 3.26 1998/04/27 03:15:07 robin Exp $ */
/*
 * Common/useful definitions for XTERM application.
 *
 * This is also where we put the fallback definitions if we do not build using
 * the configure script.
 */
#ifndef	included_xterm_h
#define	included_xterm_h

#ifndef GCC_UNUSED
#define GCC_UNUSED /* nothing */
#endif

#ifndef HAVE_CONFIG_H

#ifndef HAVE_X11_DECKEYSYM_H
#define HAVE_X11_DECKEYSYM_H 1
#endif

#ifndef DFT_TERMTYPE
#define DFT_TERMTYPE "xterm"
#endif

#ifndef X_NOT_POSIX
#define HAVE_WAITPID 1
#define HAVE_SYS_WAIT_H 1
#define HAVE_UNISTD_H 1
#endif

#ifndef X_NOT_STDC_ENV
#define HAVE_STDLIB_H 1
#define DECL_ERRNO 1
#define size_t int
#define time_t long
#endif

#endif /* HAVE_CONFIG_H */

/***====================================================================***/

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#else
extern char *calloc();
extern char *getenv();
extern char *malloc();
extern char *realloc();
extern void exit();
extern void free();
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#include <errno.h>
#ifdef DECL_ERRNO
extern int errno;
#endif

/*
 * FIXME:  Toggling logging from xterm hangs under Linux 2.0.29 with libc5 if
 * we use 'waitpid()', while 'wait()' seems to work properly.
 */
#ifdef linux
#undef HAVE_WAITPID
#endif

/***====================================================================***/

#include "proto.h"

/* Tekproc.c */
extern int TekInit PROTO((void));
extern void ChangeTekColors PROTO((TScreen *screen, ScrnColors *pNew));
extern void TCursorToggle PROTO((int toggle));
extern void TekCopy PROTO((void));
extern void TekEnqMouse PROTO((int c));
extern void TekExpose PROTO((Widget w, XEvent *event, Region region));
extern void TekGINoff PROTO((void));
extern void TekReverseVideo PROTO((TScreen *screen));
extern void TekRun PROTO((void));
extern void TekSetFontSize PROTO((int newitem));
extern void TekSimulatePageButton PROTO((Bool reset));
extern void dorefresh PROTO((void));

/* button.c */
extern Boolean SendMousePosition PROTO((Widget w, XEvent* event));
extern int SetCharacterClassRange PROTO((int low, int high, int value));
extern void DiredButton               PROTO_XT_ACTIONS_ARGS;
extern void DisownSelection PROTO((XtermWidget termw));
extern void HandleGINInput            PROTO_XT_ACTIONS_ARGS;
extern void HandleInsertSelection     PROTO_XT_ACTIONS_ARGS;
extern void HandleKeyboardSelectEnd   PROTO_XT_ACTIONS_ARGS;
extern void HandleKeyboardSelectStart PROTO_XT_ACTIONS_ARGS;
extern void HandleKeyboardStartExtend PROTO_XT_ACTIONS_ARGS;
extern void HandleSecure              PROTO_XT_ACTIONS_ARGS;
extern void HandleSelectEnd           PROTO_XT_ACTIONS_ARGS;
extern void HandleSelectExtend        PROTO_XT_ACTIONS_ARGS;
extern void HandleSelectSet           PROTO_XT_ACTIONS_ARGS;
extern void HandleSelectStart         PROTO_XT_ACTIONS_ARGS;
extern void HandleStartExtend         PROTO_XT_ACTIONS_ARGS;
extern void ResizeSelection PROTO((TScreen *screen, int rows, int cols));
extern void ScrollSelection PROTO((TScreen* screen, int amount));
extern void TrackMouse PROTO((int func, int startrow, int startcol, int firstrow, int lastrow));
extern void TrackText PROTO((int frow, int fcol, int trow, int tcol));
extern void ViButton                  PROTO_XT_ACTIONS_ARGS;

/* charproc.c */
extern int VTInit PROTO((void));
extern int v_write PROTO((int f, char *d, int len));
extern void FindFontSelection PROTO((char *atom_name, Bool justprobe));
extern void HideCursor PROTO((void));
extern void SetVTFont PROTO((int i, Bool doresize, char *name1, char *name2));
extern void ShowCursor PROTO((void));
extern void SwitchBufPtrs PROTO((TScreen *screen));
extern void ToggleAlternate PROTO((TScreen *screen));
extern void VTReset PROTO((int full, int saved));
extern void VTRun PROTO((void));
extern void dotext PROTO((TScreen *screen, int charset, Char *buf, Char *ptr));
extern void resetCharsets PROTO((TScreen *screen));
extern void set_cursor_gcs PROTO((TScreen *screen));
extern void unparseputc PROTO((int c, int fd));
extern void unparseputc1 PROTO((int c, int fd));
extern void unparseseq PROTO((ANSI *ap, int fd));

#if OPT_ISO_COLORS
extern void SGR_Background PROTO((int color));
extern void SGR_Foreground PROTO((int color));
#endif

/* charsets.c */
extern int xtermCharSets (Char *buf, Char *ptr, char charset);

/* cursor.c */
extern void CarriageReturn PROTO((TScreen *screen));
extern void CursorBack PROTO((TScreen *screen, int  n));
extern void CursorDown PROTO((TScreen *screen, int  n));
extern void CursorForward PROTO((TScreen *screen, int  n));
extern void CursorNextLine PROTO((TScreen *screen, int count));
extern void CursorPrevLine PROTO((TScreen *screen, int count));
extern void CursorRestore PROTO((XtermWidget tw, SavedCursor *sc));
extern void CursorSave PROTO((XtermWidget tw, SavedCursor *sc));
extern void CursorSet PROTO((TScreen *screen, int row, int col, unsigned flags));
extern void CursorUp PROTO((TScreen *screen, int  n));
extern void Index PROTO((TScreen *screen, int amount));
extern void RevIndex PROTO((TScreen *screen, int amount));

/* doublechr.c */
extern void xterm_DECDHL PROTO((Bool top));
extern void xterm_DECSWL PROTO((void));
extern void xterm_DECDWL PROTO((void));

/* input.c */
extern void Input PROTO((TKeyboard *keyboard, TScreen *screen, XKeyEvent *event, Bool eightbit));
extern void StringInput PROTO((TScreen *screen, char *string, size_t nbytes));

/* main.c */
extern int main PROTO((int argc, char **argv));

extern int GetBytesAvailable PROTO((int fd));
extern int kill_process_group PROTO((int pid, int sig));
extern int nonblocking_wait PROTO((void));
extern void first_map_occurred PROTO((void));

#ifdef SIGNAL_T
extern SIGNAL_T Exit PROTO((int n));
#endif

/* menu.c */
extern void do_hangup          PROTO_XT_CALLBACK_ARGS;
extern void show_8bit_control  PROTO((Bool value));

/* misc.c */
extern Cursor make_colored_cursor PROTO((unsigned cursorindex, unsigned long fg, unsigned long bg));
extern char *SysErrorMsg PROTO((int n));
extern char *strindex PROTO((char *s1, char *s2));
extern char *udk_lookup PROTO((int keycode, int *len));
extern int XStrCmp PROTO((char *s1, char *s2));
extern int xerror PROTO((Display *d, XErrorEvent *ev));
extern int xioerror PROTO((Display *dpy));
extern void Bell PROTO((int which, int percent));
extern void Changename PROTO((char *name));
extern void Changetitle PROTO((char *name));
extern void Cleanup PROTO((int code));
extern void Error PROTO((int i));
extern void HandleBellPropertyChange PROTO_XT_EV_HANDLER_ARGS;
extern void HandleEightBitKeyPressed PROTO_XT_ACTIONS_ARGS;
extern void HandleEnterWindow PROTO_XT_EV_HANDLER_ARGS;
extern void HandleFocusChange PROTO_XT_EV_HANDLER_ARGS;
extern void HandleKeyPressed PROTO_XT_ACTIONS_ARGS;
extern void HandleLeaveWindow PROTO_XT_EV_HANDLER_ARGS;
extern void HandleStringEvent PROTO_XT_ACTIONS_ARGS;
extern void Panic PROTO((char *s, int a));
extern void Redraw PROTO((void));
extern void ReverseOldColors PROTO((void));
extern void Setenv PROTO((char *var, char *value));
extern void SysError PROTO((int i));
extern void VisualBell PROTO((void));
extern void creat_as PROTO((int uid, int gid, char *pathname, int mode));
extern void do_dcs PROTO((Char *buf, size_t len));
extern void do_osc PROTO((Char *buf, int len));
extern void do_xevents PROTO((void));
extern void end_tek_mode PROTO((void));
extern void end_vt_mode PROTO((void));
extern void hide_tek_window PROTO((void));
extern void hide_vt_window PROTO((void));
extern void reset_decudk PROTO((void));
extern void set_tek_visibility PROTO((int on));
extern void set_vt_visibility PROTO((int on));
extern void switch_modes PROTO((Bool tovt));
extern void xevents PROTO((void));
extern void xt_error PROTO((String message));

#ifdef ALLOWLOGGING
extern void StartLog PROTO((TScreen *screen));
extern void CloseLog PROTO((TScreen *screen));
extern void FlushLog PROTO((TScreen *screen));
#endif

/* print.c */
extern int xtermPrinterControl PROTO((int chr));
extern void xtermAutoPrint PROTO((int chr));
extern void xtermMediaControl PROTO((int param, int private));
extern void xtermPrintScreen PROTO((void));

/* screen.c */
extern Bool non_blank_line PROTO((ScrnBuf sb, int row, int col, int len));
extern ScrnBuf Allocate PROTO((int nrow, int ncol, Char **addr));
extern int ScreenResize PROTO((TScreen *screen, int width, int height, unsigned *flags));
extern int ScrnTstWrapped PROTO((TScreen *screen, int row));
extern size_t ScrnPointers PROTO((TScreen *screen, size_t len));
extern void ClearBufRows PROTO((TScreen *screen, int first, int last));
extern void ScreenWrite PROTO((TScreen *screen, Char *str, unsigned flags, unsigned cur_fg_bg, int length));
extern void ScrnClrWrapped PROTO((TScreen *screen, int row));
extern void ScrnDeleteChar PROTO((TScreen *screen, int n, int size));
extern void ScrnDeleteLine PROTO((TScreen *screen, ScrnBuf sb, int n, int last, int size, int where));
extern void ScrnInsertChar PROTO((TScreen *screen, int n, int size));
extern void ScrnInsertLine PROTO((TScreen *screen, ScrnBuf sb, int last, int where, int n, int size));
extern void ScrnRefresh PROTO((TScreen *screen, int toprow, int leftcol, int nrows, int ncols, int force));
extern void ScrnSetWrapped PROTO((TScreen *screen, int row));

/* scrollbar.c */
extern void DoResizeScreen PROTO((XtermWidget xw));
extern void HandleScrollBack PROTO_XT_ACTIONS_ARGS;
extern void HandleScrollForward PROTO_XT_ACTIONS_ARGS;
extern void ResizeScrollBar PROTO((TScreen *screen));
extern void ScrollBarDrawThumb PROTO((Widget scrollWidget));
extern void ScrollBarOff PROTO((TScreen *screen));
extern void ScrollBarOn PROTO((XtermWidget xw, int init, int doalloc));
extern void ScrollBarReverseVideo PROTO((Widget scrollWidget));
extern void WindowScroll PROTO((TScreen *screen, int top));

/* tabs.c */
extern Boolean TabToNextStop PROTO((void));
extern Boolean TabToPrevStop PROTO((void));
extern int TabNext PROTO((Tabs tabs, int col));
extern int TabPrev PROTO((Tabs tabs, int col));
extern void TabClear PROTO((Tabs tabs, int col));
extern void TabReset PROTO((Tabs tabs));
extern void TabSet PROTO((Tabs tabs, int col));
extern void TabZonk PROTO((Tabs	tabs));

/* util.c */
extern GC updatedXtermGC PROTO((TScreen *screen, int flags, int fg_bg, Bool hilite));
extern int AddToRefresh PROTO((TScreen *screen));
extern int HandleExposure PROTO((TScreen *screen, XEvent *event));
extern int drawXtermText PROTO((TScreen *screen, unsigned flags, GC gc, int x, int y, int chrset, Char *text, int len));
extern void ChangeColors PROTO((XtermWidget tw, ScrnColors *pNew));
extern void ClearRight PROTO((TScreen *screen, int n));
extern void ClearScreen PROTO((TScreen *screen));
extern void DeleteChar PROTO((TScreen *screen, int n));
extern void DeleteLine PROTO((TScreen *screen, int n));
extern void FlushScroll PROTO((TScreen *screen));
extern void GetColors PROTO((XtermWidget tw, ScrnColors *pColors));
extern void InsertChar PROTO((TScreen *screen, int n));
extern void InsertLine PROTO((TScreen *screen, int n));
extern void RevScroll PROTO((TScreen *screen, int amount));
extern void ReverseVideo PROTO((XtermWidget termw));
extern void Scroll PROTO((TScreen *screen, int amount));
extern void do_erase_display PROTO((TScreen *screen, int param, int mode));
extern void do_erase_line PROTO((TScreen *screen, int param, int mode));
extern void recolor_cursor PROTO((Cursor cursor, unsigned long fg, unsigned long bg));
extern void resetXtermGC PROTO((TScreen *screen, int flags, Bool hilite));
extern void scrolling_copy_area PROTO((TScreen *screen, int firstline, int nlines, int amount));

#if OPT_ISO_COLORS

extern Pixel getXtermBackground PROTO((int flags, int color));
extern Pixel getXtermForeground PROTO((int flags, int color));
extern int extract_bg PROTO((unsigned color));
extern int extract_fg PROTO((unsigned color, unsigned flags));
extern unsigned makeColorPair PROTO((int fg, int bg));
extern unsigned xtermColorPair PROTO((void));
extern void ClearCurBackground PROTO((TScreen *screen, int top, int left, unsigned height, unsigned width));
extern void useCurBackground PROTO((Bool flag));

#else /* !OPT_ISO_COLORS */

#define ClearCurBackground(screen, top, left, height, width) \
	XClearArea (screen->display, TextWindow(screen), \
		left, top, width, height, FALSE)

#define extract_fg(color, flags) term->cur_foreground
#define extract_bg(color) term->cur_background

		/* FIXME: Reverse-Video? */
#define getXtermBackground(flags, color) term->core.background_pixel
#define getXtermForeground(flags, color) term->screen.foreground
#define xtermColorPair() 0

#define useCurBackground(flag) /*nothing*/

#endif	/* OPT_ISO_COLORS */

#define FillCurBackground(screen, left, top, width, height) \
	useCurBackground(TRUE); \
	XFillRectangle (screen->display, TextWindow(screen), \
		ReverseGC(screen), left, top, width, height); \
	useCurBackground(FALSE)

#if OPT_DEC_CHRSET
extern int getXtermChrSet PROTO((int row, int col));
extern int curXtermChrSet PROTO((int row));
#else
#define getXtermChrSet(row, col) 0
#define curXtermChrSet(row) 0
#endif

#if OPT_XMC_GLITCH
extern void Mark_XMC PROTO((TScreen *screen, int param));
extern void Jump_XMC PROTO((TScreen *screen));
extern void Resolve_XMC PROTO((TScreen *screen));
#endif

#endif	/* included_xterm_h */
