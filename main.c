#ifndef lint
static char *rid="$TOG: main.c /main/249 1997/08/26 14:13:43 kaleb $";
#endif /* lint */

/*
 *				 W A R N I N G
 *
 * If you think you know what all of this code is doing, you are
 * probably very mistaken.  There be serious and nasty dragons here.
 *
 * This client is *not* to be taken as an example of how to write X
 * Toolkit applications.  It is in need of a substantial rewrite,
 * ideally to create a generic tty widget with several different parsing
 * widgets so that you can plug 'em together any way you want.  Don't
 * hold your breath, though....
 */

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

/* $XFree86: xc/programs/xterm/main.c,v 3.103 1999/12/14 02:10:37 robin Exp $ */


/* main.c */

#include <version.h>
#include <xterm.h>

#include <X11/StringDefs.h>
#include <X11/Shell.h>

#include <X11/Xos.h>
#include <X11/cursorfont.h>
#include <X11/Xlocale.h>

#if OPT_TOOLBAR
#include <X11/Xaw/Form.h>
#endif

#include <pwd.h>
#include <ctype.h>

#include <data.h>
#include <error.h>
#include <menu.h>

#ifdef AMOEBA
#include <amoeba.h>
#include <cmdreg.h>
#include <stderr.h>
#include <thread.h>
#define  _POSIX_SOURCE
#include <limits.h>
#include <module/proc.h>
#include <module/name.h>

#define USE_TERMIOS
#define USE_POSIX_WAIT
#define NILCAP ((capability *)NULL)
#endif

#ifdef MINIX
#include <sys/nbio.h>

#define setpgrp(pid, pgid) setpgid(pid, pgid)
#define USE_TERMIOS
#define MNX_LASTLOG
#define WTMP
/* Remap or define non-existing termios flags */
#define OCRNL	0
#define ONLRET	0
#define NLDLY	0
#define CRDLY	0
#define TABDLY	0
#define BSDLY	0
#define VTDLY	0
#define FFDLY	0
#endif

#ifdef att
#define ATT
#endif

#ifdef __osf__
#define USE_SYSV_SIGNALS
#endif

#ifdef SVR4
#undef  SYSV			/* predefined on Solaris 2.4 */
#define SYSV			/* SVR4 is (approx) superset of SVR3 */
#define ATT
#ifndef __sgi
#define USE_TERMIOS
#endif
#endif

#if defined(SYSV) && defined(i386) && !defined(SVR4)
#define ATT
#define USE_HANDSHAKE
static Bool IsPts = False;
#endif

#if (defined(ATT) && !defined(__sgi)) || (defined(SYSV) && defined(i386)) || (defined (__GLIBC__) && ((__GLIBC__ > 2) || (__GLIBC__ == 2) && (__GLIBC_MINOR__ >= 1)))
#define USE_USG_PTYS
#else
#define USE_HANDSHAKE
#endif

#if defined(SYSV) && !defined(SVR4) && !defined(ISC22) && !defined(ISC30)
/* older SYSV systems cannot ignore SIGHUP.
   Shell hangs, or you get extra shells, or something like that */
#define USE_SYSV_SIGHUP
#endif

#if defined(sony) && defined(bsd43) && !defined(KANJI)
#define KANJI
#endif

#ifdef TIOCSLTC
#define HAS_LTCHARS
#endif

#ifdef linux
#define USE_TERMIOS
#define USE_SYSV_PGRP
#define USE_SYSV_SIGNALS
#define WTMP
#undef  HAS_LTCHARS
#if (__GLIBC__ > 2) || ((__GLIBC__ == 2) && (__GLIBC_MINOR__ >= 1))
#include <pty.h>
#endif
#endif

#ifdef __MVS__
#define SVR4
#define USE_POSIX_TERMIOS
#define USE_USG_PTYS
#define USE_SYSV_PGRP
#define USE_SYSV_SIGNALS
#undef  HAS_LTCHARS
#endif

#ifdef __CYGWIN__
#define SYSV
#define SVR4
#define WTMP
#define ATT
#endif

#ifdef __QNX__
#define USE_POSIX_TERMIOS
#include <ioctl.h>
#endif

#ifdef Lynx
#define USE_SYSV_TERMIO
#undef  HAS_LTCHARS
#include <termio.h>
#endif

#ifdef SCO325
#define _SVID3
#endif

#ifdef __GNU__
#define USE_SYSV_PGRP
#define WTMP
#define HAS_BSD_GROUPS
#endif

#ifdef USE_TTY_GROUP
#include <grp.h>
#endif

#ifndef __CYGWIN__
#include <sys/ioctl.h>
#endif

#include <sys/stat.h>

#ifdef Lynx
#ifndef BSDLY
#define BSDLY	0
#endif
#ifndef VTDLY
#define VTDLY	0
#endif
#ifndef FFDLY
#define FFDLY	0
#endif
#endif

#if defined(USE_POSIX_TERMIOS)
#include <termios.h>
#elif defined(USE_TERMIOS)
#include <termios.h>
/* this hacked termios support only works on SYSV */
#define USE_SYSV_TERMIO
#define termio termios
#undef TCGETA
#define TCGETA TCGETS
#undef TCSETA
#define TCSETA TCSETS
#elif defined(SYSV)
#include <sys/termio.h>
#elif defined(sun) && !defined(SVR4)
#include <sys/ttycom.h>
#ifdef TIOCSWINSZ
#undef TIOCSSIZE
#endif
#endif /* USE_POSIX_TERMIOS */

#ifdef SVR4
#undef HAS_LTCHARS			/* defined, but not useable */
#endif
#define USE_TERMCAP_ENVVARS	/* every one uses this except SYSV maybe */

#if defined(__sgi) && (OSMAJORVERSION >= 5)
#undef TIOCLSET				/* defined, but not useable */
#endif

#if defined(__GNU__) || defined(__MVS__)
#undef TIOCLSET
#undef TIOCSLTC
#endif

#ifdef SYSV /* { */

#ifdef USE_USG_PTYS			/* AT&T SYSV has no ptyio.h */
#include <sys/stream.h>			/* get typedef used in ptem.h */
#include <sys/stropts.h>		/* for I_PUSH */
#if !defined(SVR4) || defined(SCO325)
#include <sys/ptem.h>			/* get struct winsize */
#endif
#include <poll.h>			/* for POLLIN */
#endif /* USE_USG_PTYS */
#define USE_SYSV_TERMIO
#define USE_SYSV_SIGNALS
#define	USE_SYSV_PGRP
#if !defined(TIOCSWINSZ)
#define USE_SYSV_ENVVARS		/* COLUMNS/LINES vs. TERMCAP */
#endif
/*
 * now get system-specific includes
 */
#ifdef CRAY
#define HAS_BSD_GROUPS
#endif

#ifdef macII
#define HAS_BSD_GROUPS
#include <sys/ttychars.h>
#undef USE_SYSV_ENVVARS
#undef FIOCLEX
#undef FIONCLEX
#define setpgrp2 setpgrp
#include <sgtty.h>
#include <sys/resource.h>
#endif

#ifdef SCO
#define USE_POSIX_WAIT
#endif /* SCO */

#ifdef __hpux
#define HAS_BSD_GROUPS
#define USE_POSIX_WAIT
#include <sys/ptyio.h>
#endif /* __hpux */

#ifdef __sgi
#define HAS_BSD_GROUPS
#include <sys/sysmacros.h>
#endif /* __sgi */

#ifdef sun
#include <sys/strredir.h>
#endif

#else /* } !SYSV { */			/* BSD systems */

#ifdef MINIX /* { */

#else /* } !MINIX { */

#ifdef __QNX__
#undef TIOCSLTC  /* <sgtty.h> conflicts with <termios.h> */
#undef TIOCLSET
#define USE_POSIX_WAIT
#ifndef __QNXNTO__
#define ttyslot() 1
#else
#define USE_SYSV_PGRP
extern __inline__ ttyslot() {return 1;} /* yuk */
#endif
#else

#ifndef linux
#ifndef USE_POSIX_TERMIOS
#ifndef USE_SYSV_TERMIO
#include <sgtty.h>
#endif
#endif /* USE_POSIX_TERMIOS */
#ifndef Lynx
#include <sys/resource.h>
#else
#include <resource.h>
#endif
#define HAS_BSD_GROUPS
#ifdef __osf__
#define setpgrp setpgid
#endif
#endif /* !linux */

#endif /* __QNX__ */

#endif /* } MINIX */

#endif	/* } !SYSV */

#ifdef _POSIX_SOURCE
#define USE_POSIX_WAIT
#endif

#ifdef SVR4
#define USE_POSIX_WAIT
#define HAS_SAVED_IDS_AND_SETEUID
#endif

#ifdef linux
#define HAS_SAVED_IDS_AND_SETEUID
#endif

/* Xpoll.h and <sys/param.h> on glibc 2.1 systems have colliding NBBY's */
#if defined(__GLIBC__) && ((__GLIBC__ > 2) || ((__GLIBC__ == 2) && (__GLIBC_MINOR__ >= 1)))
#ifndef NOFILE
#define NOFILE OPEN_MAX
#endif
#elif !defined(MINIX) && !defined(WIN32) && !defined(Lynx) && !defined(__GNU__) && !defined(__MVS__)
#include <sys/param.h>	/* for NOFILE */
#endif

#if defined(BSD) && (BSD >= 199103)
#define USE_POSIX_WAIT
#define WTMP
#define HAS_SAVED_IDS_AND_SETEUID
#endif

#include <stdio.h>

#ifdef X_NOT_STDC_ENV
extern time_t time ();
#else
#include <time.h>
#endif

#ifdef __hpux
#include <sys/utsname.h>
#endif /* __hpux */

#if defined(apollo) && (OSMAJORVERSION == 10) && (OSMINORVERSION < 4)
#define ttyslot() 1
#endif /* apollo */

#ifdef sun
#include <sys/filio.h>
#endif

#include <sys/types.h>

#if defined(UTMPX_FOR_UTMP)

#include <utmpx.h>
#define setutent setutxent
#define getutent getutxent
#define getutid getutxid
#define endutent endutxent
#define pututline pututxline

#elif defined(HAVE_UTMP)

#include <utmp.h>
#if defined(_CRAY) && (OSMAJORVERSION < 8)
extern struct utmp *getutid __((struct utmp *_Id));
#endif

#endif

#if defined(USE_LASTLOG) && defined(HAVE_LASTLOG_H)
#include <lastlog.h>
#endif

#ifdef  PUCC_PTYD
#include <local/openpty.h>
int	Ptyfd;
#endif /* PUCC_PTYD */

#ifdef sequent
#define USE_GET_PSEUDOTTY
#endif

#ifndef UTMP_FILENAME
#ifdef UTMP_FILE
#define UTMP_FILENAME UTMP_FILE
#else
#ifdef _PATH_UTMP
#define UTMP_FILENAME _PATH_UTMP
#else
#define UTMP_FILENAME "/etc/utmp"
#endif
#endif
#endif

#ifndef LASTLOG_FILENAME
#ifdef _PATH_LASTLOG
#define LASTLOG_FILENAME _PATH_LASTLOG
#else
#define LASTLOG_FILENAME "/usr/adm/lastlog"  /* only on BSD systems */
#endif
#endif

#ifndef WTMP_FILENAME
#ifdef WTMP_FILE
#define WTMP_FILENAME WTMP_FILE
#else
#ifdef _PATH_WTMP
#define WTMP_FILENAME _PATH_WTMP
#else
#ifdef SYSV
#define WTMP_FILENAME "/etc/wtmp"
#else
#define WTMP_FILENAME "/usr/adm/wtmp"
#endif
#endif
#endif
#endif

#include <signal.h>

#if defined(sco) || (defined(ISC) && !defined(_POSIX_SOURCE))
#undef SIGTSTP			/* defined, but not the BSD way */
#endif

#ifdef SIGTSTP
#include <sys/wait.h>
#ifdef __hpux
#include <sys/bsdtty.h>
#endif
#endif

#ifdef X_NOT_POSIX
extern long lseek();
#if defined(USG)
extern unsigned sleep();
#else
extern void sleep();
#endif
extern char *ttyname();
#endif

#ifdef SYSV
extern char *ptsname(int);
#endif

#ifdef	__cplusplus
extern "C" {
#endif

extern int tgetent (char *ptr, char *name);
extern char *tgetstr (char *name, char **ptr);

#ifdef	__cplusplus
	}
#endif

static SIGNAL_T reapchild (int n);
static char *base_name (char *name);
static int pty_search (int *pty);
static int spawn (void);
static void get_terminal (void);
static void remove_termcap_entry (char *buf, char *str);
static void resize (TScreen *s, char *oldtc, char *newtc);

static Bool added_utmp_entry = False;

#ifdef USE_SYSV_UTMP
static Bool xterm_exiting = False;
#endif

/*
** Ordinarily it should be okay to omit the assignment in the following
** statement. Apparently the c89 compiler on AIX 4.1.3 has a bug, or does
** it? Without the assignment though the compiler will init command_to_exec
** to 0xffffffff instead of NULL; and subsequent usage, e.g. in spawn() to
** SEGV.
*/
static char **command_to_exec = NULL;

#ifdef USE_SYSV_TERMIO
#ifndef ICRNL
#include <sys/termio.h>
#endif
#if defined (__sgi) || (defined(__linux__) && defined(__sparc__))
#undef TIOCLSET /* XXX why is this undef-ed again? */
#endif
#endif /* USE_SYSV_TERMIO */

#define TERMCAP_ERASE "kb"
#define VAL_INITIAL_ERASE A2E(127)

/* allow use of system default characters if defined and reasonable */
#ifndef CBRK
#define CBRK 0
#endif
#ifndef CDSUSP
#define CDSUSP CONTROL('Y')
#endif
#ifndef CEOF
#define CEOF CONTROL('D')
#endif
#ifndef CEOL
#define CEOL 0
#endif
#ifndef CFLUSH
#define CFLUSH CONTROL('O')
#endif
#ifndef CINTR
#define CINTR 0177
#endif
#ifndef CKILL
#define CKILL '@'
#endif
#ifndef CLNEXT
#define CLNEXT CONTROL('V')
#endif
#ifndef CNUL
#define CNUL 0
#endif
#ifndef CQUIT
#define CQUIT CONTROL('\\')
#endif
#ifndef CRPRNT
#define CRPRNT CONTROL('R')
#endif
#ifndef CSTART
#define CSTART CONTROL('Q')
#endif
#ifndef CSTOP
#define CSTOP CONTROL('S')
#endif
#ifndef CSUSP
#define CSUSP CONTROL('Z')
#endif
#ifndef CSWTCH
#define CSWTCH 0
#endif
#ifndef CWERASE
#define CWERASE CONTROL('W')
#endif

#ifdef USE_SYSV_TERMIO
/* The following structures are initialized in main() in order
** to eliminate any assumptions about the internal order of their
** contents.
*/
static struct termio d_tio;
#ifdef HAS_LTCHARS
static struct ltchars d_ltc;
#endif	/* HAS_LTCHARS */

#ifdef TIOCLSET
static unsigned int d_lmode;
#endif	/* TIOCLSET */

#elif defined(USE_POSIX_TERMIOS)
static struct termios d_tio;
#else /* !USE_SYSV_TERMIO && !USE_POSIX_TERMIOS */
static struct  sgttyb d_sg = {
	0, 0, 0177, CKILL, EVENP|ODDP|ECHO|XTABS|CRMOD
};
static struct  tchars d_tc = {
	CINTR, CQUIT, CSTART,
	CSTOP, CEOF, CBRK
};
static struct  ltchars d_ltc = {
	CSUSP, CDSUSP, CRPRNT,
	CFLUSH, CWERASE, CLNEXT
};
static int d_disipline = NTTYDISC;
static long int d_lmode = LCRTBS|LCRTERA|LCRTKIL|LCTLECH;
#ifdef sony
static long int d_jmode = KM_SYSSJIS|KM_ASCII;
static struct jtchars d_jtc = {
	'J', 'B'
};
#endif /* sony */
#endif /* USE_SYSV_TERMIO */

