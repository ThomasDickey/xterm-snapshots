/*
 *	$XConsortium: misc.c /main/112 1996/11/29 10:34:07 swick $
 *	$XFree86: xc/programs/xterm/misc.c,v 3.30 1998/06/04 16:43:59 hohndel Exp $
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

#ifdef HAVE_CONFIG_H
#include <xtermcfg.h>
#endif

#include "ptyx.h"		/* X headers included here. */

#include <X11/Xos.h>
#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <ctype.h>
#include <pwd.h>

#include <X11/Xatom.h>
#include <X11/cursorfont.h>

#include <X11/Shell.h>
#include <X11/Xmu/Error.h>
#include <X11/Xmu/SysUtil.h>
#include <X11/Xmu/WinUtil.h>

#include "xterm.h"

#include "VTparse.h"
#include "data.h"
#include "error.h"
#include "menu.h"

#if XtSpecificationRelease < 6
#ifndef X_GETTIMEOFDAY
#define X_GETTIMEOFDAY(t) gettimeofday(t,(struct timezone *)0)
#endif
#endif

#ifdef AMOEBA
#include "amoeba.h"
#include "module/proc.h"
#endif

#ifdef __EMX__
#define environ gblenvp		/* circumvent a bug */
#endif

extern char **environ;		/* used in 'Setenv()' */

extern jmp_buf Tekend;
extern jmp_buf VTend;

extern Widget toplevel;		/* used in 'ChangeGroup()' */

#if OPT_TEK4014
#define OUR_EVENT(event,Type) \
		(event.type == Type && \
		  (event.xcrossing.window == XtWindow(XtParent(term)) || \
		    (tekWidget && \
		     event.xcrossing.window == XtWindow(XtParent(tekWidget)))))
#else
#define OUR_EVENT(event,Type) \
		(event.type == Type && \
		   (event.xcrossing.window == XtWindow(XtParent(term))))
#endif

static Boolean ChangeColorsRequest (XtermWidget pTerm, int start, char *names);
static void DoSpecialEnterNotify (XEnterWindowEvent *ev);
static void DoSpecialLeaveNotify (XEnterWindowEvent *ev);
static void selectwindow (TScreen *screen, int flag);
static void unselectwindow (TScreen *screen, int flag);

void
do_xevents(void)
{
	register TScreen *screen = &term->screen;

	if (XtAppPending(app_con)
	|| GetBytesAvailable (ConnectionNumber(screen->display)) > 0)
		xevents();
}

void
xevents(void)
{
	XEvent event;
	XtInputMask input_mask;
	register TScreen *screen = &term->screen;

	if(screen->scroll_amt)
		FlushScroll(screen);
	/*
	 * process timeouts, relying on the fact that XtAppProcessEvent
	 * will process the timeout and return without blockng on the
	 * XEvent queue.  Other sources i.e. the pty are handled elsewhere
	 * with select().
	 */
	while ((input_mask = XtAppPending(app_con)) & XtIMTimer)
		XtAppProcessEvent(app_con, XtIMTimer);
	/*
	 * If there's no XEvents, don't wait around...
	 */
	if ((input_mask & XtIMXEvent) != XtIMXEvent)
	    return;
	do {
		if (waitingForTrackInfo)
			return;
		XtAppNextEvent (app_con, &event);
		/*
		 * Hack to get around problems with the toolkit throwing away
		 * eventing during the exclusive grab of the menu popup.  By
		 * looking at the event ourselves we make sure that we can
		 * do the right thing.
		 */
		if(OUR_EVENT(event, EnterNotify))
		  DoSpecialEnterNotify (&event.xcrossing);
		else
		if(OUR_EVENT(event, LeaveNotify))
		  DoSpecialLeaveNotify (&event.xcrossing);

		if (!event.xany.send_event ||
		    screen->allowSendEvents ||
		    ((event.xany.type != KeyPress) &&
		     (event.xany.type != KeyRelease) &&
		     (event.xany.type != ButtonPress) &&
		     (event.xany.type != ButtonRelease)))
		    XtDispatchEvent(&event);
	} while ((input_mask = XtAppPending(app_con)) & XtIMXEvent);
}


Cursor make_colored_cursor (
	unsigned cursorindex,			/* index into font */
	unsigned long fg,			/* pixel value */
	unsigned long bg)			/* pixel value */
{
	register TScreen *screen = &term->screen;
	Cursor c;
	register Display *dpy = screen->display;

	c = XCreateFontCursor (dpy, cursorindex);
	if (c == (Cursor) 0) return (c);

	recolor_cursor (c, fg, bg);
	return (c);
}

/* ARGSUSED */
void HandleKeyPressed(
	Widget w GCC_UNUSED,
	XEvent *event,
	String *params GCC_UNUSED,
	Cardinal *nparams GCC_UNUSED)
{
    register TScreen *screen = &term->screen;

#ifdef ACTIVEWINDOWINPUTONLY
    if (w == CURRENT_EMU(screen))
#endif
	Input (&term->keyboard, screen, &event->xkey, False);
}

/* ARGSUSED */
void HandleEightBitKeyPressed(
	Widget w GCC_UNUSED,
	XEvent *event,
	String *params GCC_UNUSED,
	Cardinal *nparams GCC_UNUSED)
{
    register TScreen *screen = &term->screen;

#ifdef ACTIVEWINDOWINPUTONLY
    if (w == CURRENT_EMU(screen))
#endif
	Input (&term->keyboard, screen, &event->xkey, True);
}

