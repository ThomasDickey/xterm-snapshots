/*
 *	$XConsortium: resize.c,v 1.34 95/05/24 22:12:04 gildea Exp $
 *	$XFree86: xc/programs/xterm/resize.c,v 3.18.2.3 1997/05/25 05:07:01 dawes Exp $
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


/* resize.c */

#ifdef HAVE_CONFIG_H
#include <xtermcfg.h>
#endif

#include <X11/Xos.h>
#include <stdio.h>
#include <ctype.h>

#if defined(att) || (defined(SYSV) && defined(i386))
#define ATT
#endif

#if defined(sgi) && defined(SVR4)
#undef SVR4
#define SYSV
#endif

#ifdef SVR4
#undef  SYSV			/* predefined on Solaris 2.4 */
#define SYSV
#define ATT
#endif

#ifdef ATT
#define USE_USG_PTYS
#endif

#ifdef APOLLO_SR9
#define CANT_OPEN_DEV_TTY
#endif

#ifdef __EMX__
#define USE_SYSV_TERMIO
#define USE_TERMCAP
#endif

#ifdef macII
#define USE_SYSV_TERMIO
#undef SYSV				/* pretend to be bsd */
#endif /* macII */

#ifdef SCO
#define USE_TERMCAP
#define USE_TERMINFO
#endif

#ifdef linux
#define USE_TERMIOS
#define USE_TERMINFO
#define USE_SYSV_UTMP
#endif

#if defined(SYSV) || defined(Lynx)
#define USE_SYSV_TERMIO
#ifndef Lynx
#define USE_SYSV_UTMP
#else
#define USE_TERMCAP
#define NO_TERMCAP_H
#endif
#else /* else not SYSV */
#define USE_TERMCAP
#endif /* SYSV */

/*
 * some OS's may want to use both, like SCO for example we catch
 * here anyone who hasn't decided what they want.
 */
#if !defined(USE_TERMCAP) && !defined(USE_TERMINFO)
#define USE_TERMINFO
#endif

#if defined(CSRG_BASED)
#define USE_TERMIOS
#endif

#include <sys/ioctl.h>
#ifdef USE_SYSV_TERMIO
# ifndef Lynx
#  include <sys/termio.h>
# else
#  include <termio.h>
# endif
#else /* else not USE_SYSV_TERMIO */
# ifdef USE_TERMIOS
#  include <termios.h>
# else /* not USE_TERMIOS */
#  include <sgtty.h>
# endif /* USE_TERMIOS */
#endif	/* USE_SYSV_TERMIO */

#ifdef USE_USG_PTYS
#include <sys/stream.h>
#ifndef SVR4
#include <sys/ptem.h>
#endif
#endif

#include <signal.h>
#include <pwd.h>

#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#else
char *getenv();
#endif

#ifdef USE_SYSV_TERMIO
#ifdef X_NOT_POSIX
#if !defined(SYSV) && !defined(i386)
extern struct passwd *getpwuid(); 	/* does ANYBODY need this? */
#endif /* SYSV && i386 */
#endif /* X_NOT_POSIX */
#define	bzero(s, n)	memset(s, 0, n)
#endif	/* USE_SYSV_TERMIO */

#ifdef MINIX
#define USE_SYSV_TERMIO
#include <sys/termios.h>
#define termio termios
#define TCGETA TCGETS
#define TCSETAW TCSETSW
#ifndef IUCLC
#define IUCLC	0
#endif
#endif

#define	EMULATIONS	2
#define	SUN		1
#define	TIMEOUT		10
#define	VT100		0

#define	SHELL_UNKNOWN	0
#define	SHELL_C		1
#define	SHELL_BOURNE	2
struct {
	char *name;
	int type;
} shell_list[] = {
	{ "csh",	SHELL_C },	/* vanilla cshell */
	{ "tcsh",	SHELL_C },
	{ "jcsh",	SHELL_C },
	{ "sh",		SHELL_BOURNE },	/* vanilla Bourne shell */
	{ "ksh",	SHELL_BOURNE },	/* Korn shell (from AT&T toolchest) */
	{ "ksh-i",	SHELL_BOURNE },	/* other name for latest Korn shell */
	{ "bash",	SHELL_BOURNE },	/* GNU Bourne again shell */
	{ "jsh",	SHELL_BOURNE },
	{ NULL,		SHELL_BOURNE }	/* default (same as xterm's) */
};

