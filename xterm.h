/* $XFree86: xc/programs/xterm/xterm.h,v 3.29 1998/07/04 14:48:30 robin Exp $ */
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

#include <X11/Xos.h>

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
#else
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
extern int TekInit (void);
extern void ChangeTekColors (TScreen *screen, ScrnColors *pNew);
extern void TCursorToggle (int toggle);
extern void TekCopy (void);
extern void TekEnqMouse (int c);
extern void TekExpose (Widget w, XEvent *event, Region region);
extern void TekGINoff (void);
extern void TekReverseVideo (TScreen *screen);
extern void TekRun (void);
extern void TekSetFontSize (int newitem);
extern void TekSimulatePageButton (Bool reset);
extern void dorefresh (void);

/* button.c */
extern Boolean SendMousePosition (Widget w, XEvent* event);
extern int SetCharacterClassRange (int low, int high, int value);
extern void DiredButton               PROTO_XT_ACTIONS_ARGS;
extern void DisownSelection (XtermWidget termw);
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
extern void ResizeSelection (TScreen *screen, int rows, int cols);
extern void ScrollSelection (TScreen* screen, int amount);
extern void TrackMouse (int func, int startrow, int startcol, int firstrow, int lastrow);
extern void TrackText (int frow, int fcol, int trow, int tcol);
extern void ViButton                  PROTO_XT_ACTIONS_ARGS;

/* charproc.c */
extern int VTInit (void);
extern int v_write (int f, char *d, int len);
extern void FindFontSelection (char *atom_name, Bool justprobe);
extern void HideCursor (void);
extern void SetVTFont (int i, Bool doresize, char *name1, char *name2);
extern void ShowCursor (void);
extern void SwitchBufPtrs (TScreen *screen);
extern void ToggleAlternate (TScreen *screen);
extern void VTReset (int full, int saved);
extern void VTRun (void);
extern void dotext (TScreen *screen, int charset, Char *buf, Char *ptr);
extern void resetCharsets (TScreen *screen);
extern void set_cursor_gcs (TScreen *screen);
extern void unparseputc (int c, int fd);
extern void unparseputc1 (int c, int fd);
extern void unparseseq (ANSI *ap, int fd);

#if OPT_ISO_COLORS
extern void SGR_Background (int color);
extern void SGR_Foreground (int color);
#endif

/* charsets.c */
extern int xtermCharSets (Char *buf, Char *ptr, char charset);

/* cursor.c */
extern void CarriageReturn (TScreen *screen);
extern void CursorBack (TScreen *screen, int  n);
extern void CursorDown (TScreen *screen, int  n);
extern void CursorForward (TScreen *screen, int  n);
extern void CursorNextLine (TScreen *screen, int count);
extern void CursorPrevLine (TScreen *screen, int count);
extern void CursorRestore (XtermWidget tw, SavedCursor *sc);
extern void CursorSave (XtermWidget tw, SavedCursor *sc);
extern void CursorSet (TScreen *screen, int row, int col, unsigned flags);
extern void CursorUp (TScreen *screen, int  n);
extern void Index (TScreen *screen, int amount);
extern void RevIndex (TScreen *screen, int amount);

/* doublechr.c */
extern void xterm_DECDHL (Bool top);
extern void xterm_DECSWL (void);
extern void xterm_DECDWL (void);

/* input.c */
extern void Input (TKeyboard *keyboard, TScreen *screen, XKeyEvent *event, Bool eightbit);
extern void StringInput (TScreen *screen, char *string, size_t nbytes);

/* main.c */
#ifndef __EMX__
extern int main (int argc, char **argv);
#else
extern int main (int argc, char **argv,char **envp);
#endif

extern int GetBytesAvailable (int fd);
extern int kill_process_group (int pid, int sig);
extern int nonblocking_wait (void);
extern void first_map_occurred (void);

#ifdef SIGNAL_T
extern SIGNAL_T Exit (int n);
#endif

/* menu.c */
extern void do_hangup          PROTO_XT_CALLBACK_ARGS;
extern void show_8bit_control  (Bool value);

/* misc.c */
extern Cursor make_colored_cursor (unsigned cursorindex, unsigned long fg, unsigned long bg);
extern char *SysErrorMsg (int n);
extern char *strindex (char *s1, char *s2);
extern char *udk_lookup (int keycode, int *len);
extern int XStrCmp (char *s1, char *s2);
extern int xerror (Display *d, XErrorEvent *ev);
extern int xioerror (Display *dpy);
extern void Bell (int which, int percent);
extern void Changename (char *name);
extern void Changetitle (char *name);
extern void Cleanup (int code);
extern void Error (int i);
extern void HandleBellPropertyChange PROTO_XT_EV_HANDLER_ARGS;
extern void HandleEightBitKeyPressed PROTO_XT_ACTIONS_ARGS;
extern void HandleEnterWindow PROTO_XT_EV_HANDLER_ARGS;
extern void HandleFocusChange PROTO_XT_EV_HANDLER_ARGS;
extern void HandleKeyPressed PROTO_XT_ACTIONS_ARGS;
extern void HandleLeaveWindow PROTO_XT_EV_HANDLER_ARGS;
extern void HandleStringEvent PROTO_XT_ACTIONS_ARGS;
extern void Panic (char *s, int a);
extern void Redraw (void);
extern void ReverseOldColors (void);
extern void Setenv (char *var, char *value);
extern void SysError (int i);
extern void VisualBell (void);
extern void creat_as (int uid, int gid, char *pathname, int mode);
extern void do_dcs (Char *buf, size_t len);
extern void do_osc (Char *buf, int len);
extern void do_xevents (void);
extern void end_tek_mode (void);
extern void end_vt_mode (void);
extern void hide_tek_window (void);
extern void hide_vt_window (void);
extern void reset_decudk (void);
extern void set_tek_visibility (Boolean on);
extern void set_vt_visibility (Boolean on);
extern void switch_modes (Bool tovt);
extern void xevents (void);
extern void xt_error (String message);