/* ARGSUSED */
void HandleStringEvent(
	Widget w GCC_UNUSED,
	XEvent *event GCC_UNUSED,
	String *params,
	Cardinal *nparams)
{
    register TScreen *screen = &term->screen;

#ifdef ACTIVEWINDOWINPUTONLY
    if (w != CURRENT_EMU(screen))
    	return;
#endif

    if (*nparams != 1) return;

    if ((*params)[0] == '0' && (*params)[1] == 'x' && (*params)[2] != '\0') {
	char c, *p, hexval[2];
	hexval[0] = hexval[1] = 0;
	for (p = *params+2; (c = *p); p++) {
	    hexval[0] *= 16;
	    if (isupper(c)) c = tolower(c);
	    if (c >= '0' && c <= '9')
		hexval[0] += c - '0';
	    else if (c >= 'a' && c <= 'f')
		hexval[0] += c - 'a' + 10;
	    else break;
	}
	if (c == '\0')
	    StringInput (screen, hexval, 1);
    }
    else {
	StringInput (screen, *params, strlen(*params));
    }
}

static void DoSpecialEnterNotify (register XEnterWindowEvent *ev)
{
    register TScreen *screen = &term->screen;

#ifdef ACTIVEWINDOWINPUTONLY
    if (ev->window == XtWindow(XtParent(CURRENT_EMU(screen))))
#endif
      if (((ev->detail) != NotifyInferior) &&
	  ev->focus &&
	  !(screen->select & FOCUS))
	selectwindow(screen, INWINDOW);
}

/*ARGSUSED*/
void HandleEnterWindow(
	Widget w GCC_UNUSED,
	XtPointer eventdata GCC_UNUSED,
	XEvent *event GCC_UNUSED,
	Boolean *cont GCC_UNUSED)
{
    /* NOP since we handled it above */
}


static void DoSpecialLeaveNotify (register XEnterWindowEvent *ev)
{
    register TScreen *screen = &term->screen;

#ifdef ACTIVEWINDOWINPUTONLY
    if (ev->window == XtWindow(XtParent(CURRENT_EMU(screen))))
#endif
      if (((ev->detail) != NotifyInferior) &&
	  ev->focus &&
	  !(screen->select & FOCUS))
	unselectwindow(screen, INWINDOW);
}


/*ARGSUSED*/
void HandleLeaveWindow(
	Widget w GCC_UNUSED,
	XtPointer eventdata GCC_UNUSED,
	XEvent *event GCC_UNUSED,
	Boolean *cont GCC_UNUSED)
{
    /* NOP since we handled it above */
}


/*ARGSUSED*/
void HandleFocusChange(
	Widget w GCC_UNUSED,
	XtPointer eventdata GCC_UNUSED,
	XEvent *ev,
	Boolean *cont GCC_UNUSED)
{
	register XFocusChangeEvent *event = (XFocusChangeEvent  *)ev;
        register TScreen *screen = &term->screen;

        if(event->type == FocusIn)
                selectwindow(screen,
			     (event->detail == NotifyPointer) ? INWINDOW :
								FOCUS);
        else {
                unselectwindow(screen,
			       (event->detail == NotifyPointer) ? INWINDOW :
								  FOCUS);
		if (screen->grabbedKbd && (event->mode == NotifyUngrab)) {
		    Bell(XkbBI_Info, 100);
		    ReverseVideo(term);
		    screen->grabbedKbd = FALSE;
		    update_securekbd();
		}
	}
}


static void
selectwindow(register TScreen *screen, register int flag)
{
#if OPT_TEK4014
	if(screen->TekEmu) {
		if(!Ttoggled)
			TCursorToggle(TOGGLE);
		screen->select |= flag;
		if(!Ttoggled)
			TCursorToggle(TOGGLE);
		return;
	} else
#endif
	{
		if (screen->xic)
		    XSetICFocus(screen->xic);

		if(screen->cursor_state &&
		   (screen->cursor_col != screen->cur_col ||
		    screen->cursor_row != screen->cur_row))
		    HideCursor();
		screen->select |= flag;
		if(screen->cursor_state)
			ShowCursor();
		return;
	}
}

static void
unselectwindow(register TScreen *screen, register int flag)
{
    if (screen->always_highlight) return;

#if OPT_TEK4014
    if(screen->TekEmu) {
	if(!Ttoggled) TCursorToggle(TOGGLE);
	screen->select &= ~flag;
	if(!Ttoggled) TCursorToggle(TOGGLE);
    } else
#endif
    {
	if (screen->xic)
	    XUnsetICFocus(screen->xic);
	screen->select &= ~flag;
	if(screen->cursor_state &&
	   (screen->cursor_col != screen->cur_col ||
	    screen->cursor_row != screen->cur_row))
	      HideCursor();
	if(screen->cursor_state)
	  ShowCursor();
    }
}

static long lastBellTime;	/* in milliseconds */

