#!/usr/bin/env perl
# $XTermId: report-sgr.pl,v 1.9 2018/08/04 00:27:39 tom Exp $
# -----------------------------------------------------------------------------
# this file is part of xterm
#
# Copyright 2018 by Thomas E. Dickey
#
#                         All Rights Reserved
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
# Except as contained in this notice, the name(s) of the above copyright
# holders shall not be used in advertising or otherwise to promote the
# sale, use or other dealings in this Software without prior written
# authorization.
# -----------------------------------------------------------------------------
# Test the report-sgr option of xterm.

# TODO: v/V toggles
# TODO: f/F toggles
# TODO: b/B toggles
# TODO: ? help

use strict;
use warnings;

use Term::ReadKey;

our $csi = "\033[";

our @sgr_names = qw(
  Bold
  Faint
  Italicized
  Underlined
  Blink
  Fast-Blink
  Inverse
  Invisible
  Crossed-Out
);

our ( $row_max, $col_max );
our ( $mark,    $top_row );
our ( $row_1st, $col_1st, $row_now, $col_now );

sub cup($$) {
    my $r = shift;
    my $c = shift;
    printf "%s%d;%dH", $csi, $r, $c;
}

sub el($) {
    printf "%s%sK", $csi, $_[0];
}

sub ed($) {
    printf "%s%sJ", $csi, $_[0];
}

sub sgr($) {
    printf "%s%sm", $csi, $_[0];
}

sub show_string($) {
    my $value = $_[0];
    my $n;

    $value = "" unless $value;
    my $result = "";
    for ( $n = 0 ; $n < length($value) ; $n += 1 ) {
        my $c = ord substr( $value, $n, 1 );
        if ( $c == ord '\\' ) {
            $result .= "\\\\";
        }
        elsif ( $c == 0x1b ) {
            $result .= "\\E";
        }
        elsif ( $c == 0x7f ) {
            $result .= "^?";
        }
        elsif ( $c == 32 ) {
            $result .= "\\s";
        }
        elsif ( $c < 32 ) {
            $result .= sprintf( "^%c", $c + 64 );
        }
        elsif ( $c > 128 ) {
            $result .= sprintf( "\\%03o", $c );
        }
        else {
            $result .= chr($c);
        }
    }

    return $result;
}

sub get_reply($) {
    my $command = $_[0];
    my $reply   = "";

    print STDOUT $command;
    autoflush STDOUT 1;
    while (1) {
        my $test = ReadKey 0.02;
        last if not defined $test;

        $reply .= $test;
    }
    return $reply;
}

sub show_status() {
    &cup( 1, 1 );
    &el(2);
    my $show = "";
    my $parm = "";
    if ($mark) {
        my $r1 = ( $row_now > $row_1st ) ? $row_1st : $row_now;
        my $r2 = ( $row_now < $row_1st ) ? $row_1st : $row_now;
        my $c1 = ( $col_now > $col_1st ) ? $col_1st : $col_now;
        my $c2 = ( $col_now < $col_1st ) ? $col_1st : $col_now;
        $show = sprintf "[%d,%d] [%d,%d] ", $r1, $c1, $r2, $c2;
        $parm = sprintf "%d;%d;%d;%d",      $r1, $c1, $r2, $c2;
    }
    else {
        $show = sprintf "[%d,%d] ", $row_now, $col_now;
        $parm = sprintf "%d;%d;%d;%d",    #
          $row_now, $col_now,    #
          $row_now, $col_now;
    }
    my $send = sprintf "%s%s#|", $csi, $parm;
    printf "%s %s ", $show, &show_string($send);
    &cup( $row_now, $col_now );
    my $reply = &get_reply($send);
    &cup( 2, 1 );
    &el(2);
    printf "read %s", &show_string($reply);
    &cup( $row_now, $col_now );
}

sub show_example() {
    &cup( $top_row, 1 );
    &ed(1);
    for my $n ( 1 .. 9 ) {
        &cup( $n + $top_row, 1 );
        printf "SGR %d: %-12s", $n, $sgr_names[ $n - 1 ];
        &sgr($n);
        printf "abcdefghijklmnopqrstuvwxyz" . "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
          "0123456789";
        &sgr(0);
    }
}

$row_max = 24;
$col_max = 80;

# TODO - get actual screen-size

$mark    = 0;
$top_row = 3;
$row_now = $row_1st = $top_row;
$col_now = $col_1st = 1;

&ed(2);
&show_example;

ReadMode 'ultra-raw', 'STDIN';
while (1) {
    my $cmd;

    &show_status;
    &cup( $row_now, $col_now );
    $cmd = ReadKey 0;
    if ( $cmd eq " " ) {
        $mark    = ( $mark != 0 ) ? 0 : 1;
        $row_1st = $row_now;
        $col_1st = $col_now;
    }
    elsif ( $cmd eq "h" ) {
        $col_now-- if ( $col_now > 1 );
    }
    elsif ( $cmd eq "j" ) {
        $row_now++ if ( $row_now < $row_max );
    }
    elsif ( $cmd eq "k" ) {
        $row_now-- if ( $row_now > 1 );
    }
    elsif ( $cmd eq "l" ) {
        $col_now++ if ( $col_now < $col_max );
    }
    elsif ( $cmd eq "q" ) {
        &cup( $row_max, 1 );
        ReadMode 'restore', 'STDIN';
        printf "\r\n...quit\r\n";
        last;
    }
    else {
        printf "\a";
    }
}

1;
