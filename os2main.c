/* removed all foreign stuff to get the code more clear (hv) 
 * and did some rewrite for the obscure OS/2 environment
 */

#ifndef lint
static char *rid="$XConsortium: main.c,v 1.227.1.2 95/06/29 18:13:15 kaleb Exp $";
#endif /* lint */
/* $XFree86: xc/programs/xterm/os2main.c,v 3.5.2.1 1997/05/12 12:52:44 hohndel Exp $ */

/***********************************************************


Copyright (c) 1987, 1988  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.


Copyright 1987, 1988 by Digital Equipment Corporation, Maynard.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be used in 
advertising or publicity pertaining to distribution of the software 
without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/


/* main.c */

#ifdef __EMX__
#define INCL_DOSFILEMGR
#define INCL_DOSDEVIOCTL
#define INCL_DOSSEMAPHORES
#include <os2.h>
#endif

#include "ptyx.h"
#include "data.h"
#include "error.h"
#include "menu.h"
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#ifdef I18N
#include <X11/Xlocale.h>
#endif

#include <X11/Xos.h>
#include <X11/cursorfont.h>
#include <X11/Xaw/SimpleMenu.h>
#include <pwd.h>
#include <ctype.h>

#include <sys/termio.h>

int setpgrp(pid_t pid ,gid_t pgid) {}
int chown(const char* fn, pid_t pid, gid_t gid) {}
char *ttyname(int fd) { return "/dev/tty"; }

#include <sys/ioctl.h>
#include <sys/stat.h>

#include <sys/param.h>	/* for NOFILE */

#include <stdio.h>
#include <errno.h>
#include <setjmp.h>

#include <time.h>
#define Time_t time_t

#include <signal.h>
#include <sys/wait.h>

#define SIGNAL_T void
#define SIGNAL_RETURN return

SIGNAL_T Exit();

#include <unistd.h>
#include <stdlib.h>

extern char *strindex ();
extern void HandlePopupMenu();

int switchfb[] = {0, 2, 1, 3};

static SIGNAL_T reapchild ();

static Bool added_utmp_entry = False;

static char **command_to_exec;

/* The following structures are initialized in main() in order
** to eliminate any assumptions about the internal order of their
** contents.
*/
static struct termio d_tio;

/* allow use of system default characters if defined and reasonable */
#ifndef CEOF
#define CEOF ('D'&037)
#endif
#ifndef CSUSP
#define CSUSP ('Z'&037)
#endif
#ifndef CQUIT
#define CQUIT ('\\'&037)
#endif
#ifndef CEOL
#define CEOL 0
#endif
#ifndef CSWTCH
#define CSWTCH 0
#endif
#ifndef CLNEXT
#define CLNEXT ('V'&037)
#endif
#ifndef CWERASE
#define CWERASE ('W'&037)
#endif
#ifndef CRPRNT
#define CRPRNT ('R'&037)
#endif
#ifndef CFLUSH
#define CFLUSH ('O'&037)
#endif
#ifndef CSTOP
#define CSTOP ('S'&037)
#endif
#ifndef CSTART
#define CSTART ('Q'&037)
#endif

static int parse_tty_modes ();
/*
 * SYSV has the termio.c_cc[V] and ltchars; BSD has tchars and ltchars;
 * SVR4 has only termio.c_cc, but it includes everything from ltchars.
 */
static int override_tty_modes = 0;
struct _xttymodes {
    char *name;
    int len;
    int set;
    char value;
} ttymodelist[] = {
{ "intr", 4, 0, '\0' },			/* tchars.t_intrc ; VINTR */
#define XTTYMODE_intr 0
{ "quit", 4, 0, '\0' },			/* tchars.t_quitc ; VQUIT */
#define XTTYMODE_quit 1
{ "erase", 5, 0, '\0' },		/* sgttyb.sg_erase ; VERASE */
#define XTTYMODE_erase 2
{ "kill", 4, 0, '\0' },			/* sgttyb.sg_kill ; VKILL */
#define XTTYMODE_kill 3
{ "eof", 3, 0, '\0' },			/* tchars.t_eofc ; VEOF */
#define XTTYMODE_eof 4
{ "eol", 3, 0, '\0' },			/* VEOL */
#define XTTYMODE_eol 5
{ "swtch", 5, 0, '\0' },		/* VSWTCH */
#define XTTYMODE_swtch 6
{ "start", 5, 0, '\0' },		/* tchars.t_startc */
#define XTTYMODE_start 7
{ "stop", 4, 0, '\0' },			/* tchars.t_stopc */
#define XTTYMODE_stop 8
{ "brk", 3, 0, '\0' },			/* tchars.t_brkc */
#define XTTYMODE_brk 9
{ "susp", 4, 0, '\0' },			/* ltchars.t_suspc ; VSUSP */
#define XTTYMODE_susp 10
{ "dsusp", 5, 0, '\0' },		/* ltchars.t_dsuspc ; VDSUSP */
#define XTTYMODE_dsusp 11
{ "rprnt", 5, 0, '\0' },		/* ltchars.t_rprntc ; VREPRINT */
#define XTTYMODE_rprnt 12
{ "flush", 5, 0, '\0' },		/* ltchars.t_flushc ; VDISCARD */
#define XTTYMODE_flush 13
{ "weras", 5, 0, '\0' },		/* ltchars.t_werasc ; VWERASE */
#define XTTYMODE_weras 14
{ "lnext", 5, 0, '\0' },		/* ltchars.t_lnextc ; VLNEXT */
#define XTTYMODE_lnext 15
{ NULL, 0, 0, '\0' },			/* end of data */
};

static int inhibit;
static char passedPty[2];	/* name if pty if slave */


#ifdef __EMX__
#define TIOCCONS	108
#endif

static int Console;
#include <X11/Xmu/SysUtil.h>	/* XmuGetHostname */
#define MIT_CONSOLE_LEN	12
#define MIT_CONSOLE "MIT_CONSOLE_"
static char mit_console_name[255 + MIT_CONSOLE_LEN + 1] = MIT_CONSOLE;
static Atom mit_console;

static int tslot;
static jmp_buf env;

char *ProgramName;

static struct _resource {
    char *xterm_name;
    char *icon_geometry;
    char *title;
    char *icon_name;
    char *term_name;
    char *tty_modes;
    Boolean utmpInhibit;
    Boolean sunFunctionKeys;	/* %%% should be widget resource? */
    Boolean wait_for_map;
    Boolean useInsertMode;
} resource;

/* used by VT (charproc.c) */

#define offset(field)	XtOffsetOf(struct _resource, field)

static XtResource application_resources[] = {
    {"name", "Name", XtRString, sizeof(char *),
	offset(xterm_name), XtRString, "xterm"},
    {"iconGeometry", "IconGeometry", XtRString, sizeof(char *),
	offset(icon_geometry), XtRString, (caddr_t) NULL},
    {XtNtitle, XtCTitle, XtRString, sizeof(char *),
	offset(title), XtRString, (caddr_t) NULL},
    {XtNiconName, XtCIconName, XtRString, sizeof(char *),
	offset(icon_name), XtRString, (caddr_t) NULL},
    {"termName", "TermName", XtRString, sizeof(char *),
	offset(term_name), XtRString, (caddr_t) NULL},
    {"ttyModes", "TtyModes", XtRString, sizeof(char *),
	offset(tty_modes), XtRString, (caddr_t) NULL},
    {"utmpInhibit", "UtmpInhibit", XtRBoolean, sizeof (Boolean),
	offset(utmpInhibit), XtRString, "false"},
    {"sunFunctionKeys", "SunFunctionKeys", XtRBoolean, sizeof (Boolean),
	offset(sunFunctionKeys), XtRString, "false"},
    {"waitForMap", "WaitForMap", XtRBoolean, sizeof (Boolean),
        offset(wait_for_map), XtRString, "false"},
    {"useInsertMode", "UseInsertMode", XtRBoolean, sizeof (Boolean),
        offset(useInsertMode), XtRString, "false"},
};
#undef offset