void
Bell(int which GCC_UNUSED, int percent)
{
    register TScreen *screen = &term->screen;
    struct timeval curtime;
    long now_msecs;

    /* has enough time gone by that we are allowed to ring
       the bell again? */
    if(screen->bellSuppressTime) {
	if(screen->bellInProgress) {
	    do_xevents();
	    if(screen->bellInProgress) { /* even after new events? */
		return;
	    }
	}
	X_GETTIMEOFDAY(&curtime);
	now_msecs = 1000*curtime.tv_sec + curtime.tv_usec/1000;
	if(lastBellTime != 0  &&  now_msecs - lastBellTime >= 0  &&
	   now_msecs - lastBellTime < screen->bellSuppressTime) {
	    return;
	}
	lastBellTime = now_msecs;
    }

    if (screen->visualbell)
	VisualBell();
    else
#ifdef XKB
	XkbStdBell(screen->display,TWindow(screen),percent,which);
#else
	XBell(screen->display, percent);
#endif

    if(screen->bellSuppressTime) {
	/* now we change a property and wait for the notify event to come
	   back.  If the server is suspending operations while the bell
	   is being emitted (problematic for audio bell), this lets us
	   know when the previous bell has finished */
	Widget w = CURRENT_EMU(screen);
	XChangeProperty(XtDisplay(w), XtWindow(w),
			XA_NOTICE, XA_NOTICE, 8, PropModeAppend, NULL, 0);
	screen->bellInProgress = TRUE;
    }
}

void
VisualBell(void)
{
    register TScreen *screen = &term->screen;
    register Pixel xorPixel = screen->foreground ^ term->core.background_pixel;
    XGCValues gcval;
    GC visualGC;

    gcval.function = GXxor;
    gcval.foreground = xorPixel;
    visualGC = XtGetGC((Widget)term, GCFunction+GCForeground, &gcval);
#if OPT_TEK4014
    if(screen->TekEmu) {
	XFillRectangle(
		       screen->display,
		       TWindow(screen),
		       visualGC,
		       0, 0,
		       (unsigned) TFullWidth(screen),
		       (unsigned) TFullHeight(screen));
	XFlush(screen->display);
	XFillRectangle(
		       screen->display,
		       TWindow(screen),
		       visualGC,
		       0, 0,
		       (unsigned) TFullWidth(screen),
		       (unsigned) TFullHeight(screen));
    } else
#endif
    {
	XFillRectangle(
		       screen->display,
		       VWindow(screen),
		       visualGC,
		       0, 0,
		       (unsigned) FullWidth(screen),
		       (unsigned) FullHeight(screen));
	XFlush(screen->display);
	XFillRectangle(
		       screen->display,
		       VWindow(screen),
		       visualGC,
		       0, 0,
		       (unsigned) FullWidth(screen),
		       (unsigned) FullHeight(screen));
    }
}

/* ARGSUSED */
void HandleBellPropertyChange(
	Widget w GCC_UNUSED,
	XtPointer data GCC_UNUSED,
	XEvent *ev,
	Boolean *more GCC_UNUSED)
{
    register TScreen *screen = &term->screen;

    if (ev->xproperty.atom == XA_NOTICE) {
	screen->bellInProgress = FALSE;
    }
}

void
Redraw(void)
{
	register TScreen *screen = &term->screen;
	XExposeEvent event;

	event.type = Expose;
	event.display = screen->display;
	event.x = 0;
	event.y = 0;
	event.count = 0;

	if(VWindow(screen)) {
	        event.window = VWindow(screen);
		event.width = term->core.width;
		event.height = term->core.height;
		(*term->core.widget_class->core_class.expose)((Widget)term, (XEvent *)&event, NULL);
		if(Scrollbar(screen))
			(*screen->scrollWidget->core.widget_class->core_class.expose)(screen->scrollWidget, (XEvent *)&event, NULL);
		}

#if OPT_TEK4014
	if(TWindow(screen) && screen->Tshow) {
	        event.window = TWindow(screen);
		event.width = tekWidget->core.width;
		event.height = tekWidget->core.height;
		TekExpose ((Widget)tekWidget, (XEvent *)&event, NULL);
	}
#endif
}

#if defined(ALLOWLOGGING) || defined(DEBUG)

/*
 * create a file only if we could with the permissions of the real user id.
 * We could emulate this with careful use of access() and following
 * symbolic links, but that is messy and has race conditions.
 * Forking is messy, too, but we can't count on setreuid() or saved set-uids
 * being available.
 *
 * Note:  when called for user logging, we have ensured that the real and
 * effective user ids are the same, so this remains as a convenience function
 * for the debug logs.
 */
void
creat_as(int uid, int gid, char *pathname, int mode)
{
    int fd;
    int pid;
#ifndef HAVE_WAITPID
    int waited;
    SIGNAL_T (*chldfunc) (int);

    chldfunc = signal(SIGCHLD, SIG_DFL);
#endif

    pid = fork();
    switch (pid)
    {
    case 0:			/* child */
	setgid(gid);
	setuid(uid);
	fd = open(pathname, O_WRONLY|O_CREAT|O_APPEND, mode);
	if (fd >= 0) {
	    close(fd);
	    _exit(0);
	} else
	    _exit(1);
    case -1:			/* error */
	return;
    default:			/* parent */
#ifdef HAVE_WAITPID
	waitpid(pid, NULL, 0);
#else
	waited = wait(NULL);
	signal(SIGCHLD, chldfunc);
	/*
	  Since we had the signal handler uninstalled for a while,
	  we might have missed the termination of our screen child.
	  If we can check for this possibility without hanging, do so.
	*/
	do
	    if (waited == term->screen.pid)
		Cleanup(0);
	while ( (waited=nonblocking_wait()) > 0);
#endif
    }
}
#endif

