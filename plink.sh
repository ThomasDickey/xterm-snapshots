#!/bin/sh
# $XFree86$
#
# Reduce the number of dynamic libraries used to link an executable.
CMD=
while test $# != 0
do
	OPT="$1"
	shift
	case $OPT in
	-l*)
		echo "testing if $OPT is needed"
		if ( eval $CMD $* >/dev/null 2>/dev/null )
		then
			: echo ...no
		else
			echo ...yes
			CMD="$CMD $OPT"
		fi
		;;
	*)
		CMD="$CMD $OPT"
		;;
	esac
done
eval $CMD