static char *fallback_resources[] = {
    "XTerm*SimpleMenu*menuLabel.vertSpace: 100",
    "XTerm*SimpleMenu*HorizontalMargins: 16",
    "XTerm*SimpleMenu*Sme.height: 16",
    "XTerm*SimpleMenu*Cursor: left_ptr",
    "XTerm*mainMenu.Label:  Main Options (no app-defaults)",
    "XTerm*vtMenu.Label:  VT Options (no app-defaults)",
    "XTerm*fontMenu.Label:  VT Fonts (no app-defaults)",
    "XTerm*tekMenu.Label:  Tek Options (no app-defaults)",
    NULL
};

/* Command line options table.  Only resources are entered here...there is a
   pass over the remaining options after XrmParseCommand is let loose. */

static XrmOptionDescRec optionDescList[] = {
{"-geometry",	"*vt100.geometry",XrmoptionSepArg,	(caddr_t) NULL},
{"-132",	"*c132",	XrmoptionNoArg,		(caddr_t) "on"},
{"+132",	"*c132",	XrmoptionNoArg,		(caddr_t) "off"},
{"-ah",		"*alwaysHighlight", XrmoptionNoArg,	(caddr_t) "on"},
{"+ah",		"*alwaysHighlight", XrmoptionNoArg,	(caddr_t) "off"},
{"-aw",		"*autoWrap",	XrmoptionNoArg,		(caddr_t) "on"},
{"+aw",		"*autoWrap",	XrmoptionNoArg,		(caddr_t) "off"},
#ifndef NO_ACTIVE_ICON
{"-ai",		"*activeIcon",	XrmoptionNoArg,		(caddr_t) "off"},
{"+ai",		"*activeIcon",	XrmoptionNoArg,		(caddr_t) "on"},
#endif /* NO_ACTIVE_ICON */
{"-b",		"*internalBorder",XrmoptionSepArg,	(caddr_t) NULL},
{"-bdc",	"*colorBDMode",	XrmoptionNoArg,		(caddr_t) "off"},
{"+bdc",	"*colorBDMode",	XrmoptionNoArg,		(caddr_t) "on"},
{"-cb",		"*cutToBeginningOfLine", XrmoptionNoArg, (caddr_t) "off"},
{"+cb",		"*cutToBeginningOfLine", XrmoptionNoArg, (caddr_t) "on"},
{"-cc",		"*charClass",	XrmoptionSepArg,	(caddr_t) NULL},
{"-cm",		"*colorMode",	XrmoptionNoArg,		(caddr_t) "off"},
{"+cm",		"*colorMode",	XrmoptionNoArg,		(caddr_t) "on"},
{"-cn",		"*cutNewline",	XrmoptionNoArg,		(caddr_t) "off"},
{"+cn",		"*cutNewline",	XrmoptionNoArg,		(caddr_t) "on"},
{"-cr",		"*cursorColor",	XrmoptionSepArg,	(caddr_t) NULL},
{"-cu",		"*curses",	XrmoptionNoArg,		(caddr_t) "on"},
{"+cu",		"*curses",	XrmoptionNoArg,		(caddr_t) "off"},
{"-dc",		"*dynamicColors",XrmoptionNoArg,	(caddr_t) "off"},
{"+dc",		"*dynamicColors",XrmoptionNoArg,	(caddr_t) "on"},
{"-e",		NULL,		XrmoptionSkipLine,	(caddr_t) NULL},
{"-fb",		"*boldFont",	XrmoptionSepArg,	(caddr_t) NULL},
#ifndef NO_ACTIVE_ICON
{"-fi",		"*iconFont",	XrmoptionSepArg,	(caddr_t) NULL},
#endif /* NO_ACTIVE_ICON */
{"-j",		"*jumpScroll",	XrmoptionNoArg,		(caddr_t) "on"},
{"+j",		"*jumpScroll",	XrmoptionNoArg,		(caddr_t) "off"},
/* parse logging options anyway for compatibility */
{"-l",		"*logging",	XrmoptionNoArg,		(caddr_t) "on"},
{"+l",		"*logging",	XrmoptionNoArg,		(caddr_t) "off"},
{"-lf",		"*logFile",	XrmoptionSepArg,	(caddr_t) NULL},
{"-ls",		"*loginShell",	XrmoptionNoArg,		(caddr_t) "on"},
{"+ls",		"*loginShell",	XrmoptionNoArg,		(caddr_t) "off"},
{"-mb",		"*marginBell",	XrmoptionNoArg,		(caddr_t) "on"},
{"+mb",		"*marginBell",	XrmoptionNoArg,		(caddr_t) "off"},
{"-mc",		"*multiClickTime", XrmoptionSepArg,	(caddr_t) NULL},
{"-ms",		"*pointerColor",XrmoptionSepArg,	(caddr_t) NULL},
{"-nb",		"*nMarginBell",	XrmoptionSepArg,	(caddr_t) NULL},
{"-nul",	"*underLine",	XrmoptionNoArg,		(caddr_t) "off"},
{"+nul",	"*underLine",	XrmoptionNoArg,		(caddr_t) "on"},
{"-rw",		"*reverseWrap",	XrmoptionNoArg,		(caddr_t) "on"},
{"+rw",		"*reverseWrap",	XrmoptionNoArg,		(caddr_t) "off"},
{"-aw",		"*autoWrap",	XrmoptionNoArg,		(caddr_t) "on"},
{"+aw",		"*autoWrap",	XrmoptionNoArg,		(caddr_t) "off"},
{"-s",		"*multiScroll",	XrmoptionNoArg,		(caddr_t) "on"},
{"+s",		"*multiScroll",	XrmoptionNoArg,		(caddr_t) "off"},
{"-sb",		"*scrollBar",	XrmoptionNoArg,		(caddr_t) "on"},
{"+sb",		"*scrollBar",	XrmoptionNoArg,		(caddr_t) "off"},
{"-sf",		"*sunFunctionKeys", XrmoptionNoArg,	(caddr_t) "on"},
{"+sf",		"*sunFunctionKeys", XrmoptionNoArg,	(caddr_t) "off"},
{"-si",		"*scrollTtyOutput",	XrmoptionNoArg,		(caddr_t) "off"},
{"+si",		"*scrollTtyOutput",	XrmoptionNoArg,		(caddr_t) "on"},
{"-sk",		"*scrollKey",	XrmoptionNoArg,		(caddr_t) "on"},
{"+sk",		"*scrollKey",	XrmoptionNoArg,		(caddr_t) "off"},
{"-sl",		"*saveLines",	XrmoptionSepArg,	(caddr_t) NULL},
{"-t",		"*tekStartup",	XrmoptionNoArg,		(caddr_t) "on"},
{"+t",		"*tekStartup",	XrmoptionNoArg,		(caddr_t) "off"},
{"-tm",		"*ttyModes",	XrmoptionSepArg,	(caddr_t) NULL},
{"-tn",		"*termName",	XrmoptionSepArg,	(caddr_t) NULL},
{"-ulc",	"*colorULMode",	XrmoptionNoArg,		(caddr_t) "off"},
{"+ulc",	"*colorULMode",	XrmoptionNoArg,		(caddr_t) "on"},
{"-ut",		"*utmpInhibit",	XrmoptionNoArg,		(caddr_t) "on"},
{"+ut",		"*utmpInhibit",	XrmoptionNoArg,		(caddr_t) "off"},
{"-im",		"*useInsertMode", XrmoptionNoArg,	(caddr_t) "on"},
{"+im",		"*useInsertMode", XrmoptionNoArg,	(caddr_t) "off"},
{"-vb",		"*visualBell",	XrmoptionNoArg,		(caddr_t) "on"},
{"+vb",		"*visualBell",	XrmoptionNoArg,		(caddr_t) "off"},
{"-wf",		"*waitForMap",	XrmoptionNoArg,		(caddr_t) "on"},
{"+wf",		"*waitForMap",	XrmoptionNoArg,		(caddr_t) "off"},
/* bogus old compatibility stuff for which there are
   standard XtAppInitialize options now */
{"%",		"*tekGeometry",	XrmoptionStickyArg,	(caddr_t) NULL},
{"#",		".iconGeometry",XrmoptionStickyArg,	(caddr_t) NULL},
{"-T",		"*title",	XrmoptionSepArg,	(caddr_t) NULL},
{"-n",		"*iconName",	XrmoptionSepArg,	(caddr_t) NULL},
{"-r",		"*reverseVideo",XrmoptionNoArg,		(caddr_t) "on"},
{"+r",		"*reverseVideo",XrmoptionNoArg,		(caddr_t) "off"},
{"-rv",		"*reverseVideo",XrmoptionNoArg,		(caddr_t) "on"},
{"+rv",		"*reverseVideo",XrmoptionNoArg,		(caddr_t) "off"},
{"-w",		".borderWidth", XrmoptionSepArg,	(caddr_t) NULL},
};