/*
 * SYSV has the termio.c_cc[V] and ltchars; BSD has tchars and ltchars;
 * SVR4 has only termio.c_cc, but it includes everything from ltchars.
 * POSIX termios has termios.c_cc, which is similar to SVR4.
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
{ "status", 6, 0, '\0' },		/* VSTATUS */
#define XTTYMODE_status 16
{ NULL, 0, 0, '\0' },			/* end of data */
};

#define TMODE(ind,var) if (ttymodelist[ind].set) var = ttymodelist[ind].value

static int parse_tty_modes (char *s, struct _xttymodes *modelist);

#ifdef USE_SYSV_UTMP
#if defined(X_NOT_STDC_ENV) || (defined(AIXV3) && (OSMAJORVERSION < 4))
extern struct utmp *getutent();
extern struct utmp *getutid();
extern struct utmp *getutline();
#endif /* X_NOT_STDC_ENV || AIXV3 */

#ifdef X_NOT_STDC_ENV		/* could remove paragraph unconditionally? */
extern struct passwd *getpwent();
extern struct passwd *getpwuid();
extern struct passwd *getpwnam();
extern void setpwent();
extern void endpwent();
#endif

#else	/* not USE_SYSV_UTMP */
static char etc_utmp[] = UTMP_FILENAME;
#endif	/* USE_SYSV_UTMP */

#ifdef USE_LASTLOG
static char etc_lastlog[] = LASTLOG_FILENAME;
#endif

#ifdef WTMP
static char etc_wtmp[] = WTMP_FILENAME;
#endif

/*
 * Some people with 4.3bsd /bin/login seem to like to use login -p -f user
 * to implement xterm -ls.  They can turn on USE_LOGIN_DASH_P and turn off
 * WTMP and USE_LASTLOG.
 */
#ifdef USE_LOGIN_DASH_P
#ifndef LOGIN_FILENAME
#define LOGIN_FILENAME "/bin/login"
#endif
static char bin_login[] = LOGIN_FILENAME;
#endif

static int inhibit;
static char passedPty[2];	/* name if pty if slave */

#if defined(TIOCCONS) || defined(SRIOCSREDIR)
static int Console;
#include <X11/Xmu/SysUtil.h>	/* XmuGetHostname */
#define MIT_CONSOLE_LEN	12
#define MIT_CONSOLE "MIT_CONSOLE_"
static char mit_console_name[255 + MIT_CONSOLE_LEN + 1] = MIT_CONSOLE;
static Atom mit_console;
#endif	/* TIOCCONS */

#ifndef USE_SYSV_UTMP
static int tslot;
#endif	/* USE_SYSV_UTMP */
static sigjmp_buf env;

char *ProgramName;

static struct _resource {
    char *xterm_name;
    char *icon_geometry;
    char *title;
    char *icon_name;
    char *term_name;
    char *tty_modes;
    Boolean hold_screen;	/* true if we keep window open	*/
    Boolean utmpInhibit;
    Boolean messages;
    Boolean sunFunctionKeys;	/* %%% should be widget resource? */
#if OPT_SUNPC_KBD
    Boolean sunKeyboard;
#endif
#if OPT_HP_FUNC_KEYS
    Boolean hpFunctionKeys;
#endif
#if OPT_INITIAL_ERASE
    Boolean ptyInitialErase;	/* if true, use pty's sense of erase char */
    Boolean backarrow_is_erase;	/* override backspace/delete */
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
    {"hold", "Hold", XtRBoolean, sizeof (Boolean),
	offset(hold_screen), XtRString, "false"},
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
#if OPT_INITIAL_ERASE
    {"ptyInitialErase", "PtyInitialErase", XtRBoolean, sizeof (Boolean),
	offset(ptyInitialErase), XtRString, "false"},
    {"backarrowKeyIsErase", "BackarrowKeyIsErase", XtRBoolean, sizeof(Boolean),
	offset(backarrow_is_erase), XtRString, "false"},
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
    "XTerm*SimpleMenu*menuLabel.vertSpace: 100",
    "XTerm*SimpleMenu*HorizontalMargins: 16",
    "XTerm*SimpleMenu*Sme.height: 16",
    "XTerm*SimpleMenu*Cursor: left_ptr",
    "XTerm*mainMenu.Label:  Main Options (no app-defaults)",
    "XTerm*vtMenu.Label:  VT Options (no app-defaults)",
    "XTerm*fontMenu.Label:  VT Fonts (no app-defaults)",
#if OPT_TEK4014
    "XTerm*tekMenu.Label:  Tek Options (no app-defaults)",
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
{"-hold",	"*hold",	XrmoptionNoArg,		(caddr_t) "on"},
{"+hold",	"*hold",	XrmoptionNoArg,		(caddr_t) "off"},
#if OPT_INITIAL_ERASE
{"-ie",		"*ptyInitialErase", XrmoptionNoArg,	(caddr_t) "on"},
{"+ie",		"*ptyInitialErase", XrmoptionNoArg,	(caddr_t) "off"},
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
{ "-/+hold",		   "turn on/off logic that retains window after exit" },
#if OPT_INITIAL_ERASE
{ "-/+ie",		   "turn on/off initialization of 'erase' from pty" },
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
{ "-/+ulc",                "turn off/on display of underline as color" },
#if OPT_WIDE_CHARS
{ "-/+u8",                 "turn on/off UTF-8 mode (implies wide-characters)" },
#endif
#ifdef HAVE_UTMP
{ "-/+ut",                 "turn on/off utmp inhibit" },
#else
{ "-/+ut",                 "turn on/off utmp inhibit (not supported)" },
#endif
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
#if defined(TIOCCONS) || defined(SRIOCSREDIR)
{ "-C",                    "intercept console messages" },
#else
{ "-C",                    "intercept console messages (not supported)" },
#endif
{ "-Sxxd",                 "slave mode on \"ttyxx\", file descriptor \"d\"" },
#if OPT_ZICONBEEP
{ "-ziconbeep percent",    "beep and flag icon of window having hidden output" },
#endif
#if OPT_SAME_NAME
{"-/+sameName",	   "Turn on/off the no flicker option for title and icon name" },
#endif
{ NULL, NULL }};

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

#if defined(TIOCCONS) || defined(SRIOCSREDIR)
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
#endif /* TIOCCONS */

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
  if (w == toplevel) {
    if (term->screen.Tshow)
      hide_vt_window();
    else
      do_hangup(w, (XtPointer)0, (XtPointer)0);
  } else
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

int
main (int argc, char *argv[])
{
	Widget form_top, menu_top;
	register TScreen *screen;
	int mode;

	/* Do these first, since we may not be able to open the display */
	ProgramName = argv[0];
	if (argc > 1) {
		if (abbrev(argv[1], "-version"))
			Version();
		if (abbrev(argv[1], "-help"))
			Help();
	}

	/* This dumps core on HP-UX 9.05 with X11R5 */
#if OPT_I18N_SUPPORT
	XtSetLanguageProc (NULL, NULL, NULL);
#endif

#ifndef AMOEBA
	/* +2 in case longer tty name like /dev/ttyq255 */
	ttydev = (char *) malloc (sizeof(TTYDEV) + 2);
#ifndef __osf__
	ptydev = (char *) malloc (sizeof(PTYDEV) + 2);
	if (!ttydev || !ptydev)
#else
	if (!ttydev)
#endif
	{
	    fprintf (stderr,
		     "%s:  unable to allocate memory for ttydev or ptydev\n",
		     ProgramName);
	    exit (1);
	}
	strcpy (ttydev, TTYDEV);
#ifndef __osf__
	strcpy (ptydev, PTYDEV);
#endif

#ifdef MINIX
	d_tio.c_iflag= TINPUT_DEF;
	d_tio.c_oflag= TOUTPUT_DEF;
	d_tio.c_cflag= TCTRL_DEF;
	d_tio.c_lflag= TLOCAL_DEF;
	cfsetispeed(&d_tio, TSPEED_DEF);
	cfsetispeed(&d_tio, TSPEED_DEF);
	d_tio.c_cc[VEOF]= TEOF_DEF;
	d_tio.c_cc[VEOL]= TEOL_DEF;
	d_tio.c_cc[VERASE]= TERASE_DEF;
	d_tio.c_cc[VINTR]= TINTR_DEF;
	d_tio.c_cc[VKILL]= TKILL_DEF;
	d_tio.c_cc[VMIN]= TMIN_DEF;
	d_tio.c_cc[VQUIT]= TQUIT_DEF;
	d_tio.c_cc[VTIME]= TTIME_DEF;
	d_tio.c_cc[VSUSP]= TSUSP_DEF;
	d_tio.c_cc[VSTART]= TSTART_DEF;
	d_tio.c_cc[VSTOP]= TSTOP_DEF;
	d_tio.c_cc[VREPRINT]= TREPRINT_DEF;
	d_tio.c_cc[VLNEXT]= TLNEXT_DEF;
	d_tio.c_cc[VDISCARD]= TDISCARD_DEF;
#elif defined(USE_SYSV_TERMIO) || defined(USE_POSIX_TERMIOS) /* { */
	/* Initialization is done here rather than above in order
	** to prevent any assumptions about the order of the contents
	** of the various terminal structures (which may change from
	** implementation to implementation).
	*/
	d_tio.c_iflag = ICRNL|IXON;
#ifdef TAB3
	d_tio.c_oflag = OPOST|ONLCR|TAB3;
#else
#ifdef ONLCR
	d_tio.c_oflag = OPOST|ONLCR;
#else
	d_tio.c_oflag = OPOST;
#endif
#endif
#if defined(macII) || defined(ATT) || defined(CRAY) /* { */
	d_tio.c_cflag = B9600|CS8|CREAD|PARENB|HUPCL;
	d_tio.c_lflag = ISIG|ICANON|ECHO|ECHOE|ECHOK;
#ifdef ECHOKE
	d_tio.c_lflag |= ECHOKE|IEXTEN;
#endif
#ifdef ECHOCTL
	d_tio.c_lflag |= ECHOCTL|IEXTEN;
#endif

#ifndef USE_TERMIOS /* { */
	d_tio.c_line = 0;
#endif /* } */

	d_tio.c_cc[VINTR] = CINTR;
	d_tio.c_cc[VQUIT] = CQUIT;
	d_tio.c_cc[VERASE] = CERASE;
	d_tio.c_cc[VKILL] = CKILL;
	d_tio.c_cc[VEOF] = CEOF;
	d_tio.c_cc[VEOL] = CNUL;
	d_tio.c_cc[VEOL2] = CNUL;
#ifdef VSWTCH
	d_tio.c_cc[VSWTCH] = CNUL;
#endif

#if defined(USE_TERMIOS) || defined(USE_POSIX_TERMIOS) /* { */
	d_tio.c_cc[VSUSP] = CSUSP;
#ifdef VDSUSP
	d_tio.c_cc[VDSUSP] = CDSUSP;
#endif
	d_tio.c_cc[VREPRINT] = CRPRNT;
	d_tio.c_cc[VDISCARD] = CFLUSH;
	d_tio.c_cc[VWERASE] = CWERASE;
	d_tio.c_cc[VLNEXT] = CLNEXT;
	d_tio.c_cc[VMIN] = 1;
	d_tio.c_cc[VTIME] = 0;
#endif /* } */
#ifdef HAS_LTCHARS /* { */
	d_ltc.t_suspc = CSUSP;		/* t_suspc */
	d_ltc.t_dsuspc = CDSUSP;	/* t_dsuspc */
	d_ltc.t_rprntc = CRPRNT;
	d_ltc.t_flushc = CFLUSH;
	d_ltc.t_werasc = CWERASE;
	d_ltc.t_lnextc = CLNEXT;
#endif /* } HAS_LTCHARS */
#ifdef TIOCLSET /* { */
	d_lmode = 0;
#endif /* } TIOCLSET */
#else  /* }{ else !macII, ATT, CRAY */
#ifndef USE_POSIX_TERMIOS
#ifdef BAUD_0 /* { */
	d_tio.c_cflag = CS8|CREAD|PARENB|HUPCL;
#else	/* }{ !BAUD_0 */
	d_tio.c_cflag = B9600|CS8|CREAD|PARENB|HUPCL;
#endif	/* } !BAUD_0 */
#else /* USE_POSIX_TERMIOS */
	d_tio.c_cflag = CS8|CREAD|PARENB|HUPCL;
	cfsetispeed(&d_tio, B9600);
	cfsetospeed(&d_tio, B9600);
#endif
	d_tio.c_lflag = ISIG|ICANON|ECHO|ECHOE|ECHOK;
#ifdef ECHOKE
	d_tio.c_lflag |= ECHOKE|IEXTEN;
#endif
#ifdef ECHOCTL
	d_tio.c_lflag |= ECHOCTL|IEXTEN;
#endif
#ifndef USE_POSIX_TERMIOS
#ifdef NTTYDISC
	d_tio.c_line = NTTYDISC;
#else
	d_tio.c_line = 0;
#endif
#endif /* USE_POSIX_TERMIOS */
#ifdef __sgi
	d_tio.c_cflag &= ~(HUPCL|PARENB);
	d_tio.c_iflag |= BRKINT|ISTRIP|IGNPAR;
#endif
	d_tio.c_cc[VINTR] = CONTROL('C');	/* '^C'	*/
	d_tio.c_cc[VERASE] = 0x7f;		/* DEL	*/
	d_tio.c_cc[VKILL] = CONTROL('U');	/* '^U'	*/
	d_tio.c_cc[VQUIT] = CQUIT;		/* '^\'	*/
	d_tio.c_cc[VEOF] = CEOF;		/* '^D' */
	d_tio.c_cc[VEOL] = CEOL;		/* '^@'	*/
	d_tio.c_cc[VMIN] = 1;
	d_tio.c_cc[VTIME] = 0;
#ifdef VSWTCH
	d_tio.c_cc[VSWTCH] = CSWTCH;            /* usually '^Z' */
#endif
#ifdef VLNEXT
	d_tio.c_cc[VLNEXT] = CLNEXT;
#endif
#ifdef VWERASE
	d_tio.c_cc[VWERASE] = CWERASE;
#endif
#ifdef VREPRINT
	d_tio.c_cc[VREPRINT] = CRPRNT;
#endif
#ifdef VRPRNT
	d_tio.c_cc[VRPRNT] = CRPRNT;
#endif
#ifdef VDISCARD
	d_tio.c_cc[VDISCARD] = CFLUSH;
#endif
#ifdef VFLUSHO
	d_tio.c_cc[VFLUSHO] = CFLUSH;
#endif
#ifdef VSTOP
	d_tio.c_cc[VSTOP] = CSTOP;
#endif
#ifdef VSTART
	d_tio.c_cc[VSTART] = CSTART;
#endif
#ifdef VSUSP
	d_tio.c_cc[VSUSP] = CSUSP;
#endif
#ifdef VDSUSP
	d_tio.c_cc[VDSUSP] = CDSUSP;
#endif
#ifdef VSTATUS
	d_tio.c_cc[VSTATUS] = CSTATUS;
#endif
	/* now, try to inherit tty settings */
	{
	    int i;

	    for (i = 0; i <= 2; i++) {
#ifndef USE_POSIX_TERMIOS
		struct termio deftio;
		if (ioctl (i, TCGETA, &deftio) == 0)
#else
		struct termios deftio;
		if (tcgetattr(i, &deftio) == 0)
#endif
		{
		    d_tio.c_cc[VINTR] = deftio.c_cc[VINTR];
		    d_tio.c_cc[VQUIT] = deftio.c_cc[VQUIT];
		    d_tio.c_cc[VERASE] = deftio.c_cc[VERASE];
		    d_tio.c_cc[VKILL] = deftio.c_cc[VKILL];
		    d_tio.c_cc[VEOF] = deftio.c_cc[VEOF];
		    d_tio.c_cc[VEOL] = deftio.c_cc[VEOL];
#ifdef VSWTCH
		    d_tio.c_cc[VSWTCH] = deftio.c_cc[VSWTCH];
#endif
#ifdef VEOL2
		    d_tio.c_cc[VEOL2] = deftio.c_cc[VEOL2];
#endif
#ifdef VLNEXT
		    d_tio.c_cc[VLNEXT] = deftio.c_cc[VLNEXT];
#endif
#ifdef VWERASE
		    d_tio.c_cc[VWERASE] = deftio.c_cc[VWERASE];
#endif
#ifdef VREPRINT
		    d_tio.c_cc[VREPRINT] = deftio.c_cc[VREPRINT];
#endif
#ifdef VRPRNT
		    d_tio.c_cc[VRPRNT] = deftio.c_cc[VRPRNT];
#endif
#ifdef VDISCARD
		    d_tio.c_cc[VDISCARD] = deftio.c_cc[VDISCARD];
#endif
#ifdef VFLUSHO
		    d_tio.c_cc[VFLUSHO] = deftio.c_cc[VFLUSHO];
#endif
#ifdef VSTOP
		    d_tio.c_cc[VSTOP] = deftio.c_cc[VSTOP];
#endif
#ifdef VSTART
		    d_tio.c_cc[VSTART] = deftio.c_cc[VSTART];
#endif
#ifdef VSUSP
		    d_tio.c_cc[VSUSP] = deftio.c_cc[VSUSP];
#endif
#ifdef VDSUSP
		    d_tio.c_cc[VDSUSP] = deftio.c_cc[VDSUSP];
#endif
#ifdef VSTATUS
		    d_tio.c_cc[VSTATUS] = deftio.c_cc[VSTATUS];
#endif
		    break;
		}
	    }
	}
#ifdef HAS_LTCHARS /* { */
	d_ltc.t_suspc = '\000';		/* t_suspc */
	d_ltc.t_dsuspc = '\000';	/* t_dsuspc */
	d_ltc.t_rprntc = '\377';	/* reserved...*/
	d_ltc.t_flushc = '\377';
	d_ltc.t_werasc = '\377';
	d_ltc.t_lnextc = '\377';
#endif	/* } HAS_LTCHARS */
#if defined(USE_TERMIOS) || defined(USE_POSIX_TERMIOS) /* { */
	d_tio.c_cc[VSUSP] = CSUSP;
#ifdef VDSUSP
	d_tio.c_cc[VDSUSP] = '\000';
#endif
#ifdef VSTATUS
	d_tio.c_cc[VSTATUS] = '\377';
#endif
#ifdef VREPRINT
	d_tio.c_cc[VREPRINT] = '\377';
#endif
#ifdef VDISCARD
	d_tio.c_cc[VDISCARD] = '\377';
#endif
#ifdef VWERASE
	d_tio.c_cc[VWERASE] = '\377';
#endif
#ifdef VLNEXT
	d_tio.c_cc[VLNEXT] = '\377';
#endif
#endif /* } USE_TERMIOS */
#ifdef TIOCLSET /* { */
	d_lmode = 0;
#endif	/* } TIOCLSET */
#endif  /* } macII, ATT, CRAY */
#endif	/* } MINIX, etc */
#endif  /* AMOEBA */

	/* Init the Toolkit. */
	{
#ifdef HAS_SAVED_IDS_AND_SETEUID
	    uid_t euid = geteuid();
	    gid_t egid = getegid();
	    uid_t ruid = getuid();
	    gid_t rgid = getgid();

	    if (setegid(rgid) == -1)
#ifdef __MVS__
	       if (!(errno == EMVSERR)) /* could happen if _BPX_SHAREAS=REUSE */
#endif
		(void) fprintf(stderr, "setegid(%d): %s\n",
			       (int) rgid, strerror(errno));

	    if (seteuid(ruid) == -1)
#ifdef __MVS__
	       if (!(errno == EMVSERR))
#endif
		(void) fprintf(stderr, "seteuid(%d): %s\n",
			       (int) ruid, strerror(errno));
#endif

	    XtSetErrorHandler(xt_error);
	    toplevel = XtAppInitialize (&app_con, "XTerm",
					optionDescList,
					XtNumber(optionDescList),
					&argc, argv, fallback_resources,
					NULL, 0);
	    XtSetErrorHandler((XtErrorHandler)0);

	    XtGetApplicationResources(toplevel, (XtPointer) &resource,
				      application_resources,
				      XtNumber(application_resources), NULL, 0);

#ifdef HAS_SAVED_IDS_AND_SETEUID
	    if (seteuid(euid) == -1)
#ifdef __MVS__
	       if (!(errno == EMVSERR))
#endif
		(void) fprintf(stderr, "seteuid(%d): %s\n",
			       (int) euid, strerror(errno));

	    if (setegid(egid) == -1)
#ifdef __MVS__
	       if (!(errno == EMVSERR))
#endif
		(void) fprintf(stderr, "setegid(%d): %s\n",
			       (int) egid, strerror(errno));
#endif
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
	hold_screen = resource.hold_screen ? 1 : 0;
	xterm_name = resource.xterm_name;
	sunFunctionKeys = resource.sunFunctionKeys;
#if OPT_SUNPC_KBD
	sunKeyboard = resource.sunKeyboard;
#endif
#if OPT_HP_FUNC_KEYS
	hpFunctionKeys = resource.hpFunctionKeys;
#endif
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
#if defined(TIOCCONS) || defined(SRIOCSREDIR)
#ifndef __sgi
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
#else /* __sgi */
		Console = TRUE;
#endif /* __sgi */
#endif	/* TIOCCONS */
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
		    resource.title = base_name (command_to_exec[0]);
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

#ifndef MINIX
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
#if defined(USE_SYSV_TERMIO) && !defined(SVR4) && !defined(linux)
		/* SYSV has another pointer which should be part of the
		** FILE structure but is actually a separate array.
		*/
		unsigned char *old_bufend;

		old_bufend = (unsigned char *) _bufend(stderr);
#ifdef __hpux
		stderr->__fileH = (i >> 8);
		stderr->__fileL = i;
#else
		stderr->_file = i;
#endif
		_bufend(stderr) = old_bufend;
#else	/* USE_SYSV_TERMIO */
#ifndef linux
		stderr->_file = i;
#else
		setfileno(stderr, i);
#endif
#endif	/* USE_SYSV_TERMIO */

		/* mark this file as close on exec */
		(void) fcntl(i, F_SETFD, 1);
	}
    }
