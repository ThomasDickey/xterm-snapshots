/* removed all foreign stuff to get the code more clear (hv)
 * and did some rewrite for the obscure OS/2 environment
 */

#ifndef lint
static char *rid="$XConsortium: main.c,v 1.227.1.2 95/06/29 18:13:15 kaleb Exp $";
#endif /* lint */
/* $XFree86: xc/programs/xterm/os2main.c,v 3.41 2000/11/01 01:12:41 dawes Exp $ */

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

#include <version.h>
#include <xterm.h>

#include <X11/StringDefs.h>
#include <X11/Shell.h>

#include <X11/Xos.h>
#include <X11/cursorfont.h>
#ifdef I18N
#include <X11/Xlocale.h>
#endif

#if OPT_TOOLBAR
#include <X11/Xaw/Form.h>
#endif

#include <pwd.h>
#include <ctype.h>

#include <data.h>
#include <error.h>
#include <menu.h>
#include <main.h>
#include <xstrings.h>

#include <sys/termio.h>

int setpgrp(pid_t pid ,gid_t pgid) {}
int chown(const char* fn, pid_t pid, gid_t gid) {}
char *ttyname(int fd) { return "/dev/tty"; }

#include <sys/ioctl.h>
#include <sys/stat.h>

#include <sys/param.h>	/* for NOFILE */

#include <stdio.h>
#include <time.h>

#include <signal.h>

static SIGNAL_T reapchild (int n);
static int spawn (void);
static void get_terminal (void);
static void resize (TScreen *s, char *oldtc, char *newtc);

static Bool added_utmp_entry = False;

static char **command_to_exec;

/* The following structures are initialized in main() in order
** to eliminate any assumptions about the internal order of their
** contents.
*/
static struct termio d_tio;

/* allow use of system default characters if defined and reasonable */
#ifndef CEOF
#define CEOF     CONTROL('D')
#endif
#ifndef CSUSP
#define CSUSP    CONTROL('Z')
#endif
#ifndef CQUIT
#define CQUIT    CONTROL('\\')
#endif
#ifndef CEOL
#define CEOL 0
#endif
#ifndef CNUL
#define CNUL 0
#endif
#ifndef CSWTCH
#define CSWTCH 0
#endif
#ifndef CLNEXT
#define CLNEXT   CONTROL('V')
#endif
#ifndef CWERASE
#define CWERASE  CONTROL('W')
#endif
#ifndef CRPRNT
#define CRPRNT   CONTROL('R')
#endif
#ifndef CFLUSH
#define CFLUSH   CONTROL('O')
#endif
#ifndef CSTOP
#define CSTOP    CONTROL('S')
#endif
#ifndef CSTART
#define CSTART   CONTROL('Q')
#endif

/*
 * SYSV has the termio.c_cc[V] and ltchars; BSD has tchars and ltchars;
 * SVR4 has only termio.c_cc, but it includes everything from ltchars.
 */
static int override_tty_modes = 0;
struct _xttymodes {
    char *name;
    size_t len;
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

static int parse_tty_modes (char *s, struct _xttymodes *modelist);

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
    Boolean messages;
    Boolean sunFunctionKeys;	/* %%% should be widget resource? */
#if OPT_SUNPC_KBD
    Boolean sunKeyboard;
#endif
#if OPT_HP_FUNC_KEYS
    Boolean hpFunctionKeys;
#endif
    Boolean wait_for_map;
    Boolean useInsertMode;
#if OPT_ZICONBEEP
    int zIconBeep;		/* beep level when output while iconified */
#endif
#if OPT_SAME_NAME
    Boolean sameName;		/* Don't change the title or icon name if it is
				 * the same.  This prevents flicker on the
				 * screen at the cost of an extra request to
				 * the server.
				 */
#endif
} resource;

/* used by VT (charproc.c) */

#define offset(field)	XtOffsetOf(struct _resource, field)

