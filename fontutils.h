/*
 * $XFree86: xc/programs/xterm/fontutils.h,v 1.6 1999/09/25 14:38:30 dawes Exp $
 */

/************************************************************

Copyright 1998 by Thomas E. Dickey <dickey@clark.net>

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

#ifndef included_fontutils_h
#define included_fontutils_h 1

#include <ptyx.h>
#include <proto.h>

extern int xtermLoadFont (TScreen *screen, char *nfontname, char *bfontname, Bool doresize, int fontnum);
extern void HandleSetFont PROTO_XT_ACTIONS_ARGS;
extern void SetVTFont (int i, Bool doresize, char *name1, char *name2);
extern void xtermComputeFontInfo (TScreen *screen, struct _vtwin *win, XFontStruct *font, int sbwidth);
extern void xtermSaveFontInfo (TScreen *screen, XFontStruct *font);
extern void xtermSetCursorBox (TScreen *screen);
extern void xtermUpdateFontInfo (TScreen *screen, Bool doresize);


#if OPT_DEC_CHRSET
extern char *xtermSpecialFont(unsigned atts, unsigned chrset);
#endif

#if OPT_BOX_CHARS
extern Bool xtermMissingChar(unsigned ch, XFontStruct *font);
extern void xtermDrawBoxChar(TScreen *screen, int ch, unsigned flags, GC gc, int x, int y);
#endif

#if OPT_SHIFT_KEYS
extern void HandleSmallerFont PROTO_XT_ACTIONS_ARGS;
extern void HandleLargerFont PROTO_XT_ACTIONS_ARGS;
#endif

#endif /* included_fontutils_h */