#endif	/* DEBUG */
#endif /* MINIX */

	/* open a terminal for client */
	get_terminal ();

	spawn ();

	/* Child process is out there, let's catch its termination */
	(void) signal (SIGCHLD, reapchild);

	/* Realize procs have now been executed */

#ifndef AMOEBA
	if (am_slave) { /* Write window id so master end can read and use */
	    char buf[80];

	    buf[0] = '\0';
	    sprintf (buf, "%lx\n", XtWindow (XtParent (CURRENT_EMU(screen))));
	    write (screen->respond, buf, strlen (buf));
	}
#endif /* !AMOEBA */

	screen->inhibit = inhibit;

#ifdef AIXV3
#if (OSMAJORVERSION < 4)
	/* In AIXV3, xterms started from /dev/console have CLOCAL set.
	 * This means we need to clear CLOCAL so that SIGHUP gets sent
	 * to the slave-pty process when xterm exits.
	 */

	{
	    struct termio tio;

	    if(ioctl(screen->respond, TCGETA, &tio) == -1)
		SysError(ERROR_TIOCGETP);

	    tio.c_cflag &= ~(CLOCAL);

	    if (ioctl (screen->respond, TCSETA, &tio) == -1)
		SysError(ERROR_TIOCSETP);
	}
#endif
#endif
#ifndef AMOEBA
#ifdef MINIX
	if ((mode = fcntl(screen->respond, F_GETFD, 0)) == -1)
		Error(1);
	mode |= FD_ASYNCHIO;
	if (fcntl(screen->respond, F_SETFD, mode) == -1)
		Error(1);
	nbio_register(screen->respond);
#elif defined(USE_SYSV_TERMIO) || defined(__MVS__)
	if (0 > (mode = fcntl(screen->respond, F_GETFL, 0)))
		Error(1);
#ifdef O_NDELAY
	mode |= O_NDELAY;
#else
	mode |= O_NONBLOCK;
#endif /* O_NDELAY */
	if (fcntl(screen->respond, F_SETFL, mode))
		Error(1);
#else	/* !MINIX && !USE_SYSV_TERMIO */
	mode = 1;
	if (ioctl (screen->respond, FIONBIO, (char *)&mode) == -1) SysError (ERROR_FIONBIO);
#endif	/* MINIX, etc */
#endif  /* AMOEBA */

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
}

static char *
base_name(char *name)
{
	register char *cp;

	cp = strrchr(name, '/');
	return(cp ? cp + 1 : name);
}

#ifndef AMOEBA
/* This function opens up a pty master and stuffs its value into pty.
 * If it finds one, it returns a value of 0.  If it does not find one,
 * it returns a value of !0.  This routine is designed to be re-entrant,
 * so that if a pty master is found and later, we find that the slave
 * has problems, we can re-enter this function and get another one.
 */

static int
get_pty (int *pty)
{
#if defined(__osf__) || (defined(__GLIBC__) && !defined(USE_USG_PTYS))
	int tty;
	return (openpty(pty, &tty, ttydev, NULL, NULL));
#elif (defined(SYSV) && defined(i386) && !defined(SVR4)) || defined(__QNXNTO__)
	/*
	  The order of this code is *important*.  On SYSV/386 we want to open
	  a /dev/ttyp? first if at all possible.  If none are available, then
	  we'll try to open a /dev/pts??? device.

	  The reason for this is because /dev/ttyp? works correctly, where
	  as /dev/pts??? devices have a number of bugs, (won't update
	  screen correcly, will hang -- it more or less works, but you
	  really don't want to use it).

	  Most importantly, for boxes of this nature, one of the major
	  "features" is that you can emulate a 8086 by spawning off a UNIX
	  program on 80386/80486 in v86 mode.  In other words, you can spawn
	  off multiple MS-DOS environments.  On ISC the program that does
	  this is named "vpix."  The catcher is that "vpix" will *not* work
	  with a /dev/pts??? device, will only work with a /dev/ttyp? device.

	  Since we can open either a /dev/ttyp? or a /dev/pts??? device,
	  the flag "IsPts" is set here so that we know which type of
	  device we're dealing with in routine spawn().  That's the reason
	  for the "if (IsPts)" statement in spawn(); we have two different
	  device types which need to be handled differently.
	  */
	if (pty_search(pty) == 0)
	    return 0;
	return 1;
#elif defined(USE_USG_PTYS) || defined(__CYGWIN__)
#ifdef __GLIBC__ /* if __GLIBC__ and USE_USG_PTYS, we know glibc >= 2.1 */
	/* GNU libc 2 allows us to abstract away from having to know the
	   master pty device name. */
	if ((*pty = getpt()) < 0) {
	    return 1;
	}
	strcpy(ttydev, ptsname(*pty));
#elif defined(__MVS__)
	return pty_search(pty);
#else
	if ((*pty = open ("/dev/ptmx", O_RDWR)) < 0) {
	    return 1;
	}
#endif
#if defined(SVR4) || defined(SCO325) || (defined(i386) && defined(SYSV))
	strcpy(ttydev, ptsname(*pty));
#if defined (SYSV) && defined(i386) && !defined(SVR4)
	IsPts = True;
#endif
#endif
	return 0;
#elif defined(AIXV3)
	if ((*pty = open ("/dev/ptc", O_RDWR)) < 0) {
	    return 1;
	}
	strcpy(ttydev, ttyname(*pty));
	return 0;
#elif defined(__sgi) && (OSMAJORVERSION >= 4)
	char    *tty_name;

	tty_name = _getpty (pty, O_RDWR, 0622, 0);
	if (tty_name == 0)
	    return 1;
	strcpy (ttydev, tty_name);
	return 0;
#elif defined(__convex__)
	char *pty_name, *getpty();

	while ((pty_name = getpty()) != NULL) {
	    if ((*pty = open (pty_name, O_RDWR)) >= 0) {
		strcpy(ptydev, pty_name);
		strcpy(ttydev, pty_name);
		ttydev[5] = 't';
		return 0;
	    }
	}
	return 1;
#elif defined(USE_GET_PSEUDOTTY)
	return ((*pty = getpseudotty (&ttydev, &ptydev)) >= 0 ? 0 : 1);
#elif (defined(__sgi) && (OSMAJORVERSION < 4)) || (defined(umips) && defined (SYSTYPE_SYSV))
	struct stat fstat_buf;

	*pty = open ("/dev/ptc", O_RDWR);
	if (*pty < 0 || (fstat (*pty, &fstat_buf)) < 0) {
	  return(1);
	}
	sprintf (ttydev, "/dev/ttyq%d", minor(fstat_buf.st_rdev));
#ifndef __sgi
	sprintf (ptydev, "/dev/ptyq%d", minor(fstat_buf.st_rdev));
	if ((*tty = open (ttydev, O_RDWR)) < 0) {
	  close (*pty);
	  return(1);
	}
#endif /* !__sgi */
	/* got one! */
	return(0);
#else /* __sgi or umips */

#ifdef __hpux
	/*
	 * Use the clone device if it works, otherwise use pty_search logic.
	 */
	if ((*pty = open("/dev/ptym/clone", O_RDWR)) >= 0) {
		strcpy(ttydev, ptsname(*pty));
		return(0);
	}
#endif

	return pty_search(pty);

#endif
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
    static int devindex, letter = 0;

#if defined(CRAY) || defined(__MVS__)
    for (; devindex < MAXPTTYS; devindex++) {
	sprintf (ttydev, TTYFORMAT, devindex);
	sprintf (ptydev, PTYFORMAT, devindex);

	if ((*pty = open (ptydev, O_RDWR)) >= 0) {
	    /* We need to set things up for our next entry
	     * into this function!
	     */
	    (void) devindex++;
	    return 0;
	}
    }
#else /* CRAY || __MVS__ */
    while (PTYCHAR1[letter]) {
	ttydev [strlen(ttydev) - 2]  = ptydev [strlen(ptydev) - 2] =
	    PTYCHAR1 [letter];

	while (PTYCHAR2[devindex]) {
	    ttydev [strlen(ttydev) - 1] = ptydev [strlen(ptydev) - 1] =
		PTYCHAR2 [devindex];
	    /* for next time around loop or next entry to this function */
	    devindex++;
	    if ((*pty = open (ptydev, O_RDWR)) >= 0) {
#ifdef sun
		/* Need to check the process group of the pty.
		 * If it exists, then the slave pty is in use,
		 * and we need to get another one.
		 */
		int pgrp_rtn;
		if (ioctl(*pty, TIOCGPGRP, &pgrp_rtn) == 0 || errno != EIO) {
		    close(*pty);
		    continue;
		}
#endif /* sun */
		return 0;
	    }
	}
	devindex = 0;
	(void) letter++;
    }
#endif /* CRAY else */
    /*
     * We were unable to allocate a pty master!  Return an error
     * condition and let our caller terminate cleanly.
     */
    return 1;
}
#endif /* AMOEBA */

static void
get_terminal (void)
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
       siglongjmp(env, 1);
	SIGNAL_RETURN;
}

/*
 * declared outside USE_HANDSHAKE so HsSysError() callers can use
 */
static int pc_pipe[2];	/* this pipe is used for parent to child transfer */
static int cp_pipe[2];	/* this pipe is used for child to parent transfer */

#ifdef USE_HANDSHAKE
typedef enum {		/* c == child, p == parent                        */
	PTY_BAD,	/* c->p: can't open pty slave for some reason     */
	PTY_FATALERROR,	/* c->p: we had a fatal error with the pty        */
	PTY_GOOD,	/* c->p: we have a good pty, let's go on          */
	PTY_NEW,	/* p->c: here is a new pty slave, try this        */
	PTY_NOMORE,	/* p->c; no more pty's, terminate                 */
	UTMP_ADDED,	/* c->p: utmp entry has been added                */
	UTMP_TTYSLOT,	/* c->p: here is my ttyslot                       */
	PTY_EXEC	/* p->c: window has been mapped the first time    */
} status_t;

