/* $XTermId: charclass.h,v 1.6 2006/02/13 01:14:58 tom Exp $ */

/* $XFree86: xc/programs/xterm/charclass.h,v 1.3 2006/02/13 01:14:58 dickey Exp $ */

#ifndef CHARCLASS_H
#define CHARCLASS_H

extern void init_classtab(void);
/* intialise the table. needs calling before either of the 
   others. */

extern int SetCharacterClassRange(int low, int high, int value);
extern int CharacterClass(int c);

#ifdef NO_LEAKS
extern void noleaks_CharacterClass(void);
#endif

#endif
