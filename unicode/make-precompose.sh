#!/bin/sh
# $XTermId: make-precompose.sh,v 1.4 2004/11/30 21:16:54 tom Exp $
cat precompose.c.head | sed -e's/@/$/g'
cut UnicodeData.txt -d ";" -f 1,6 | \
 grep ";[0-9,A-F]" | grep " " | \
 sed -e "s/ /, 0x/;s/^/{ 0x/;s/;/, 0x/;s/$/},/" | (sort -k 3 || sort +2)
cat precompose.c.tail
