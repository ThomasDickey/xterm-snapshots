#!/usr/bin/env perl

use strict;
use warnings;

use Encode 'encode_utf8';

our ($screen_rows, $screen_cols);

sub get_screen_size()
{
    my $sizes=`stty size`;
    if ($? == 0) {
	($screen_rows, $screen_cols) = split(/ /,$sizes);
    }
}

sub color_slice ()
{
    get_screen_size();

    my $rows = ($screen_rows - 2);
    my $cols = ($screen_cols - 1);
    my $blue_step = (255.0 / $rows);
    my $red_green_step = (255.0 / $cols);
    my $row = 0;
    my $b = 0;

    while ( $row < $rows ) {
	my $col=0;
	my $r=0;
	my $g=255;
	while ( $col < $cols ) {
	    printf("\x1b[48;2;%d;%d;%dm ", $r, $g, $b);
	    $r=($r + $red_green_step);
	    $g=($g - $red_green_step);
	    $col=($col + 1);
	}
	printf "\e[0m\n";
	$b=($b + $blue_step);
	$row=($row + 1);
    }
    printf("\e[0m");
}

color_slice();