#ifdef ALLOWLOGGING

/*
 * Logging is a security hole, since it allows a setuid program to write
 * arbitrary data to an arbitrary file.  So it is disabled by default.
 */

#ifdef ALLOWLOGFILEEXEC
static SIGNAL_T logpipe (int sig GCC_UNUSED)
{
	register TScreen *screen = &term->screen;

#ifdef SYSV
	(void) signal(SIGPIPE, SIG_IGN);
#endif	/* SYSV */
	if(screen->logging)
		CloseLog(screen);
}
#endif /* ALLOWLOGFILEEXEC */

void
StartLog(register TScreen *screen)
{
	static char *log_default;
#ifdef ALLOWLOGFILEEXEC
	register char *cp;
	register int i;
#ifdef SYSV
	/* SYSV has another pointer which should be part of the
	** FILE structure but is actually a separate array.
	*/
	unsigned char *old_bufend;
#endif	/* SYSV */
#endif /* ALLOWLOGFILEEXEC */

	if(screen->logging || (screen->inhibit & I_LOG))
		return;
	if(screen->logfile == NULL || *screen->logfile == 0) {
		if(screen->logfile)
			free(screen->logfile);
		if(log_default == NULL) {
			log_default = malloc(strlen(log_def_name) + 1);
			if (log_default == 0)
				return;
			strcpy(log_default, log_def_name);
			mktemp(log_default);
		}
		if((screen->logfile = malloc(strlen(log_default) + 1)) == NULL)
			return;
		strcpy(screen->logfile, log_default);
	}
	if(*screen->logfile == '|') {	/* exec command */
#ifdef ALLOWLOGFILEEXEC
		/*
		 * Warning, enabling this "feature" allows arbitrary programs
		 * to be run.  If ALLOWLOGFILECHANGES is enabled, this can be
		 * done through escape sequences....  You have been warned.
		 */
		int p[2];
		static char *shell;
		register struct passwd *pw;

		if(pipe(p) < 0 || (i = fork()) < 0)
			return;
		if(i == 0) {	/* child */
			/*
			 * Close our output (we won't be talking back to the
			 * parent), and redirect our child's output to the
			 * original stderr.
			 */
			close(p[1]);
			dup2(p[0], 0);
			close(p[0]);
			dup2(fileno(stderr), 1);
			dup2(fileno(stderr), 2);

			close(fileno(stderr));
			close(ConnectionNumber(screen->display));
			close(screen->respond);

			if ((((cp = getenv("SHELL")) == NULL || *cp == 0)
			  && ((pw = getpwuid(screen->uid)) == NULL
			   || *(cp = pw->pw_shell) == 0))
			 || (shell = malloc(strlen(cp) + 1)) == NULL)
				shell = "/bin/sh";
			else
				strcpy(shell, cp);

			signal(SIGHUP, SIG_DFL);
			signal(SIGCHLD, SIG_DFL);

			/* (this is redundant) */
			setgid(screen->gid);
			setuid(screen->uid);

			execl(shell, shell, "-c", &screen->logfile[1], 0);

			fprintf(stderr, "%s: Can't exec `%s'\n", xterm_name,
			 &screen->logfile[1]);
			exit(ERROR_LOGEXEC);
		}
		close(p[0]);
		screen->logfd = p[1];
		signal(SIGPIPE, logpipe);
#else
		Bell(XkbBI_Info,0);
		Bell(XkbBI_Info,0);
		return;
#endif
	} else {
		if(access(screen->logfile, F_OK) != 0) {
		    if (errno == ENOENT)
			creat_as(screen->uid, screen->gid,
				 screen->logfile, 0644);
		    else
			return;
		}

		if(access(screen->logfile, F_OK) != 0
		   || access(screen->logfile, W_OK) != 0)
		    return;
		if((screen->logfd = open(screen->logfile, O_WRONLY | O_APPEND,
					 0644)) < 0)
			return;
	}
	screen->logstart = CURRENT_EMU_VAL(screen, Tbptr, bptr);
	screen->logging = TRUE;
	update_logging();
}

void
CloseLog(register TScreen *screen)
{
	if(!screen->logging || (screen->inhibit & I_LOG))
		return;
	FlushLog(screen);
	close(screen->logfd);
	screen->logging = FALSE;
	update_logging();
}

void
FlushLog(register TScreen *screen)
{
	register Char *cp;
	register int i;

	cp = CURRENT_EMU_VAL(screen, Tbptr, bptr);
	if((i = cp - screen->logstart) > 0)
		write(screen->logfd, (char *)screen->logstart, i);
	screen->logstart = CURRENT_EMU_VAL(screen, Tbuffer, VTbuffer);
}

#endif /* ALLOWLOGGING */

