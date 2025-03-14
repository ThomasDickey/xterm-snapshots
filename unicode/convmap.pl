#!/usr/bin/env perl
# $XTermId: convmap.pl,v 1.18 2025/03/14 08:13:58 tom Exp $
# -----------------------------------------------------------------------------
# this file is part of xterm
#
# Copyright 2007-2018,2025 by Thomas E. Dickey
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
# Generate keysym2ucs.c file
#
# See also:
# http://mail.nl.linux.org/linux-utf8/2001-04/msg00248.html
#
# $XFree86: xc/programs/xterm/unicode/convmap.pl,v 1.5 2000/01/24 22:22:05 dawes Exp $

use strict;
use warnings;

our $keysym;
our %name;
our %keysym_custom;
our %keysym_to_ucs;
our %keysym_to_keysymname;

our ($opt_v);

sub utf8 ($);

sub utf8 ($) {
    my $c = shift(@_);

    if ( $c < 0x80 ) {
        return sprintf( "%c", $c );
    }
    elsif ( $c < 0x800 ) {
        return sprintf( "%c%c", 0xc0 | ( $c >> 6 ), 0x80 | ( $c & 0x3f ) );
    }
    elsif ( $c < 0x10000 ) {
        return sprintf( "%c%c%c",
            0xe0 | ( $c >> 12 ),
            0x80 | ( ( $c >> 6 ) & 0x3f ),
            0x80 | ( $c & 0x3f ) );
    }
    elsif ( $c < 0x200000 ) {
        return sprintf( "%c%c%c%c",
            0xf0 | ( $c >> 18 ),
            0x80 | ( ( $c >> 12 ) & 0x3f ),
            0x80 | ( ( $c >> 6 ) & 0x3f ),
            0x80 | ( $c & 0x3f ) );
    }
    elsif ( $c < 0x4000000 ) {
        return sprintf( "%c%c%c%c%c",
            0xf8 | ( $c >> 24 ),
            0x80 | ( ( $c >> 18 ) & 0x3f ),
            0x80 | ( ( $c >> 12 ) & 0x3f ),
            0x80 | ( ( $c >> 6 ) & 0x3f ),
            0x80 | ( $c & 0x3f ) );

    }
    elsif ( $c < 0x80000000 ) {
        return sprintf( "%c%c%c%c%c%c",
            0xfe | ( $c >> 30 ),
            0x80 | ( ( $c >> 24 ) & 0x3f ),
            0x80 | ( ( $c >> 18 ) & 0x3f ),
            0x80 | ( ( $c >> 12 ) & 0x3f ),
            0x80 | ( ( $c >> 6 ) & 0x3f ),
            0x80 | ( $c & 0x3f ) );
    }
    else {
        return utf8(0xfffd);
    }
}

sub is_technical($) {
    my $keysym = shift;
    return ( $keysym >= 0x8b0 and $keysym < 0x8bf );
}

sub non_unicode ($) {
    my $keysym = shift;
    my $rc     = 0;
    if ( $keysym ne "" ) {
        if ( defined( $name{$keysym} ) and index( $name{$keysym}, "PUA" ) == 0 )
        {
            $rc = 1;
        }
        elsif ( defined( $keysym_to_ucs{$keysym} ) ) {
            $rc = private_use($keysym);
        }
        elsif ( is_technical($keysym)
            or ( $keysym >= 0xfd00 and $keysym <= 0xffff ) )
        {
            $rc = 1;
        }
    }
    return $rc;
}

sub private_use($) {
    my $keysym = shift;
    my $result = $keysym;
    if ( is_technical($keysym) ) {
        $result = $result - 0x8b1 + 0xeeee;
    }
    elsif ( $keysym > 0xfd00 and $keysym < 0xffff ) {
        $result = $result - 0xfa00 + 0xe000;
    }
    return $result;
}

my $unicodedata = "UnicodeData.txt";

# read list of all Unicode names
if ( !open( UDATA, $unicodedata ) && !open( UDATA, "$unicodedata" ) ) {
    die(    "Can't open Unicode database '$unicodedata':\n$!\n\n"
          . "Please make sure that you have downloaded the file\n"
          . "ftp://ftp.unicode.org/Public/UNIDATA/UnicodeData.txt\n" );
}
while (<UDATA>) {
    if (
/^([0-9,A-F]{4,6});([^;]*);([^;]*);([^;]*);([^;]*);([^;]*);([^;]*);([^;]*);([^;]*);([^;]*);([^;]*);([^;]*);([^;]*);([^;]*);([^;]*)$/
      )
    {
        $name{ hex($1) } = $2;
    }
    else {
        die("Syntax error in line '$_' in file '$unicodedata'");
    }
}
close(UDATA);