static struct _options {
  char *opt;
  char *desc;
} options[] = {
{ "-help",                 "print out this message" },
{ "-display displayname",  "X server to contact" },
{ "-geometry geom",        "size (in characters) and position" },
{ "-/+rv",                 "turn on/off reverse video" },
{ "-bg color",             "background color" },
{ "-fg color",             "foreground color" },
{ "-bd color",             "border color" },
{ "-bw number",            "border width in pixels" },
{ "-fn fontname",          "normal text font" },
{ "-iconic",               "start iconic" },
{ "-name string",          "client instance, icon, and title strings" },
{ "-title string",         "title string" },
{ "-xrm resourcestring",   "additional resource specifications" },
{ "-/+132",                "turn on/off column switch inhibiting" },
{ "-/+ah",                 "turn on/off always highlight" },
#ifndef NO_ACTIVE_ICON
{ "-/+ai",		   "turn on/off active icon" },
{ "-fi fontname",	   "icon font for active icon" },
#endif /* NO_ACTIVE_ICON */
{ "-b number",             "internal border in pixels" },
{ "-/+bdc",                "turn off/on display of bold as color"},
{ "-/+cb",                 "turn on/off cut-to-beginning-of-line inhibit" },
{ "-cc classrange",        "specify additional character classes" },
{ "-/+cm",                 "turn off/on ANSI color mode" },
{ "-/+cn",                 "turn on/off cut newline inhibit" },
{ "-cr color",             "text cursor color" },
{ "-/+cu",                 "turn on/off curses emulation" },
{ "-/+dc",		   "turn off/on dynamic color selection" },
{ "-fb fontname",          "bold text font" },
{ "-/+im",		   "use insert mode for TERMCAP" },
{ "-/+j",                  "turn on/off jump scroll" },
#ifdef ALLOWLOGGING
{ "-/+l",                  "turn on/off logging" },
{ "-lf filename",          "logging filename" },
#else
{ "-/+l",                  "turn on/off logging (not supported)" },
{ "-lf filename",          "logging filename (not supported)" },
#endif
{ "-/+ls",                 "turn on/off login shell" },
{ "-/+mb",                 "turn on/off margin bell" },
{ "-mc milliseconds",      "multiclick time in milliseconds" },
{ "-ms color",             "pointer color" },
{ "-nb number",            "margin bell in characters from right end" },
{ "-/+nul",                "turn on/off display of underlining" },
{ "-/+aw",                 "turn on/off auto wraparound" },
{ "-/+rw",                 "turn on/off reverse wraparound" },
{ "-/+s",                  "turn on/off multiscroll" },
{ "-/+sb",                 "turn on/off scrollbar" },
{ "-/+sf",                 "turn on/off Sun Function Key escape codes" },
{ "-/+si",                 "turn on/off scroll-on-tty-output inhibit" },
{ "-/+sk",                 "turn on/off scroll-on-keypress" },
{ "-sl number",            "number of scrolled lines to save" },
{ "-/+t",                  "turn on/off Tek emulation window" },
{ "-tm string",            "terminal mode keywords and characters" },
{ "-tn name",              "TERM environment variable name" },
{ "-/+ulc",                "turn off/on display of underline as color" },
{ "-/+ut",                 "turn on/off utmp inhibit (not supported)" },
{ "-/+vb",                 "turn on/off visual bell" },
{ "-/+wf",                 "turn on/off wait for map before command exec" },
{ "-e command args ...",   "command to execute" },
{ "%geom",                 "Tek window geometry" },
{ "#geom",                 "icon window geometry" },
{ "-T string",             "title name for window" },
{ "-n string",             "icon name for window" },
{ "-C",                    "intercept console messages" },
{ "-Sxxd",                 "slave mode on \"ttyxx\", file descriptor \"d\"" },
{ NULL, NULL }};

/*debug FILE *confd;*/
/*static void opencons()
{
        if ((confd=fopen("/dev/console$","w")) < 0) {
                fputs("!!! Cannot open console device.\n",
                        stderr);
                exit(1);
        }
}

static void closecons()
{
	fclose(confd);
}
*/
static char *message[] = {
"Fonts must be fixed width and, if both normal and bold are specified, must",
"have the same size.  If only a normal font is specified, it will be used for",
"both normal and bold text (by doing overstriking).  The -e option, if given,",
"must be appear at the end of the command line, otherwise the user's default",
"shell will be started.  Options that start with a plus sign (+) restore the",
"default.",
NULL};

static void Syntax (badOption)
    char *badOption;
{
    struct _options *opt;
    int col;

    fprintf (stderr, "%s:  bad command line option \"%s\"\r\n\n",
	     ProgramName, badOption);

    fprintf (stderr, "usage:  %s", ProgramName);
    col = 8 + strlen(ProgramName);
    for (opt = options; opt->opt; opt++) {
	int len = 3 + strlen(opt->opt);	 /* space [ string ] */
	if (col + len > 79) {
	    fprintf (stderr, "\r\n   ");  /* 3 spaces */
	    col = 3;
	}
	fprintf (stderr, " [%s]", opt->opt);
	col += len;
    }

    fprintf (stderr, "\r\n\nType %s -help for a full description.\r\n\n",
	     ProgramName);
    exit (1);
}

static void Help ()
{
    struct _options *opt;
    char **cpp;

    fprintf (stderr, "usage:\n        %s [-options ...] [-e command args]\n\n",
	     ProgramName);
    fprintf (stderr, "where options include:\n");
    for (opt = options; opt->opt; opt++) {
	fprintf (stderr, "    %-28s %s\n", opt->opt, opt->desc);
    }

    putc ('\n', stderr);
    for (cpp = message; *cpp; cpp++) {
	fputs (*cpp, stderr);
	putc ('\n', stderr);
    }
    putc ('\n', stderr);

    exit (0);
}