static XtResource application_resources[] = {
    {"name", "Name", XtRString, sizeof(char *),
	offset(xterm_name), XtRString, DFT_TERMTYPE},
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
    {"messages", "Messages", XtRBoolean, sizeof (Boolean),
	offset(messages), XtRString, "true"},
    {"sunFunctionKeys", "SunFunctionKeys", XtRBoolean, sizeof (Boolean),
	offset(sunFunctionKeys), XtRString, "false"},
#if OPT_SUNPC_KBD
    {"sunKeyboard", "SunKeyboard", XtRBoolean, sizeof (Boolean),
	offset(sunKeyboard), XtRString, "false"},
#endif
#if OPT_HP_FUNC_KEYS
    {"hpFunctionKeys", "HpFunctionKeys", XtRBoolean, sizeof (Boolean),
	offset(hpFunctionKeys), XtRString, "false"},
#endif
    {"waitForMap", "WaitForMap", XtRBoolean, sizeof (Boolean),
        offset(wait_for_map), XtRString, "false"},
    {"useInsertMode", "UseInsertMode", XtRBoolean, sizeof (Boolean),
        offset(useInsertMode), XtRString, "false"},
#if OPT_ZICONBEEP
    {"zIconBeep", "ZIconBeep", XtRInt, sizeof (int),
	offset(zIconBeep), XtRImmediate, 0},
#endif
#if OPT_SAME_NAME
    {"sameName", "SameName", XtRBoolean, sizeof (Boolean),
	offset(sameName), XtRString, "true"},
#endif
};
#undef offset

