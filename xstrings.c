/* $XFree86: xc/programs/xterm/xstrings.c,v 1.2 2001/06/18 19:09:27 dickey Exp $ */

/************************************************************

Copyright 2000,2001 by Thomas E. Dickey

                        All Rights Reserved

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name(s) of the above copyright
holders shall not be used in advertising or otherwise to promote the
sale, use or other dealings in this Software without prior written
authorization.

********************************************************/

#include <xterm.h>

#include <sys/types.h>
#include <string.h>

#include <xstrings.h>

char *
x_basename(char *name)
{
    char *cp;

    cp = strrchr(name, '/');
#ifdef __EMX__
    if (cp == 0)
	cp = strrchr(name, '\\');
#endif
    return (cp ? cp + 1 : name);
}

/*
 * Allocates a copy of a string
 */
char *
x_strdup(char *s)
{
    if (s != 0) {
	char *t = malloc(strlen(s) + 1);
	if (t != 0) {
	    strcpy(t, s);
	}
	s = t;
    }
    return s;
}

/*
 * Returns a pointer to the first occurrence of s2 in s1,
 * or NULL if there are none.
 */
char *
x_strindex(char *s1, char *s2)
{
    char *s3;
    size_t s2len = strlen(s2);

    while ((s3 = strchr(s1, *s2)) != NULL) {
	if (strncmp(s3, s2, s2len) == 0)
	    return (s3);
	s1 = ++s3;
    }
    return (NULL);
}
