#!/bin/sh
# $XFree86: xc/programs/xterm/vttests/resize.sh,v 1.1 1999/03/28 15:33:29 dawes Exp $
#
# -- Thomas Dickey (1999/3/27)
# Obtain the current screen size, then resize the terminal to the nominal
# screen width/height, and restore the size.

exec </dev/tty
old=`stty -g`
stty raw -echo min 0  time 0

echo -n "[18t" > /dev/tty
IFS=';' read junk height width

echo -n "[19t" > /dev/tty
IFS=';' read junk maxheight maxwidth

stty $old

width=`echo $width|sed -e 's/t.*//'`
maxwidth=`echo $maxwidth|sed -e 's/t.*//'`

trap 'echo -n "[8;${height};${width}t"; exit' 0 1 2 5 15
w=$width
h=$height
a=1
while true
do
#	sleep 1
	echo resizing to $h by $w
	echo -n "[8;${h};${w}t"
	if test $a = 1 ; then
		if test $w = $maxwidth ; then
			h=`expr $h + $a`
			if test $h = $maxheight ; then
				a=-1
			fi
		else
			w=`expr $w + $a`
		fi
	else
		if test $w = $width ; then
			h=`expr $h + $a`
			if test $h = $height ; then
				a=1
			fi
		else
			w=`expr $w + $a`
		fi
	fi
done