/* ARGSUSED */
static Boolean
ConvertConsoleSelection(w, selection, target, type, value, length, format)
    Widget w;
    Atom *selection, *target, *type;
    XtPointer *value;
    unsigned long *length;
    int *format;
{
    /* we don't save console output, so can't offer it */
    return False;
}


extern WidgetClass xtermWidgetClass;

Arg ourTopLevelShellArgs[] = {
	{ XtNallowShellResize, (XtArgVal) TRUE },	
	{ XtNinput, (XtArgVal) TRUE },
};
int number_ourTopLevelShellArgs = 2;
	
XtAppContext app_con;
Widget toplevel;
Bool waiting_for_initial_map;

extern void do_hangup();
extern void xt_error();

/*
 * DeleteWindow(): Action proc to implement ICCCM delete_window.
 */
/* ARGSUSED */
void
DeleteWindow(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
  if (w == toplevel)
    if (term->screen.Tshow)
      hide_vt_window();
    else
      do_hangup(w);
  else
    if (term->screen.Vshow)
      hide_tek_window();
    else
      do_hangup(w);
}

/* ARGSUSED */
void
KeyboardMapping(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    switch (event->type) {
       case MappingNotify:
	  XRefreshKeyboardMapping(&event->xmapping);
	  break;
    }
}

XtActionsRec actionProcs[] = {
    "DeleteWindow", DeleteWindow,
    "KeyboardMapping", KeyboardMapping,
};

Atom wm_delete_window;
extern fd_set Select_mask;
extern fd_set X_mask;
extern fd_set pty_mask;

#ifdef __EMX__

#define XFREE86_PTY	0x76

#define XTY_TIOCSETA	0x48
#define XTY_TIOCSETAW	0x49
#define XTY_TIOCSETAF	0x4a
#define XTY_TIOCCONS	0x4d
#define XTY_TIOCSWINSZ	0x53
#define XTY_ENADUP	0x5a
#define XTY_TRACE	0x5b
#define XTY_TIOCGETA	0x65
#define XTY_TIOCGWINSZ	0x66
#define PTMS_GETPTY	0x64
#define PTMS_BUFSZ	14
#ifndef NCCS
#define NCCS 11
#endif

#define TIOCSWINSZ	113
#define TIOCGWINSZ	117

struct pt_termios
{
        unsigned short  c_iflag;
        unsigned short  c_oflag;
        unsigned short  c_cflag;
        unsigned short  c_lflag;
        unsigned char   c_cc[NCCS];
        long            _reserved_[4];
};

struct winsize {
        unsigned short  ws_row;         /* rows, in characters */
        unsigned short  ws_col;         /* columns, in characters */
        unsigned short  ws_xpixel;      /* horizontal size, pixels */
        unsigned short  ws_ypixel;      /* vertical size, pixels */
};

int ptioctl(int fd, int func, void* data)
{
	APIRET rc;
	ULONG  len;
	struct pt_termios pt;
	struct termio *t;
	int i;

	switch (func) {
	case TCGETA:
		rc = DosDevIOCtl(fd,XFREE86_PTY, func,
		NULL, 0, NULL,
		(ULONG*)&pt, sizeof(struct pt_termios), &len);
		if (rc) return -1;
		t = (struct termio*)data;
		t->c_iflag = pt.c_iflag;
		t->c_oflag = pt.c_oflag;
		t->c_cflag = pt.c_cflag;
		t->c_lflag = pt.c_lflag;		
		for (i=0; i<NCC; i++)
			t->c_cc[i] = pt.c_cc[i];
		return 0;
	case TCSETA:
	case TCSETAW:
	case TCSETAF:
		t = (struct termio*)data;
		pt.c_iflag = t->c_iflag;
		pt.c_oflag = t->c_oflag;
		pt.c_cflag = t->c_cflag;
		pt.c_lflag = t->c_lflag;

		for (i=0; i<NCC; i++)
			pt.c_cc[i] = t->c_cc[i];
		if (func==TCSETA) 
			i = XTY_TIOCSETA;
		else if (func==TCSETAW) 
			i = XTY_TIOCSETAW;
		else
			i = XTY_TIOCSETAF;
		rc = DosDevIOCtl(fd,XFREE86_PTY, i,
			(ULONG*)&pt, sizeof(struct pt_termios), &len,
			NULL, 0, NULL);
		return (rc) ? -1 : 0;
	case TIOCCONS:
		return DosDevIOCtl(fd,XFREE86_PTY, XTY_TIOCCONS,
			(ULONG*)data, sizeof(ULONG), &len,
			NULL, 0, NULL);
	case TIOCSWINSZ:				
		return DosDevIOCtl(fd,XFREE86_PTY, XTY_TIOCSWINSZ,
			(ULONG*)data, sizeof(struct winsize), &len,
			NULL, 0, NULL);
	case TIOCGWINSZ:
		return DosDevIOCtl(fd,XFREE86_PTY, XTY_TIOCGWINSZ,
			NULL, 0, NULL,
			(ULONG*)data, sizeof(struct winsize), &len);
	case XTY_ENADUP:
		i = 1;
		return DosDevIOCtl(fd,XFREE86_PTY, XTY_ENADUP,
			(ULONG*)&i, sizeof(ULONG), &len,
			NULL, 0, NULL);
	case XTY_TRACE:
		i = 2;
		return DosDevIOCtl(fd,XFREE86_PTY, XTY_TRACE,
			(ULONG*)&i, sizeof(ULONG), &len,
			NULL, 0, NULL);
	case PTMS_GETPTY:
		i = 1;
		return DosDevIOCtl(fd,XFREE86_PTY, PTMS_GETPTY,
			(ULONG*)&i, sizeof(ULONG), &len,
			(UCHAR*)data, 14, &len);
	default:
		return -1;
	}
}
#endif /* __EMX__ */

char **gblenvp;