void
do_osc(Char *oscbuf, int len GCC_UNUSED)
{
	register int mode;
	register Char *cp;
	int state = 0;
	char *buf = 0;

	/*
	 * lines should be of the form <OSC> number ; string <ST>
	 */
	mode = 0;
	for (cp = oscbuf; *cp != '\0'; cp++) {
		switch (state) {
		case 0:
			if (isdigit(*cp)) {
				mode = 10 * mode + (*cp - '0');
				break;
			}
			/* FALLTHRU */
		case 1:
			if (*cp != ';')
				return;
			state = 2;
			break;
		case 2:
			buf = (char *)cp;
			state = 3;
			/* FALLTHRU */
		default:
			if (!isprint(*cp & 0x7f))
				return;
		}
	}

	switch(mode) {
	case 0:	/* new icon name and title*/
		Changename(buf);
		Changetitle(buf);
		break;

	case 1:	/* new icon name only */
		Changename(buf);
		break;

	case 2:	/* new title only */
		Changetitle(buf);
		break;

        case 10:       case 11:        case 12:
        case 13:       case 14:        case 15:
        case 16:
               {
                   if (term->misc.dynamicColors)
                       ChangeColorsRequest(term,mode-10,buf);
               }
               break;

#ifdef ALLOWLOGGING
	 case 46:	/* new log file */
#ifdef ALLOWLOGFILECHANGES
		/*
		 * Warning, enabling this feature allows people to overwrite
		 * arbitrary files accessible to the person running xterm.
		 */
		if((cp = malloc((unsigned)strlen(buf) + 1)) == NULL)
			break;
		strcpy(cp, buf);
		if(term->screen.logfile)
			free(term->screen.logfile);
		term->screen.logfile = cp;
#else
		Bell(XkbBI_Info,0);
		Bell(XkbBI_Info,0);
#endif
		break;
#endif /* ALLOWLOGGING */

	case 50:
		SetVTFont (fontMenu_fontescape, True, buf, NULL);
		break;

	/*
	 * One could write code to send back the display and host names,
	 * but that could potentially open a fairly nasty security hole.
	 */
	}
}

#define MAX_UDK 35
static struct {
	char *str;
	int   len;
	} user_keys[MAX_UDK];

static int
hexvalue(int c)
{
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	return -1;
}

void
reset_decudk(void)
{
	int n;
	for (n = 0; n < MAX_UDK; n++) {
		if (user_keys[n].str != 0) {
			free(user_keys[n].str);
			user_keys[n].str = 0;
			user_keys[n].len = 0;
		}
	}
}

void
do_dcs(Char *dcsbuf, size_t dcslen)
{
	register TScreen *screen = &term->screen;
	char *cp = (char *)dcsbuf;

	if (*cp == '$') { /* DECRQSS */
		char reply[BUFSIZ];
		Bool okay = True;

		cp++;
		if (*cp++ == 'q') {
			if (!strcmp(cp, "\"q")) {		/* DECSCA */
				sprintf(reply, "%d%s",
					(screen->protected_mode == DEC_PROTECT)
					 && (term->flags & PROTECTED) ? 1 : 0,
					cp);
			} else if (!strcmp(cp, "\"p")) {	/* DECSCL */
				sprintf(reply, "%d%s",
					(screen->ansi_level ?
					 screen->ansi_level : 1) + 60,
					cp);
			} else if (!strcmp(cp, "r")) {		/* DECSTBM */
				sprintf(reply, "%d;%dr",
					screen->top_marg + 1,
					screen->bot_marg + 1);
			} else if (!strcmp(cp, "m")) {		/* SGR */
				strcpy(reply, "0");
				if (term->flags & BOLD)
					strcat(reply, ";1");
				if (term->flags & UNDERLINE)
					strcat(reply, ";4");
				if (term->flags & BLINK)
					strcat(reply, ";5");
				if (term->flags & INVERSE)
					strcat(reply, ";7");
				if (term->flags & INVISIBLE)
					strcat(reply, ";8");
				if_OPT_ISO_COLORS(screen,{
				if (term->flags & FG_COLOR)
					sprintf(reply+strlen(reply),
						";3%d", term->cur_foreground);
				if (term->flags & BG_COLOR)
					sprintf(reply+strlen(reply),
						";4%d", term->cur_background);
				})
				strcat(reply, "m");
			} else
				okay = False;

			unparseputc1(DCS, screen->respond);
			unparseputc(okay ? '1' : '0', screen->respond);
			unparseputc('$', screen->respond);
			unparseputc('r', screen->respond);
			if (okay)
				cp = reply;
			while (*cp != '\0')
				unparseputc(*cp++, screen->respond);
			unparseputc1(ST, screen->respond);
		} else {
			unparseputc(CAN, screen->respond);
		}
	} else { /* DECUDK */
		Bool clear_all = True;
		Bool lock_keys = True;

		if (dcslen != strlen(cp))
			/* shouldn't have nulls in the string */
			return;

		if (*cp == '0') {
			cp++;
		} else if (*cp == '1') {
			cp++;
			clear_all = False;
		}

		if (*cp == ';')
			cp++;
		else if (*cp != '|')
			return;

		if (*cp == '0') {
			cp++;
		} else if (*cp == '1') {
			cp++;
			lock_keys = False;
		}

		if (*cp++ != '|')
			return;

		if (clear_all)
			reset_decudk();

		while (*cp) {
			char *str = malloc(strlen(cp) + 2);
			int key = 0;
			int len = 0;

			while (isdigit(*cp))
				key = (key * 10) + (*cp++ - '0');
			if (*cp == '/') {
				cp++;
				while (*cp != ';' && *cp != '\0') {
					int hi = hexvalue(*cp++);
					int lo = hexvalue(*cp++);
					if (hi >= 0 && lo >= 0)
						str[len++] = (hi << 4) | lo;
					else
						return;
				}
			}
			if (len > 0 && key < MAX_UDK) {
				if (user_keys[key].str != 0)
					free(user_keys[key].str);
				user_keys[key].str = str;
				user_keys[key].len = len;
			} else {
				free(str);
			}
			if (*cp == ';')
				cp++;
		}
	}
}