#ifdef ALLOWLOGGING
extern void StartLog (TScreen *screen);
extern void CloseLog (TScreen *screen);
extern void FlushLog (TScreen *screen);
#endif

/* print.c */
extern int xtermPrinterControl (int chr);
extern void xtermAutoPrint (int chr);
extern void xtermMediaControl (int param, int private);
extern void xtermPrintScreen (void);

/* screen.c */
extern Bool non_blank_line (ScrnBuf sb, int row, int col, int len);
extern ScrnBuf Allocate (int nrow, int ncol, Char **addr);
extern int ScreenResize (TScreen *screen, int width, int height, unsigned *flags);
extern int ScrnTstWrapped (TScreen *screen, int row);
extern size_t ScrnPointers (TScreen *screen, size_t len);
extern void ClearBufRows (TScreen *screen, int first, int last);
extern void ScreenWrite (TScreen *screen, Char *str, unsigned flags, unsigned cur_fg_bg, int length);
extern void ScrnClrWrapped (TScreen *screen, int row);
extern void ScrnDeleteChar (TScreen *screen, int n, int size);
extern void ScrnDeleteLine (TScreen *screen, ScrnBuf sb, int n, int last, int size, int where);
extern void ScrnInsertChar (TScreen *screen, int n, int size);
extern void ScrnInsertLine (TScreen *screen, ScrnBuf sb, int last, int where, int n, int size);
extern void ScrnRefresh (TScreen *screen, int toprow, int leftcol, int nrows, int ncols, int force);
extern void ScrnSetWrapped (TScreen *screen, int row);

/* scrollbar.c */
extern void DoResizeScreen (XtermWidget xw);
extern void HandleScrollBack PROTO_XT_ACTIONS_ARGS;
extern void HandleScrollForward PROTO_XT_ACTIONS_ARGS;
extern void ResizeScrollBar (TScreen *screen);
extern void ScrollBarDrawThumb (Widget scrollWidget);
extern void ScrollBarOff (TScreen *screen);
extern void ScrollBarOn (XtermWidget xw, int init, int doalloc);
extern void ScrollBarReverseVideo (Widget scrollWidget);
extern void WindowScroll (TScreen *screen, int top);

/* tabs.c */
extern Boolean TabToNextStop (void);
extern Boolean TabToPrevStop (void);
extern int TabNext (Tabs tabs, int col);
extern int TabPrev (Tabs tabs, int col);
extern void TabClear (Tabs tabs, int col);
extern void TabReset (Tabs tabs);
extern void TabSet (Tabs tabs, int col);
extern void TabZonk (Tabs	tabs);

/* util.c */
extern GC updatedXtermGC (TScreen *screen, int flags, int fg_bg, Bool hilite);
extern int AddToRefresh (TScreen *screen);
extern int HandleExposure (TScreen *screen, XEvent *event);
extern int drawXtermText (TScreen *screen, unsigned flags, GC gc, int x, int y, int chrset, Char *text, int len);
extern void ChangeColors (XtermWidget tw, ScrnColors *pNew);
extern void ClearRight (TScreen *screen, int n);
extern void ClearScreen (TScreen *screen);
extern void DeleteChar (TScreen *screen, int n);
extern void DeleteLine (TScreen *screen, int n);
extern void FlushScroll (TScreen *screen);
extern void GetColors (XtermWidget tw, ScrnColors *pColors);
extern void InsertChar (TScreen *screen, int n);
extern void InsertLine (TScreen *screen, int n);
extern void RevScroll (TScreen *screen, int amount);
extern void ReverseVideo (XtermWidget termw);
extern void Scroll (TScreen *screen, int amount);
extern void do_erase_display (TScreen *screen, int param, int mode);
extern void do_erase_line (TScreen *screen, int param, int mode);
extern void recolor_cursor (Cursor cursor, unsigned long fg, unsigned long bg);
extern void resetXtermGC (TScreen *screen, int flags, Bool hilite);
extern void scrolling_copy_area (TScreen *screen, int firstline, int nlines, int amount);

#if OPT_ISO_COLORS

extern Pixel getXtermBackground (int flags, int color);
extern Pixel getXtermForeground (int flags, int color);
extern int extract_bg (unsigned color);
extern int extract_fg (unsigned color, unsigned flags);
extern unsigned makeColorPair (int fg, int bg);
extern unsigned xtermColorPair (void);
extern void ClearCurBackground (TScreen *screen, int top, int left, unsigned height, unsigned width);
extern void useCurBackground (Bool flag);

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

#if OPT_DEC_CHRSET
extern int getXtermChrSet (int row, int col);
extern int curXtermChrSet (int row);
#else
#define getXtermChrSet(row, col) 0
#define curXtermChrSet(row) 0
#endif

#if OPT_XMC_GLITCH
extern void Mark_XMC (TScreen *screen, int param);
extern void Jump_XMC (TScreen *screen);
extern void Resolve_XMC (TScreen *screen);
#endif

#endif	/* included_xterm_h */