char *emuname[EMULATIONS] = {
	"VT100",
	"Sun",
};
char *myname;
int shell_type = SHELL_UNKNOWN;
char *getsize[EMULATIONS] = {
	"\0337\033[r\033[999;999H\033[6n",
	"\033[18t",
};
#if !defined(sun) || defined(SVR4)
#ifdef TIOCSWINSZ
char *getwsize[EMULATIONS] = {	/* size in pixels */
	0,
	"\033[14t",
};
#endif	/* TIOCSWINSZ */
#endif	/* sun */
char *restore[EMULATIONS] = {
	"\0338",
	0,
};
char *setname = "";
char *setsize[EMULATIONS] = {
	0,
	"\033[8;%s;%st",
};
#ifdef USE_SYSV_TERMIO
struct termio tioorig;
#else /* not USE_SYSV_TERMIO */
# ifdef USE_TERMIOS
struct termios tioorig;
# else /* not USE_TERMIOS */
struct sgttyb sgorig;
# endif /* USE_TERMIOS */
#endif /* USE_SYSV_TERMIO */
char *size[EMULATIONS] = {
	"\033[%d;%dR",
	"\033[8;%d;%dt",
};
char sunname[] = "sunsize";
int tty;
FILE *ttyfp;
#if !defined(sun) || defined(SVR4)
#ifdef TIOCSWINSZ
char *wsize[EMULATIONS] = {
	0,
	"\033[4;%hd;%hdt",
};
#endif	/* TIOCSWINSZ */
#endif	/* sun */

#include "proto.h"

extern int main PROTO((int argc, char **argv));

static SIGNAL_T onintr PROTO((int sig));
static SIGNAL_T resize_timeout PROTO((int sig));
static int checkdigits PROTO((char *str));
static void Usage PROTO((void));
static void readstring PROTO((FILE *fp, char *buf, char *str));

#ifdef USE_TERMCAP
static char *strindex PROTO((char *s1, char *s2));
#if !defined(NO_TERMCAP_H)
#include <termcap.h>
#if defined(NCURSES_VERSION)
	/* The tgetent emulation function in SVr4-style curses implementations
	 * (e.g., ncurses) ignores the buffer, so TERMCAP can't be set from it. 
	 * Instead, just use terminfo.
	 */
#undef USE_TERMCAP
#include <curses.h>
#endif
#else
#include <curses.h>
#endif /* ! NO_TERMCAP_H  */
#endif

/*
   resets termcap string to reflect current screen size
 */