char *
udk_lookup(int keycode, int *len)
{
	if (keycode < MAX_UDK) {
		*len = user_keys[keycode].len;
		return user_keys[keycode].str;
	}
	return 0;
}

static void
ChangeGroup(String attribute, XtArgVal value)
{
	Arg args[1];
	char *name = (value != 0) ? (char *)value : "";

#if OPT_SAME_NAME
	/* If the attribute isn't going to change, then don't bother... */

	if( sameName ) {
	    char *buf;
	    XtSetArg( args[0], attribute, &buf );
	    XtGetValues( toplevel, args, 1 );
	    if (strcmp(name, buf) == 0)
		return;
	}
#endif /* OPT_SAME_NAME */

	XtSetArg( args[0], attribute, name );
	XtSetValues( toplevel, args, 1 );
}

void
Changename(register char *name)
{
#if OPT_ZICONBEEP	/* If warning should be given then give it */
    if ( zIconBeep && zIconBeep_flagged ) {
	char *newname = malloc(strlen(name)+ 4 + 1);
	if (!newname) {
	    fprintf(stderr, "malloc failed in Changename\n");
	    return;
	}
	strcpy(newname, "*** ");
	strcat(newname, name);
	ChangeGroup( XtNiconName, (XtArgVal)newname );
	free(newname);
    } else
#endif /* OPT_ZICONBEEP */
	ChangeGroup( XtNiconName, (XtArgVal)name );
}

void
Changetitle(register char *name)
{
    ChangeGroup( XtNtitle, (XtArgVal)name );
}

/***====================================================================***/

ScrnColors	*pOldColors= NULL;

static Boolean
GetOldColors(XtermWidget pTerm)
{
int	i;
    if (pOldColors==NULL) {
	pOldColors=	(ScrnColors *)XtMalloc(sizeof(ScrnColors));
	if (pOldColors==NULL) {
	    fprintf(stderr,"allocation failure in GetOldColors\n");
	    return(FALSE);
	}
	pOldColors->which=	0;
	for (i=0;i<NCOLORS;i++) {
	    pOldColors->colors[i]=	0;
	    pOldColors->names[i]=	NULL;
	}
	GetColors(pTerm,pOldColors);
    }
    return(TRUE);
}

static Boolean
UpdateOldColors(
	XtermWidget	pTerm GCC_UNUSED,
	ScrnColors	*pNew)
{
int	i;

    /* if we were going to free old colors, this would be the place to
     * do it.   I've decided not to (for now), because it seems likely
     * that we'd have a small set of colors we use over and over, and that
     * we could save some overhead this way.   The only case in which this
     * (clearly) fails is if someone is trying a boatload of colors, in
     * which case they can restart xterm
     */
    for (i=0;i<NCOLORS;i++) {
	if (COLOR_DEFINED(pNew,i)) {
	    if (pOldColors->names[i]!=NULL) {
		XtFree(pOldColors->names[i]);
		pOldColors->names[i]= NULL;
	    }
	    if (pNew->names[i]) {
		pOldColors->names[i]= pNew->names[i];
	    }
	    pOldColors->colors[i]=	pNew->colors[i];
	}
    }
    return(TRUE);
}

void
ReverseOldColors(void)
{
register ScrnColors	*pOld= pOldColors;
Pixel	 tmpPix;
char	*tmpName;

    if (pOld) {
	/* change text cursor, if necesary */
	if (pOld->colors[TEXT_CURSOR]==pOld->colors[TEXT_FG]) {
	    pOld->colors[TEXT_CURSOR]=	pOld->colors[TEXT_BG];
	    if (pOld->names[TEXT_CURSOR]) {
		XtFree(pOldColors->names[TEXT_CURSOR]);
		pOld->names[TEXT_CURSOR]= NULL;
	    }
	    if (pOld->names[TEXT_BG]) {
		tmpName= XtMalloc(strlen(pOld->names[TEXT_BG])+1);
		if (tmpName) {
		    strcpy(tmpName,pOld->names[TEXT_BG]);
		    pOld->names[TEXT_CURSOR]= tmpName;
		}
	    }
	}

	/* swap text FG and BG */
	tmpPix=		pOld->colors[TEXT_FG];
	tmpName=	pOld->names[TEXT_FG];
	pOld->colors[TEXT_FG]=	pOld->colors[TEXT_BG];
	pOld->names[TEXT_FG]=	pOld->names[TEXT_BG];
	pOld->colors[TEXT_BG]=	tmpPix;
	pOld->names[TEXT_BG]=	tmpName;

	/* swap mouse FG and BG */
	tmpPix=		pOld->colors[MOUSE_FG];
	tmpName=	pOld->names[MOUSE_FG];
	pOld->colors[MOUSE_FG]=	pOld->colors[MOUSE_BG];
	pOld->names[MOUSE_FG]=	pOld->names[MOUSE_BG];
	pOld->colors[MOUSE_BG]=	tmpPix;
	pOld->names[MOUSE_BG]=	tmpName;

	/* swap Tek FG and BG */
	tmpPix=		pOld->colors[TEK_FG];
	tmpName=	pOld->names[TEK_FG];
	pOld->colors[TEK_FG]=	pOld->colors[TEK_BG];
	pOld->names[TEK_FG]=	pOld->names[TEK_BG];
	pOld->colors[TEK_BG]=	tmpPix;
	pOld->names[TEK_BG]=	tmpName;
    }
    return;
}

