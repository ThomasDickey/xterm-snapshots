/*
 * $XFree86: xc/programs/xterm/trace.c,v 3.2 1997/08/26 10:01:59 hohndel Exp $
 */

/************************************************************

Copyright 1997 by Thomas E. Dickey <dickey@clark.net>

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

********************************************************/

/*
 * debugging support via TRACE macro.
 */
#ifdef HAVE_CONFIG_H
#include <xtermcfg.h>
#endif

#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include "trace.h"

#if __STDC__ || CC_HAS_PROTOS
#define ANSI_VARARGS 1
#include <stdarg.h>
#else
#define ANSI_VARARGS 0
#include <varargs.h>
#endif

void
#if	ANSI_VARARGS
Trace(char *fmt, ...)
#else
Trace(va_alist)
va_dcl
#endif
{
#if	!ANSI_VARARGS
	register char *fmt;
#endif
	static	FILE	*fp;
	va_list ap;

	if (!fp) {
		fp = fopen("Trace.out", "w");
		if (fp != 0) {
#if HAVE_UNISTD_H
			time_t now;
			fprintf(fp, "process %d real (%d/%d) effective (%d/%d) -- %s",
				getpid(),
				getuid(), getgid(),
				geteuid(), getegid(),
				ctime(&now));
#else
			time_t now;
			fprintf(fp, "process %d -- %s",
				getpid(),
				ctime(&now));
#endif
		}
	}
	if (!fp)
		abort();

#if	ANSI_VARARGS
	va_start(ap,fmt);
#else
	va_start(ap);
	fmt = va_arg(ap, char *);
#endif
	if (fmt != 0) {
		vfprintf(fp, fmt, ap);
		va_end(ap);
		(void)fflush(fp);
	} else {
		(void)fclose(fp);
		(void)fflush(stdout);
		(void)fflush(stderr);
	}
}
