#!/bin/sh
# $XFree86$
#
# -- Thomas Dickey (1999/3/27)
# Show a simple 8-color test pattern

trap 'echo -n "[0m"; exit' 0 1 2 5 15
echo "[0m"
while true
do
    for AT in 0 1 4 7
    do
    	case $AT in
	0) attr="normal  ";;
	1) attr="bold    ";;
	4) attr="under   ";;
	7) attr="reverse ";;
	esac
	for FG in 0 1 2 3 4 5 6 7
	do
	    case $FG in
	    0) fcolor="black   ";;
	    1) fcolor="red     ";;
	    2) fcolor="green   ";;
	    3) fcolor="yellow  ";;
	    4) fcolor="blue    ";;
	    5) fcolor="magenta ";;
	    6) fcolor="cyan    ";;
	    7) fcolor="white   ";;
	    esac
	    echo -n "[0;${AT}m$attr"
	    echo -n "[3${FG}m$fcolor"
	    for BG in 1 2 3 4 5 6 7
	    do
		case $BG in
		0) bcolor="black   ";;
		1) bcolor="red     ";;
		2) bcolor="green   ";;
		3) bcolor="yellow  ";;
		4) bcolor="blue    ";;
		5) bcolor="magenta ";;
		6) bcolor="cyan    ";;
		7) bcolor="white   ";;
		esac
		echo -n "[4${BG}m$bcolor"
	    done
	    echo "[0m"
	done
	sleep 1
    done
done