static Boolean
AllocateColor(
	XtermWidget	 pTerm,
	ScrnColors	*pNew,
	int		 ndx,
	char		*name)
{
XColor			 def;
register TScreen	*screen=	&pTerm->screen;
Colormap		 cmap=		pTerm->core.colormap;
char			*newName;

    if ((XParseColor(screen->display,cmap,name,&def))&&
	(XAllocColor(screen->display,cmap,&def))) {
	SET_COLOR_VALUE(pNew,ndx,def.pixel);
	newName= XtMalloc(strlen(name)+1);
	if (newName) {
	    strcpy(newName,name);
	    SET_COLOR_NAME(pNew,ndx,newName);
	}
	return(TRUE);
    }
    return(FALSE);
}

static Boolean
ChangeColorsRequest(
	XtermWidget	pTerm,
	int		start,
	register char	*names)
{
char		*thisName;
ScrnColors	newColors;
int		i,ndx;

    if ((pOldColors==NULL)&&(!GetOldColors(pTerm))) {
	return(FALSE);
    }
    newColors.which=	0;
    for (i=0;i<NCOLORS;i++) {
	newColors.names[i]=	NULL;
    }
    for (i=start;i<NCOLORS;i++) {
	if (term->misc.re_verse)	ndx=	OPPOSITE_COLOR(i);
	else				ndx=	i;
	if ((names==NULL)||(names[0]=='\0')) {
	    newColors.names[ndx]=	NULL;
	}
	else {
	    if (names[0]==';')
		 thisName=	NULL;
	    else thisName=	names;
	    names = strchr(names,';');
	    if (names!=NULL) {
		*names=	'\0';
		names++;
	    }
	    if ((!pOldColors->names[ndx])||
		(thisName&&(strcmp(thisName,pOldColors->names[ndx])))) {
		AllocateColor(pTerm,&newColors,ndx,thisName);
	    }
	}
    }

    if (newColors.which==0)
	return(TRUE);

    ChangeColors(pTerm,&newColors);
    UpdateOldColors(pTerm,&newColors);
    return(TRUE);
}

/***====================================================================***/

#ifndef DEBUG
/* ARGSUSED */
#endif
void
Panic(char *s GCC_UNUSED, int a GCC_UNUSED)
{
#ifdef DEBUG
	if(debug) {
		fprintf(stderr, "%s: PANIC!\t", xterm_name);
		fprintf(stderr, s, a);
		fputs("\r\n", stderr);
		fflush(stderr);
	}
#endif	/* DEBUG */
}

char *SysErrorMsg (int n)
{
    register char *s = strerror(n);
    return s ? s : "unknown error";
}

void
SysError (int i)
{
	int oerrno;

	oerrno = errno;
	/* perror(3) write(2)s to file descriptor 2 */
	fprintf (stderr, "%s: Error %d, errno %d: ", xterm_name, i, oerrno);
	fprintf (stderr, "%s\n", SysErrorMsg (oerrno));
	Cleanup(i);
}

void
Error (int i)
{
	fprintf (stderr, "%s: Error %d\n", xterm_name, i);
	Cleanup(i);
}


/*
 * cleanup by sending SIGHUP to client processes
 */
void
Cleanup (int code)
{
	register TScreen *screen;

	screen = &term->screen;
	if (screen->pid > 1) {
	    (void) kill_process_group (screen->pid, SIGHUP);
	}
#ifdef AMOEBA
	if (!NULLPORT(&screen->proccap.cap_port))
	    (void) pro_stun(&screen->proccap, -1L);
	cb_close(screen->tty_outq);
	cb_close(screen->tty_inq);
#endif
	Exit (code);
}

/*
 * sets the value of var to be arg in the Unix 4.2 BSD environment env.
 * Var should end with '=' (bindings are of the form "var=value").
 * This procedure assumes the memory for the first level of environ
 * was allocated using calloc, with enough extra room at the end so not
 * to have to do a realloc().
 */
void
Setenv (register char *var, register char *value)
{
	register int envindex = 0;
	register size_t len = strlen(var);

	TRACE(("Setenv(var=%s, value=%s)\n", var, value))

	while (environ [envindex] != NULL) {
	    if (strncmp (environ [envindex], var, len) == 0) {
		/* found it */
		environ[envindex] = (char *)malloc ((unsigned)len + strlen (value) + 1);
		strcpy (environ [envindex], var);
		strcat (environ [envindex], value);
		return;
	    }
	    envindex ++;
	}

	TRACE(("...expanding env to %d\n", envindex+1))

	environ [envindex] = (char *) malloc ((unsigned)len + strlen (value) + 1);
	(void) strcpy (environ [envindex], var);
	strcat (environ [envindex], value);
	environ [++envindex] = NULL;
}

