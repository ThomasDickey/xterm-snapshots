/* $XFree86: xc/programs/xterm/xterm.h,v 3.9.2.1 1997/05/23 09:24:46 dawes Exp $ */
/*
 * Common/useful definitions for XTERM application
 */
#ifndef	included_xterm_h
#define	included_xterm_h

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
extern void VTReset PROTO((int full));
extern void VTRun PROTO((void));
extern void set_cursor_gcs PROTO((TScreen *screen));
extern void unparseputc1 PROTO((int c, int fd));
extern void unparseputc PROTO((int c, int fd));
extern void unparseseq PROTO((ANSI *ap, int fd));

#if OPT_ISO_COLORS
extern void SGR_Background PROTO((int color));
extern void SGR_Foreground PROTO((int color));
#endif

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

/* input.c */
extern void Input PROTO((TKeyboard *keyboard, TScreen *screen, XKeyEvent *event, Bool eightbit));
extern void StringInput PROTO((TScreen *screen, char *string, Size_t nbytes));

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
extern void do_dcs PROTO((Char *buf, int len));
extern void do_osc PROTO((Char *buf, int len));
extern void do_xevents PROTO((void));
extern void end_tek_mode PROTO((void));
extern void end_vt_mode PROTO((void));
extern void hide_tek_window PROTO((void));
extern void hide_vt_window PROTO((void));
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

/* screen.c */
extern Bool non_blank_line PROTO((ScrnBuf sb, int row, int col, int len));
extern ScrnBuf Allocate PROTO((int nrow, int ncol, Char **addr));
extern int ScreenResize PROTO((TScreen *screen, int width, int height, unsigned *flags));
extern int ScrnGetAttributes PROTO((TScreen *screen, int row, int col, Char *str, int length));
extern void ClearBufRows PROTO((TScreen *screen, int first, int last));
extern void ScreenWrite PROTO((TScreen *screen, char *str, unsigned flags, unsigned cur_fg, unsigned cur_bg, int length));
extern void ScrnDeleteChar PROTO((TScreen *screen, int n, int size));
extern void ScrnDeleteLine PROTO((ScrnBuf sb, int n, int last, int size, int where));
extern void ScrnInsertChar PROTO((TScreen *screen, int n, int size));
extern void ScrnInsertLine PROTO((ScrnBuf sb, int last, int where, int n, int size));
extern void ScrnRefresh PROTO((TScreen *screen, int toprow, int leftcol, int nrows, int ncols, int force));
extern void ScrnSetAttributes PROTO((TScreen *screen, int row, int col, unsigned mask, unsigned value, int length));

/* scrollbar.c */
extern void DoResizeScreen PROTO((XtermWidget xw));
extern void HandleScrollBack PROTO_XT_ACTIONS_ARGS;
extern void HandleScrollForward PROTO_XT_ACTIONS_ARGS;
extern void ResizeScrollBar PROTO((Widget scrollWidget, int x, int y, unsigned height));
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
extern GC updatedXtermGC PROTO((TScreen *screen, int flags, int fg, int bg, Bool hilite));
extern int AddToRefresh PROTO((TScreen *screen));
extern int HandleExposure PROTO((TScreen *screen, XEvent *event));
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
extern void drawXtermText PROTO((TScreen *screen, unsigned flags, GC gc, int x, int y, char *text, int len));
extern void recolor_cursor PROTO((Cursor cursor, unsigned long fg, unsigned long bg));
extern void resetXtermGC PROTO((TScreen *screen, int flags, Bool hilite));
extern void scrolling_copy_area PROTO((TScreen *screen, int firstline, int nlines, int amount));

#if OPT_ISO_COLORS

extern Pixel getXtermBackground PROTO((int flags, int color));
extern Pixel getXtermForeground PROTO((int flags, int color));
extern unsigned extract_bg PROTO((unsigned color));
extern unsigned extract_fg PROTO((unsigned color, unsigned flags));
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

#endif	/* included_xterm_h */
