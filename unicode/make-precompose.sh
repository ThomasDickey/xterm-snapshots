#!/bin/sh
cat precompose.c.head
cut UnicodeData-Latest.txt -d ";" -f 1,6 | \
 grep ";[0-9,A-F]" | grep " " | \
 sed -e "s/ /, 0x/;s/^/{ 0x/;s/;/, 0x/;s/$/},/"
cat precompose.c.tail