main (argc, argv, envp)
int argc;
char **argv;
char **envp;
{
	register TScreen *screen;
	int mode;
	char *base_name();
	int xerror(), xioerror();
	extern char **environ;

	/* XXX: for some obscure reason EMX seems to lose the value of 
	 * the environ variable, don't understand why, so save it recently
	 */
	gblenvp = envp;

#ifdef I18N
	setlocale(LC_ALL, NULL);
#endif

	ProgramName = argv[0];

/*debug	opencons();*/

	ttydev = (char *) malloc (PTMS_BUFSZ);
	ptydev = (char *) malloc (PTMS_BUFSZ);
	if (!ttydev || !ptydev) {
	    fprintf (stderr, 
	    	     "%s:  unable to allocate memory for ttydev or ptydev\n",
		     ProgramName);
	    exit (1);
	}
	strcpy (ttydev, TTYDEV);
	strcpy (ptydev, PTYDEV);


	/* Initialization is done here rather than above in order
	** to prevent any assumptions about the order of the contents
	** of the various terminal structures (which may change from
	** implementation to implementation).
	*/
	d_tio.c_iflag = ICRNL|IXON;
	d_tio.c_oflag = OPOST|ONLCR|TAB3;
    	d_tio.c_cflag = B9600|CS8|CREAD|PARENB|HUPCL;
    	d_tio.c_lflag = ISIG|ICANON|ECHO|ECHOE|ECHOK;
	d_tio.c_line = 0;
	d_tio.c_cc[VINTR] = 'C' & 0x3f;		/* '^C'	*/
	d_tio.c_cc[VERASE] = 0x7f;		/* DEL	*/
	d_tio.c_cc[VKILL] = 'U' & 0x3f;		/* '^U'	*/
	d_tio.c_cc[VQUIT] = CQUIT;		/* '^\'	*/
    	d_tio.c_cc[VEOF] = CEOF;		/* '^D'	*/
	d_tio.c_cc[VEOL] = CEOL;		/* '^@'	*/

	/* Init the Toolkit. */
	{
	    XtSetErrorHandler(xt_error);
	    toplevel = XtAppInitialize (&app_con, "XTerm", 
					optionDescList,
					XtNumber(optionDescList), 
					&argc, argv, fallback_resources,
					NULL, 0);

	    XtGetApplicationResources(toplevel, (XtPointer) &resource,
				      application_resources,
				      XtNumber(application_resources), NULL, 0);
	}

	waiting_for_initial_map = resource.wait_for_map;

	/*
	 * ICCCM delete_window.
	 */
	XtAppAddActions(app_con, actionProcs, XtNumber(actionProcs));

	/*
	 * fill in terminal modes
	 */
	if (resource.tty_modes) {
	    int n = parse_tty_modes (resource.tty_modes, ttymodelist);
	    if (n < 0) {
		fprintf (stderr, "%s:  bad tty modes \"%s\"\n",
			 ProgramName, resource.tty_modes);
	    } else if (n > 0) {
		override_tty_modes = 1;
	    }
	}

	xterm_name = resource.xterm_name;
	sunFunctionKeys = resource.sunFunctionKeys;
	if (strcmp(xterm_name, "-") == 0) xterm_name = "xterm";
	if (resource.icon_geometry != NULL) {
	    int scr, junk;
	    int ix, iy;
	    Arg args[2];

	    for(scr = 0;	/* yyuucchh */
		XtScreen(toplevel) != ScreenOfDisplay(XtDisplay(toplevel),scr);
		scr++);

	    args[0].name = XtNiconX;
	    args[1].name = XtNiconY;
	    XGeometry(XtDisplay(toplevel), scr, resource.icon_geometry, "",
		      0, 0, 0, 0, 0, &ix, &iy, &junk, &junk);
	    args[0].value = (XtArgVal) ix;
	    args[1].value = (XtArgVal) iy;
	    XtSetValues( toplevel, args, 2);
	}

	XtSetValues (toplevel, ourTopLevelShellArgs, 
		     number_ourTopLevelShellArgs);

	/* Parse the rest of the command line */
	for (argc--, argv++ ; argc > 0 ; argc--, argv++) {
	    if(**argv != '-') Syntax (*argv);

	    switch(argv[0][1]) {
	     case 'h':
		Help ();
		/* NOTREACHED */
	     case 'C':
		{
		    struct stat sbuf;

		    /* Must be owner and have read/write permission.
		       xdm cooperates to give the console the right user. */
		    if ( !stat("/dev/console", &sbuf) &&
			 (sbuf.st_uid == getuid()) &&
			 !access("/dev/console", R_OK|W_OK))
		    {
			Console = TRUE;
		    } else
			Console = FALSE;
		}
		continue;
	     case 'S':
		if (sscanf(*argv + 2, "%c%c%d", passedPty, passedPty+1,
			   &am_slave) != 3)
		    Syntax(*argv);
		continue;
#ifdef DEBUG
	     case 'D':
		debug = TRUE;
		continue;
#endif	/* DEBUG */
	     case 'e':
		if (argc <= 1) Syntax (*argv);
		command_to_exec = ++argv;
		break;
	     default:
		Syntax (*argv);
	    }
	    break;
	}

	XawSimpleMenuAddGlobalActions (app_con);
	XtRegisterGrabAction (HandlePopupMenu, True,
			      (ButtonPressMask|ButtonReleaseMask),
			      GrabModeAsync, GrabModeAsync);

        term = (XtermWidget) XtCreateManagedWidget(
	    "vt100", xtermWidgetClass, toplevel, NULL, 0);

	/* this causes the initialize method to be called */
        screen = &term->screen;
	if (screen->savelines < 0) screen->savelines = 0;

	term->flags = 0;
	if (!screen->jumpscroll) {
	    term->flags |= SMOOTHSCROLL;
	    update_jumpscroll();
	}
	if (term->misc.reverseWrap) {
	    term->flags |= REVERSEWRAP;
	    update_reversewrap();
	}
	if (term->misc.autoWrap) {
	    term->flags |= WRAPAROUND;
	    update_autowrap();
	}
	if (term->misc.re_verse) {
	    term->flags |= REVERSE_VIDEO;
	    update_reversevideo();
	}

	inhibit = 0;
#ifdef ALLOWLOGGING
	if (term->misc.logInhibit) 	    inhibit |= I_LOG;
#endif
	if (term->misc.signalInhibit)		inhibit |= I_SIGNAL;
	if (term->misc.tekInhibit)			inhibit |= I_TEK;

	term->initflags = term->flags;

	if (term->misc.appcursorDefault) {
	    term->keyboard.flags |= MODE_DECCKM;
	    update_appcursor();
	}

	if (term->misc.appkeypadDefault) {
	    term->keyboard.flags |= MODE_DECKPAM;
	    update_appkeypad();
	}

/*
 * Set title and icon name if not specified
 */

	if (command_to_exec) {
	    Arg args[2];

	    if (!resource.title) {
		if (command_to_exec) {
		    resource.title = base_name (command_to_exec[0]);
		} /* else not reached */
	    }

	    if (!resource.icon_name) 
	      resource.icon_name = resource.title;
	    XtSetArg (args[0], XtNtitle, resource.title);
	    XtSetArg (args[1], XtNiconName, resource.icon_name);		

	    XtSetValues (toplevel, args, 2);
	}

	if(inhibit & I_TEK)
		screen->TekEmu = FALSE;

	if(screen->TekEmu && !TekInit())
		exit(ERROR_INIT);

#ifdef DEBUG
    {
	/* Set up stderr properly.  Opening this log file cannot be
	 done securely by a privileged xterm process (although we try),
	 so the debug feature is disabled by default. */
	int i = -1;
	if(debug) {
	        creat_as (getuid(), getgid(), "xterm.debug.log", 0666);
		i = open ("xterm.debug.log", O_WRONLY | O_TRUNC, 0666);
	}
	if(i >= 0) {
		dup2(i,2);

		/* mark this file as close on exec */
		fcntl(i, F_SETFD, 1);
	}
    }
#endif	/* DEBUG */

	/* open a terminal for client */
	get_terminal ();

	spawn ();

	/* Child process is out there, let's catch its termination */
	signal (SIGCHLD, reapchild);

	/* Realize procs have now been executed */

	if (am_slave) { /* Write window id so master end can read and use */
	    char buf[80];

	    buf[0] = '\0';
	    sprintf (buf, "%lx\n", 
	    	     screen->TekEmu ? XtWindow (XtParent (tekWidget)) :
				      XtWindow (XtParent (term)));
	    write (screen->respond, buf, strlen (buf));
	}

#ifdef ALLOWLOGGING
	if (term->misc.log_on) {
		StartLog(screen);
	}
#endif
	screen->inhibit = inhibit;

	if (0 > (mode = fcntl(screen->respond, F_GETFL, 0)))
		Error();
	mode |= O_NDELAY;

	if (fcntl(screen->respond, F_SETFL, mode))
		Error();

	FD_ZERO (&pty_mask);
	FD_ZERO (&X_mask);
	FD_ZERO (&Select_mask);
	FD_SET (screen->respond, &pty_mask);
	FD_SET (ConnectionNumber(screen->display), &X_mask);
	FD_SET (screen->respond, &Select_mask);
	FD_SET (ConnectionNumber(screen->display), &Select_mask);
	max_plus1 = (screen->respond < ConnectionNumber(screen->display)) ? 
		(1 + ConnectionNumber(screen->display)) : 
		(1 + screen->respond);
	
#ifdef DEBUG
	if (debug) printf ("debugging on\n");
#endif	/* DEBUG */
	XSetErrorHandler(xerror);
	XSetIOErrorHandler(xioerror);

	for( ; ; ) {
		if(screen->TekEmu) {
			TekRun();
		} else {
			VTRun();
		}
	}
}