# read mapping (from http://wsinwp07.win.tue.nl:1234/unicode/keysym.map)
our $keysym_custom = "keysym.map";
our $lineno        = 0;
open( LIST, "<$keysym_custom" ) || die("Can't open map file:\n$!\n");
while (<LIST>) {
    ++$lineno;
    if (/^0x([0-9a-f]{4})\s+U([0-9a-f]{4})\s*(\#.*)?$/) {
        my $keysym  = hex($1);
        my $ucs     = hex($2);
        my $comment = $3;
        $comment =~ s/^#\s*//;
        $keysym_custom{$keysym} = "$keysym_custom:$lineno"
          unless defined $keysym_custom{$keysym};
        $keysym_to_ucs{$keysym}        = $ucs;
        $keysym_to_keysymname{$keysym} = $comment;
    }
    elsif ( /^\s*\#/ || /^\s*$/ ) {
    }
    else {
        die("Syntax error in 'list' in line\n$_\n");
    }
}
close(LIST);

# read entries in keysymdef.h
open( LIST, "</usr/include/X11/keysymdef.h" )
  || die("Can't open keysymdef.h:\n$!\n");
while (<LIST>) {
    if (/^\#define\s+XK_([A-Za-z_0-9]+)\s+0x([0-9a-fA-F]+)\s*(\/.*)?$/) {
        next if /\/\* deprecated \*\//;
        printf STDERR "LIST %s", $_ if ($opt_v);
        my $keysymname = $1;
        my $keysym     = hex($2);
        $keysym_to_keysymname{$keysym} = $keysymname;

        my $comment = $_;
        chomp $comment;
        my $check = $comment;
        $comment =~ s%^.*/\*\s*(.*)\s*\*/.*$%$1%;
        $comment =~ s%\(([^)]*)\)%$1%g;
        $comment = "" if ( $comment eq $check );

        if ( $comment ne "" ) {
            printf STDERR "->%s\n", $comment if ($opt_v);
            my $expect = "";
            $expect = $keysym_to_ucs{$keysym}
              if defined $keysym_to_ucs{$keysym};
            my $actual = "";

            my $explain = $comment;
            if ( $comment =~ /^U\+[[:xdigit:]]{4,}.*/ ) {
                $explain =~ s/^[^\s]+\s+//;
                $explain =~ s/\s+$//;
                $comment =~ s/\s.*//;
                $actual = hex( substr( $comment, 2 ) );
                $expect = $keysym - 0x1000000 if ( $keysym >= 0x1000000 );
                if ( defined( $keysym_custom{$keysym} )
                    and $actual eq $keysym_to_ucs{$keysym} )
                {
                    printf STDERR "%s: duplicates $_", $keysym_custom{$keysym};
                }

            }
            if ( $actual eq "" and $expect eq "" ) {
                my $ucs = private_use($keysym);
                $name{$ucs}             = "PUA $comment";
                $keysym_to_ucs{$keysym} = $ucs;
            }
            elsif ( $actual eq $expect ) {
                printf STDERR "OK %#06x -> U+%04X\n", $keysym, $actual
                  if ( $opt_v and $actual ne "" );
            }
            elsif ( $actual eq "" ) {
                printf STDERR "ERR $comment -> expect:U+%04X\n", $expect
                  unless non_unicode($keysym);
            }
            elsif ( $expect eq "" ) {
                $name{$actual}          = "$explain";
                $keysym_to_ucs{$keysym} = $actual;
            }
            else {
                printf STDERR "ERR $comment -> actual:%#x, expect:U+%04X\n",
                  $actual, $expect;
            }
        }
        elsif (
            non_unicode($keysym)
            and ( not defined $keysym_to_ucs{$keysym} or is_technical($keysym) )
          )
        {
            my $ucs = private_use($keysym);
            $name{$ucs}             = "PUA";
            $keysym_to_ucs{$keysym} = $ucs;
        }
    }
}
close(LIST);

