#!/usr/bin/perl
# $XFree86: xterm/vttests/88colors.pl,v 1.1 1999/09/14 13:03:03 swall Exp $
# Made from 256colors.pl

for ($bg = 0; $bg < 88; $bg++) {
    print "\x1b[9;1H\x1b[48;5;${bg}m\x1b[2J";
    for ($fg = 0; $fg < 88; $fg++) {
	print "\x1b[38;5;${fg}m";
	printf "%03.3d/%03.3d ", $fg, $bg;
    }
    sleep 1;
    print "\n";
}
