#!/bin/sh
# $XFree86: xc/programs/xterm/vttests/dynamic.sh,v 1.1 1999/03/28 15:33:29 dawes Exp $
#
# -- Thomas Dickey (1999/3/27)
# Demonstrate the use of dynamic colors by setting the background successively
# to different values.

LIST="00 30 d0 ff"

exec </dev/tty
old=`stty -g`
stty raw -echo min 0  time 0
echo -n ']11;?' > /dev/tty
read original
stty $old

trap 'echo -n "$original"; exit' 0 1 2 5 15
for R in $LIST
do
    for G in $LIST
    do
	for B in $LIST
	do
	    echo -n "]11;rgb:$R/$G/$B"
	    sleep 1
	done
    done
done
