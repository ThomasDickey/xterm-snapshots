/* $XTermId: version.h,v 1.291 2008/03/05 21:18:37 tom Exp $ */

/*
 * These definitions are used to build the string that's printed in response to
 * "xterm -version", or embedded in "xterm -help".  It usually indicates the
 * version of X to which this version of xterm has been built.  The number in
 * parentheses is my patch number (Thomas E. Dickey).
 */
#define XTERM_PATCH   235

#ifndef __vendorversion__
#define __vendorversion__ "XTerm"
#endif