int
main (argc, argv)
    int argc;
    char **argv;
{
	register char *ptr, *env;
	register int emu = VT100;
	char *shell;
	struct passwd *pw;
	int i;
	int rows, cols;
#ifdef USE_SYSV_TERMIO
	struct termio tio;
#else /* not USE_SYSV_TERMIO */
#ifdef USE_TERMIOS
	struct termios tio;
#else /* not USE_TERMIOS */
	struct sgttyb sg;
#endif /* USE_TERMIOS */
#endif /* USE_SYSV_TERMIO */
#ifdef USE_TERMCAP
	char termcap [1024];
	char newtc [1024];
#endif /* USE_TERMCAP */
	char buf[BUFSIZ];
#if defined(sun) && !defined(SVR4)
#ifdef TIOCSSIZE
	struct ttysize ts;
#endif	/* TIOCSSIZE */
#else	/* sun */
#ifdef TIOCSWINSZ
	struct winsize ws;
#endif	/* TIOCSWINSZ */
#endif	/* sun */
	char *name_of_tty;
#ifdef CANT_OPEN_DEV_TTY
	extern char *ttyname();
#endif

	ptr = strrchr(myname = argv[0], '/');
	if(ptr)
		myname = ptr + 1;
	if(strcmp(myname, sunname) == 0)
		emu = SUN;
	for(argv++, argc-- ; argc > 0 && **argv == '-' ; argv++, argc--) {
		switch((*argv)[1]) {
		 case 's':	/* Sun emulation */
			if(emu == SUN)
				Usage();	/* Never returns */
			emu = SUN;
			break;
		 case 'u':	/* Bourne (Unix) shell */
			shell_type = SHELL_BOURNE;
			break;
		 case 'c':	/* C shell */
			shell_type = SHELL_C;
			break;
		 default:
			Usage();	/* Never returns */
		}
	}

	if (SHELL_UNKNOWN == shell_type) {
		/* Find out what kind of shell this user is running.
		 * This is the same algorithm that xterm uses.
		 */
		if (((ptr = getenv("SHELL")) == NULL || *ptr == 0) &&
		 (((pw = getpwuid(getuid())) == NULL) ||
		 *(ptr = pw->pw_shell) == 0))
			/* this is the same default that xterm uses */
			ptr = "/bin/sh";

		shell = strrchr(ptr, '/');
		if(shell)
			shell++;
		else
			shell = ptr;

		/* now that we know, what kind is it? */
		for (i = 0; shell_list[i].name; i++)
			if (!strcmp(shell_list[i].name, shell))
				break;
		shell_type = shell_list[i].type;
	}

	if(argc == 2) {
		if(!setsize[emu]) {
			fprintf(stderr,
			 "%s: Can't set window size under %s emulation\n",
			 myname, emuname[emu]);
			exit(1);
		}
		if(!checkdigits(argv[0]) || !checkdigits(argv[1]))
			Usage();	/* Never returns */
	} else if(argc != 0)
		Usage();	/* Never returns */

#ifdef CANT_OPEN_DEV_TTY
	if ((name_of_tty = ttyname(fileno(stderr))) == NULL) 
#endif
	  name_of_tty = "/dev/tty";

	if ((ttyfp = fopen (name_of_tty, "r+")) == NULL) {
	    fprintf (stderr, "%s:  can't open terminal %s\n",
		     myname, name_of_tty);
	    exit (1);
	}
	tty = fileno(ttyfp);
#ifdef USE_TERMCAP
	if(!(env = getenv("TERM")) || !*env) {
	    env = "xterm";
	    if(SHELL_BOURNE == shell_type)
		setname = "TERM=xterm;\nexport TERM;\n";
	    else
		setname = "setenv TERM xterm;\n";
	}
	termcap[0] = 0;	/* ...just in case we've accidentally gotten terminfo */
	if(tgetent (termcap, env) <= 0) {
	    fprintf(stderr, "%s: Can't get entry \"%s\"\n",
		    myname, env);
	    exit(1);
	}
#endif /* USE_TERMCAP */
#ifdef USE_TERMINFO
	if(!(env = getenv("TERM")) || !*env) {
		env = "xterm";
		if(SHELL_BOURNE == shell_type)
			setname = "TERM=xterm;\nexport TERM;\n";
		else	setname = "setenv TERM xterm;\n";
	}
#endif	/* USE_TERMINFO */

#ifdef USE_SYSV_TERMIO
	ioctl (tty, TCGETA, &tioorig);
	tio = tioorig;
	tio.c_iflag &= ~(ICRNL | IUCLC);
	tio.c_lflag &= ~(ICANON | ECHO);
	tio.c_cflag |= CS8;
	tio.c_cc[VMIN] = 6;
	tio.c_cc[VTIME] = 1;
#else	/* else not USE_SYSV_TERMIO */
#if defined(USE_TERMIOS)
	tcgetattr(tty, &tioorig);
	tio = tioorig;
	tio.c_iflag &= ~ICRNL;
	tio.c_lflag &= ~(ICANON | ECHO);
	tio.c_cflag |= CS8;
	tio.c_cc[VMIN] = 6;
	tio.c_cc[VTIME] = 1;
#else	/* not USE_TERMIOS */
 	ioctl (tty, TIOCGETP, &sgorig);
	sg = sgorig;
	sg.sg_flags |= RAW;
	sg.sg_flags &= ~ECHO;
#endif  /* USE_TERMIOS */
#endif	/* USE_SYSV_TERMIO */
	signal(SIGINT, onintr);
	signal(SIGQUIT, onintr);
	signal(SIGTERM, onintr);
#ifdef USE_SYSV_TERMIO
	ioctl (tty, TCSETAW, &tio);
#else	/* not USE_SYSV_TERMIO */
#ifdef USE_TERMIOS
	tcsetattr(tty, TCSADRAIN, &tio);
#else   /* not USE_TERMIOS */
	ioctl (tty, TIOCSETP, &sg);
#endif  /* USE_TERMIOS */
#endif	/* USE_SYSV_TERMIO */

	if (argc == 2) {
		sprintf (buf, setsize[emu], argv[0], argv[1]);
		write(tty, buf, strlen(buf));
	}
	write(tty, getsize[emu], strlen(getsize[emu]));
	readstring(ttyfp, buf, size[emu]);
	if(sscanf (buf, size[emu], &rows, &cols) != 2) {
		fprintf(stderr, "%s: Can't get rows and columns\r\n", myname);
		onintr(0);
	}
	if(restore[emu])
		write(tty, restore[emu], strlen(restore[emu]));
#if defined(sun) && !defined(SVR4)
#ifdef TIOCGSIZE
	/* finally, set the tty's window size */
	if (ioctl (tty, TIOCGSIZE, &ts) != -1) {
		ts.ts_lines = rows;
		ts.ts_cols = cols;
		ioctl (tty, TIOCSSIZE, &ts);
	}
#endif	/* TIOCGSIZE */
#else	/* sun */
#ifdef TIOCGWINSZ
	/* finally, set the tty's window size */
	if(getwsize[emu]) {
	    /* get the window size in pixels */
	    write (tty, getwsize[emu], strlen (getwsize[emu]));
	    readstring(ttyfp, buf, wsize[emu]);
	    if(sscanf (buf, wsize[emu], &ws.ws_xpixel, &ws.ws_ypixel) != 2) {
		fprintf(stderr, "%s: Can't get window size\r\n", myname);
		onintr(0);
	    }
	    ws.ws_row = rows;
	    ws.ws_col = cols;
	    ioctl (tty, TIOCSWINSZ, &ws);
	} else if (ioctl (tty, TIOCGWINSZ, &ws) != -1) {
	    /* we don't have any way of directly finding out
	       the current height & width of the window in pixels.  We try
	       our best by computing the font height and width from the "old"
	       struct winsize values, and multiplying by these ratios...*/
	    if (ws.ws_col != 0)
	        ws.ws_xpixel = cols * (ws.ws_xpixel / ws.ws_col);
	    if (ws.ws_row != 0)
	        ws.ws_ypixel = rows * (ws.ws_ypixel / ws.ws_row);
	    ws.ws_row = rows;
	    ws.ws_col = cols;
	    ioctl (tty, TIOCSWINSZ, &ws);
	}
#endif	/* TIOCGWINSZ */
#endif	/* sun */

#ifdef USE_SYSV_TERMIO
	ioctl (tty, TCSETAW, &tioorig);
#else	/* not USE_SYSV_TERMIO */
#ifdef USE_TERMIOS
	tcsetattr(tty, TCSADRAIN, &tioorig);
#else   /* not USE_TERMIOS */
	ioctl (tty, TIOCSETP, &sgorig);
#endif  /* USE_TERMIOS */
#endif	/* USE_SYSV_TERMIO */
	signal(SIGINT, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
	signal(SIGTERM, SIG_DFL);

#ifdef USE_TERMCAP
	/* update termcap string */
	/* first do columns */
	if ((ptr = strindex (termcap, "co#")) == NULL) {
		fprintf(stderr, "%s: No `co#'\n", myname);
		exit (1);
	}

	i = ptr - termcap + 3;
	strncpy (newtc, termcap, i);
	sprintf (newtc + i, "%d", cols);
	ptr = strchr(ptr, ':');
	strcat (newtc, ptr);

	/* now do lines */
	if ((ptr = strindex (newtc, "li#")) == NULL) {
		fprintf(stderr, "%s: No `li#'\n", myname);
		exit (1);
	}

	i = ptr - newtc + 3;
	strncpy (termcap, newtc, i);
	sprintf (termcap + i, "%d", rows);
	ptr = strchr(ptr, ':');
	strcat (termcap, ptr);
#endif /* USE_TERMCAP */

	if(SHELL_BOURNE == shell_type) {

#ifdef USE_TERMCAP
		printf ("%sTERMCAP='%s';\n",
		 setname, termcap);
#endif /* USE_TERMCAP */
#ifdef USE_TERMINFO
#ifndef SVR4
		printf ("%sCOLUMNS=%d;\nLINES=%d;\nexport COLUMNS LINES;\n",
			setname, cols, rows);
#endif /* !SVR4 */
#endif	/* USE_TERMINFO */

	} else {		/* not Bourne shell */

#ifdef USE_TERMCAP
		printf ("set noglob;\n%ssetenv TERMCAP '%s';\nunset noglob;\n",
		 setname, termcap);
#endif /* USE_TERMCAP */
#ifdef USE_TERMINFO
#ifndef SVR4
		printf ("set noglob;\n%ssetenv COLUMNS '%d';\nsetenv LINES '%d';\nunset noglob;\n",
			setname, cols, rows);
#endif /* !SVR4 */
#endif	/* USE_TERMINFO */
	}
	exit(0);
}

#ifdef USE_TERMCAP
static char *
strindex (s1, s2)
/*
   returns a pointer to the first occurrence of s2 in s1, or NULL if there are
   none.
 */
register char *s1, *s2;
{
	register char *s3;
	Size_t s2len = strlen (s2);

	while ((s3 = strchr(s1, *s2)) != NULL)
	{
		if (strncmp (s3, s2, s2len) == 0) return (s3);
		s1 = ++s3;
	}
	return (NULL);
}
#endif

static int
checkdigits(str)
register char *str;
{
	while(*str) {
		if(!isdigit(*str))
			return(0);
		str++;
	}
	return(1);
}

static void
readstring(fp, buf, str)
    register FILE *fp;
    register char *buf;
    char *str;
{
	register int last, c;
#if !defined(USG) && !defined(AMOEBA) && !defined(MINIX) && !(__EMX__)
	/* What is the advantage of setitimer() over alarm()? */
	struct itimerval it;
#endif

	signal(SIGALRM, resize_timeout);
#if defined(USG) || defined(AMOEBA) || defined(MINIX) || defined(__EMX__)
	alarm (TIMEOUT);
#else
	bzero((char *)&it, sizeof(struct itimerval));
	it.it_value.tv_sec = TIMEOUT;
	setitimer(ITIMER_REAL, &it, (struct itimerval *)NULL);
#endif
	if ((c = getc(fp)) == 0233) {	/* meta-escape, CSI */
		*buf++ = c = '\033';
		*buf++ = '[';
	} else
		*buf++ = c;
	if(c != *str) {
		fprintf(stderr, "%s: unknown character, exiting.\r\n", myname);
		onintr(0);
	}
	last = str[strlen(str) - 1];
	while((*buf++ = getc(fp)) != last)
	    ;
#if defined(USG) || defined(AMOEBA) || defined(MINIX) || defined(__EMX__)
	alarm (0);
#else
	bzero((char *)&it, sizeof(struct itimerval));
	setitimer(ITIMER_REAL, &it, (struct itimerval *)NULL);
#endif
	*buf = 0;
}

static void
Usage()
{
	fprintf(stderr, strcmp(myname, sunname) == 0 ?
	 "Usage: %s [rows cols]\n" :
	 "Usage: %s [-u] [-c] [-s [rows cols]]\n", myname);
	exit(1);
}

static SIGNAL_T
resize_timeout(sig)
    int sig;
{
	fprintf(stderr, "\n%s: Time out occurred\r\n", myname);
	onintr(sig);
}

/* ARGSUSED */
static SIGNAL_T
onintr(sig)
    int sig;
{
#ifdef USE_SYSV_TERMIO
	ioctl (tty, TCSETAW, &tioorig);
#else	/* not USE_SYSV_TERMIO */
#ifdef USE_TERMIOS
	tcsetattr (tty, TCSADRAIN, &tioorig);
#else   /* not USE_TERMIOS */
	ioctl (tty, TIOCSETP, &sgorig);
#endif  /* use TERMIOS */
#endif	/* USE_SYSV_TERMIO */
	exit(1);
}
