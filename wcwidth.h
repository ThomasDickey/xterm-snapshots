/* $XFree86: xc/programs/xterm/wcwidth.h,v 1.2 2001/06/18 19:09:27 dickey Exp $ */

#ifndef	included_wcwidth_h
#define	included_wcwidth_h 1

#include <stddef.h>

extern int my_wcwidth(wchar_t ucs);
extern int wcswidth(const wchar_t *pwcs, size_t n);
extern int wcswidth_cjk(const wchar_t *pwcs, size_t n);

#endif /* included_wcwidth_h */
