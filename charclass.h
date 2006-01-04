/* $XTermId: charclass.h,v 1.4 2006/01/04 02:10:19 tom Exp $ */

/* $XFree86: xc/programs/xterm/charclass.h,v 1.2 2006/01/04 02:10:19 dickey Exp $ */

#ifndef CHARCLASS_H
#define CHARCLASS_H

extern void init_classtab(void);
/* intialise the table. needs calling before either of the 
   others. */

extern int SetCharacterClassRange(int low, int high, int value);
extern int CharacterClass(int c);

#if OPT_TRACE || defined(NO_LEAKS)
extern void noleaks_CharacterClass(void);
#endif

#endif
