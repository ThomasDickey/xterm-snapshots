/*
 * $XFree86: xc/programs/xterm/trace.c,v 3.11 2000/06/13 02:28:41 dawes Exp $
 */

/************************************************************

Copyright 1997-2000 by Thomas E. Dickey

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
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdarg.h>

#include <trace.h>

char *trace_who = "parent";

void
Trace(char *fmt, ...)
{
	static	FILE	*fp;
	static	char	*trace_out;
	va_list ap;

	if (fp != 0
	 && trace_who != trace_out) {
		fclose(fp);
		fp = 0;
	}
	trace_out = trace_who;

	if (!fp) {
		char name[BUFSIZ];
		sprintf(name, "Trace-%s.out", trace_who);
		fp = fopen(name, "w");
		if (fp != 0) {
			time_t now = time((time_t*)0);
#ifdef HAVE_UNISTD_H
			fprintf(fp, "process %d real (%d/%d) effective (%d/%d) -- %s",
				getpid(),
				getuid(), getgid(),
				geteuid(), getegid(),
				ctime(&now));
#else
			fprintf(fp, "process %d -- %s",
				getpid(),
				ctime(&now));
#endif
		}
	}
	if (!fp)
		abort();

	va_start(ap,fmt);
	if (fmt != 0) {
		vfprintf(fp, fmt, ap);
		(void)fflush(fp);
	} else {
		(void)fclose(fp);
		(void)fflush(stdout);
		(void)fflush(stderr);
	}
	va_end(ap);
}

char *
visibleChars(PAIRED_CHARS(Char *buf, Char *buf2), unsigned len)
{
	static char *result;
	static unsigned used;
	unsigned limit = ((len + 1) * 8) + 1;
	char *dst;

	if (limit > used) {
		used = limit;
		result = XtRealloc(result, used);
	}
	dst = result;
	while (len--) {
		unsigned value = *buf++;
#if OPT_WIDE_CHARS
		if (buf2 != 0) {
			value |= (*buf2 << 8);
			buf2++;
		}
		if (value > 255)
			sprintf(dst, "\\u+%04X", value);
		else
#endif
		if (E2A(value) < 32 || (E2A(value) >= 127 && E2A(value) < 160))
			sprintf(dst, "\\%03o", value);
		else
			sprintf(dst, "%c", value);
		dst += strlen(dst);
	}
	return result;
}

char *
visibleIChar(IChar *buf, unsigned len)
{
	static char *result;
	static unsigned used;
	unsigned limit = ((len + 1) * 6) + 1;
	char *dst;

	if (limit > used) {
		used = limit;
		result = XtRealloc(result, used);
	}
	dst = result;
	while (len--) {
		unsigned value = *buf++;
#if OPT_WIDE_CHARS
		if (value > 255)
			sprintf(dst, "\\u+%04X", value);
		else
#endif
		if (E2A(value) < 32 || (E2A(value) >= 127 && E2A(value) < 160))
			sprintf(dst, "\\%03o", value);
		else
			sprintf(dst, "%c", value);
		dst += strlen(dst);
	}
	return result;
}