print <<EOT;
/* \$XTermId\$
 * ----------------------------------------------------------------------------
 * this file is part of xterm
 *
 * Copyright 2007-2018,2025 by Thomas E. Dickey
 *
 *                         All Rights Reserved
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name(s) of the above copyright
 * holders shall not be used in advertising or otherwise to promote the
 * sale, use or other dealings in this Software without prior written
 * authorization.
 * ----------------------------------------------------------------------------
 * Note:
 * ----
 * This file has been updated and revised to provide for mapping all keysyms
 * to UCS.  The BMP private use area is used for keysyms which do not map to
 * characters.
 *
 * Original header:
 * ---------------
 * This module converts keysym values into the corresponding ISO 10646
 * (UCS, Unicode) values.
 *
 * The array keysymtab[] contains pairs of X11 keysym values for graphical
 * characters and the corresponding Unicode value. The function
 * keysym2ucs() maps a keysym onto a Unicode value using a binary search,
 * therefore keysymtab[] must remain SORTED by keysym value.
 *
 * The keysym -> UTF-8 conversion will hopefully one day be provided
 * by Xlib via XmbLookupString() and should ideally not have to be
 * done in X applications. But we are not there yet.
 *
 * We allow to represent any UCS character in the range U-00000000 to
 * U-00FFFFFF by a keysym value in the range 0x01000000 to 0x01ffffff.
 * This admittedly does not cover the entire 31-bit space of UCS, but
 * it does cover all of the characters up to U-10FFFF, which can be
 * represented by UTF-16, and more, and it is very unlikely that higher
 * UCS codes will ever be assigned by ISO. So to get Unicode character
 * U+ABCD you can directly use keysym 0x0100abcd.
 *
 * NOTE: The comments in the table below contain the actual character
 * encoded in UTF-8, so for viewing and editing best use an editor in
 * UTF-8 mode.
 *
 * Author: Markus G. Kuhn <mkuhn\@acm.org>, University of Cambridge, April 2001
 *
 * Special thanks to Richard Verhoeven <river\@win.tue.nl> for preparing
 * an initial draft of the mapping table.
 *
 * This software is in the public domain. Share and enjoy!
 *
 * AUTOMATICALLY GENERATED FILE, DO NOT EDIT !!! (unicode/convmap.pl)
 */

#include <keysym2ucs.h>

static struct codepair {
    unsigned short keysym;
    unsigned short ucs;
} keysymtab[] = {
/* *INDENT-OFF* */
EOT

for $keysym ( sort { $a <=> $b } keys(%keysym_to_keysymname) ) {
    my $ucs = $keysym_to_ucs{$keysym};
    next if $keysym > 0xffff or $keysym < 0x100;
    if ($ucs) {
        my $pua = 0;
        $pua = 1 if index( $name{$ucs}, "PUA" ) == 0;
        $pua = 1 if $keysym == 0xffff;
        printf(
            "%s{ 0x%04x, 0x%04x }, %s*%28s %s %s */\n",
            $pua ? "/*" : "  ",
            $keysym,
            $ucs,
            $pua ? "*" : "/",
            $keysym_to_keysymname{$keysym},
            $pua        ? "?"
            : $ucs < 32 ? ( "^" . chr( 64 + $ucs ) )
            : utf8($ucs),
            defined( $name{$ucs} ) ? $name{$ucs} : "???"
        );
    }
    else {
        printf( "/*  0x%04x   %39s ? ??? */\n",
            $keysym, $keysym_to_keysymname{$keysym} );
    }
}

print <<EOT;
/* *INDENT-ON* */
};

long
keysym2ucs(KeySym keysym)
{
    long result = -1;		/* no matching Unicode value found */
    int min = 0;
    int max = sizeof(keysymtab) / sizeof(struct codepair) - 1;

    if ((keysym >= 0x0020 && keysym <= 0x007e) ||
	(keysym >= 0x00a0 && keysym <= 0x00ff)) {
	/* found Latin-1 characters (1:1 mapping) */
	result = (long) keysym;
    } else if ((keysym & 0xff000000) == 0x01000000) {
	/* found directly encoded 24-bit UCS characters */
	result = (long) (keysym & 0x00ffffff);
    } else if (keysym >= 0x08b0 && keysym <= 0x08b7) {
	result = (long) keysym - 0x8b1 + 0xeeee;
    } else {
	/* binary search in table */
	while (max >= min) {
	    int mid = (min + max) / 2;
	    if (keysymtab[mid].keysym < keysym) {
		min = mid + 1;
	    } else if (keysymtab[mid].keysym > keysym) {
		max = mid - 1;
	    } else {
		/* found it in table */
		result = keysymtab[mid].ucs;
		break;
	    }
	}
	if (result == -1) {
	    if (keysym >= 0xfd01 && keysym <= 0xffff) {
		result = (long) keysym - 0xfa00 + 0xe000;
	    }
	}
    }

    return result;
}
EOT
