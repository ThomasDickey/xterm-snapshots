#ifndef	included_wcwidth_h
#define	included_wcwidth_h 1

#include <stddef.h>

extern int my_wcwidth(wchar_t ucs);
extern int wcswidth(const wchar_t *pwcs, size_t n);

#endif /* included_wcwidth_h */