typedef struct {
	status_t status;
	int error;
	int fatal_error;
	int tty_slot;
	int rows;
	int cols;
	char buffer[1024];
} handshake_t;

/* HsSysError()
 *
 * This routine does the equivalent of a SysError but it handshakes
 * over the errno and error exit to the master process so that it can
 * display our error message and exit with our exit code so that the
 * user can see it.
 */

static void
HsSysError(int pf, int error)
{
	handshake_t handshake;

	handshake.status = PTY_FATALERROR;
	handshake.error = errno;
	handshake.fatal_error = error;
	strcpy(handshake.buffer, ttydev);
	write(pf, (char *) &handshake, sizeof(handshake));
	exit(error);
}

void first_map_occurred (void)
{
    handshake_t handshake;
    register TScreen *screen = &term->screen;

    handshake.status = PTY_EXEC;
    handshake.rows = screen->max_row;
    handshake.cols = screen->max_col;
    write (pc_pipe[1], (char *) &handshake, sizeof(handshake));
    close (cp_pipe[0]);
    close (pc_pipe[1]);
    waiting_for_initial_map = False;
}
#else
/*
 * temporary hack to get xterm working on att ptys
 */
static void
HsSysError(int pf, int error)
{
    fprintf(stderr, "%s: fatal pty error %d (errno=%d) on tty %s\n",
	    xterm_name, error, errno, ttydev);
    exit(error);
}

void first_map_occurred (void)
{
    return;
}
#endif /* USE_HANDSHAKE else !USE_HANDSHAKE */


#ifndef AMOEBA
extern char **environ;

