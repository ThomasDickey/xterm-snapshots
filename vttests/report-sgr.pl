#!/usr/bin/env perl
# $XTermId: report-sgr.pl,v 1.19 2018/08/06 00:48:24 tom Exp $
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

# TODO: improve f/F and b/B logic for direct-colors vs indexed-colors

use strict;
use warnings;

use Getopt::Long qw(:config auto_help no_ignore_case);
use Pod::Usage;
use Term::ReadKey;

our ( $opt_colors, $opt_direct, $opt_help, $opt_man );

our $csi = "\033[";

our @sgr_names = qw(
  Normal
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

our $cur_sgr = 0;
our $cur_fg  = -1;
our $cur_bg  = -1;

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

sub color_name($) {
    my $code = shift;
    my $result;
    if ( $code < 0 ) {
        $result = "default";
    }
    else {
        $result = $code;
    }
    return $result;
}

sub color_code($$) {
    my $code   = shift;
    my $isfg   = shift;
    my $result = "";
    my $base   = $isfg ? 30 : 40;
    if ($opt_direct) {
        $result = sprintf "%d:5:%d", $base + 8, $code;
    }
    else {
        if ( $code < 0 ) {
            $result = $base + 9;
        }
        else {
            if ( $opt_colors <= 16 ) {
                $base += 60 if ( $code >= 8 );
                $result = $base + $code;
            }
            else {
                $result = sprintf "%d:2:%d:%d:%d", $base + 8, $code, $code,
                  $code;
            }
        }
    }
    return $result;
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

sub set_cur_sgr($) {
    my $inc = shift;
    $cur_sgr = ( $cur_sgr + 10 + $inc ) % 10;
    &show_example;
}

sub toggle_color($$) {
    my $inc = shift;
    my $cur = shift;
    $cur =
      ( ( $cur + 1 ) + ( $opt_colors + 1 ) + $inc ) % ( $opt_colors + 1 ) - 1;
    return $cur;
}

sub set_cur_fg($) {
    my $inc = shift;
    $cur_fg = &toggle_color( $inc, $cur_fg );
    &show_example;
}

sub set_cur_bg($) {
    my $inc = shift;
    $cur_bg = &toggle_color( $inc, $cur_bg );
    &show_example;
}

sub show_example() {
    &cup( $top_row, 1 );
    my $init = "0";
    $init .= sprintf ";%s", &color_code( $cur_fg, 1 ) unless ( $cur_fg < 0 );
    $init .= sprintf ";%s", &color_code( $cur_bg, 0 ) unless ( $cur_bg < 0 );
    &ed(0);
    for my $n ( 0 .. 9 ) {
        my $mode = $n;
        $mode = $init if ( $n == 0 );
        &cup( $n + $top_row, 1 );
        &sgr( $cur_fg eq $cur_bg ? "0" : $init );
        printf "%s SGR %d: %-12s",    #
          ( $cur_sgr == $n ) ? "-->" : "   ",    #
          $n, $sgr_names[$n];
        $mode .= ";$cur_sgr" unless ( $cur_sgr eq "0" );
        &sgr($mode);
        printf "abcdefghijklmnopqrstuvwxyz" . "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
          "0123456789";
    }
    &sgr(0);
    &cup( $top_row + 11, 1 );
    printf "Current SGR %d (%s)", $cur_sgr, $sgr_names[$cur_sgr];
    printf ", fg=%s",             &color_name($cur_fg);
    printf ", bg=%s",             &color_name($cur_bg);
    printf ' ("q" to quit, "?" for help)';
}

sub init_screensize() {
    $row_max = 24;
    $col_max = 80;
    &cup( 9999, 9999 );
    my $result = &get_reply( $csi . "6n" );
    if ( $result =~ /^$csi[[:digit:];]+R$/ ) {
        $result =~ s/^$csi[;]*//;
        $result =~ s/[;]*R$//;
        my @params = split /;/, $result;
        if ( $#params == 1 ) {
            $row_max = $params[0];
            $col_max = $params[1];
        }
    }
    &cup( 1, 1 );
}

GetOptions( 'colors=i', 'help|?', 'direct', 'man' ) || pod2usage(2);
pod2usage(1) if $opt_help;
pod2usage( -verbose => 2 ) if $opt_man;

$opt_colors = 8 unless ($opt_colors);
$opt_colors = 8 if ( $opt_colors < 8 );

ReadMode 'ultra-raw', 'STDIN';

&init_screensize;

$mark    = 0;
$top_row = 4;
$row_now = $row_1st = $top_row;
$col_now = $col_1st = 1;

&ed(2);
&show_example;

while (1) {
    my $cmd;

    &show_status;
    &cup( $row_now, $col_now );
    $cmd = ReadKey 0;
    if ( $cmd eq "?" ) {
        &cup( $row_max, 1 );
        ReadMode 'restore', 'STDIN';
        system( $0 . " -man" );
        ReadMode 'ultra-raw', 'STDIN';
        &show_example;
        $cmd = ReadKey 0;
    }
    elsif ( $cmd eq " " ) {
        $mark    = ( $mark != 0 ) ? 0 : 1;
        $row_1st = $row_now;
        $col_1st = $col_now;
    }
    elsif ( $cmd eq chr(12) ) {
        &show_example;
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
    elsif ( $cmd eq "=" ) {
        &cup( $row_now = $row_1st + $cur_sgr, $col_now = 24 );
    }
    elsif ( $cmd eq "v" ) {
        &set_cur_sgr(1);
    }
    elsif ( $cmd eq "V" ) {
        &set_cur_sgr(-1);
    }
    elsif ( $cmd eq "f" ) {
        &set_cur_fg(1);
    }
    elsif ( $cmd eq "F" ) {
        &set_cur_fg(-1);
    }
    elsif ( $cmd eq "b" ) {
        &set_cur_bg(1);
    }
    elsif ( $cmd eq "B" ) {
        &set_cur_bg(-1);
    }
    else {
        printf "\a";
    }
}

1;

__END__

=head1 NAME

report-sgr.pl - demonstrate xterm's report-SGR control sequence

=head1 SYNOPSIS

report-sgr.pl [options]

  Options:
    -help            brief help message
    -direct          use direct-colors, rather than indexed

=head1 OPTIONS

=over 8

=item B<-help>

Print a brief help message and exit.

=item B<-man>

Print the extended help message and exit.

=item B<-direct>

Use direct-colors (e.g., an RGB value), rather than indexed (e.g., ANSI colors).

=back

=head1 DESCRIPTION

B<report-sgr> displays a normal line, as well as one for each SGR code 1-9,
with a test-string showing the effect of the SGR.  Two SGR codes can be
combined, as well as foreground and background colors.

=head1 Commands

=over 8

=item B<q>

Quit the program with B<q>.  It will ignore B<^C> and other control characters.

=item B<h>, B<j>, B<k>, B<l>

As you move the cursor around the screen (with vi-style h,j,k,l characters),
the script sends an XTREPORTSGR control to the terminal, asking what the video
attributes are for the currently selected cell.  The script displays the result
on the second line of the screen.

=item B<space>

XTREPORTSGR returns an SGR control sequence which could be used to set the
terminal's current video attributes to match the attributes found in all cells
of the rectangle specified by this script.  Use the spacebar to toggle the mark
which denotes one corner of the rectangle.  The current cursor position is the
other corner.

=item B<=>

Move the cursor to the first cell of the test-data for the currently selected
SGR code (the one with B<-->>).

=item B<v> and B<V>

Move the selector for SGR code forward or backward.  That code is combined
with the codes for each test-string.

=item B<F> and B<f>

Increase or decrease the foreground color value.

=item B<B> and B<b>

Increase or decrease the background color value.

=item B<^L>

Repaint the screen.

=back

=cut
