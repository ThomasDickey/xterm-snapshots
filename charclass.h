#ifndef CHARCLASS_H
#define CHARCLASS_H

extern void init_classtab(void);
/* intialise the table. needs calling before either of the 
   others. */

extern int SetCharacterClassRange(int low, int high, int value);
extern int CharacterClass(int c);

#endif
