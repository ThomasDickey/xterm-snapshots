#!/bin/sh
# $XFree86: xc/programs/xterm/vttests/title.sh,v 1.1 1999/03/28 15:33:30 dawes Exp $
#
# -- Thomas Dickey (1999/3/27)
# Obtain the current title of the window, set up a simple clock which runs
# until this script is interrupted, then restore the title.

exec </dev/tty
old=`stty -g`
stty raw -echo min 0  time 0
echo -n "[21t" > /dev/tty
read title
stty $old

title=`echo "$title" |sed -e 's/^...//' -e 's/.$//'`

trap 'echo -n "]2;$title"; exit' 0 1 2 5 15
while true
do
	sleep 1
	echo -n "]2;`date`"
done