char *base_name(name)
char *name;
{
	register char *cp;

	cp = strrchr(name, '/');
	return(cp ? cp + 1 : name);
}

/* This function opens up a pty master and stuffs its value into pty.
 * If it finds one, it returns a value of 0.  If it does not find one,
 * it returns a value of !0.  This routine is designed to be re-entrant,
 * so that if a pty master is found and later, we find that the slave
 * has problems, we can re-enter this function and get another one.
 */

get_pty (pty)
    int *pty;
{
	return pty_search(pty);
}

/*
 * Called from get_pty to iterate over likely pseudo terminals
 * we might allocate.  Used on those systems that do not have
 * a functional interface for allocating a pty.
 * Returns 0 if found a pty, 1 if fails.
 */
int pty_search(pty)
    int *pty;
{
	char namebuf[PTMS_BUFSZ];

	/* ask the PTY manager */
	int fd = open("/dev/ptms$",0);
	if (fd && ptioctl(fd,PTMS_GETPTY,namebuf)==0) {
		strcpy(ttydev,namebuf);
		strcpy(ptydev,namebuf);
		ttydev[5] = 't';
		close (fd);
		if ((*pty = open(ptydev, O_RDWR)) >= 0) {
#ifdef PTYDEBUG
			ptioctl(*pty,XTY_TRACE,0);
#endif
			return 0;
		}
	} 
	return 1;
}

get_terminal ()
/* 
 * sets up X and initializes the terminal structure except for term.buf.fildes.
 */
{
	register TScreen *screen = &term->screen;
	
	screen->arrow = make_colored_cursor (XC_left_ptr, 
					     screen->mousecolor,
					     screen->mousecolorback);
}

/*
 * The only difference in /etc/termcap between 4014 and 4015 is that 
 * the latter has support for switching character sets.  We support the
 * 4015 protocol, but ignore the character switches.  Therefore, we 
 * choose 4014 over 4015.
 *
 * Features of the 4014 over the 4012: larger (19") screen, 12-bit
 * graphics addressing (compatible with 4012 10-bit addressing),
 * special point plot mode, incremental plot mode (not implemented in
 * later Tektronix terminals), and 4 character sizes.
 * All of these are supported by xterm.
 */

static char *tekterm[] = {
	"tek4014",
	"tek4015",		/* 4014 with APL character set support */
	"tek4012",		/* 4010 with lower case */
	"tek4013",		/* 4012 with APL character set support */
	"tek4010",		/* small screen, upper-case only */
	"dumb",
	0
};

/* The VT102 is a VT100 with the Advanced Video Option included standard.
 * It also adds Escape sequences for insert/delete character/line.
 * The VT220 adds 8-bit character sets, selective erase.
 * The VT320 adds a 25th status line, terminal state interrogation.
 * The VT420 has up to 48 lines on the screen.
 */

static char *vtterm[] = {
#ifdef USE_X11TERM
	"x11term",		/* for people who want special term name */
#endif
	"xterm",		/* the prefered name, should be fastest */
	"vt102",
	"vt100",
	"ansi",
	"dumb",
	0
};

/* ARGSUSED */
SIGNAL_T hungtty(i)
	int i;
{
	longjmp(env, 1);
	SIGNAL_RETURN;
}

struct {
	int rows;
	int cols;
} handshake = {-1,-1};

void first_map_occurred ()
{
    register TScreen *screen = &term->screen;
    handshake.rows = screen->max_row;
    handshake.cols = screen->max_col;
    waiting_for_initial_map = False;
}

#define THE_PARENT 1
#define THE_CHILD  2
int whoami = -1;

SIGNAL_T killit(int sig)
{
	switch (whoami) {
	case -1:
		signal(sig,killit);
		kill(-getpid(),sig);
		break;
	case THE_PARENT:
		wait(NULL);
		signal(SIGTERM,SIG_DFL);
		kill(-getpid(),SIGTERM);
		Exit(0);
		break;
	case THE_CHILD:
		signal(SIGTERM,SIG_DFL);
		kill(-getppid(),SIGTERM);
		Exit(0);
		break;
	}

	SIGNAL_RETURN;
}

