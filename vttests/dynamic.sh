#!/bin/sh
# $XFree86: xc/programs/xterm/vttests/dynamic.sh,v 1.2 1999/04/11 13:11:43 dawes Exp $
#
# -- Thomas Dickey (1999/3/27)
# Demonstrate the use of dynamic colors by setting the background successively
# to different values.

ESC=""
CMD='echo'
OPT='-n'
SUF=''
TMP=/tmp/xterm$$
for verb in print printf ; do
    rm -f $TMP
    eval '$verb "\c" >$TMP || echo fail >$TMP' 2>/dev/null
    if test -f $TMP ; then
	if test ! -s $TMP ; then
	    CMD="$verb"
	    OPT=
	    SUF='\c'
	    break
	fi
    fi
done
rm -f $TMP

LIST="00 30 80 d0 ff"

exec </dev/tty
old=`stty -g`
stty raw -echo min 0  time 5

$CMD $OPT "${ESC}]11;?${SUF}" > /dev/tty
read original
stty $old
original=${original}${SUF}

trap '$CMD $OPT "$original" >/dev/tty; exit' 0 1 2 5 15
while true
do
    for R in $LIST
    do
	for G in $LIST
	do
	    for B in $LIST
	    do
		$CMD $OPT "${ESC}]11;rgb:$R/$G/$B${SUF}" >/dev/tty
		sleep 1
	    done
	done
    done
done