static void
set_owner(char *device, int uid, int gid, int mode)
{
	if (chown (device, uid, gid) < 0) {
		if (errno != ENOENT
		 && getuid() == 0) {
			fprintf(stderr, "Cannot chown %s to %d,%d: %s\n",
				device, uid, gid, strerror(errno));
		}
	}
	chmod (device, mode);
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
#ifdef USE_HANDSHAKE
	handshake_t handshake;
#endif
#if OPT_INITIAL_ERASE
	int initial_erase = VAL_INITIAL_ERASE;
#endif
	int tty = -1;
	int done;
#ifdef USE_SYSV_TERMIO
	struct termio tio;
#ifdef TIOCLSET
	unsigned lmode;
#endif	/* TIOCLSET */
#ifdef HAS_LTCHARS
	struct ltchars ltc;
#endif	/* HAS_LTCHARS */
#elif defined(USE_POSIX_TERMIOS)
	struct termios tio;
#else /* !USE_SYSV_TERMIO && !USE_POSIX_TERMIOS */
	int ldisc = 0;
	int discipline;
	unsigned lmode;
	struct tchars tc;
	struct ltchars ltc;
	struct sgttyb sg;
#ifdef sony
	int jmode;
	struct jtchars jtc;
#endif /* sony */
#endif	/* USE_SYSV_TERMIO */

	char termcap [TERMCAP_SIZE];
	char newtc [TERMCAP_SIZE];
	char *ptr, *shname, *shname_minus;
	int i, no_dev_tty = FALSE;
	char **envnew;		/* new environment */
	int envsize;		/* elements in new environment */
	char buf[64];
	char *TermName = NULL;
#if defined(TIOCSSIZE) && (defined(sun) && !defined(SVR4))
	struct ttysize ts;
#elif defined(TIOCSWINSZ)
	struct winsize ws;
#endif
	struct passwd *pw = NULL;
#ifdef HAVE_UTMP
#if defined(UTMPX_FOR_UTMP)
	struct utmpx utmp;
#else
	struct utmp utmp;
#endif
#ifdef USE_LASTLOG
	struct lastlog lastlog;
#endif	/* USE_LASTLOG */
#endif	/* HAVE_UTMP */

	screen->uid = getuid();
	screen->gid = getgid();

	termcap[0] = '\0';
	newtc[0] = '\0';

#ifdef SIGTTOU
	/* so that TIOCSWINSZ || TIOCSIZE doesn't block */
	signal(SIGTTOU,SIG_IGN);
#endif

	if (am_slave) {
		screen->respond = am_slave;
#ifndef __osf__
		ptydev[strlen(ptydev) - 2] = ttydev[strlen(ttydev) - 2] =
			passedPty[0];
		ptydev[strlen(ptydev) - 1] = ttydev[strlen(ttydev) - 1] =
			passedPty[1];
#endif /* __osf__ */
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
	       if (! sigsetjmp(env, 1)) {
			tty = open ("/dev/tty", O_RDWR, 0);
			alarm(0);
			tty_got_hung = False;
		} else {
			tty_got_hung = True;
			tty = -1;
			errno = ENXIO;
		}
#if OPT_INITIAL_ERASE
		initial_erase = VAL_INITIAL_ERASE;
#endif
		signal(SIGALRM, SIG_DFL);

		/*
		 * Check results and ignore current control terminal if
		 * necessary.  ENXIO is what is normally returned if there is
		 * no controlling terminal, but some systems (e.g. SunOS 4.0)
		 * seem to return EIO.  Solaris 2.3 is said to return EINVAL.
		 */
		no_dev_tty = FALSE;
		if (tty < 0) {
			if (tty_got_hung || errno == ENXIO || errno == EIO ||
			    errno == EINVAL || errno == ENOTTY || errno == EACCES) {
				no_dev_tty = TRUE;
#ifdef HAS_LTCHARS
				ltc = d_ltc;
#endif	/* HAS_LTCHARS */
#ifdef TIOCLSET
				lmode = d_lmode;
#endif	/* TIOCLSET */
#if defined(USE_SYSV_TERMIO) || defined(USE_POSIX_TERMIOS)
				tio = d_tio;
#else	/* not USE_SYSV_TERMIO and not USE_POSIX_TERMIOS */
				sg = d_sg;
				tc = d_tc;
				discipline = d_disipline;
#ifdef sony
				jmode = d_jmode;
				jtc = d_jtc;
#endif /* sony */
#endif	/* USE_SYSV_TERMIO or USE_POSIX_TERMIOS */
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
#ifdef HAS_LTCHARS
			if(ioctl(tty, TIOCGLTC, &ltc) == -1)
				ltc = d_ltc;
#endif	/* HAS_LTCHARS */
#ifdef TIOCLSET
			if(ioctl(tty, TIOCLGET, &lmode) == -1)
				lmode = d_lmode;
#endif	/* TIOCLSET */
#ifdef USE_SYSV_TERMIO
		        if(ioctl(tty, TCGETA, &tio) == -1)
			        tio = d_tio;
#elif defined(USE_POSIX_TERMIOS)
			if (tcgetattr(tty, &tio) == -1)
			        tio = d_tio;
#else   /* !USE_SYSV_TERMIO && !USE_POSIX_TERMIOS */
			if(ioctl(tty, TIOCGETP, (char *)&sg) == -1)
			        sg = d_sg;
			if(ioctl(tty, TIOCGETC, (char *)&tc) == -1)
			        tc = d_tc;
			if(ioctl(tty, TIOCGETD, (char *)&discipline) == -1)
			        discipline = d_disipline;
#ifdef sony
			if(ioctl(tty, TIOCKGET, (char *)&jmode) == -1)
			        jmode = d_jmode;
			if(ioctl(tty, TIOCKGETC, (char *)&jtc) == -1)
				jtc = d_jtc;
#endif /* sony */
#endif	/* USE_SYSV_TERMIO */

#if OPT_INITIAL_ERASE
			if (resource.ptyInitialErase) {
#ifdef USE_SYSV_TERMIO
				initial_erase = tio.c_cc[VERASE];
#elif defined(USE_POSIX_TERMIOS)
				initial_erase = tio.c_cc[VERASE];
#else   /* !USE_SYSV_TERMIO && !USE_POSIX_TERMIOS */
				initial_erase = sg.sg_erase;
#endif	/* USE_SYSV_TERMIO */
			}
			if (resource.backarrow_is_erase)
			if (initial_erase == 127) {	/* see input.c */
				term->keyboard.flags &= ~MODE_DECBKM;
			}
			TRACE(("%s @%d, ptyInitialErase:%d, backspace_is_erase:%d, initial_erase:%d\n",
				__FILE__, __LINE__,
				resource.ptyInitialErase,
				resource.backarrow_is_erase,
				initial_erase))
#endif

#ifdef MINIX
			/* Editing shells interfere with xterms started in
			 * the background.
			 */
			tio = d_tio;
#endif
			close (tty);
			/* tty is no longer an open fd! */
			tty = -1;
		}

#ifdef	PUCC_PTYD
		if(-1 == (screen->respond = openrpty(ttydev, ptydev,
				(resource.utmpInhibit ?  OPTY_NOP : OPTY_LOGIN),
				getuid(), XDisplayString(screen->display))))
#else /* not PUCC_PTYD */
		if (get_pty (&screen->respond))
#endif /* PUCC_PTYD */
		{
			/*  no ptys! */
			(void) fprintf(stderr, "%s: no available ptys: %s\n",
				       xterm_name, strerror(errno));
			exit (ERROR_PTYS);
		}
#ifdef PUCC_PTYD
		  else {
			/*
			 *  set the fd of the master in a global var so
			 *  we can undo all this on exit
			 *
			 */
			Ptyfd = screen->respond;
		  }
#endif /* PUCC_PTYD */
	}

	/* avoid double MapWindow requests */
	XtSetMappedWhenManaged(XtParent(CURRENT_EMU(screen)), False );

	wm_delete_window = XInternAtom(XtDisplay(toplevel), "WM_DELETE_WINDOW",
				       False);

	if (!TEK4014_ACTIVE(screen))
	    VTInit();		/* realize now so know window size for tty driver */
#if defined(TIOCCONS) || defined(SRIOCSREDIR)
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
#endif
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

#if OPT_INITIAL_ERASE
	TRACE(("%s @%d, resource ptyInitialErase:%d, backspace_is_erase:%d\n",
		__FILE__, __LINE__,
		resource.ptyInitialErase,
		resource.backarrow_is_erase))
	if (!resource.ptyInitialErase) {
		char temp[1024], *p = temp;
		char *s = tgetstr(TERMCAP_ERASE, &p);
		TRACE(("extracting initial_erase value from termcap\n"))
		if (s != 0) {
			if (*s == '^') {
				if (*++s == '?') {
					initial_erase = 127;
				} else {
					initial_erase = CONTROL(*s);
				}
			} else if (*s == '\\') {
				char *d;
				int value = strtol(s, &d, 8);
				if (value > 0 && d != s)
					initial_erase = value;
			} else {
				initial_erase = *s;
			}
			initial_erase = CharOf(initial_erase);
			TRACE(("... initial_erase:%d\n", initial_erase))
		}
	}
	if (resource.backarrow_is_erase && initial_erase == 127) {
		/* see input.c */
		term->keyboard.flags &= ~MODE_DECBKM;
	}
#endif

#if defined(TIOCSSIZE) && (defined(sun) && !defined(SVR4))
	/* tell tty how big window is */
	if(TEK4014_ACTIVE(screen)) {
		ts.ts_lines = 38;
		ts.ts_cols = 81;
	} else {
		ts.ts_lines = screen->max_row + 1;
		ts.ts_cols = screen->max_col + 1;
	}
#elif defined(TIOCSWINSZ)
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
#endif	/* sun vs TIOCSWINSZ */

	if (!am_slave) {
#ifdef USE_HANDSHAKE
	    if (pipe(pc_pipe) || pipe(cp_pipe))
		SysError (ERROR_FORK);
#endif
	    if ((screen->pid = fork ()) == -1)
		SysError (ERROR_FORK);

	    if (screen->pid == 0) {
		/*
		 * now in child process
		 */
		TRACE_CHILD
#if defined(_POSIX_SOURCE) || defined(SVR4) || defined(__convex__) || defined(SCO325) || defined(__QNX__)
		int pgrp = setsid();
#else
		int pgrp = getpid();
#endif
#if defined(HAVE_UTMP) && defined(USE_SYSV_UTMP)
		char* ptyname;
		char* ptynameptr = 0;
#endif

#ifdef USE_USG_PTYS
#if defined(SYSV) && defined(i386) && !defined(SVR4)
		if (IsPts) {	/* SYSV386 supports both, which did we open? */
#endif /* SYSV && i386 && !SVR4 */
		int ptyfd;

		setpgrp();
		grantpt (screen->respond);
		unlockpt (screen->respond);
		if ((ptyfd = open (ptsname(screen->respond), O_RDWR)) < 0) {
		    SysError (1);
		}
#ifdef I_PUSH
		if (ioctl (ptyfd, I_PUSH, "ptem") < 0) {
		    SysError (2);
		}
#if !defined(SVR4) && !(defined(SYSV) && defined(i386))
		if (!getenv("CONSEM") && ioctl (ptyfd, I_PUSH, "consem") < 0) {
		    SysError (3);
		}
#endif /* !SVR4 */
		if (ioctl (ptyfd, I_PUSH, "ldterm") < 0) {
		    SysError (4);
		}
#ifdef SVR4			/* from Sony */
		if (ioctl (ptyfd, I_PUSH, "ttcompat") < 0) {
		    SysError (5);
		}
#endif /* SVR4 */
#endif /* I_PUSH */
		tty = ptyfd;
		close (screen->respond);
#ifdef TIOCSWINSZ
		/* tell tty how big window is */
#if OPT_TEK4014
		if(TEK4014_ACTIVE(screen)) {
			ws.ws_row = 24;
			ws.ws_col = 80;
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
#endif
#if defined(SYSV) && defined(i386) && !defined(SVR4)
		} else {	/* else pty, not pts */
#endif /* SYSV && i386 && !SVR4 */
#endif /* USE_USG_PTYS */

#ifdef USE_HANDSHAKE		/* warning, goes for a long ways */
		/* close parent's sides of the pipes */
		close (cp_pipe[0]);
		close (pc_pipe[1]);

		/* Make sure that our sides of the pipes are not in the
		 * 0, 1, 2 range so that we don't fight with stdin, out
		 * or err.
		 */
		if (cp_pipe[1] <= 2) {
			if ((i = fcntl(cp_pipe[1], F_DUPFD, 3)) >= 0) {
				(void) close(cp_pipe[1]);
				cp_pipe[1] = i;
			}
		}
		if (pc_pipe[0] <= 2) {
			if ((i = fcntl(pc_pipe[0], F_DUPFD, 3)) >= 0) {
				(void) close(pc_pipe[0]);
				pc_pipe[0] = i;
			}
		}

		/* we don't need the socket, or the pty master anymore */
		close (ConnectionNumber(screen->display));
		close (screen->respond);

		/* Now is the time to set up our process group and
		 * open up the pty slave.
		 */
#ifdef	USE_SYSV_PGRP
#if defined(CRAY) && (OSMAJORVERSION > 5)
		(void) setsid();
#else
		(void) setpgrp();
#endif
#endif /* USE_SYSV_PGRP */

#if defined(__QNX__) && !defined(__QNXNTO__)
		qsetlogin( getlogin(), ttydev );
#endif
		while (1) {
#if defined(TIOCNOTTY) && !((__GLIBC__ > 2) || ((__GLIBC__ == 2) && (__GLIBC_MINOR__ >= 1)))
			if (!no_dev_tty && (tty = open ("/dev/tty", O_RDWR)) >= 0) {
				ioctl (tty, TIOCNOTTY, (char *) NULL);
				close (tty);
			}
#endif /* TIOCNOTTY && !glibc >= 2.1 */
#ifdef CSRG_BASED
			(void)revoke(ttydev);
#endif
			if ((tty = open(ttydev, O_RDWR, 0)) >= 0) {
#if defined(CRAY) && defined(TCSETCTTY)
			    /* make /dev/tty work */
			    ioctl(tty, TCSETCTTY, 0);
#endif
#ifdef	USE_SYSV_PGRP
				/* We need to make sure that we are acutally
				 * the process group leader for the pty.  If
				 * we are, then we should now be able to open
				 * /dev/tty.
				 */
				if ((i = open("/dev/tty", O_RDWR, 0)) >= 0) {
					/* success! */
					close(i);
					break;
				}
#else	/* USE_SYSV_PGRP */
				break;
#endif	/* USE_SYSV_PGRP */
			}

#ifdef TIOCSCTTY
			ioctl(tty, TIOCSCTTY, 0);
#endif
			/* let our master know that the open failed */
			handshake.status = PTY_BAD;
			handshake.error = errno;
			strcpy(handshake.buffer, ttydev);
			write(cp_pipe[1], (char *) &handshake,
			    sizeof(handshake));

			/* get reply from parent */
			i = read(pc_pipe[0], (char *) &handshake,
			    sizeof(handshake));
			if (i <= 0) {
				/* parent terminated */
				exit(1);
			}

			if (handshake.status == PTY_NOMORE) {
				/* No more ptys, let's shutdown. */
				exit(1);
			}

			/* We have a new pty to try */
			free(ttydev);
			ttydev = (char *)malloc((unsigned)
			    (strlen(handshake.buffer) + 1));
			if (ttydev == NULL) {
			    SysError(ERROR_SPREALLOC);
			}
			strcpy(ttydev, handshake.buffer);
		}

		/* use the same tty name that everyone else will use
		 * (from ttyname)
		 */
		if ((ptr = ttyname(tty)) != 0)
		{
			/* it may be bigger */
			ttydev = (char *)realloc (ttydev,
				(unsigned) (strlen(ptr) + 1));
			if (ttydev == NULL) {
			    SysError(ERROR_SPREALLOC);
			}
			(void) strcpy(ttydev, ptr);
		}
#if defined(SYSV) && defined(i386) && !defined(SVR4)
		} /* end of IsPts else clause */
#endif /* SYSV && i386 && !SVR4 */

#endif /* USE_HANDSHAKE -- from near fork */

#ifdef USE_TTY_GROUP
	{
		struct group *ttygrp;
		if ((ttygrp = getgrnam("tty")) != 0) {
			/* change ownership of tty to real uid, "tty" gid */
			set_owner (ttydev, screen->uid, ttygrp->gr_gid,
				   (resource.messages? 0620 : 0600));
		}
		else {
			/* change ownership of tty to real group and user id */
			set_owner (ttydev, screen->uid, screen->gid,
				   (resource.messages? 0622 : 0600));
		}
		endgrent();
	}
#else /* else !USE_TTY_GROUP */
		/* change ownership of tty to real group and user id */
		set_owner (ttydev, screen->uid, screen->gid,
			   (resource.messages? 0622 : 0600));
#endif /* USE_TTY_GROUP */

		/*
		 * set up the tty modes
		 */
		{
#if defined(USE_SYSV_TERMIO) || defined(USE_POSIX_TERMIOS)
#if defined(umips) || defined(CRAY) || defined(linux)
		    /* If the control tty had its modes screwed around with,
		       eg. by lineedit in the shell, or emacs, etc. then tio
		       will have bad values.  Let's just get termio from the
		       new tty and tailor it.  */
		    if (ioctl (tty, TCGETA, &tio) == -1)
		      SysError (ERROR_TIOCGETP);
		    tio.c_lflag |= ECHOE;
#endif /* umips */
		    /* Now is also the time to change the modes of the
		     * child pty.
		     */
		    /* input: nl->nl, don't ignore cr, cr->nl */
		    tio.c_iflag &= ~(INLCR|IGNCR);
		    tio.c_iflag |= ICRNL;
		    /* ouput: cr->cr, nl is not return, no delays, ln->cr/nl */
#ifndef USE_POSIX_TERMIOS
		    tio.c_oflag &=
		     ~(OCRNL|ONLRET|NLDLY|CRDLY|TABDLY|BSDLY|VTDLY|FFDLY);
#endif /* USE_POSIX_TERMIOS */
#ifdef ONLCR
		    tio.c_oflag |= ONLCR;
#endif /* ONLCR */
#ifdef OPOST
		    tio.c_oflag |= OPOST;
#endif /* OPOST */
#ifdef MINIX	/* should be ifdef _POSIX_SOURCE */
		    cfsetispeed(&tio, B9600);
		    cfsetospeed(&tio, B9600);
#else /* !MINIX */
#ifndef USE_POSIX_TERMIOS
#ifdef BAUD_0
		    /* baud rate is 0 (don't care) */
		    tio.c_cflag &= ~(CBAUD);
#else	/* !BAUD_0 */
		    /* baud rate is 9600 (nice default) */
		    tio.c_cflag &= ~(CBAUD);
		    tio.c_cflag |= B9600;
#endif	/* !BAUD_0 */
#else /* USE_POSIX_TERMIOS */
		    cfsetispeed(&tio, B9600);
		    cfsetospeed(&tio, B9600);
#ifdef __MVS__
		    /* turn off bits that can't be set from the slave side */
		    tio.c_cflag &= ~(PACKET|PKT3270|PTU3270|PKTXTND);
#endif /* __MVS__ */
		    /* Clear CLOCAL so that SIGHUP is sent to us
		       when the xterm ends */
		    tio.c_cflag &= ~CLOCAL;
#endif /* USE_POSIX_TERMIOS */
#endif /* MINIX */
		    tio.c_cflag &= ~CSIZE;
		    if (screen->input_eight_bits)
			tio.c_cflag |= CS8;
		    else
			tio.c_cflag |= CS7;
		    /* enable signals, canonical processing (erase, kill, etc),
		    ** echo
		    */
		    tio.c_lflag |= ISIG|ICANON|ECHO|ECHOE|ECHOK;
#ifdef ECHOKE
		    tio.c_lflag |= ECHOKE|IEXTEN;
#endif
#ifdef ECHOCTL
		    tio.c_lflag |= ECHOCTL|IEXTEN;
#endif
#ifndef __MVS__
		    /* reset EOL to default value */
		    tio.c_cc[VEOL] = CEOL;			/* '^@' */
		    /* certain shells (ksh & csh) change EOF as well */
		    tio.c_cc[VEOF] = CEOF;			/* '^D' */
#else
		    if(tio.c_cc[VEOL]==0) tio.c_cc[VEOL] = CEOL;			/* '^@' */
		    if(tio.c_cc[VEOF]==0) tio.c_cc[VEOF] = CEOF;			/* '^D' */
#endif
#ifdef VLNEXT
		    tio.c_cc[VLNEXT] = CLNEXT;
#endif
#ifdef VWERASE
		    tio.c_cc[VWERASE] = CWERASE;
#endif
#ifdef VREPRINT
		    tio.c_cc[VREPRINT] = CRPRNT;
#endif
#ifdef VRPRNT
		    tio.c_cc[VRPRNT] = CRPRNT;
#endif
#ifdef VDISCARD
		    tio.c_cc[VDISCARD] = CFLUSH;
#endif
#ifdef VFLUSHO
		    tio.c_cc[VFLUSHO] = CFLUSH;
#endif
#ifdef VSTOP
		    tio.c_cc[VSTOP] = CSTOP;
#endif
#ifdef VSTART
		    tio.c_cc[VSTART] = CSTART;
#endif
#ifdef VSUSP
		    tio.c_cc[VSUSP] = CSUSP;
#endif
#ifdef VDSUSP
		    tio.c_cc[VDSUSP] = CDSUSP;
#endif
		    if (override_tty_modes) {
			/* sysv-specific */
			TMODE (XTTYMODE_intr, tio.c_cc[VINTR]);
			TMODE (XTTYMODE_quit, tio.c_cc[VQUIT]);
			TMODE (XTTYMODE_erase, tio.c_cc[VERASE]);
			TMODE (XTTYMODE_kill, tio.c_cc[VKILL]);
			TMODE (XTTYMODE_eof, tio.c_cc[VEOF]);
			TMODE (XTTYMODE_eol, tio.c_cc[VEOL]);
#ifdef VSWTCH
			TMODE (XTTYMODE_swtch, tio.c_cc[VSWTCH]);
#endif
#ifdef VSUSP
			TMODE (XTTYMODE_susp, tio.c_cc[VSUSP]);
#endif
#ifdef VDSUSP
			TMODE (XTTYMODE_dsusp, tio.c_cc[VDSUSP]);
#endif
#ifdef VREPRINT
			TMODE (XTTYMODE_rprnt, tio.c_cc[VREPRINT]);
#endif
#ifdef VRPRNT
			TMODE (XTTYMODE_rprnt, tio.c_cc[VRPRNT]);
#endif
#ifdef VDISCARD
			TMODE (XTTYMODE_flush, tio.c_cc[VDISCARD]);
#endif
#ifdef VFLUSHO
			TMODE (XTTYMODE_flush, tio.c_cc[VFLUSHO]);
#endif
#ifdef VWERASE
			TMODE (XTTYMODE_weras, tio.c_cc[VWERASE]);
#endif
#ifdef VLNEXT
			TMODE (XTTYMODE_lnext, tio.c_cc[VLNEXT]);
#endif
#ifdef VSTART
			TMODE (XTTYMODE_start, tio.c_cc[VSTART]);
#endif
#ifdef VSTOP
			TMODE (XTTYMODE_stop, tio.c_cc[VSTOP]);
#endif
#ifdef VSTATUS
			TMODE (XTTYMODE_status, tio.c_cc[VSTATUS]);
#endif
#ifdef HAS_LTCHARS
			/* both SYSV and BSD have ltchars */
			TMODE (XTTYMODE_susp, ltc.t_suspc);
			TMODE (XTTYMODE_dsusp, ltc.t_dsuspc);
			TMODE (XTTYMODE_rprnt, ltc.t_rprntc);
			TMODE (XTTYMODE_flush, ltc.t_flushc);
			TMODE (XTTYMODE_weras, ltc.t_werasc);
			TMODE (XTTYMODE_lnext, ltc.t_lnextc);
#endif
		    }

#ifdef HAS_LTCHARS
#ifdef __hpux
		    /* ioctl chokes when the "reserved" process group controls
		     * are not set to _POSIX_VDISABLE */
		    ltc.t_rprntc = ltc.t_rprntc = ltc.t_flushc =
		    ltc.t_werasc = ltc.t_lnextc = _POSIX_VDISABLE;
#endif /* __hpux */
		    if (ioctl (tty, TIOCSLTC, &ltc) == -1)
			    HsSysError(cp_pipe[1], ERROR_TIOCSETC);
#endif	/* HAS_LTCHARS */
#ifdef TIOCLSET
		    if (ioctl (tty, TIOCLSET, (char *)&lmode) == -1)
			    HsSysError(cp_pipe[1], ERROR_TIOCLSET);
#endif	/* TIOCLSET */
#ifndef USE_POSIX_TERMIOS
		    if (ioctl (tty, TCSETA, &tio) == -1)
			    HsSysError(cp_pipe[1], ERROR_TIOCSETP);
#else   /* USE_POSIX_TERMIOS */
		    if (tcsetattr (tty, TCSANOW, &tio) == -1)
			    HsSysError(cp_pipe[1], ERROR_TIOCSETP);
#endif  /* USE_POSIX_TERMIOS */
#else	/* USE_SYSV_TERMIO or USE_POSIX_TERMIOS */
		    sg.sg_flags &= ~(ALLDELAY | XTABS | CBREAK | RAW);
		    sg.sg_flags |= ECHO | CRMOD;
		    /* make sure speed is set on pty so that editors work right*/
		    sg.sg_ispeed = B9600;
		    sg.sg_ospeed = B9600;
		    /* reset t_brkc to default value */
		    tc.t_brkc = -1;
#ifdef LPASS8
		    if (screen->input_eight_bits)
			lmode |= LPASS8;
		    else
			lmode &= ~(LPASS8);
#endif
#ifdef sony
		    jmode &= ~KM_KANJI;
#endif /* sony */

		    ltc = d_ltc;

		    if (override_tty_modes) {
			TMODE (XTTYMODE_intr, tc.t_intrc);
			TMODE (XTTYMODE_quit, tc.t_quitc);
			TMODE (XTTYMODE_erase, sg.sg_erase);
			TMODE (XTTYMODE_kill, sg.sg_kill);
			TMODE (XTTYMODE_eof, tc.t_eofc);
			TMODE (XTTYMODE_start, tc.t_startc);
			TMODE (XTTYMODE_stop, tc.t_stopc);
			TMODE (XTTYMODE_brk, tc.t_brkc);
			/* both SYSV and BSD have ltchars */
			TMODE (XTTYMODE_susp, ltc.t_suspc);
			TMODE (XTTYMODE_dsusp, ltc.t_dsuspc);
			TMODE (XTTYMODE_rprnt, ltc.t_rprntc);
			TMODE (XTTYMODE_flush, ltc.t_flushc);
			TMODE (XTTYMODE_weras, ltc.t_werasc);
			TMODE (XTTYMODE_lnext, ltc.t_lnextc);
		    }

		    if (ioctl (tty, TIOCSETP, (char *)&sg) == -1)
			    HsSysError (cp_pipe[1], ERROR_TIOCSETP);
		    if (ioctl (tty, TIOCSETC, (char *)&tc) == -1)
			    HsSysError (cp_pipe[1], ERROR_TIOCSETC);
		    if (ioctl (tty, TIOCSETD, (char *)&discipline) == -1)
			    HsSysError (cp_pipe[1], ERROR_TIOCSETD);
		    if (ioctl (tty, TIOCSLTC, (char *)&ltc) == -1)
			    HsSysError (cp_pipe[1], ERROR_TIOCSLTC);
		    if (ioctl (tty, TIOCLSET, (char *)&lmode) == -1)
			    HsSysError (cp_pipe[1], ERROR_TIOCLSET);
#ifdef sony
		    if (ioctl (tty, TIOCKSET, (char *)&jmode) == -1)
			    HsSysError (cp_pipe[1], ERROR_TIOCKSET);
		    if (ioctl (tty, TIOCKSETC, (char *)&jtc) == -1)
			    HsSysError (cp_pipe[1], ERROR_TIOCKSETC);
#endif /* sony */
#endif	/* !USE_SYSV_TERMIO */
#if defined(TIOCCONS) || defined(SRIOCSREDIR)
		    if (Console) {
#ifdef TIOCCONS
			int on = 1;
			if (ioctl (tty, TIOCCONS, (char *)&on) == -1)
			    fprintf(stderr, "%s: cannot open console: %s\n",
				    xterm_name, strerror(errno));
#endif
#ifdef SRIOCSREDIR
			int fd = open("/dev/console",O_RDWR);
			if (fd == -1 || ioctl (fd, SRIOCSREDIR, tty) == -1)
			    fprintf(stderr, "%s: cannot open console: %s\n",
				    xterm_name, strerror(errno));
			(void) close (fd);
#endif
		    }
#endif	/* TIOCCONS */
		}

		signal (SIGCHLD, SIG_DFL);
#ifdef USE_SYSV_SIGHUP
		/* watch out for extra shells (I don't understand either) */
		signal (SIGHUP, SIG_DFL);
#else
		signal (SIGHUP, SIG_IGN);
#endif
		/* restore various signals to their defaults */
		signal (SIGINT, SIG_DFL);
		signal (SIGQUIT, SIG_DFL);
		signal (SIGTERM, SIG_DFL);

#if OPT_INITIAL_ERASE
		TRACE(("%s @%d, ptyInitialErase:%d, overide_tty_modes:%d, XTTYMODE_erase:%d\n",
			__FILE__, __LINE__,
			resource.ptyInitialErase,
			override_tty_modes,
			ttymodelist[XTTYMODE_erase].set))
		if (! resource.ptyInitialErase
		 && !override_tty_modes
		 && !ttymodelist[XTTYMODE_erase].set) {
#ifdef USE_SYSV_TERMIO
			if(ioctl(tty, TCGETA, &tio) == -1)
				tio = d_tio;
			tio.c_cc[VERASE] = initial_erase;
			ioctl(tty, TCSETA, &tio);
#elif defined(USE_POSIX_TERMIOS)
			if (tcgetattr(tty, &tio) == -1)
				tio = d_tio;
			tio.c_cc[VERASE] = initial_erase;
			tcsetattr(tty, TCSANOW, &tio);
#else   /* !USE_SYSV_TERMIO && !USE_POSIX_TERMIOS */
			if(ioctl(tty, TIOCGETP, (char *)&sg) == -1)
				sg = d_sg;
			sg.sg_erase = initial_erase;
			ioctl(tty, TIOCSETP, (char *)&sg);
#endif	/* USE_SYSV_TERMIO */
		}
#endif

		/* copy the environment before Setenving */
		for (i = 0 ; environ [i] != NULL ; i++)
		    ;
		/* compute number of Setenv() calls below */
		envsize = 1;	/* (NULL terminating entry) */
		envsize += 3;	/* TERM, WINDOWID, DISPLAY */
#ifdef HAVE_UTMP
		envsize += 1;   /* LOGNAME */
#endif /* HAVE_UTMP */
#ifdef USE_SYSV_ENVVARS
		envsize += 2;	/* COLUMNS, LINES */
#ifdef HAVE_UTMP
		envsize += 2;   /* HOME, SHELL */
#endif /* HAVE_UTMP */
#ifdef OWN_TERMINFO_DIR
		envsize += 1;	/* TERMINFO */
#endif
#else /* USE_SYSV_ENVVARS */
		envsize += 1;	/* TERMCAP */
#endif /* USE_SYSV_ENVVARS */
		envnew = (char **) calloc ((unsigned) i + envsize, sizeof(char *));
		memmove( (char *)envnew, (char *)environ, i * sizeof(char *));
		environ = envnew;
		Setenv ("TERM=", TermName);
		if(!TermName)
			*newtc = 0;

		sprintf (buf, "%lu",
			 ((unsigned long) XtWindow (XtParent(CURRENT_EMU(screen)))));
		Setenv ("WINDOWID=", buf);

		/* put the display into the environment of the shell*/
		Setenv ("DISPLAY=", XDisplayString (screen->display));

		signal(SIGTERM, SIG_DFL);

#ifndef AMOEBA
		/* this is the time to go and set up stdin, out, and err
		 */
		{
#if defined(CRAY) && (OSMAJORVERSION >= 6)
		    (void) close(tty);
		    (void) close(0);

		    if (open ("/dev/tty", O_RDWR)) {
			fprintf(stderr, "cannot open /dev/tty: %s\n", strerror(errno));
			exit(1);
		    }
		    (void) close(1);
		    (void) close(2);
		    dup(0);
		    dup(0);
#else
		    /* dup the tty */
		    for (i = 0; i <= 2; i++)
			if (i != tty) {
			    (void) close(i);
			    (void) dup(tty);
			}

#ifndef ATT
		    /* and close the tty */
		    if (tty > 2)
			(void) close(tty);
#endif
#endif /* CRAY */
		}

#ifndef	USE_SYSV_PGRP
#ifdef TIOCSCTTY
		setsid();
		ioctl(0, TIOCSCTTY, 0);
#endif
		ioctl(0, TIOCSPGRP, (char *)&pgrp);
#ifndef __osf__
		setpgrp(0,0);
#else
		setpgid(0,0);
#endif
		close(open(ttydev, O_WRONLY, 0));
#ifndef __osf__
		setpgrp (0, pgrp);
#else
		setpgid (0, pgrp);
#endif
#endif /* !USE_SYSV_PGRP */

#if defined(__QNX__)
		tcsetpgrp( 0, pgrp /*setsid()*/ );
#endif

#endif /* AMOEBA */

#ifdef Lynx
		{
		struct termio	t;
		if (ioctl(0, TCGETA, &t) >= 0)
		{
			/* this gets lost somewhere on our way... */
			t.c_oflag |= OPOST;
			ioctl(0, TCSETA, &t);
		}
		}
#endif

#ifdef HAVE_UTMP
		pw = getpwuid(screen->uid);
		if (pw && pw->pw_name)
		    Setenv ("LOGNAME=", pw->pw_name); /* for POSIX */
#ifdef USE_SYSV_UTMP
		/* Set up our utmp entry now.  We need to do it here
		** for the following reasons:
		**   - It needs to have our correct process id (for
		**     login).
		**   - If our parent was to set it after the fork(),
		**     it might make it out before we need it.
		**   - We need to do it before we go and change our
		**     user and group id's.
		*/
#ifdef CRAY
#define PTYCHARLEN 4
#endif

#ifdef __osf__
#define PTYCHARLEN 5
#endif

#ifndef PTYCHARLEN
#define PTYCHARLEN 2
#endif

		(void) setutent ();
		/* set up entry to search for */
		ptyname = ttydev;
		bzero(&utmp, sizeof(utmp));
#ifndef __sgi
		if (PTYCHARLEN >= (int)strlen(ptyname))
		    ptynameptr = ptyname;
		else
		    ptynameptr = ptyname + strlen(ptyname) - PTYCHARLEN;
#else
		ptynameptr = ptyname + sizeof("/dev/tty")-1;
#endif
		(void) strncpy(utmp.ut_id, ptynameptr, sizeof (utmp.ut_id));

		utmp.ut_type = DEAD_PROCESS;

		/* position to entry in utmp file */
		(void) getutid(&utmp);

		/* set up the new entry */
		utmp.ut_type = USER_PROCESS;
#ifdef HAVE_UTMP_UT_XSTATUS
		utmp.ut_xstatus = 2;
#endif
		(void) strncpy(utmp.ut_user,
			       (pw && pw->pw_name) ? pw->pw_name : "????",
			       sizeof(utmp.ut_user));

		/* why are we copying this string again? look up 16 lines. */
		(void)strncpy(utmp.ut_id, ptynameptr, sizeof(utmp.ut_id));
		(void) strncpy (utmp.ut_line,
			ptyname + strlen("/dev/"), sizeof (utmp.ut_line));

#ifdef HAVE_UTMP_UT_HOST
		(void) strncpy(buf, DisplayString(screen->display),
			       sizeof(buf));
#ifndef linux
	        {
		    char *disfin = strrchr(buf, ':');
		    if (disfin)
			*disfin = '\0';
		}
#endif
		(void) strncpy(utmp.ut_host, buf, sizeof(utmp.ut_host));
#endif
		(void) strncpy(utmp.ut_name, pw->pw_name,
			       sizeof(utmp.ut_name));

		utmp.ut_pid = getpid();
#if defined(HAVE_UTMP_UT_XTIME)
#if defined(HAVE_UTMP_UT_SESSION)
		utmp.ut_session = getsid(0);
#endif
		utmp.ut_xtime = time ((time_t *) 0);
		utmp.ut_tv.tv_usec = 0;
#else
		utmp.ut_time = time ((time_t *) 0);
#endif

		/* write out the entry */
		if (!resource.utmpInhibit)
		    (void) pututline(&utmp);
#ifdef WTMP
#if defined(SVR4) || defined(SCO325)
		if (term->misc.login_shell)
		    updwtmpx(WTMPX_FILE, &utmp);
#elif defined(linux) && defined(__GLIBC__) && (__GLIBC__ >= 2) && !(defined(__powerpc__) && (__GLIBC__ == 2) && (__GLIBC_MINOR__ == 0))
		if (term->misc.login_shell)
		    updwtmp(etc_wtmp, &utmp);
#else
		if (term->misc.login_shell &&
		     (i = open(etc_wtmp, O_WRONLY|O_APPEND)) >= 0) {
		    write(i, (char *)&utmp, sizeof(struct utmp));
		    close(i);
		}
#endif
#endif
		/* close the file */
		(void) endutent();

#else	/* USE_SYSV_UTMP */
		/* We can now get our ttyslot!  We can also set the initial
		 * utmp entry.
		 */
		tslot = ttyslot();
		added_utmp_entry = False;
		{
			if (pw && !resource.utmpInhibit &&
			    (i = open(etc_utmp, O_WRONLY)) >= 0) {
				bzero((char *)&utmp, sizeof(struct utmp));
				(void) strncpy(utmp.ut_line,
					       ttydev + strlen("/dev/"),
					       sizeof(utmp.ut_line));
				(void) strncpy(utmp.ut_name, pw->pw_name,
					       sizeof(utmp.ut_name));
#ifdef MINIX
				utmp.ut_pid = getpid();
				utmp.ut_type = USER_PROCESS;
#endif /* MINIX */
#ifdef HAVE_UTMP_UT_HOST
				(void) strncpy(utmp.ut_host,
					       XDisplayString (screen->display),
					       sizeof(utmp.ut_host));
#endif
				/* cast needed on Ultrix 4.4 */
				time((time_t*)&utmp.ut_time);
				lseek(i, (long)(tslot * sizeof(struct utmp)), 0);
				write(i, (char *)&utmp, sizeof(struct utmp));
				close(i);
				added_utmp_entry = True;
#if defined(WTMP)
				if (term->misc.login_shell &&
				(i = open(etc_wtmp, O_WRONLY|O_APPEND)) >= 0) {
				    int status;
				    status = write(i, (char *)&utmp,
						   sizeof(struct utmp));
				    status = close(i);
				}
#elif defined(MNX_LASTLOG)
				if (term->misc.login_shell &&
				(i = open(_U_LASTLOG, O_WRONLY)) >= 0) {
				    lseek(i, (long)(screen->uid *
					sizeof (struct utmp)), 0);
				    write(i, (char *)&utmp,
					sizeof (struct utmp));
				    close(i);
				}
#endif /* WTMP or MNX_LASTLOG */
			} else
				tslot = -tslot;
		}

		/* Let's pass our ttyslot to our parent so that it can
		 * clean up after us.
		 */
#ifdef USE_HANDSHAKE
		handshake.tty_slot = tslot;
#endif /* USE_HANDSHAKE */
#endif /* USE_SYSV_UTMP */

#ifdef USE_LASTLOG
				if (term->misc.login_shell &&
				(i = open(etc_lastlog, O_WRONLY)) >= 0) {
				    bzero((char *)&lastlog,
					sizeof (struct lastlog));
				    (void) strncpy(lastlog.ll_line, ttydev +
					sizeof("/dev"),
					sizeof (lastlog.ll_line));
				    (void) strncpy(lastlog.ll_host,
					  XDisplayString (screen->display),
					  sizeof (lastlog.ll_host));
				    time(&lastlog.ll_time);
				    lseek(i, (long)(screen->uid *
					sizeof (struct lastlog)), 0);
				    write(i, (char *)&lastlog,
					sizeof (struct lastlog));
				    close(i);
				}
#endif /* USE_LASTLOG */

#ifdef USE_HANDSHAKE
		/* Let our parent know that we set up our utmp entry
		 * so that it can clean up after us.
		 */
		handshake.status = UTMP_ADDED;
		handshake.error = 0;
		strcpy(handshake.buffer, ttydev);
		(void)write(cp_pipe[1], (char *)&handshake, sizeof(handshake));
#endif /* USE_HANDSHAKE */
#endif/* HAVE_UTMP */

		(void) setgid (screen->gid);
#ifdef HAS_BSD_GROUPS
		if (geteuid() == 0 && pw)
		  initgroups (pw->pw_name, pw->pw_gid);
#endif
		(void) setuid (screen->uid);

#ifdef USE_HANDSHAKE
		/* mark the pipes as close on exec */
		fcntl(cp_pipe[1], F_SETFD, 1);
		fcntl(pc_pipe[0], F_SETFD, 1);

		/* We are at the point where we are going to
		 * exec our shell (or whatever).  Let our parent
		 * know we arrived safely.
		 */
		handshake.status = PTY_GOOD;
		handshake.error = 0;
		(void)strcpy(handshake.buffer, ttydev);
		(void)write(cp_pipe[1], (char *)&handshake, sizeof(handshake));

		if (waiting_for_initial_map) {
		    i = read (pc_pipe[0], (char *) &handshake,
			      sizeof(handshake));
		    if (i != sizeof(handshake) ||
			handshake.status != PTY_EXEC) {
			/* some very bad problem occurred */
			exit (ERROR_PTY_EXEC);
		    }
		    if(handshake.rows > 0 && handshake.cols > 0) {
			screen->max_row = handshake.rows;
			screen->max_col = handshake.cols;
#if defined(TIOCSSIZE) && (defined(sun) && !defined(SVR4))
			ts.ts_lines = screen->max_row + 1;
			ts.ts_cols = screen->max_col + 1;
#elif defined(TIOCSWINSZ)
			ws.ws_row = screen->max_row + 1;
			ws.ws_col = screen->max_col + 1;
			ws.ws_xpixel = FullWidth(screen);
			ws.ws_ypixel = FullHeight(screen);
#endif /* sun vs TIOCSWINSZ */
		    }
		}
#endif /* USE_HANDSHAKE */

#ifdef USE_SYSV_ENVVARS
		{
		char numbuf[12];
		sprintf (numbuf, "%d", screen->max_col + 1);
		Setenv("COLUMNS=", numbuf);
		sprintf (numbuf, "%d", screen->max_row + 1);
		Setenv("LINES=", numbuf);
		}
#ifdef HAVE_UTMP
		if (pw) {	/* SVR4 doesn't provide these */
		    if (!getenv("HOME"))
			Setenv("HOME=", pw->pw_dir);
		    if (!getenv("SHELL"))
			Setenv("SHELL=", pw->pw_shell);
		}
#endif /* HAVE_UTMP */
#ifdef OWN_TERMINFO_DIR
		Setenv("TERMINFO=", OWN_TERMINFO_DIR);
#endif
#else /* USE_SYSV_ENVVARS */
		if(!TEK4014_ACTIVE(screen) && *newtc) {
		    strcpy (termcap, newtc);
		    resize (screen, termcap, newtc);
		}
		if (term->misc.titeInhibit) {
		    remove_termcap_entry (newtc, "ti=");
		    remove_termcap_entry (newtc, "te=");
		}
		/*
		 * work around broken termcap entries */
		if (resource.useInsertMode)	{
		    remove_termcap_entry (newtc, "ic=");
		    /* don't get duplicates */
		    remove_termcap_entry (newtc, "im=");
		    remove_termcap_entry (newtc, "ei=");
		    remove_termcap_entry (newtc, "mi");
		    if(*newtc)
			strcat (newtc, ":im=\\E[4h:ei=\\E[4l:mi:");
		}
#if OPT_INITIAL_ERASE
		if (*newtc) {
		    remove_termcap_entry (newtc, TERMCAP_ERASE "=");
		    sprintf(newtc + strlen(newtc), ":%s=\\%03o", TERMCAP_ERASE, initial_erase & 0377);
		}
#endif
		if(*newtc)
		    Setenv ("TERMCAP=", newtc);
#endif /* USE_SYSV_ENVVARS */


		/* need to reset after all the ioctl bashing we did above */
#if defined(TIOCSSIZE) && (defined(sun) && !defined(SVR4))
		i = ioctl (0, TIOCSSIZE, &ts);
		TRACE(("spawn TIOCSSIZE %dx%d return %d\n", ts.ts_lines, ts.ts_cols, i))
#elif defined(TIOCSWINSZ)
		i = ioctl (0, TIOCSWINSZ, (char *)&ws);
		TRACE(("spawn TIOCSWINSZ %dx%d return %d\n", ws.ws_row, ws.ws_col, i))
#else
		TRACE(("spawn cannot tell pty its size\n"))
#endif	/* sun vs TIOCSWINSZ */

		signal(SIGHUP, SIG_DFL);
		if (command_to_exec) {
			execvp(*command_to_exec, command_to_exec);
			/* print error message on screen */
			fprintf(stderr, "%s: Can't execvp %s: %s\n",
				xterm_name, *command_to_exec, strerror(errno));
		}

#ifdef USE_SYSV_SIGHUP
		/* fix pts sh hanging around */
		signal (SIGHUP, SIG_DFL);
#endif

#ifdef HAVE_UTMP
		if(((ptr = getenv("SHELL")) == NULL || *ptr == 0) &&
		 ((pw == NULL && (pw = getpwuid(screen->uid)) == NULL) ||
		 *(ptr = pw->pw_shell) == 0))
#else	/* HAVE_UTMP */
		if(((ptr = getenv("SHELL")) == NULL || *ptr == 0) &&
		 ((pw = getpwuid(screen->uid)) == NULL ||
		 *(ptr = pw->pw_shell) == 0))
#endif	/* HAVE_UTMP */
			ptr = "/bin/sh";
		if ((shname = strrchr(ptr, '/')) != 0)
			shname++;
		else
			shname = ptr;
		shname_minus = (char *)malloc(strlen(shname) + 2);
		(void) strcpy(shname_minus, "-");
		(void) strcat(shname_minus, shname);
#if !defined(USE_SYSV_TERMIO) && !defined(USE_POSIX_TERMIOS)
		ldisc = XStrCmp("csh", shname + strlen(shname) - 3) == 0 ?
		 NTTYDISC : 0;
		ioctl(0, TIOCSETD, (char *)&ldisc);
#endif	/* !USE_SYSV_TERMIO && !USE_POSIX_TERMIOS */

#ifdef USE_LOGIN_DASH_P
		if (term->misc.login_shell && pw && added_utmp_entry)
		  execl (bin_login, "login", "-p", "-f", pw->pw_name, 0);
#endif
		execlp (ptr, (term->misc.login_shell ? shname_minus : shname),
			0);

		/* Exec failed. */
		fprintf (stderr, "%s: Could not exec %s: %s\n", xterm_name,
			ptr, strerror(errno));
		(void) sleep(5);
		exit(ERROR_EXEC);
	    }				/* end if in child after fork */

#ifdef USE_HANDSHAKE
	    /* Parent process.  Let's handle handshaked requests to our
	     * child process.
	     */

	    /* close childs's sides of the pipes */
	    close (cp_pipe[1]);
	    close (pc_pipe[0]);

	    for (done = 0; !done; ) {
		if (read(cp_pipe[0], (char *) &handshake, sizeof(handshake)) <= 0) {
			/* Our child is done talking to us.  If it terminated
			 * due to an error, we will catch the death of child
			 * and clean up.
			 */
			break;
		}

		switch(handshake.status) {
		case PTY_GOOD:
			/* Success!  Let's free up resources and
			 * continue.
			 */
			done = 1;
			break;

		case PTY_BAD:
			/* The open of the pty failed!  Let's get
			 * another one.
			 */
			(void) close(screen->respond);
			if (get_pty(&screen->respond)) {
			    /* no more ptys! */
			    fprintf(stderr,
				    "%s: child process can find no available ptys: %s\n",
				    xterm_name, strerror(errno));
			    handshake.status = PTY_NOMORE;
			    write(pc_pipe[1], (char *) &handshake, sizeof(handshake));
			    exit (ERROR_PTYS);
			}
			handshake.status = PTY_NEW;
			(void) strcpy(handshake.buffer, ttydev);
			write(pc_pipe[1], (char *) &handshake, sizeof(handshake));
			break;

		case PTY_FATALERROR:
			errno = handshake.error;
			close(cp_pipe[0]);
			close(pc_pipe[1]);
			SysError(handshake.fatal_error);

		case UTMP_ADDED:
			/* The utmp entry was set by our slave.  Remember
			 * this so that we can reset it later.
			 */
			added_utmp_entry = True;
#ifndef	USE_SYSV_UTMP
			tslot = handshake.tty_slot;
#endif	/* USE_SYSV_UTMP */
			free(ttydev);
			ttydev = (char *)malloc((unsigned) strlen(handshake.buffer) + 1);
			strcpy(ttydev, handshake.buffer);
			break;
		default:
			fprintf(stderr, "%s: unexpected handshake status %d\n",
			        xterm_name, handshake.status);
		}
	    }
	    /* close our sides of the pipes */
	    if (!waiting_for_initial_map) {
		close (cp_pipe[0]);
		close (pc_pipe[1]);
	    }
#endif /* USE_HANDSHAKE */
	}				/* end if no slave */

	/*
	 * still in parent (xterm process)
	 */

#ifdef USE_SYSV_SIGHUP
	/* hung sh problem? */
	signal (SIGHUP, SIG_DFL);
#else
	signal (SIGHUP, SIG_IGN);
#endif

/*
 * Unfortunately, System V seems to have trouble divorcing the child process
 * from the process group of xterm.  This is a problem because hitting the
 * INTR or QUIT characters on the keyboard will cause xterm to go away if we
 * don't ignore the signals.  This is annoying.
 */

#if defined(USE_SYSV_SIGNALS) && !defined(SIGTSTP)
	signal (SIGINT, SIG_IGN);

#ifndef SYSV
	/* hung shell problem */
	signal (SIGQUIT, SIG_IGN);
#endif
	signal (SIGTERM, SIG_IGN);
#elif defined(SYSV) || defined(__osf__)
	/* if we were spawned by a jobcontrol smart shell (like ksh or csh),
	 * then our pgrp and pid will be the same.  If we were spawned by
	 * a jobcontrol dumb shell (like /bin/sh), then we will be in our
	 * parent's pgrp, and we must ignore keyboard signals, or we will
	 * tank on everything.
	 */
	if (getpid() == getpgrp()) {
	    (void) signal(SIGINT, Exit);
	    (void) signal(SIGQUIT, Exit);
	    (void) signal(SIGTERM, Exit);
	} else {
	    (void) signal(SIGINT, SIG_IGN);
	    (void) signal(SIGQUIT, SIG_IGN);
	    (void) signal(SIGTERM, SIG_IGN);
	}
	(void) signal(SIGPIPE, Exit);
#else	/* SYSV */
	signal (SIGINT, Exit);
	signal (SIGQUIT, Exit);
	signal (SIGTERM, Exit);
	signal (SIGPIPE, Exit);
#endif /* USE_SYSV_SIGNALS and not SIGTSTP */

	return 0;
}							/* end spawn */
#else  /* AMOEBA */
/* manifest constants */
#define	TTY_NTHREADS		2
#define	TTY_INQSIZE		2000
#define	TTY_OUTQSIZE		1000
#define	TTY_THREAD_STACKSIZE	4096

#define	XWATCHDOG_THREAD_SIZE	4096

/* acceptable defaults */
#define	DEF_HOME		"/home"
#define	DEF_SHELL		"/bin/sh"
#define	DEF_PATH		"/bin:/usr/bin:/profile/util"

extern capability ttycap;
extern char **environ;
extern struct caplist *capv;

/*
 * Set capability.
 * I made this a function since it cannot be a macro.
 */
void
setcap(struct caplist *capvec, int n, char *name, capability *cap)
{
    capvec[n].cl_name = name;
    capvec[n].cl_cap = cap;
}

/*
 * Find process descriptor for specified program,
 * necessarily running down the user's PATH.
 */
errstat
find_program(char *program, capability *programcap)
{
    errstat err;

    if ((err = name_lookup(program, programcap)) != STD_OK) {
	char *path, *name;
	char programpath[1024];

	if ((path = getenv("PATH")) == NULL)
	    path = DEF_PATH;
	if ((name = strrchr(program, '/')) != NULL)
	    name++;
	else
	    name = program;

	do {
	    register char *p = programpath;
	    register char *n = name;
	    char *c1 = path;

	    while (*path && *path != ':')
		*p++ = *path++;
	    if (path != c1) *p++ = '/';
	    if (*path) path++;
	    while (*n) *p++ = *n++;
	    *p = '\0';
	    if ((err = name_lookup(programpath, programcap)) == STD_OK)
		break;
	} while (*path);
    }
    return err;
}

/* Semaphore on which the main thread blocks until it can do something
 * useful (which is made known by a call to WakeupMainThread()).
 */
static semaphore main_sema;

void
InitMainThread(void)
{
    sema_init(&main_sema, 0);
}

void
WakeupMainThread(void)
{
    sema_up(&main_sema);
}

/*
 * Spawn off tty threads and fork the login process.
 */
static int spawn(void)
{
    register TScreen *screen = &term->screen;
    char *TermName = NULL;
    char termcap[TERMCAP_SIZE];
    char newtc[TERMCAP_SIZE];
    char **envnew;		/* new environment */
    int envsize;		/* elements in new environment */
    char *ptr;
    int i, n, ncap;
    errstat err;
    struct caplist *cl;
    char buf[64];
    struct caplist *capvnew;
    int ttythread();
    int xwatchdogthread();

    screen->pid = 2;		/* at least > 1 */
    screen->uid = getuid();
    screen->gid = getgid();
    screen->respond = OPEN_MAX + 1;
    screen->tty_inq = cb_alloc(TTY_INQSIZE);
    screen->tty_outq = cb_alloc(TTY_OUTQSIZE);

    InitMainThread();
    if (!thread_newthread(xwatchdogthread, XWATCHDOG_THREAD_SIZE, 0, 0)) {
	fprintf(stderr, "%s:  unable to start tty thread.\n", ProgramName);
	Exit(1);
    }

    /*
     * Start tty threads. Ordinarily two should suffice, one for standard
     * input and one for standard (error) output.
     */
    ttyinit((char *) NULL);
    for (i = 0; i < TTY_NTHREADS; i++) {
	if (!thread_newthread(ttythread, TTY_THREAD_STACKSIZE, 0, 0)) {
	    fprintf(stderr, "%s:  unable to start tty thread.\n", ProgramName);
	    Exit(1);
	}
    }

    /* avoid double MapWindow requests */
    XtSetMappedWhenManaged( XtParent(CURRENT_EMU(screen)), False );
    wm_delete_window = XInternAtom(XtDisplay(toplevel), "WM_DELETE_WINDOW",
					False);

    /* realize now so know window size for tty driver */
    if (!TEK4014_ACTIVE(screen)) VTInit();

#if OPT_TEK4014
    if (screen->TekEmu) {
	envnew = tekterm;
	ptr = newtc;
    } else
#endif
    {
	envnew = vtterm;
	ptr = termcap;
    }

    *ptr = 0;
    TermName = NULL;
    if (resource.term_name) {
	TermName = resource.term_name;
	if (tgetent (ptr, resource.term_name) == 1) {
	    if (*ptr)
		if (!TEK4014_ACTIVE(screen))
		    resize (screen, termcap, newtc);
	}
    }

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

    /*
     * Setup new capability environment. The whole point of the game is
     * to redirect the shell's stdin/stdout/stderr and tty to our own
     * tty server instead of the initial one.
     */
    for (ncap = 4, cl = capv; cl->cl_name != (char *)NULL; cl++)
	if (strcmp("STDIN", cl->cl_name) && strcmp("STDOUT", cl->cl_name)
	  && strcmp("STDERR", cl->cl_name) && strcmp("TTY", cl->cl_name))
	    ncap++;

    capvnew = (struct caplist *)
	calloc((unsigned) ncap + 1, sizeof(struct caplist));
    setcap(capvnew, 0, "STDIN", &ttycap);
    setcap(capvnew, 1, "STDOUT", &ttycap);
    setcap(capvnew, 2, "STDERR", &ttycap);
    setcap(capvnew, 3, "TTY", &ttycap);
    for (n = 4, cl = capv; cl->cl_name != (char *) NULL; cl++) {
	if (strcmp("STDIN", cl->cl_name)
	  && strcmp("STDOUT", cl->cl_name)
	  && strcmp("STDERR", cl->cl_name)
	  && strcmp("TTY", cl->cl_name))
	    setcap(capvnew, n++, cl->cl_name, cl->cl_cap);
    }
    setcap(capvnew, ncap, (char *)NULL, (capability *)NULL);
    if (n != ncap) {
	fprintf(stderr, "%s: bad capability set.\n", ProgramName);
	Exit(1);
    }

    /*
     * Setup environment variables. We add some extra ones to denote
     * window id, terminal type, display name, termcap entry, and some
     * standard one (which are required by every shell) HOME and SHELL.
     * Note that the two shell variables COLUMNS and LINES are not needed
     * under Amoeba since the tty server provides an RPC to query the
     * window sizes.
     */
    /* copy the environment before Setenving */
    for (i = 0 ; environ[i] != NULL ; i++)
	;

    /* compute number of Setenv() calls below */
    envsize = 1;	/* (NULL terminating entry) */
    envsize += 3;	/* TERM, WINDOWID, DISPLAY */
    envsize += 2;	/* HOME, SHELL */
    envsize += 1;	/* TERMCAP */
    envnew = (char **) calloc ((unsigned) i + envsize, sizeof(char *));
    bcopy((char *)environ, (char *)envnew, i * sizeof(char *));
    environ = envnew;
    Setenv ("TERM=", TermName);
    if(!TermName) *newtc = 0;

    sprintf (buf, "%lu",
	((unsigned long) XtWindow (XtParent(CURRENT_EMU(screen)))));
    Setenv ("WINDOWID=", buf);

    /* put the display into the environment of the shell*/
    Setenv ("DISPLAY=", XDisplayString (screen->display));

    /* always provide a HOME and SHELL definition */
    if (!getenv("HOME")) Setenv("HOME=", DEF_HOME);
    if (!getenv("SHELL")) Setenv("SHELL=", DEF_SHELL);

    if(!TEK4014_ACTIVE(screen) && *newtc) {
	strcpy (termcap, newtc);
	resize (screen, termcap, newtc);
    }
    if (term->misc.titeInhibit) {
	remove_termcap_entry (newtc, "ti=");
	remove_termcap_entry (newtc, "te=");
    }
    /* work around broken termcap entries */
    if (resource.useInsertMode) {
	remove_termcap_entry (newtc, "ic=");
	/* don't get duplicates */
	remove_termcap_entry (newtc, "im=");
	remove_termcap_entry (newtc, "ei=");
	remove_termcap_entry (newtc, "mi");
	if (*newtc)
	    strcat (newtc, ":im=\\E[4h:ei=\\E[4l:mi:");
    }
    if (*newtc)
	Setenv ("TERMCAP=", newtc);

    /*
     * Execute specified program or shell. Use find_program to
     * simulate the same behaviour as the original execvp.
     */
    if (command_to_exec) {
	capability programcap;

	if (find_program(*command_to_exec, &programcap) != STD_OK) {
	    fprintf(stderr, "%s: Could not find %s!\n",
		xterm_name, *command_to_exec);
	    exit(ERROR_EXEC);
	}

	err = exec_file(&programcap, NILCAP, &ttycap, 0,
	    command_to_exec, envnew, capvnew, &screen->proccap);
	if (err != STD_OK) {
	    fprintf(stderr, "%s: Could not exec %s!\n",
		xterm_name, *command_to_exec);
	    exit(ERROR_EXEC);
	}
    } else {
	char *shell, *shname, *shname_minus;
	capability shellcap;
	char *argvec[2];

	if ((shell = getenv("SHELL")) == NULL)
	    shell = DEF_SHELL; /* "cannot happen" */
	if ((shname = strrchr(shell, '/')) != NULL)
	    shname++;
	else
	    shname = shell;

	shname_minus = malloc(strlen(shname) + 2);
	(void) strcpy(shname_minus, "-");
	(void) strcat(shname_minus, shname);

	argvec[0] = term->misc.login_shell ? shname_minus : shname;
	argvec[1] = NULL;

	if (find_program(shell, &shellcap) != STD_OK) {
	    fprintf(stderr, "%s: Could not find %s!\n", xterm_name, shell);
	    exit(ERROR_EXEC);
	}

	err = exec_file(&shellcap, NILCAP, &ttycap, 0, argvec,
	    envnew, capvnew, &screen->proccap);
	if (err != STD_OK) {
	    fprintf(stderr, "%s: Could not exec %s!\n", xterm_name, shell);
	    exit(ERROR_EXEC);
	}

	free(shname_minus);
    }
    free(capvnew);

    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    signal(SIGPIPE, Exit);
    return 0;
}

/*
 * X watch-dog thread. This thread unblocks the main
 * thread when there's an X event.
 */
xwatchdogthread(void)
{
    register TScreen *screen = &term->screen;

    for (;;) {
	int n = _X11TransAmSelect(ConnectionNumber(screen->display), 10);
	if (n < 0 && errno != EINTR) {
	    fprintf(stderr, "%s: X watch dog: Xselect failed: %s\n",
		ProgramName, SysErrorMsg(errno));
	    Cleanup(1);
	} else if (n > 0)
	    WakeupMainThread();
	threadswitch();
    }
}

void
SleepMainThread(void)
{
    int remaining;

    /* Wait for at least one event */
    sema_down(&main_sema);

    /* Since the main thread will continue handling all outstanding events
     * shortly, we can ignore the remaining wakeups that were done.
     */
    if ((remaining = sema_level(&main_sema)) > 1) {
	sema_mdown(&main_sema, remaining);
    }
}
#endif /* AMOEBA */

SIGNAL_T
Exit(int n)
{
	register TScreen *screen = &term->screen;
#ifdef HAVE_UTMP
#ifdef USE_SYSV_UTMP
#if defined(UTMPX_FOR_UTMP)
	struct utmpx utmp;
	struct utmpx *utptr;
#else
	struct utmp utmp;
	struct utmp *utptr;
#endif
	char* ptyname;
	char* ptynameptr = 0;
#if defined(WTMP) && !defined(SVR4) && !(defined(linux) && defined(__GLIBC__) && (__GLIBC__ >= 2) && !(defined(__powerpc__) && (__GLIBC__ == 2) && (__GLIBC_MINOR__ == 0)))
	int fd;			/* for /etc/wtmp */
	int i;
#endif

	/* don't do this more than once */
	if (xterm_exiting)
	    SIGNAL_RETURN;
	xterm_exiting = True;

#ifdef PUCC_PTYD
	closepty(ttydev, ptydev, (resource.utmpInhibit ?  OPTY_NOP : OPTY_LOGIN), Ptyfd);
#endif /* PUCC_PTYD */

	/* cleanup the utmp entry we forged earlier */
	if (!resource.utmpInhibit
#ifdef USE_HANDSHAKE		/* without handshake, no way to know */
	    && added_utmp_entry
#endif /* USE_HANDSHAKE */
	    ) {
	    ptyname = ttydev;
	    utmp.ut_type = USER_PROCESS;
	    if (PTYCHARLEN >= (int)strlen(ptyname))
		ptynameptr = ptyname;
	    else
		ptynameptr = ptyname + strlen(ptyname) - PTYCHARLEN;
	    (void) strncpy(utmp.ut_id, ptynameptr, sizeof(utmp.ut_id));
	    (void) setutent();
	    utptr = getutid(&utmp);
	    /* write it out only if it exists, and the pid's match */
	    if (utptr && (utptr->ut_pid == screen->pid)) {
		    utptr->ut_type = DEAD_PROCESS;
#if defined(HAVE_UTMP_UT_XTIME)
#if defined(HAVE_UTMP_UT_SESSION)
		    utptr->ut_session = getsid(0);
#endif
		    utptr->ut_xtime = time ((time_t *) 0);
		    utptr->ut_tv.tv_usec = 0;
#else
		    *utptr->ut_user = 0;
		    utptr->ut_time = time((time_t *) 0);
#endif
		    (void) pututline(utptr);
#ifdef WTMP
#if defined(SVR4) || defined(SCO325)
		    if (term->misc.login_shell)
			updwtmpx(WTMPX_FILE, utptr);
#elif defined(linux) && defined(__GLIBC__) && (__GLIBC__ >= 2) && !(defined(__powerpc__) && (__GLIBC__ == 2) && (__GLIBC_MINOR__ == 0))
		    strncpy (utmp.ut_line, utptr->ut_line, sizeof (utmp.ut_line));
		    if (term->misc.login_shell)
			updwtmp(etc_wtmp, utptr);
#else
		    /* set wtmp entry if wtmp file exists */
		    if (term->misc.login_shell &&
			(fd = open(etc_wtmp, O_WRONLY | O_APPEND)) >= 0) {
		      i = write(fd, utptr, sizeof(*utptr));
		      i = close(fd);
		    }
#endif
#endif

	    }
	    (void) endutent();
	}
#else	/* not USE_SYSV_UTMP */
	register int wfd;
	struct utmp utmp;

	if (!resource.utmpInhibit && added_utmp_entry &&
	    (!am_slave && tslot > 0 && (wfd = open(etc_utmp, O_WRONLY)) >= 0)){
		bzero((char *)&utmp, sizeof(struct utmp));
		lseek(wfd, (long)(tslot * sizeof(struct utmp)), 0);
		write(wfd, (char *)&utmp, sizeof(struct utmp));
		close(wfd);
#ifdef WTMP
		if (term->misc.login_shell &&
		    (wfd = open(etc_wtmp, O_WRONLY | O_APPEND)) >= 0) {
			register int i;
			(void) strncpy(utmp.ut_line, ttydev +
			    sizeof("/dev"), sizeof (utmp.ut_line));
			time(&utmp.ut_time);
			i = write(wfd, (char *)&utmp, sizeof(struct utmp));
			i = close(wfd);
		}
#endif /* WTMP */
	}
#endif	/* USE_SYSV_UTMP */
#endif	/* HAVE_UTMP */
#ifndef AMOEBA
	close(screen->respond); /* close explicitly to avoid race with slave side */
#endif
#ifdef ALLOWLOGGING
	if(screen->logging)
		CloseLog(screen);
#endif

#ifndef AMOEBA
	if (!am_slave) {
		/* restore ownership of tty and pty */
		set_owner (ttydev, 0, 0, 0666);
#if (!defined(__sgi) && !defined(__osf__) && !defined(__hpux))
		set_owner (ptydev, 0, 0, 0666);
#endif
	}
#endif /* AMOEBA */
	exit(n);
	SIGNAL_RETURN;
}

/* ARGSUSED */
static void
resize(TScreen *screen, register char *oldtc, register char *newtc)
{
#ifndef USE_SYSV_ENVVARS
	register char *ptr1, *ptr2;
	register size_t i;
	register int li_first = 0;
	register char *temp;

	TRACE(("resize %s\n", oldtc))
	if ((ptr1 = strindex (oldtc, "co#")) == NULL){
		strcat (oldtc, "co#80:");
		ptr1 = strindex (oldtc, "co#");
	}
	if ((ptr2 = strindex (oldtc, "li#")) == NULL){
		strcat (oldtc, "li#24:");
		ptr2 = strindex (oldtc, "li#");
	}
	if(ptr1 > ptr2) {
		li_first++;
		temp = ptr1;
		ptr1 = ptr2;
		ptr2 = temp;
	}
	ptr1 += 3;
	ptr2 += 3;
	strncpy (newtc, oldtc, i = ptr1 - oldtc);
	temp = newtc + i;
	sprintf (temp, "%d", li_first
			? screen->max_row + 1
			: screen->max_col + 1);
	temp += strlen(temp);
	ptr1 = strchr(ptr1, ':');
	strncpy (temp, ptr1, i = ptr2 - ptr1);
	temp += i;
	sprintf (temp, "%d", li_first
			? screen->max_col + 1
			: screen->max_row + 1);
	ptr2 = strchr(ptr2, ':');
	strcat (temp, ptr2);
	TRACE(("   ==> %s\n", newtc))
#endif /* USE_SYSV_ENVVARS */
}

/*
 * Does a non-blocking wait for a child process.  If the system
 * doesn't support non-blocking wait, do nothing.
 * Returns the pid of the child, or 0 or -1 if none or error.
 */
int
nonblocking_wait(void)
{
#ifdef USE_POSIX_WAIT
	pid_t pid;

	pid = waitpid(-1, NULL, WNOHANG);
#elif defined(USE_SYSV_SIGNALS) && (defined(CRAY) || !defined(SIGTSTP))
	/* cannot do non-blocking wait */
	int pid = 0;
#else	/* defined(USE_SYSV_SIGNALS) && (defined(CRAY) || !defined(SIGTSTP)) */
#if defined(Lynx)
	int status;
#else
	union wait status;
#endif
	register int pid;

	pid = wait3 (&status, WNOHANG, (struct rusage *)NULL);
#endif /* USE_POSIX_WAIT else */
	return pid;
}

/* ARGSUSED */
static SIGNAL_T reapchild (int n GCC_UNUSED)
{
    int pid;

    pid = wait(NULL);

#ifdef USE_SYSV_SIGNALS
    /* cannot re-enable signal before waiting for child
     * because then SVR4 loops.  Sigh.  HP-UX 9.01 too.
     */
    (void) signal(SIGCHLD, reapchild);
#endif

    do {
	if (pid == term->screen.pid) {
#ifdef DEBUG
	    if (debug) fputs ("Exiting\n", stderr);
#endif
	    if (!hold_screen)
		Cleanup (0);
	}
    } while ( (pid=nonblocking_wait()) > 0);

    SIGNAL_RETURN;
}

static void
remove_termcap_entry (char *buf, char *str)
{
    char *base = buf;
    char *first = base;
    int count = 0;
    size_t len = strlen(str);

    TRACE(("*** remove_termcap_entry('%s', '%s')\n", str, buf))

    while (*buf != 0) {
	if (!count && !strncmp(buf, str, len)) {
	    while (*buf != 0) {
		if (*buf == '\\')
		    buf++;
		else if (*buf == ':')
		    break;
		if (*buf != 0)
		    buf++;
	    }
	    while ((*first++ = *buf++) != 0)
		;
	    TRACE(("...removed_termcap_entry('%s', '%s')\n", str, base))
	    return;
	} else if (*buf == '\\') {
	    buf++;
	} else if (*buf == ':') {
	    first = buf;
	    count = 0;
	} else if (!isspace(*buf)) {
	    count++;
	}
	if (*buf != 0)
	    buf++;
    }
    TRACE(("...cannot remove\n"))
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
	    c = ((*s == '?') ? 0177 : CONTROL(*s));
	    if (*s == '-') {
#if HAVE_TERMIOS_H && HAVE_TCGETATTR
#  if HAVE_POSIX_VDISABLE
		c = _POSIX_VDISABLE;
#  else
		errno = 0;
		c = fpathconf(0, _PC_VDISABLE);
		if (c == -1) {
		    if (errno != 0)
			continue;	/* skip this (error) */
		    c = 0377;
		}
#  endif
#elif defined(VDISABLE)
		c = VDISABLE;
#else
		continue;		/* ignore */
#endif
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
#ifdef AMOEBA
    /*
     * Since this routine is only used to poll X connections
     * we can use an internal Xlib routine (oh what ugly).
     */
    register TScreen *screen = &term->screen;
    int count;

    if (ConnectionNumber(screen->display) != fd) {
	Panic("Cannot get bytes available");
	return -1;
    }
    return _X11TransAmFdBytesReadable(fd, &count) < 0 ? -1 : count;
#elif defined(FIONREAD)
    long arg;
    ioctl (fd, FIONREAD, (char *) &arg);
    return (int) arg;
#elif defined(__CYGWIN__)
    fd_set set;
    struct timeval timeout = {0, 0};

    FD_ZERO (&set);
    FD_SET (fd, &set);
    if (select (fd+1, &set, NULL, NULL, &timeout) > 0)
      return 1;
    else
      return 0;
#elif defined(MINIX)
    /* The answer doesn't have to correct. Calling nbio_isinprogress is
     * much cheaper than called nbio_select.
     */
    if (nbio_isinprogress(fd, ASIO_READ))
	return 0;
    else
	return 1;
#elif defined(FIORDCK)
    return (ioctl (fd, FIORDCHK, NULL));
#else /* !FIORDCK */
    struct pollfd pollfds[1];

    pollfds[0].fd = fd;
    pollfds[0].events = POLLIN;
    return poll (pollfds, 1, 0);
#endif
}

/* Utility function to try to hide system differences from
   everybody who used to call killpg() */

int
kill_process_group(int pid, int sig)
{
    TRACE(("kill_process_group(pid=%d, sig=%d)\n", pid, sig))
#ifdef AMOEBA
    if (pid != 2) {
	fprintf(stderr, "%s:  unexpected process id %d.\n", ProgramName, pid);
	abort();
    }
    ttysendsig(sig);
#elif defined(SVR4) || defined(SYSV) || !defined(X_NOT_POSIX)
    return kill (-pid, sig);
#else
    return killpg (pid, sig);
#endif /* AMOEBA */
}

#if OPT_EBCDIC
int A2E(int x)
{
    char c;
    c = x;
    __atoe_l(&c,1);
    return c;
}

int E2A(int x)
{
    char c;
    c = x;
    __etoa_l(&c,1);
    return c;
}

char CONTROL(char c)
{
    /* this table was built through trial & error */
    static char ebcdic_control_chars[256]={
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   /* 00 - 07 */
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   /* 08 - 0f */
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   /* 10 - 17 */
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   /* 18 - 1f */
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   /* 20 - 27 */
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   /* 28 - 2f */
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   /* 30 - 37 */
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   /* 38 - 3f */
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   /* 40 - 47 */
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   /* 48 - 4f */
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   /* 50 - 57 */
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   /* 58 - 5f */
                0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00,   /* 60 - 67 */
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07,   /* 68 - 6f */
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   /* 70 - 77 */
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   /* 78 - 7f */
                0x00, 0x01, 0x02, 0x03, 0x37, 0x2d, 0x2e, 0x2f,   /* 80 - 87 */
                0x16, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   /* 88 - 8f */
                0x00, 0x15, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,   /* 90 - 97 */
                0x11, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   /* 98 - 9f */
                0x00, 0x00, 0x13, 0x3c, 0x3d, 0x32, 0x26, 0x18,   /* a0 - a7 */
                0x19, 0x3f, 0x00, 0x00, 0x00, 0x27, 0x00, 0x00,   /* a8 - af */
                0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   /* b0 - b7 */
                0x00, 0x00, 0x00, 0x00, 0x00, 0x1d, 0x00, 0x00,   /* b8 - bf */
                0x00, 0x01, 0x02, 0x03, 0x37, 0x2d, 0x2e, 0x2f,   /* c0 - c7 */
                0x16, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   /* c8 - cf */
                0x00, 0x15, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,   /* d0 - d7 */
                0x11, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   /* d8 - df */
                0x1c, 0x00, 0x13, 0x3c, 0x3d, 0x32, 0x26, 0x18,   /* e0 - e7 */
                0x19, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   /* e8 - ef */
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   /* f0 - f7 */
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  /* f8 - ff */
    return ebcdic_control_chars[CharOf(c)];
}
#endif

#if defined(__QNX__) && !defined(__QNXNTO__)
#include <sys/types.h>
#include <sys/proc_msg.h>
#include <sys/kernel.h>
#include <string.h>
#include <errno.h>

struct _proc_session ps;
struct _proc_session_reply rps;

int qsetlogin( char *login, char *ttyname )
{
	int v = getsid( getpid() );

	memset( &ps, 0, sizeof(ps) );
	memset( &rps, 0, sizeof(rps) );

	ps.type = _PROC_SESSION;
	ps.subtype = _PROC_SUB_ACTION1;
	ps.sid = v;
	strcpy( ps.name, login );

	Send( 1, &ps, &rps, sizeof(ps), sizeof(rps) );

	if ( rps.status < 0 )
		return( rps.status );

	ps.type = _PROC_SESSION;
	ps.subtype = _PROC_SUB_ACTION2;
	ps.sid = v;
	sprintf( ps.name, "//%d%s", getnid(), ttyname );
	Send( 1, &ps, &rps, sizeof(ps), sizeof(rps) );

	return( rps.status );
}
#endif