spawn ()
/* 
 *  Inits pty and tty and forks a login process.
 *  Does not close fd Xsocket.
 *  If slave, the pty named in passedPty is already open for use
 */
{
	extern char *SysErrorMsg();

	register TScreen *screen = &term->screen;
	int Xsocket = ConnectionNumber(screen->display);

	int tty = -1;
	struct termio tio;
	int status;

	char termcap[1024], newtc[1024];
	char *TermName = NULL;
	char *ptr, *shname, buf[64];
	int i, no_dev_tty = FALSE, envsize;
	char *dev_tty_name = (char *) 0;
	struct winsize ws;
	int pgrp = getpid();
	char numbuf[12], **envnew;

	screen->uid = getuid();
	screen->gid = getgid();

	if (am_slave) {
		screen->respond = am_slave;
		ptydev[strlen(ptydev) - 2] = ttydev[strlen(ttydev) - 2] =
			passedPty[0];
		ptydev[strlen(ptydev) - 1] = ttydev[strlen(ttydev) - 1] =
			passedPty[1];

		setgid (screen->gid);
		setuid (screen->uid);
	} else {
		Bool tty_got_hung = False;

 		/*
 		 * Sometimes /dev/tty hangs on open (as in the case of a pty
 		 * that has gone away).  Simply make up some reasonable
 		 * defaults.
 		 */

 		signal(SIGALRM, hungtty);
 		alarm(2);		/* alarm(1) might return too soon */
 		if (! setjmp(env)) {
 			tty = open ("/dev/tty", O_RDWR, 0);
 			alarm(0);
 		} else {
			tty_got_hung = True;
 			tty = -1;
 			errno = ENXIO;
 		}
 		signal(SIGALRM, SIG_DFL);
 
		/*
		 * Check results and ignore current control terminal if
		 * necessary.  ENXIO is what is normally returned if there is
		 * no controlling terminal, but some systems (e.g. SunOS 4.0)
		 * seem to return EIO.  Solaris 2.3 is said to return EINVAL.
		 */
 		if (tty < 0) {
			if (tty_got_hung || errno == ENXIO || errno == EIO ||
			    errno == EINVAL || errno == ENOTTY) {
				no_dev_tty = TRUE;
				tio = d_tio;
			} else {
			    SysError(ERROR_OPDEVTTY);
			}
		} else {
			/* Get a copy of the current terminal's state,
			 * if we can.  Some systems (e.g., SVR4 and MacII)
			 * may not have a controlling terminal at this point
			 * if started directly from xdm or xinit,     
			 * in which case we just use the defaults as above.
			 */
/**/		        if(ioctl(tty, TCGETA, &tio) == -1)
			        tio = d_tio;

			close (tty);
			/* tty is no longer an open fd! */
			tty = -1;
		}

		if (get_pty (&screen->respond)) {
			/*  no ptys! */
			exit (ERROR_PTYS);
		}
	}

	/* avoid double MapWindow requests */
	XtSetMappedWhenManaged( screen->TekEmu ? XtParent(tekWidget) :
			        XtParent(term), False );

	wm_delete_window = XInternAtom(XtDisplay(toplevel), "WM_DELETE_WINDOW",
				       False);

	if (!screen->TekEmu)
	    VTInit();		/* realize now so know window size for tty driver */

	if (Console) {
	    /*
	     * Inform any running xconsole program
	     * that we are going to steal the console.
	     */
	    XmuGetHostname (mit_console_name + MIT_CONSOLE_LEN, 255);
	    mit_console = XInternAtom(screen->display, mit_console_name, False);
	    /* the user told us to be the console, so we can use CurrentTime */
	    XtOwnSelection(screen->TekEmu ? XtParent(tekWidget) : XtParent(term),
			   mit_console, CurrentTime,
			   ConvertConsoleSelection, NULL, NULL);
	}
	if(screen->TekEmu) {
		envnew = tekterm;
		ptr = newtc;
	} else {
		envnew = vtterm;
		ptr = termcap;
	}
	TermName = NULL;
	if (resource.term_name) {
	    if (tgetent (ptr, resource.term_name) == 1) {
		TermName = resource.term_name;
		if (!screen->TekEmu)
		    resize (screen, TermName, termcap, newtc);
	    } else {
		fprintf (stderr, "%s:  invalid termcap entry \"%s\".\n",
			 ProgramName, resource.term_name);
	    }
	}

	if (!TermName) {
	    while (*envnew != NULL) {
		if(tgetent(ptr, *envnew) == 1) {
			TermName = *envnew;
			if(!screen->TekEmu)
			    resize(screen, TermName, termcap, newtc);
			break;
		}
		envnew++;
	    }
	    if (TermName == NULL) {
		fprintf (stderr, "%s:  unable to find usable termcap entry.\n",
			 ProgramName);
		Exit (1);
	    }
	}

	/* tell tty how big window is */
	if(screen->TekEmu) {
		ws.ws_row = 38;
		ws.ws_col = 81;
		ws.ws_xpixel = TFullWidth(screen);
		ws.ws_ypixel = TFullHeight(screen);
	} else {
		ws.ws_row = screen->max_row + 1;
		ws.ws_col = screen->max_col + 1;
		ws.ws_xpixel = FullWidth(screen);
		ws.ws_ypixel = FullHeight(screen);
	}

	if (!am_slave) {

		char sema[40];
		HEV sev;
		/* start a child process
		 * use an event sema for sync 
		 */
		sprintf(sema,"\\SEM32\\xterm%s",&ptydev[8]);
		if (DosCreateEventSem(sema,&sev,DC_SEM_SHARED,FALSE))
			SysError(ERROR_FORK);

		switch ((screen->pid = fork())) {
		case -1:	/* error */
			SysError (ERROR_FORK);
		default:	/* parent */
			whoami = THE_PARENT;
			DosWaitEventSem(sev,1000l);
			DosCloseEventSem(sev);
			break;
		case 0:		/* child */
			whoami = THE_CHILD;

/*debug fclose(confd);
opencons();*/
			/* we don't need the socket, or the pty master anymore */
			close (ConnectionNumber(screen->display));
			close (screen->respond);

			/* Now is the time to set up our process group and
			 * open up the pty slave.
			 */
			if ((tty = open(ttydev, O_RDWR, 0)) < 0) {
				/* dumm gelaufen */
				fprintf(stderr, "Cannot open slave side of PTY\n");
		        	exit(1);
			}

			/* use the same tty name that everyone else will use
			 * (from ttyname)
			 */
#ifdef EMXNOTBOGUS
			if (ptr = ttyname(tty)) {
				/* it may be bigger */
				ttydev = realloc (ttydev, 
					(unsigned) (strlen(ptr) + 1));
				strcpy(ttydev, ptr);
			}
#else
			ptr = ttydev;
#endif
			/* for safety: enable DUPs */
			ptioctl(tty,XTY_ENADUP,0);

			/* change ownership of tty to real group and user id */
			chown (ttydev, screen->uid, screen->gid);

			/* change protection of tty */
			chmod (ttydev, 0622);

			/* for the xf86sup-pty, we set the pty to bypass: OS/2 does
			 * not have a line discipline structure 
			 */
			{
				struct termio t,t1;
				if (ptioctl(tty, TCGETA, (char*)&t) < 0)
					t = d_tio;
				
				t.c_iflag = ICRNL;
				t.c_oflag = OPOST|ONLCR;
				t.c_lflag = ISIG|ICANON|ECHO|ECHOE|ECHOK;

				/* ignore error code, user will see it :-) */
				ptioctl(tty, TCSETA, (char*)&t);

				/* set the console mode */
				if (Console) {
					int on = 1;
					if (ioctl (tty, TIOCCONS, (char *)&on) == -1)
					fprintf(stderr, "%s: cannot open console\n", xterm_name);
				}
			}

			signal (SIGCHLD, SIG_DFL);
			signal (SIGHUP, SIG_IGN);

			/* restore various signals to their defaults */
			signal (SIGINT, SIG_DFL);
			signal (SIGQUIT, SIG_DFL);
			signal (SIGTERM, SIG_DFL);

			/* copy the environment before Setenving */
			for (i = 0 ; gblenvp [i] != NULL ; i++) 
				;

			/* compute number of Setenv() calls below */
			envsize = 1;	/* (NULL terminating entry) */
			envsize += 3;	/* TERM, WINDOWID, DISPLAY */
			envsize += 2;	/* COLUMNS, LINES */

			envnew = (char **) calloc ((unsigned) i + envsize, sizeof(char *));
			memmove( (char *)envnew, (char *)gblenvp, i * sizeof(char *));
			gblenvp = envnew;
			Setenv ("TERM=", TermName);
			if(!TermName)
				*newtc = 0;

			sprintf (buf, "%lu", screen->TekEmu ? 
				((unsigned long) XtWindow (XtParent(tekWidget))) :
				((unsigned long) XtWindow (XtParent(term))));
			Setenv ("WINDOWID=", buf);

			/* put the display into the environment of the shell*/
			Setenv ("DISPLAY=", XDisplayString (screen->display));

			signal(SIGTERM, SIG_DFL);

			/* this is the time to go and set up stdin, 
			 * out, and err
			 */
			/* dup the tty */
			for (i = 0; i <= 2; i++)
				if (i != tty) {
					close(i);
					dup(tty);
				}

			/* and close the tty */
			if (tty > 2)
				close(tty);

			setpgrp (0, pgrp);
			setgid (screen->gid);
			setuid (screen->uid);

			if (handshake.rows > 0 && handshake.cols > 0) {
				screen->max_row = handshake.rows;
				screen->max_col = handshake.cols;
				ws.ws_row = screen->max_row + 1;
				ws.ws_col = screen->max_col + 1;
				ws.ws_xpixel = FullWidth(screen);
				ws.ws_ypixel = FullHeight(screen);
			}

			sprintf (numbuf, "%d", screen->max_col + 1);
			Setenv("COLUMNS=", numbuf);
			sprintf (numbuf, "%d", screen->max_row + 1);
			Setenv("LINES=", numbuf);

			/* reconstruct dead environ variable */
			environ = gblenvp;

			/* need to reset after all the ioctl bashing we did above */
			ptioctl (0, TIOCSWINSZ, (char *)&ws);

			signal(SIGHUP, SIG_DFL);

			/* okay everything seems right, so tell the parent, we are going */
			{
				char sema[40];
				HEV sev;
				sprintf(sema,"\\SEM32\\xterm%s",&ttydev[8]);
				DosOpenEventSem(sema,&sev);
				DosPostEventSem(sev);
				DosCloseEventSem(sev);
			}

			if (command_to_exec) {
				execvpe(*command_to_exec, command_to_exec,
					gblenvp);

				/* print error message on screen */
				fprintf(stderr, "%s: Can't execvp %s\n", 
					xterm_name, *command_to_exec);
			} 

			/* use a layered mechanism to find a shell */
			ptr = getenv("X11SHELL");
			if (!ptr) ptr = getenv("SHELL");
			if (!ptr) ptr = getenv("OS2_SHELL");
			if (!ptr) ptr = "SORRY_NO_SHELL_FOUND";

			if (shname = strrchr(ptr, '/'))
				shname++;
			else if (shname = strrchr(ptr,'\\'))
				shname++;
			else
				shname = ptr;

			if (command_to_exec) {
				char *exargv[10]; /*XXX*/

				exargv[0] = ptr;
				exargv[1] = "/C";
				exargv[2] = command_to_exec[0];
				exargv[3] = command_to_exec[1];
				exargv[4] = command_to_exec[2];
				exargv[5] = command_to_exec[3];
				exargv[6] = command_to_exec[4];
				exargv[7] = command_to_exec[5];
				exargv[8] = command_to_exec[6];
				exargv[9] = 0;
				execvpe(exargv[0],exargv,gblenvp);
/*
				execvpe(*command_to_exec, command_to_exec,
					gblenvp);
*/
				/* print error message on screen */
				fprintf(stderr, "%s: Can't execvp %s\n", 
					xterm_name, *command_to_exec);
			} else {
				execlpe (ptr, shname, 0, gblenvp);

				/* Exec failed. */
				fprintf (stderr, "%s: Could not exec %s!\n", 
					xterm_name, ptr);
			}
			sleep(5);

			/* preventively shoot the parent */
			kill (-getppid(),SIGTERM);

			exit(ERROR_EXEC);
		} /* endcase */
	} /* !am_slave */

	signal (SIGHUP, SIG_IGN);
/*
 * Unfortunately, System V seems to have trouble divorcing the child process
 * from the process group of xterm.  This is a problem because hitting the 
 * INTR or QUIT characters on the keyboard will cause xterm to go away if we
 * don't ignore the signals.  This is annoying.
 */

/*	signal (SIGINT, SIG_IGN);*/
signal(SIGINT, killit);
signal(SIGTERM, killit);

	/* hung shell problem */
	signal (SIGQUIT, SIG_IGN);
/*	signal (SIGTERM, SIG_IGN);*/
	return 0;
}							/* end spawn */