/*
 * returns a pointer to the first occurrence of s2 in s1,
 * or NULL if there are none.
 */
char *strindex ( register char *s1, register char *s2)
{
	register char	*s3;
	size_t s2len = strlen (s2);

	while ((s3=strchr(s1, *s2)) != NULL) {
		if (strncmp(s3, s2, s2len) == 0)
			return (s3);
		s1 = ++s3;
	}
	return (NULL);
}

/*ARGSUSED*/
int
xerror(Display *d, register XErrorEvent *ev)
{
    fprintf (stderr, "%s:  warning, error event received:\n", xterm_name);
    (void) XmuPrintDefaultErrorMessage (d, ev, stderr);
    Exit (ERROR_XERROR);
    return 0;	/* appease the compiler */
}

/*ARGSUSED*/
int
xioerror(Display *dpy)
{
    (void) fprintf (stderr,
		    "%s:  fatal IO error %d (%s) or KillClient on X server \"%s\"\r\n",
		    xterm_name, errno, SysErrorMsg (errno),
		    DisplayString (dpy));

    Exit(ERROR_XIOERROR);
    return 0;	/* appease the compiler */
}

extern char *ProgramName;

void xt_error(String message)
{
    (void) fprintf (stderr, "%s Xt error: %s\n", ProgramName, message);
    exit(1);
}

int
XStrCmp(char *s1, char *s2)
{
  if (s1 && s2) return(strcmp(s1, s2));
  if (s1 && *s1) return(1);
  if (s2 && *s2) return(-1);
  return(0);
}

#if OPT_TEK4014
static void withdraw_window (Display *dpy, Window w, int scr)
{
    (void) XmuUpdateMapHints (dpy, w, NULL);
    XWithdrawWindow (dpy, w, scr);
    return;
}
#endif


void set_vt_visibility (Boolean on)
{
    register TScreen *screen = &term->screen;

    if (on) {
	if (!screen->Vshow && term) {
	    VTInit ();
	    XtMapWidget (term->core.parent);
	    screen->Vshow = TRUE;
	}
    }
#if OPT_TEK4014
    else {
	if (screen->Vshow && term) {
	    withdraw_window (XtDisplay (term),
			     XtWindow(XtParent(term)),
			     XScreenNumberOfScreen(XtScreen(term)));
	    screen->Vshow = FALSE;
	}
    }
    set_vthide_sensitivity();
    set_tekhide_sensitivity();
    update_vttekmode();
    update_tekshow();
    update_vtshow();
#endif
    return;
}

#if OPT_TEK4014
extern Atom wm_delete_window;	/* for ICCCM delete window */

void set_tek_visibility (Boolean on)
{
    register TScreen *screen = &term->screen;
    if (on) {
	if (!screen->Tshow && (tekWidget || TekInit())) {
	    Widget tekParent = tekWidget->core.parent;
	    XtRealizeWidget (tekParent);
	    XtMapWidget (tekParent);
	    XtOverrideTranslations(tekParent,
				   XtParseTranslationTable
				   ("<Message>WM_PROTOCOLS: DeleteWindow()"));
	    (void) XSetWMProtocols (XtDisplay(tekParent),
				    XtWindow(tekParent),
				    &wm_delete_window, 1);
	    screen->Tshow = TRUE;
	}
    } else {
	if (screen->Tshow && tekWidget) {
	    withdraw_window (XtDisplay (tekWidget),
			     XtWindow(XtParent(tekWidget)),
			     XScreenNumberOfScreen(XtScreen(tekWidget)));
	    screen->Tshow = FALSE;
	}
    }
    set_tekhide_sensitivity();
    set_vthide_sensitivity();
    update_vtshow();
    update_tekshow();
    update_vttekmode();
    return;
}

void end_tek_mode (void)
{
    register TScreen *screen = &term->screen;

    if (screen->TekEmu) {
#ifdef ALLOWLOGGING
	if (screen->logging) {
	    FlushLog (screen);
	    screen->logstart = VTbuffer;
	}
#endif
	longjmp(Tekend, 1);
    }
    return;
}

void end_vt_mode (void)
{
    register TScreen *screen = &term->screen;

    if (!screen->TekEmu) {
#ifdef ALLOWLOGGING
	if(screen->logging) {
	    FlushLog(screen);
	    screen->logstart = Tbuffer;
	}
#endif
	screen->TekEmu = TRUE;
	longjmp(VTend, 1);
    }
    return;
}

void switch_modes (Bool tovt)		/* if true, then become vt mode */
{
    if (tovt) {
	if (TekRefresh) dorefresh();
	end_tek_mode ();		/* WARNING: this does a longjmp... */
    } else {
	end_vt_mode ();			/* WARNING: this does a longjmp... */
    }
}

void hide_vt_window (void)
{
    register TScreen *screen = &term->screen;

    set_vt_visibility (FALSE);
    if (!screen->TekEmu) switch_modes (False);	/* switch to tek mode */
}

void hide_tek_window (void)
{
    register TScreen *screen = &term->screen;

    set_tek_visibility (FALSE);
    TekRefresh = (TekLink *)0;
    if (screen->TekEmu) switch_modes (True);	/* does longjmp to vt mode */
}
#endif /* OPT_TEK4014 */