static char *fallback_resources[] = {
    "*SimpleMenu*menuLabel.vertSpace: 100",
    "*SimpleMenu*HorizontalMargins: 16",
    "*SimpleMenu*Sme.height: 16",
    "*SimpleMenu*Cursor: left_ptr",
    "*mainMenu.Label:  Main Options (no app-defaults)",
    "*vtMenu.Label:  VT Options (no app-defaults)",
    "*fontMenu.Label:  VT Fonts (no app-defaults)",
#if OPT_TEK4014
    "*tekMenu.Label:  Tek Options (no app-defaults)",
#endif
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
{"-bc",		"*cursorBlink",	XrmoptionNoArg,		(caddr_t) "on"},
{"+bc",		"*cursorBlink",	XrmoptionNoArg,		(caddr_t) "off"},
{"-bcf",	"*cursorOffTime",XrmoptionSepArg,	(caddr_t) NULL},
{"-bcn",	"*cursorOnTime",XrmoptionSepArg,	(caddr_t) NULL},
{"-bdc",	"*colorBDMode",	XrmoptionNoArg,		(caddr_t) "off"},
{"+bdc",	"*colorBDMode",	XrmoptionNoArg,		(caddr_t) "on"},
{"-cb",		"*cutToBeginningOfLine", XrmoptionNoArg, (caddr_t) "off"},
{"+cb",		"*cutToBeginningOfLine", XrmoptionNoArg, (caddr_t) "on"},
{"-cc",		"*charClass",	XrmoptionSepArg,	(caddr_t) NULL},
{"-class",	NULL,		XrmoptionSkipArg,	(caddr_t) NULL},
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
#if OPT_HIGHLIGHT_COLOR
{"-hc",		"*highlightColor", XrmoptionSepArg,	(caddr_t) NULL},
#endif
#if OPT_HP_FUNC_KEYS
{"-hf",		"*hpFunctionKeys",XrmoptionNoArg,	(caddr_t) "on"},
{"+hf",		"*hpFunctionKeys",XrmoptionNoArg,	(caddr_t) "off"},
#endif
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
{"-mesg",	"*messages",	XrmoptionNoArg,		(caddr_t) "off"},
{"+mesg",	"*messages",	XrmoptionNoArg,		(caddr_t) "on"},
{"-ms",		"*pointerColor",XrmoptionSepArg,	(caddr_t) NULL},
{"-nb",		"*nMarginBell",	XrmoptionSepArg,	(caddr_t) NULL},
{"-nul",	"*underLine",	XrmoptionNoArg,		(caddr_t) "off"},
{"+nul",	"*underLine",	XrmoptionNoArg,		(caddr_t) "on"},
{"-pc",		"*boldColors",	XrmoptionNoArg,		(caddr_t) "on"},
{"+pc",		"*boldColors",	XrmoptionNoArg,		(caddr_t) "off"},
{"-rw",		"*reverseWrap",	XrmoptionNoArg,		(caddr_t) "on"},
{"+rw",		"*reverseWrap",	XrmoptionNoArg,		(caddr_t) "off"},
{"-s",		"*multiScroll",	XrmoptionNoArg,		(caddr_t) "on"},
{"+s",		"*multiScroll",	XrmoptionNoArg,		(caddr_t) "off"},
{"-sb",		"*scrollBar",	XrmoptionNoArg,		(caddr_t) "on"},
{"+sb",		"*scrollBar",	XrmoptionNoArg,		(caddr_t) "off"},
#ifdef SCROLLBAR_RIGHT
{"-leftbar",	"*rightScrollBar", XrmoptionNoArg,	(caddr_t) "off"},
{"-rightbar",	"*rightScrollBar", XrmoptionNoArg,	(caddr_t) "on"},
#endif
{"-rvc",	"*colorRVMode",	XrmoptionNoArg,		(caddr_t) "off"},
{"+rvc",	"*colorRVMode",	XrmoptionNoArg,		(caddr_t) "on"},
{"-sf",		"*sunFunctionKeys", XrmoptionNoArg,	(caddr_t) "on"},
{"+sf",		"*sunFunctionKeys", XrmoptionNoArg,	(caddr_t) "off"},
{"-si",		"*scrollTtyOutput", XrmoptionNoArg,	(caddr_t) "off"},
{"+si",		"*scrollTtyOutput", XrmoptionNoArg,	(caddr_t) "on"},
{"-sk",		"*scrollKey",	XrmoptionNoArg,		(caddr_t) "on"},
{"+sk",		"*scrollKey",	XrmoptionNoArg,		(caddr_t) "off"},
{"-sl",		"*saveLines",	XrmoptionSepArg,	(caddr_t) NULL},
#if OPT_SUNPC_KBD
{"-sp",		"*sunKeyboard", XrmoptionNoArg,		(caddr_t) "on"},
{"+sp",		"*sunKeyboard", XrmoptionNoArg,		(caddr_t) "off"},
#endif
{"-t",		"*tekStartup",	XrmoptionNoArg,		(caddr_t) "on"},
{"+t",		"*tekStartup",	XrmoptionNoArg,		(caddr_t) "off"},
{"-ti",		"*decTerminalID",XrmoptionSepArg,	(caddr_t) NULL},
{"-tm",		"*ttyModes",	XrmoptionSepArg,	(caddr_t) NULL},
{"-tn",		"*termName",	XrmoptionSepArg,	(caddr_t) NULL},
#if OPT_WIDE_CHARS
{"-u8",		"*utf8",	XrmoptionNoArg,		(caddr_t) "2"},
{"+u8",		"*utf8",	XrmoptionNoArg,		(caddr_t) "0"},
#endif
{"-ulc",	"*colorULMode",	XrmoptionNoArg,		(caddr_t) "off"},
{"+ulc",	"*colorULMode",	XrmoptionNoArg,		(caddr_t) "on"},
{"-ut",		"*utmpInhibit",	XrmoptionNoArg,		(caddr_t) "on"},
{"+ut",		"*utmpInhibit",	XrmoptionNoArg,		(caddr_t) "off"},
{"-im",		"*useInsertMode", XrmoptionNoArg,	(caddr_t) "on"},
{"+im",		"*useInsertMode", XrmoptionNoArg,	(caddr_t) "off"},
{"-vb",		"*visualBell",	XrmoptionNoArg,		(caddr_t) "on"},
{"+vb",		"*visualBell",	XrmoptionNoArg,		(caddr_t) "off"},
#if OPT_WIDE_CHARS
{"-wc",		"*wideChars",	XrmoptionNoArg,		(caddr_t) "on"},
{"+wc",		"*wideChars",	XrmoptionNoArg,		(caddr_t) "off"},
#endif
{"-wf",		"*waitForMap",	XrmoptionNoArg,		(caddr_t) "on"},
{"+wf",		"*waitForMap",	XrmoptionNoArg,		(caddr_t) "off"},
#if OPT_ZICONBEEP
{"-ziconbeep",  "*zIconBeep",   XrmoptionSepArg,        (caddr_t) NULL},
#endif
#if OPT_SAME_NAME
{"-samename",	"*sameName",	XrmoptionNoArg,		(caddr_t) "on"},
{"+samename",	"*sameName",	XrmoptionNoArg,		(caddr_t) "off"},
#endif
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
{ "-version",              "print the version number" },
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
{ "-class string",         "class string (XTerm)" },
{ "-title string",         "title string" },
{ "-xrm resourcestring",   "additional resource specifications" },
{ "-/+132",                "turn on/off column switch inhibiting" },
{ "-/+ah",                 "turn on/off always highlight" },
#ifndef NO_ACTIVE_ICON
{ "-/+ai",		   "turn on/off active icon" },
{ "-fi fontname",	   "icon font for active icon" },
#endif /* NO_ACTIVE_ICON */
{ "-b number",             "internal border in pixels" },
{ "-/+bc",		   "turn on/off text cursor blinking" },
{ "-bcf milliseconds",	   "time text cursor is off when blinking"},
{ "-bcn milliseconds",	   "time text cursor is on when blinking"},
{ "-/+bdc",                "turn off/on display of bold as color"},
{ "-/+cb",                 "turn on/off cut-to-beginning-of-line inhibit" },
{ "-cc classrange",        "specify additional character classes" },
{ "-/+cm",                 "turn off/on ANSI color mode" },
{ "-/+cn",                 "turn on/off cut newline inhibit" },
{ "-cr color",             "text cursor color" },
{ "-/+cu",                 "turn on/off curses emulation" },
{ "-/+dc",		   "turn off/on dynamic color selection" },
{ "-fb fontname",          "bold text font" },
#if OPT_HIGHLIGHT_COLOR
{ "-hc",		   "selection background color" },
#endif
#if OPT_HP_FUNC_KEYS
{ "-/+hf",                 "turn on/off HP Function Key escape codes" },
#endif
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
{ "-/+mesg",		   "forbid/allow messages" },
{ "-ms color",             "pointer color" },
{ "-nb number",            "margin bell in characters from right end" },
{ "-/+nul",                "turn on/off display of underlining" },
{ "-/+aw",                 "turn on/off auto wraparound" },
{ "-/+pc",                 "turn on/off PC-style bold colors" },
{ "-/+rw",                 "turn on/off reverse wraparound" },
{ "-/+s",                  "turn on/off multiscroll" },
{ "-/+sb",                 "turn on/off scrollbar" },
#ifdef SCROLLBAR_RIGHT
{ "-rightbar",             "force scrollbar right (default left)" },
{ "-leftbar",              "force scrollbar left" },
#endif
{ "-/+rvc",		   "turn off/on display of reverse as color" },
{ "-/+sf",                 "turn on/off Sun Function Key escape codes" },
{ "-/+si",                 "turn on/off scroll-on-tty-output inhibit" },
{ "-/+sk",                 "turn on/off scroll-on-keypress" },
{ "-sl number",            "number of scrolled lines to save" },
#if OPT_SUNPC_KBD
{ "-/+sp",                 "turn on/off Sun/PC Function/Keypad mapping" },
#endif
#if OPT_TEK4014
{ "-/+t",                  "turn on/off Tek emulation window" },
#endif
{ "-ti termid",            "terminal identifier" },
{ "-tm string",            "terminal mode keywords and characters" },
{ "-tn name",              "TERM environment variable name" },
#if OPT_WIDE_CHARS
{ "-/+u8",                 "turn on/off UTF-8 mode (implies wide-characters)" },
#endif
{ "-/+ulc",                "turn off/on display of underline as color" },
{ "-/+ut",                 "turn on/off utmp inhibit (not supported)" },
{ "-/+vb",                 "turn on/off visual bell" },
#if OPT_WIDE_CHARS
{ "-/+wc",                 "turn on/off wide-character mode" },
#endif
{ "-/+wf",                 "turn on/off wait for map before command exec" },
{ "-e command args ...",   "command to execute" },
#if OPT_TEK4014
{ "%geom",                 "Tek window geometry" },
#endif
{ "#geom",                 "icon window geometry" },
{ "-T string",             "title name for window" },
{ "-n string",             "icon name for window" },
{ "-C",                    "intercept console messages" },
{ "-Sccn",                 "slave mode on \"ttycc\", file descriptor \"n\"" },
#if OPT_ZICONBEEP
{ "-ziconbeep percent",    "beep and flag icon of window having hidden output" },
#endif
#if OPT_SAME_NAME
{"-/+samename",	           "turn on/off the no flicker option for title and icon name" },
#endif
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

static void closecons(void)
{
	fclose(confd);
}
*/
static char *message[] = {
"Fonts should be fixed width and, if both normal and bold are specified, should",
"have the same size.  If only a normal font is specified, it will be used for",
"both normal and bold text (by doing overstriking).  The -e option, if given,",
"must appear at the end of the command line, otherwise the user's default shell",
"will be started.  Options that start with a plus sign (+) restore the default.",
NULL};

static int abbrev (char *tst, char *cmp)
{
	size_t len = strlen(tst);
	return ((len >= 2) && (!strncmp(tst, cmp, len)));
}

static void Syntax (char *badOption)
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

static void Version (void)
{
    printf("%s(%d)\n", XFREE86_VERSION, XTERM_PATCH);
    exit (0);
}

static void Help (void)
{
    struct _options *opt;
    char **cpp;

    fprintf (stderr, "%s(%d) usage:\n    %s [-options ...] [-e command args]\n\n",
	     XFREE86_VERSION, XTERM_PATCH, ProgramName);
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
ConvertConsoleSelection(
    Widget w GCC_UNUSED,
    Atom *selection GCC_UNUSED,
    Atom *target GCC_UNUSED,
    Atom *type GCC_UNUSED,
    XtPointer *value GCC_UNUSED,
    unsigned long *length GCC_UNUSED,
    int *format GCC_UNUSED)
{
    /* we don't save console output, so can't offer it */
    return False;
}

Arg ourTopLevelShellArgs[] = {
	{ XtNallowShellResize, (XtArgVal) TRUE },
	{ XtNinput, (XtArgVal) TRUE },
};
int number_ourTopLevelShellArgs = 2;

Bool waiting_for_initial_map;

/*
 * DeleteWindow(): Action proc to implement ICCCM delete_window.
 */
/* ARGSUSED */
static void
DeleteWindow(
    Widget w,
    XEvent *event GCC_UNUSED,
    String *params GCC_UNUSED,
    Cardinal *num_params GCC_UNUSED)
{
#if OPT_TEK4014
  if (w == toplevel)
    if (term->screen.Tshow)
      hide_vt_window();
    else
      do_hangup(w, (XtPointer)0, (XtPointer)0);
  else
    if (term->screen.Vshow)
      hide_tek_window();
    else
#endif
      do_hangup(w, (XtPointer)0, (XtPointer)0);
}

/* ARGSUSED */
static void
KeyboardMapping(
    Widget w GCC_UNUSED,
    XEvent *event,
    String *params GCC_UNUSED,
    Cardinal *num_params GCC_UNUSED)
{
    switch (event->type) {
       case MappingNotify:
	  XRefreshKeyboardMapping(&event->xmapping);
	  break;
    }
}

XtActionsRec actionProcs[] = {
    { "DeleteWindow", DeleteWindow },
    { "KeyboardMapping", KeyboardMapping },
};

Atom wm_delete_window;

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
extern char **environ;

int
main (int argc, char **argv, char **envp)
{
	Widget form_top, menu_top;
	register TScreen *screen;
	int mode;
	char *my_class = DEFCLASS;

	/* Do these first, since we may not be able to open the display */
	ProgramName = argv[0];
	if (argc > 1) {
		int n;
		if (abbrev(argv[1], "-version"))
			Version();
		if (abbrev(argv[1], "-help"))
			Help();
		for (n = 1; n < argc; n++) {
			if (strlen(argv[n]) > 2
			 && abbrev(argv[n], "-class"))
				if ((my_class = argv[++n]) == 0)
					Help();
		}
	}

	/* XXX: for some obscure reason EMX seems to lose the value of
	 * the environ variable, don't understand why, so save it recently
	 */
	gblenvp = envp;

#ifdef I18N
	setlocale(LC_ALL, NULL);
#endif

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
	d_tio.c_cflag = B38400|CS8|CREAD|PARENB|HUPCL;
	d_tio.c_lflag = ISIG|ICANON|ECHO|ECHOE|ECHOK;
	d_tio.c_line = 0;
	d_tio.c_cc[VINTR] = CONTROL('C');	/* '^C'	*/
	d_tio.c_cc[VERASE] = 0x7f;		/* DEL	*/
	d_tio.c_cc[VKILL] = CONTROL('U');	/* '^U'	*/
	d_tio.c_cc[VQUIT] = CQUIT;		/* '^\'	*/
	d_tio.c_cc[VEOF] = CEOF;		/* '^D' */
	d_tio.c_cc[VEOL] = CEOL;		/* '^@'	*/

	/* Init the Toolkit. */
	{
	    XtSetErrorHandler(xt_error);
	    toplevel = XtAppInitialize (&app_con, my_class,
					optionDescList,
					XtNumber(optionDescList),
					&argc, argv, fallback_resources,
					NULL, 0);
	    XtSetErrorHandler((XtErrorHandler)0);

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

#if OPT_ZICONBEEP
	zIconBeep = resource.zIconBeep;
	zIconBeep_flagged = False;
	if ( zIconBeep > 100 || zIconBeep < -100 ) {
	    zIconBeep = 0;	/* was 100, but I prefer to defaulting off. */
	    fprintf( stderr, "a number between -100 and 100 is required for zIconBeep.  0 used by default\n");
	}
#endif /* OPT_ZICONBEEP */
#if OPT_SAME_NAME
        sameName = resource.sameName;
#endif
	xterm_name = resource.xterm_name;
	if (strcmp(xterm_name, "-") == 0) xterm_name = DFT_TERMTYPE;
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
	     case 'c':	/* -class */
		break;
	     case 'e':
		if (argc <= 1) Syntax (*argv);
		command_to_exec = ++argv;
		break;
	     default:
		Syntax (*argv);
	    }
	    break;
	}

	SetupMenus(toplevel, &form_top, &menu_top);

        term = (XtermWidget) XtVaCreateManagedWidget(
		"vt100", xtermWidgetClass, form_top,
#if OPT_TOOLBAR
		XtNmenuBar,	menu_top,
		XtNresizable,	True,
		XtNfromVert,	menu_top,
		XtNleft,	XawChainLeft,
		XtNright,	XawChainRight,
		XtNbottom,	XawChainBottom,
#endif
		0);
	    /* this causes the initialize method to be called */

#if OPT_HP_FUNC_KEYS
	init_keyboard_type(keyboardIsHP, resource.hpFunctionKeys);
#endif
	init_keyboard_type(keyboardIsSun, resource.sunFunctionKeys);
#if OPT_SUNPC_KBD
	init_keyboard_type(keyboardIsVT220, resource.sunKeyboard);
#endif

        screen = &term->screen;

	inhibit = 0;
#ifdef ALLOWLOGGING
	if (term->misc.logInhibit)		inhibit |= I_LOG;
#endif
	if (term->misc.signalInhibit)		inhibit |= I_SIGNAL;
#if OPT_TEK4014
	if (term->misc.tekInhibit)		inhibit |= I_TEK;
#endif

/*
 * Set title and icon name if not specified
 */

	if (command_to_exec) {
	    Arg args[2];

	    if (!resource.title) {
		if (command_to_exec) {
		    resource.title = x_basename (command_to_exec[0]);
		} /* else not reached */
	    }

	    if (!resource.icon_name)
	      resource.icon_name = resource.title;
	    XtSetArg (args[0], XtNtitle, resource.title);
	    XtSetArg (args[1], XtNiconName, resource.icon_name);

	    XtSetValues (toplevel, args, 2);
	}

#if OPT_TEK4014
	if(inhibit & I_TEK)
		screen->TekEmu = FALSE;

	if(screen->TekEmu && !TekInit())
		exit(ERROR_INIT);
#endif

#ifdef DEBUG
    {
	/* Set up stderr properly.  Opening this log file cannot be
	 done securely by a privileged xterm process (although we try),
	 so the debug feature is disabled by default. */
	int i = -1;
	if(debug) {
		creat_as (getuid(), getgid(), True, "xterm.debug.log", 0666);
		i = open ("xterm.debug.log", O_WRONLY | O_TRUNC, 0666);
	}
	if(i >= 0) {
		dup2(i,2);

		/* mark this file as close on exec */
		(void) fcntl(i, F_SETFD, 1);
	}
    }
#endif	/* DEBUG */

	/* open a terminal for client */
	get_terminal ();

	spawn ();

	/* Child process is out there, let's catch its termination */
	(void) signal (SIGCHLD, reapchild);

	/* Realize procs have now been executed */

	if (am_slave >= 0) { /* Write window id so master end can read and use */
	    char buf[80];

	    buf[0] = '\0';
	    sprintf (buf, "%lx\n", XtWindow (XtParent (CURRENT_EMU(screen))));
	    write (screen->respond, buf, strlen (buf));
	}

	screen->inhibit = inhibit;

	if (0 > (mode = fcntl(screen->respond, F_GETFL, 0)))
		Error(1);
	mode |= O_NDELAY;

	if (fcntl(screen->respond, F_SETFL, mode))
		Error(1);

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

#ifdef ALLOWLOGGING
	if (term->misc.log_on) {
		StartLog(screen);
	}
#endif
	for( ; ; ) {
#if OPT_TEK4014
		if(screen->TekEmu)
			TekRun();
		else
#endif
			VTRun();
	}
	return 0;
}

/*
 * Called from get_pty to iterate over likely pseudo terminals
 * we might allocate.  Used on those systems that do not have
 * a functional interface for allocating a pty.
 * Returns 0 if found a pty, 1 if fails.
 */
static int
pty_search(int *pty)
{
	char namebuf[PTMS_BUFSZ];

	/* ask the PTY manager */
	int fd = open("/dev/ptms$",0);
	if (fd && ptioctl(fd,PTMS_GETPTY,namebuf)==0) {
		strcpy(ttydev,namebuf);
		strcpy(ptydev,namebuf);
		*x_basename(ttydev) = 't';
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

/*
 * This function opens up a pty master and stuffs its value into pty.
 *
 * If it finds one, it returns a value of 0.  If it does not find one,
 * it returns a value of !0.  This routine is designed to be re-entrant,
 * so that if a pty master is found and later, we find that the slave
 * has problems, we can re-enter this function and get another one.
 */
static int
get_pty (int *pty)
{
	return pty_search(pty);
}

/*
 * sets up X and initializes the terminal structure except for term.buf.fildes.
 */
static void
get_terminal (void)
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

#if OPT_TEK4014
static char *tekterm[] = {
	"tek4014",
	"tek4015",		/* 4014 with APL character set support */
	"tek4012",		/* 4010 with lower case */
	"tek4013",		/* 4012 with APL character set support */
	"tek4010",		/* small screen, upper-case only */
	"dumb",
	0
};
#endif

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
	DFT_TERMTYPE,		/* for people who want special term name */
	"xterm",		/* the prefered name, should be fastest */
	"vt102",
	"vt100",
	"ansi",
	"dumb",
	0
};

/* ARGSUSED */
static SIGNAL_T hungtty(int i GCC_UNUSED)
{
	longjmp(env, 1);
	SIGNAL_RETURN;
}

struct {
	int rows;
	int cols;
} handshake = {-1,-1};

void first_map_occurred (void)
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

static int
spawn (void)
/*
 *  Inits pty and tty and forks a login process.
 *  Does not close fd Xsocket.
 *  If slave, the pty named in passedPty is already open for use
 */
{
	register TScreen *screen = &term->screen;
	int Xsocket = ConnectionNumber(screen->display);

	int tty = -1;
	struct termio tio;
	int status;

	char termcap[TERMCAP_SIZE], newtc[TERMCAP_SIZE];
	char *TermName = NULL;
	char *ptr, *shname, buf[64];
	int i, no_dev_tty = FALSE, envsize;
	char *dev_tty_name = (char *) 0;
	struct winsize ws;
	int pgrp = getpid();
	char numbuf[12], **envnew;

	screen->uid = getuid();
	screen->gid = getgid();

	if (am_slave >= 0) {
		screen->respond = am_slave;
		ptydev[strlen(ptydev) - 2] =
		ttydev[strlen(ttydev) - 2] = passedPty[0];
		ptydev[strlen(ptydev) - 1] =
		ttydev[strlen(ttydev) - 1] = passedPty[1];

		setgid (screen->gid);
		setuid (screen->uid);
	} else {
		Bool tty_got_hung;

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
			tty_got_hung = False;
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
/**/			if(ioctl(tty, TCGETA, &tio) == -1)
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
	XtSetMappedWhenManaged(XtParent(CURRENT_EMU(screen)), False );

	wm_delete_window = XInternAtom(XtDisplay(toplevel), "WM_DELETE_WINDOW",
				       False);

	if (!TEK4014_ACTIVE(screen))
	    VTInit();		/* realize now so know window size for tty driver */

	if (Console) {
	    /*
	     * Inform any running xconsole program
	     * that we are going to steal the console.
	     */
	    XmuGetHostname (mit_console_name + MIT_CONSOLE_LEN, 255);
	    mit_console = XInternAtom(screen->display, mit_console_name, False);
	    /* the user told us to be the console, so we can use CurrentTime */
	    XtOwnSelection(XtParent(CURRENT_EMU(screen)),
			   mit_console, CurrentTime,
			   ConvertConsoleSelection, NULL, NULL);
	}
#if OPT_TEK4014
	if(screen->TekEmu) {
		envnew = tekterm;
		ptr = newtc;
	}
	else
#endif
	{
		envnew = vtterm;
		ptr = termcap;
	}

	/*
	 * This used to exit if no termcap entry was found for the specified
	 * terminal name.  That's a little unfriendly, so instead we'll allow
	 * the program to proceed (but not to set $TERMCAP) if the termcap
	 * entry is not found.
	 */
	*ptr = 0;	/* initialize, in case we're using terminfo's tgetent */
	TermName = NULL;
	if (resource.term_name) {
	    TermName = resource.term_name;
	    if (tgetent (ptr, resource.term_name) == 1) {
		if (*ptr)
		    if (!TEK4014_ACTIVE(screen))
			resize (screen, termcap, newtc);
	    }
	}

	/*
	 * This block is invoked only if there was no terminal name specified
	 * by the command-line option "-tn".
	 */
	if (!TermName) {
	    TermName = *envnew;
	    while (*envnew != NULL) {
		if(tgetent(ptr, *envnew) == 1) {
			TermName = *envnew;
			if (*ptr)
			    if(!TEK4014_ACTIVE(screen))
				resize(screen, termcap, newtc);
			break;
		}
		envnew++;
	    }
	}

	/* tell tty how big window is */
#if OPT_TEK4014
	if(TEK4014_ACTIVE(screen)) {
		ws.ws_row = 38;
		ws.ws_col = 81;
		ws.ws_xpixel = TFullWidth(screen);
		ws.ws_ypixel = TFullHeight(screen);
	} else
#endif
	{
		ws.ws_row = screen->max_row + 1;
		ws.ws_col = screen->max_col + 1;
		ws.ws_xpixel = FullWidth(screen);
		ws.ws_ypixel = FullHeight(screen);
	}

	if (am_slave < 0) {

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
			if ((ptr = ttyname(tty)) != 0)
			{
				/* it may be bigger */
				ttydev = realloc (ttydev,
					(unsigned) (strlen(ptr) + 1));
				(void) strcpy(ttydev, ptr);
			}
#else
			ptr = ttydev;
#endif
			/* for safety: enable DUPs */
			ptioctl(tty,XTY_ENADUP,0);

			/* change ownership of tty to real group and user id */
			chown (ttydev, screen->uid, screen->gid);

			/* change protection of tty */
			chmod (ttydev, (resource.messages? 0622 : 0600));

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

			/* compute number of xtermSetenv() calls below */
			envsize = 1;	/* (NULL terminating entry) */
			envsize += 3;	/* TERM, WINDOWID, DISPLAY */
			envsize += 2;	/* COLUMNS, LINES */

			envnew = (char **) calloc ((unsigned) i + envsize, sizeof(char *));
			memmove( (char *)envnew, (char *)gblenvp, i * sizeof(char *));
			gblenvp = envnew;
			xtermSetenv ("TERM=", TermName);
			if(!TermName)
				*newtc = 0;

			sprintf (buf, "%lu",
				((unsigned long) XtWindow (XtParent(CURRENT_EMU(screen)))));
			xtermSetenv ("WINDOWID=", buf);

			/* put the display into the environment of the shell*/
			xtermSetenv ("DISPLAY=", XDisplayString (screen->display));

			signal(SIGTERM, SIG_DFL);

			/* this is the time to go and set up stdin, out, and err
			 */
			/* dup the tty */
			for (i = 0; i <= 2; i++)
				if (i != tty) {
					(void) close(i);
					(void) dup(tty);
				}

			/* and close the tty */
			if (tty > 2)
				(void) close(tty);

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
			xtermSetenv("COLUMNS=", numbuf);
			sprintf (numbuf, "%d", screen->max_row + 1);
			xtermSetenv("LINES=", numbuf);

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

			shname = x_basename(ptr);
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
Exit(int n)
{
	register TScreen *screen = &term->screen;
        int pty = term->screen.respond;  /* file descriptor of pty */
        close(pty); /* close explicitly to avoid race with slave side */
#ifdef ALLOWLOGGING
	if(screen->logging)
		CloseLog(screen);
#endif
	if (am_slave < 0) {
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
static void
resize(TScreen *screen, register char *oldtc, register char *newtc)
{
}

/*
 * Does a non-blocking wait for a child process.  If the system
 * doesn't support non-blocking wait, do nothing.
 * Returns the pid of the child, or 0 or -1 if none or error.
 */
int
nonblocking_wait(void)
{
        pid_t pid;

	pid = waitpid(-1, NULL, WNOHANG);
	return pid;
}

/* ARGSUSED */
static SIGNAL_T reapchild (int n GCC_UNUSED)
{
    int pid;

    pid = wait(NULL);

    /* cannot re-enable signal before waiting for child
     * because then SVR4 loops.  Sigh.  HP-UX 9.01 too.
     */
    (void) signal(SIGCHLD, reapchild);

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

/*
 * parse_tty_modes accepts lines of the following form:
 *
 *         [SETTING] ...
 *
 * where setting consists of the words in the modelist followed by a character
 * or ^char.
 */
static int parse_tty_modes (char *s, struct _xttymodes *modelist)
{
    struct _xttymodes *mp;
    int c;
    int count = 0;

    while (1) {
	while (*s && isascii(CharOf(*s)) && isspace(CharOf(*s))) s++;
	if (!*s) return count;

	for (mp = modelist; mp->name; mp++) {
	    if (strncmp (s, mp->name, mp->len) == 0) break;
	}
	if (!mp->name) return -1;

	s += mp->len;
	while (*s && isascii(CharOf(*s)) && isspace(CharOf(*s))) s++;
	if (!*s) return -1;

	if (*s == '^') {
	    s++;
	    c = ((*s == '?') ? 0177 : CONTROL(*s));
	    if (*s == '-') {
		errno = 0;
		c = fpathconf(0, _PC_VDISABLE);
		if (c == -1) {
		    if (errno != 0)
			continue;	/* skip this (error) */
		    c = 0377;
		}
	    }
	} else {
	    c = *s;
	}
	mp->value = c;
	mp->set = 1;
	count++;
	s++;
    }
}

int GetBytesAvailable (int fd)
{
    long arg;
    ioctl (fd, FIONREAD, (char *) &arg);
    return (int) arg;
}

/* Utility function to try to hide system differences from
   everybody who used to call killpg() */

int
kill_process_group(int pid, int sig)
{
    return kill (-pid, sig);
}