SIGNAL_T
Exit(n)
	int n;
{
	register TScreen *screen = &term->screen;
        int pty = term->screen.respond;  /* file descriptor of pty */
        close(pty); /* close explicitly to avoid race with slave side */
#ifdef ALLOWLOGGING
	if(screen->logging)
		CloseLog(screen);
#endif
	if (!am_slave) {
		/* restore ownership of tty and pty */
		chown (ttydev, 0, 0);
		chown (ptydev, 0, 0);

		/* restore modes of tty and pty */
		chmod (ttydev, 0666);
		chmod (ptydev, 0666);
	}
	exit(n);
	SIGNAL_RETURN;
}

/* ARGSUSED */
resize(screen, TermName, oldtc, newtc)
TScreen *screen;
char *TermName;
register char *oldtc, *newtc;
{
}

/*
 * Does a non-blocking wait for a child process.  If the system
 * doesn't support non-blocking wait, do nothing.
 * Returns the pid of the child, or 0 or -1 if none or error.
 */
int
nonblocking_wait()
{
        pid_t pid;

	pid = waitpid(-1, NULL, WNOHANG);
	return pid;
}

/* ARGSUSED */
static SIGNAL_T reapchild (n)
    int n;
{
    int pid;

    pid = wait(NULL);

    /* cannot re-enable signal before waiting for child
     * because then SVR4 loops.  Sigh.  HP-UX 9.01 too. 
     */
    signal(SIGCHLD, reapchild);

    do {
	if (pid == term->screen.pid) {
#ifdef DEBUG
	    if (debug) fputs ("Exiting\n", stderr);
#endif
	    Cleanup (0);
	}
    } while ( (pid=nonblocking_wait()) > 0);

    SIGNAL_RETURN;
}

/* VARARGS1 */
consolepr(fmt,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9)
char *fmt;
{
	extern char *SysErrorMsg();
	int oerrno;
	int f;
 	char buf[ BUFSIZ ];

	oerrno = errno;
 	strcpy(buf, "xterm: ");
 	sprintf(buf+strlen(buf), fmt, x0,x1,x2,x3,x4,x5,x6,x7,x8,x9);
 	strcat(buf, ": ");
 	strcat(buf, SysErrorMsg (oerrno));
 	strcat(buf, "\n");	

	f = open("/dev/console",O_WRONLY);
	write(f, buf, strlen(buf));
	close(f);
#ifdef TIOCNOTTY
	if ((f = open("/dev/tty", 2)) >= 0) {
		ioctl(f, TIOCNOTTY, (char *)NULL);
		close(f);
	}
#endif	/* TIOCNOTTY */
}


remove_termcap_entry (buf, str)
    char *buf;
    char *str;
{
    register char *strinbuf;

    strinbuf = strindex (buf, str);
    if (strinbuf) {
        register char *colonPtr = strchr(strinbuf+1, ':');
        if (colonPtr) {
            while (*colonPtr) {
                *strinbuf++ = *colonPtr++;      /* copy down */
            }
            *strinbuf = '\0';
        } else {
            strinbuf[1] = '\0';
        }
    }
    return 0;
}

/*
 * parse_tty_modes accepts lines of the following form:
 *
 *         [SETTING] ...
 *
 * where setting consists of the words in the modelist followed by a character
 * or ^char.
 */
static int parse_tty_modes (s, modelist)
    char *s;
    struct _xttymodes *modelist;
{
    struct _xttymodes *mp;
    int c;
    int count = 0;

    while (1) {
	while (*s && isascii(*s) && isspace(*s)) s++;
	if (!*s) return count;

	for (mp = modelist; mp->name; mp++) {
	    if (strncmp (s, mp->name, mp->len) == 0) break;
	}
	if (!mp->name) return -1;

	s += mp->len;
	while (*s && isascii(*s) && isspace(*s)) s++;
	if (!*s) return -1;

	if (*s == '^') {
	    s++;
	    c = ((*s == '?') ? 0177 : *s & 31);	 /* keep control bits */
	} else {
	    c = *s;
	}
	mp->value = c;
	mp->set = 1;
	count++;
	s++;
    }
}


int GetBytesAvailable (fd)
    int fd;
{
    static long arg;
    ioctl (fd, FIONREAD, (char *) &arg);
    return (int) arg;

}

/* Utility function to try to hide system differences from
   everybody who used to call killpg() */

int
kill_process_group(pid, sig)
    int pid;
    int sig;
{
    return kill (-pid, sig);
}
