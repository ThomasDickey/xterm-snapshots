.\"#! troff -ms $1		-*- Nroff -*-
.\" "Xterm Control Sequences" document
.\" $XConsortium: ctlseqs.ms,v 1.16 94/12/28 20:45:48 gildea Exp $
.\" $XFree86: xc/doc/specs/xterm/ctlseqs.ms,v 3.8.2.1 1997/05/25 05:06:27 dawes Exp $
.\"
.\" Copyright 1991, 1994 X Consortium
.\"
.\" Permission is hereby granted, free of charge, to any person obtaining
.\" a copy of this software and associated documentation files (the
.\" "Software"), to deal in the Software without restriction, including
.\" without limitation the rights to use, copy, modify, merge, publish,
.\" distribute, sublicense, and/or sell copies of the Software, and to
.\" permit persons to whom the Software is furnished to do so, subject to
.\" the following conditions:
.\"
.\" The above copyright notice and this permission notice shall be
.\" included in all copies or substantial portions of the Software.
.\"
.\" THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
.\" EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
.\" MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
.\" IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
.\" OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
.\" ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
.\" OTHER DEALINGS IN THE SOFTWARE.
.\"
.\" Except as contained in this notice, the name of the X Consortium shall
.\" not be used in advertising or otherwise to promote the sale, use or
.\" other dealings in this Software without prior written authorization
.\" from the X Consortium.
.\"
.\" X Window System is a trademark of X Consortium, Inc.
.\"
.\" Originally written by Edward Moy, University of California,
.\" Berkeley, edmoy@violet.berkeley.edu, for the X.V10R4 xterm.
.\" The X Consortium staff has since updated it for X11.
.\" Updated by Thomas Dickey <dickey@clark.net> for XFree86 3.2.
.\"
.\" Run this file through troff and use the -ms macro package.
.\"
.if n .pl 9999v		\" no page breaks in nroff
.ND
.de St
.sp
.nr PD 0
.nr PI 1.5i
.nr VS 16
..
.de Ed
.nr PD .3v
.nr VS 12
..
.ds CH \" as nothing
.ds LH Xterm Control Sequences
.nr s 6*\n(PS/10
.ds L \s\nsBEL\s0
.ds E \s\nsESC\s0
.ds T \s\nsTAB\s0
.ds X \s\nsETX\s0
.ds N \s\nsENQ\s0
.ds e \s\nsETB\s0
.ds C \s\nsCAN\s0
.ds S \s\nsSUB\s0
.\" space between chars
.ie t .ds s \|
.el .ds s " \"
.nr [W \w'\*L'u
.nr w \w'\*E'u
.if \nw>\n([W .nr [W \nw
.nr w \w'\*T'u
.if \nw>\n([W .nr [W \nw
.nr w \w'\*X'u
.if \nw>\n([W .nr [W \nw
.nr w \w'\*N'u
.if \nw>\n([W .nr [W \nw
.nr w \w'\*e'u
.if \nw>\n([W .nr [W \nw
.nr w \w'\*C'u
.if \nw>\n([W .nr [W \nw
.nr w \w'\*S'u
.if \nw>\n([W .nr [W \nw
.nr [W +\w'\|\|'u
.de []
.nr w \w'\\$2'
.nr H \\n([Wu-\\nwu
.nr h \\nHu/2u
.\" do fancy box in troff
.ie t .ds \\$1 \(br\v'-1p'\(br\v'1p'\h'\\nhu'\\$2\h'\\nHu-\\nhu'\(br\l'-\\n([Wu\(ul'\v'-1p'\(br\l'-\\n([Wu\(rn'\v'1p'\*s
.el .ds \\$1 \\$2\*s
..
.[] Et \v'-1p'\*X\v'1p'
.[] En \v'-1p'\*N\v'1p'
.[] Be \v'-1p'\*L\v'1p'
.[] AP \v'-1p'\s\nsAPC\s0\v'1p'
.[] Bs \v'-1p'\s\nsBS\s0\v'1p'
.[] Cs \v'-1p'\s\nsCSI\s0\v'1p'
.[] Dc \v'-1p'\s\nsDCS\s0\v'1p'
.[] Os \v'-1p'\s\nsOSC\s0\v'1p'
.[] PM \v'-1p'\s\nsPM\s0\v'1p'
.[] ST \v'-1p'\s\nsST\s0\v'1p'
.[] Ta \v'-1p'\*T\v'1p'
.[] Lf \v'-1p'\s\nsLF\s0\v'1p'
.[] Vt \v'-1p'\s\nsVT\s0\v'1p'
.[] Ff \v'-1p'\s\nsFF\s0\v'1p'
.[] Cr \v'-1p'\s\nsCR\s0\v'1p'
.[] So \v'-1p'\s\nsSO\s0\v'1p'
.[] Sp \v'-1p'\s\nsSP\s0\v'1p'
.[] Si \v'-1p'\s\nsSI\s0\v'1p'
.[] Eb \v'-1p'\*e\v'1p'
.[] Ca \v'-1p'\*C\v'1p'
.[] Su \v'-1p'\*S\v'1p'
.[] Es \v'-1p'\*E\v'1p'
.[] Fs \v'-1p'\s\nsFS\s0\v'1p'
.[] Gs \v'-1p'\s\nsGS\s0\v'1p'
.[] Rs \v'-1p'\s\nsRS\s0\v'1p'
.[] Us \v'-1p'\s\nsUS\s0\v'1p'
.[] XX \v'-1p'\s\nsXX\s0\v'1p'
.[] $ $
.[] ! !
.[] # #
.[] (( (
.[] ) )
.[] * *
.[] + +
.[] 0 0
.[] 1 1
.[] 2 2
.[] 3 3
.[] 4 4
.[] 5 5
.[] 6 6
.[] 7 7
.[] 8 8
.[] 9 9
.[] : :
.[] ; ;
.[] = =
.[] / /
.[] < <
.[] > >
.[] ? ?
.[] @ @
.[] A A
.[] cB B
.[] C C
.[] D D
.[] E E
.[] F F
.[] G G
.[] H H
.[] I I
.[] J J
.[] K K
.[] L L
.[] M M
.[] N N
.[] O O
.[] P P
.[] R R
.[] S S
.[] T T
.[] V V
.[] W W
.[] XX X
.[] Y Y
.[] Z Z
.[] [[ [
.[] ]] ]
.[] bS \\e
.[] ] ]
.[] ^ ^
.[] _ _
.[] ` \`
.[] a a
.[] b b
.[] c c
.[] d d
.[] f f
.[] g g
.[] h h
.[] i i
.[] j j
.[] k k
.[] l l
.[] m m
.[] n n
.[] o o
.[] p p
.[] q q
.[] r r
.[] cs s
.[] t t
.[] x x
.[] | |
.[] } }
.[] ! !
.[] c" \(lq
.[] c~ ~
.[] Sc \fIc\fP
.ds Cc \fIC\fP
.ds Cb \fIC\v'.3m'\h'-.2m'\s-2b\s0\v'-.3m'\fP
.ds Cx \fIC\v'.3m'\h'-.2m'\s-2x\s0\v'-.3m'\fP
.ds Cy \fIC\v'.3m'\h'-.2m'\s-2y\s0\v'-.3m'\fP
.ds Ps \fIP\v'.3m'\h'-.2m'\s-2s\s0\v'-.3m'\fP
.ds Pm \fIP\v'.3m'\h'-.2m'\s-2m\s0\v'-.3m'\fP
.ds Pt \fIP\v'.3m'\h'-.2m'\s-2t\s0\v'-.3m'\fP
.ds Ix \fIx\fP
.ds Iy \fIy\fP
.ds Iw \fIw\fP
.ds Ih \fIh\fP
.ds Ir \fIr\fP
.ds Ic \fIc\fP
.ie t .nr LL 6.5i
.el .nr LL 72m
.if n .na
.TL
Xterm Control Sequences
.AU
Edward Moy
.AI
University of California, Berkeley
.sp
Revised by
.AU
Stephen Gildea
.AI
X Consortium (1994)
.AU
Thomas Dickey
.AI
XFree86 Project (1996)
.AU

.am BT                  \" add page numbers after first page
.ds CF %
..
.SH
Definitions
.IP \*(Sc
The literal character \fIc\fP.
.IP \*(Cc
A single (required) character.
.IP \*(Ps
A single (usually optional) numeric parameter, composed of one of more digits.
.IP \*(Pm
A multiple numeric parameter composed of any number of single numeric
parameters, separated by \*; character(s).
.IP \*(Pt
A text parameter composed of printable characters.
.
.SH
C1 (8-Bit) Control Characters
.ds RH C1 (8-Bit) Control Characters
.LP
The \fIxterm\fP program recognizes both 8-bit and 7-bit control characters.
It generates 7-bit controls (by default) or 8-bit if S8C1T is enabled.
The following pairs of 7-bit and 8-bit control characters are equivalent:
.St
.IP \\*(Es\\*D
Index (IND: 0x84)
.IP \\*(Es\\*E
Next Line (NEL: 0x85)
.IP \\*(Es\\*H
Tab Set (HTS: 0x88)
.IP \\*(Es\\*M
Reverse Index (RI: 0x8d)
.IP \\*(Es\\*N
Single Shift Select of G2 Character Set (SS2: 0x8e): affects next character only
.IP \\*(Es\\*O
Single Shift Select of G3 Character Set (SS3: 0x8f): affects next character only
.IP \\*(Es\\*P
Device Control String (DCS: 0x90)
.IP \\*(Es\\*V
Start of Guarded Area (SPA: 0x96)
.IP \\*(Es\\*W
End of Guarded Area (EPA: 0x97)
.IP \\*(Es\\(*X
Start of String (SOS: 0x98)
.IP \\*(Es\\*Z
Return Terminal ID (DECID: 0x9a).  Obsolete form of \*(Cs\*c (DA).
.IP \\*(Es\\*([[ 
Control Sequence Introducer (CSI: 0x9b)
.IP \\*(Es\\*(bS
String Terminator (ST: 0x9c)
.IP \\*(Es\\*(]] 
Operating System Command (OSC: 0x9d)
.IP \\*(Es\\*^ 
Privacy Message (PM: 0x9e)
.IP \\*(Es\\*_
Application Program Command (APC: 0x9f)
.Ed
.sp
.LP
These control characters are used in the vtXXX emulation.
.
.SH
VT100 Mode
.ds RH VT100 Mode
.LP
Most of these control sequences are standard VT102 control sequences,
but there is support for later DEC VT terminals (i.e., VT220 and VT320),
as well as ISO 6429 and \fIaixterm\fP color controls.
VT102 features not supported are
double size characters and blinking characters.
There are additional control sequences to provide
\fIxterm-\fPdependent functions, such as the scrollbar or window size.
Where the function is specified by DEC or ISO 6429, the code assigned
to it is given in parentheses.  The escape codes to designate and invoke
character sets are specified by ISO 2022; see that document for a
discussion of character sets.
.
.St
.\"
.\" Single-character functions
.\"
.IP \\*(Be
Bell (Ctrl-G)
.
.IP \\*(Bs
Backspace (Ctrl-H)
.
.IP \\*(Cr
Carriage Return (Ctrl-M)
.
.IP \\*(En
Return Terminal Status (Ctrl-E).
Response is the terminal name, e.g., "xterm".
.
.IP \\*(Ff
Form Feed or New Page (NP) (Ctrl-L) same as LF
.
.IP \\*(Lf
Line Feed or New Line (NL) (Ctrl-J)
.
.IP \\*(So
Shift Out (Ctrl-N) \(-> Switch to Alternate Character Set: invokes the
G1 character set.
.
.IP \\*(Sp
Space.
.
.IP \\*(Ta
Horizontal Tab (HT) (Ctrl-I)
.
.IP \\*(Vt
Vertical Tab (Ctrl-K) same as LF
.
.IP \\*(Si
Shift In (Ctrl-O) \(-> Switch to Standard Character Set: invokes the
G0 character set (the default).
.sp
.\"
.\" Controls beginning with ESC (other than those where ESC is part of a 7-bit
.\" equivalent to 8-bit C1 controls), ordered by the final character(s).
.\"
.IP \\*(Es\\*(Sp\\*F
7-bit controls (S7C1T).
.
.IP \\*(Es\\*(Sp\\*G
8-bit controls (S8C1T).
.
.IP \\*(Es\\*(Sp\\*L
Set ANSI conformance level 1 (vt100, 7-bit controls).
.
.IP \\*(Es\\*(Sp\\*M
Set ANSI conformance level 2 (vt200).
.
.IP \\*(Es\\*(Sp\\*N
Set ANSI conformance level 3 (vt300).
.
.IP \\*(Es\\*#\\*8
DEC Screen Alignment Test (DECALN)
.
.IP \\*(Es\\*(((\\*(Cc
Designate G0 Character Set (ISO 2022)
  \*(Cc = \*0 \(-> DEC Special Character and Line Drawing Set
  \*(Cc = \*A \(-> United Kingdom (UK)
  \*(Cc = \*(cB \(-> United States (USASCII)
.
.IP \\*(Es\\*)\\*(Cc
Designate G1 Character Set (ISO 2022)
  \*(Cc = \*0 \(-> DEC Special Character and Line Drawing Set
  \*(Cc = \*A \(-> United Kingdom (UK)
  \*(Cc = \*(cB \(-> United States (USASCII)
.
.IP \\*(Es\\**\\*(Cc
Designate G2 Character Set (ISO 2022)
  \*(Cc = \*0 \(-> DEC Special Character and Line Drawing Set
  \*(Cc = \*A \(-> United Kingdom (UK)
  \*(Cc = \*(cB \(-> United States (USASCII)
.
.IP \\*(Es\\*+\\*(Cc
Designate G3 Character Set (ISO 2022)
  \*(Cc = \*0 \(-> DEC Special Character and Line Drawing Set
  \*(Cc = \*A \(-> United Kingdom (UK)
  \*(Cc = \*(cB \(-> United States (USASCII)
.
.IP \\*(Es\\*7
Save Cursor (DECSC)
.
.IP \\*(Es\\*8
Restore Cursor (DECRC)
.
.IP \\*(Es\\*=
Application Keypad (DECPAM)
.
.IP \\*(Es\\*>
Normal Keypad (DECPNM)
.
.IP \\*(Es\\*F
Cursor to lower left corner of screen (if
enabled by the \fBhpLowerleftBugCompat\fP resource).
.
.IP \\*(Es\\*c
Full Reset (RIS)
.
.IP \\*(Es\\*l
Memory Lock (per HP terminals).  Locks memory above the cursor.
.
.IP \\*(Es\\*m
Memory Unlock (per HP terminals)
.
.IP \\*(Es\\*n
Invoke the G2 Character Set (LS2)
.
.IP \\*(Es\\*o
Invoke the G3 Character Set (LS3)
.
.IP \\*(Es\\*|
Invoke the G3 Character Set as GR (LS3R).  Has no visible effect in \fIxterm\fP.
.
.IP \\*(Es\\*}
Invoke the G2 Character Set as GR (LS2R).  Has no visible effect in \fIxterm\fP.
.
.IP \\*(Es\\*(c~
Invoke the G1 Character Set as GR (LS1R).  Has no visible effect in \fIxterm\fP.
.
.sp
.IP \\*(AP\\*(Pt\\*s\\*(ST
\fIxterm\fP implements no APC functions; \*(Pt is ignored.
\*(Pt need not be printable characters.
.
.sp
.\"
.\" Device-Control functions
.\"
.IP \\*(Dc\\*(Ps\\*;\\*(Ps\\*|\\*(Pt\\*s\\*(ST
User-Defined Keys (DECUDK).
The first parameter:
  \*(Ps = \*0 \(-> Clear all UDK definitions before starting (default)
  \*(Ps = \*1 \(-> Erase Below (default)
.br
The second parameter:
  \*(Ps = \*0 \(-> Lock the keys (default)
  \*(Ps = \*1 \(-> Do not lock.
.br
The third parameter is a ';'-separated list of strings denoting
the key-code separated by a '/' from the hex-encoded key value.
The key codes correspond to the DEC function-key codes (e.g., F6=17).
.
.IP \\*(Dc\\*$\\*q\\*(Pt\\*s\\*(ST
Request Status String (DECRQSS).
The string following the "q" is one of the following:
  \*(c"\*q	\(-> DECSCA
  \*(c"\*p	\(-> DECSCL
  \*r		\(-> DECSTBM
  \*m		\(-> SGR
.br
\fIxterm\fP responds with
\*(Dc\*1\*$\*r\*(Pt\*s\*(ST
for valid requests, replacing the \*(Pt with the corresponding \*(Cs
string,
or
\*(Dc\*0\*$\*r\*(Pt\*s\*(ST
for invalid requests.
.
.sp
.\"
.\" Functions using CSI, ordered by the final character(s)
.\"
.IP \\*(Cs\\*(Ps\\*s\\*@
Insert \*(Ps (Blank) Character(s) (default = 1) (ICH)
.
.IP \\*(Cs\\*(Ps\\*s\\*A
Cursor Up \*(Ps Times (default = 1) (CUU)
.
.IP \\*(Cs\\*(Ps\\*s\\*(cB
Cursor Down \*(Ps Times (default = 1) (CUD)
.
.IP \\*(Cs\\*(Ps\\*s\\*C
Cursor Forward \*(Ps Times (default = 1) (CUF)
.
.IP \\*(Cs\\*(Ps\\*s\\*D
Cursor Backward \*(Ps Times (default = 1) (CUB)
.
.IP \\*(Cs\\*(Ps\\*s\\*E
Cursor Next Line \*(Ps Times (default = 1) (CNL)
.
.IP \\*(Cs\\*(Ps\\*s\\*F
Cursor Preceding Line \*(Ps Times (default = 1) (CPL)
.
.IP \\*(Cs\\*(Ps\\*s\\*G
Cursor Character Absolute  [column] (default = [row,1]) (CHA)
.
.IP \\*(Cs\\*(Ps\\*s\\*;\\*(Ps\\*s\\*H
Cursor Position [row;column] (default = [1,1]) (CUP)
.
.IP \\*(Cs\\*(Ps\\*s\\*I
Cursor Forward Tabulation \*(Ps tab stops (default = 1) (CHT)
.
.IP \\*(Cs\\*(Ps\\*s\\*J
Erase in Display (ED)
  \*(Ps = \*0 \(-> Erase Below (default)
  \*(Ps = \*1 \(-> Erase Above
  \*(Ps = \*2 \(-> Erase All
.
.IP \\*(Cs\\*(Ps\\*s\\*?\\*s\\*J
Erase in Display (DECSED)
  \*(Ps = \*0 \(-> Selective Erase Below (default)
  \*(Ps = \*1 \(-> Selective Erase Above
  \*(Ps = \*2 \(-> Selective Erase All
.
.IP \\*(Cs\\*(Ps\\*s\\*K
Erase in Line (EL)
  \*(Ps = \*0 \(-> Erase to Right (default)
  \*(Ps = \*1 \(-> Erase to Left
  \*(Ps = \*2 \(-> Erase All
.
.IP \\*(Cs\\*(Ps\\*s\\*?\\*s\\*K
Erase in Line (DECSEL)
  \*(Ps = \*0 \(-> Selective Erase to Right (default)
  \*(Ps = \*1 \(-> Selective Erase to Left
  \*(Ps = \*2 \(-> Selective Erase All
.
.IP \\*(Cs\\*(Ps\\*s\\*L
Insert \*(Ps Line(s) (default = 1) (IL)
.
.IP \\*(Cs\\*(Ps\\*s\\*M
Delete \*(Ps Line(s) (default = 1) (DL)
.
.IP \\*(Cs\\*(Ps\\*s\\*P
Delete \*(Ps Character(s) (default = 1) (DCH)
.
.IP \\*(Cs\\*(Ps\\*s\\*S
Scroll up \*(Ps lines (default = 1) (SU)
.
.IP \\*(Cs\\*(Ps\\*s\\*T
Scroll down \*(Ps lines (default = 1) (SD, according to DEC)
.
.IP \\*(Cs\\*(Ps\\*s\\*;\\*(Ps\\*s\\*;\\*(Ps\\*s\\*;\\*(Ps\\*s\\*;\\*(Ps\\*s\\*T
Initiate hilite mouse tracking.
Parameters are [func;startx;starty;firstrow;lastrow].
See the section \fBMouse Tracking\fP.
.
.IP \\*(Cs\\*(Ps\\*s\\*(XX
Erase \*(Ps Character(s) (default = 1) (ECH)
.
.IP \\*(Cs\\*(Ps\\*s\\*Z
Cursor Backward Tabulation \*(Ps tab stops (default = 1) (CBT)
.
.IP \\*(Cs\\*(Ps\\*s\\*^
Scroll down \*(Ps lines (default = 1) (SD, according to ISO)
.
.IP \\*(Cs\\*(Pm\\*s\\*`
Character Position Absolute  [column] (default = [row,1]) (HPA)
.
.IP \\*(Cs\\*(Ps\\*s\\*b
Repeat the preceding graphic character \*(Ps times (REP)
.
.IP \\*(Cs\\*(Ps\\*s\\*c
Send Device Attributes (DA)
  \*(Ps = \*0 or omitted \(-> request attributes from terminal
  \(-> \*(Cs\*?\*1\*;\*2\*c (``I am a VT100 with Advanced Video Option.'')
.
.IP \\*(Cs\\*(Pm\\*s\\*d
Line Position Absolute  [row] (default = [1,column]) (VPA)
.
.IP \\*(Cs\\*(Ps\\*s\\*;\\*(Ps\\*s\\*f
Horizontal and Vertical Position [row;column] (default = [1,1]) (HVP)
.
.IP \\*(Cs\\*(Ps\\*s\\*g
Tab Clear (TBC)
  \*(Ps = \*0 \(-> Clear Current Column (default)
  \*(Ps = \*3 \(-> Clear All
.
.IP \\*(Cs\\*(Pm\\*s\\*h
Set Mode (SM)
  \*(Ps = \*4 \(-> Insert Mode (IRM)
  \*(Ps = \*2\*0 \(-> Automatic Newline (LNM)
.
.IP \\*(Cs\\*(Pm\\*s\\*l
Reset Mode (RM)
  \*(Ps = \*4 \(-> Replace Mode (IRM)
  \*(Ps = \*2\*0 \(-> Normal Linefeed (LNM)
.
.IP \\*(Cs\\*(Pm\\*s\\*m
Character Attributes (SGR)
  \*(Ps = \*0 \(-> Normal (default)
  \*(Ps = \*1 \(-> Bold
  \*(Ps = \*4 \(-> Underlined
  \*(Ps = \*5 \(-> Blink (appears as Bold)
  \*(Ps = \*7 \(-> Inverse
  \*(Ps = \*2\*2 \(-> Normal (neither bold nor faint)
  \*(Ps = \*2\*4 \(-> Not underlined
  \*(Ps = \*2\*5 \(-> Steady (not blinking)
  \*(Ps = \*2\*7 \(-> Positive (not inverse)
  \*(Ps = \*3\*0 \(-> Set foreground color to Black
  \*(Ps = \*3\*1 \(-> Set foreground color to Red
  \*(Ps = \*3\*2 \(-> Set foreground color to Green
  \*(Ps = \*3\*3 \(-> Set foreground color to Yellow
  \*(Ps = \*3\*4 \(-> Set foreground color to Blue
  \*(Ps = \*3\*5 \(-> Set foreground color to Magenta
  \*(Ps = \*3\*7 \(-> Set foreground color to Cyan
  \*(Ps = \*3\*7 \(-> Set foreground color to White
  \*(Ps = \*3\*9 \(-> Set foreground color to default (original)
  \*(Ps = \*4\*0 \(-> Set background color to Black
  \*(Ps = \*4\*1 \(-> Set background color to Red
  \*(Ps = \*4\*2 \(-> Set background color to Green
  \*(Ps = \*4\*3 \(-> Set background color to Yellow
  \*(Ps = \*4\*4 \(-> Set background color to Blue
  \*(Ps = \*4\*5 \(-> Set background color to Magenta
  \*(Ps = \*4\*6 \(-> Set background color to Cyan
  \*(Ps = \*4\*7 \(-> Set background color to White
  \*(Ps = \*4\*9 \(-> Set background color to default (original).
.sp
If 16-color support is compiled, the following apply.
Assume that \fIxterm\fP's resources
are set so that the ISO color codes are the first 8 of a set of 16.
Then the \fIaixterm\fP colors are the bright versions of the ISO colors:
  \*(Ps = \*9\*0 \(-> Set foreground color to Black
  \*(Ps = \*9\*1 \(-> Set foreground color to Red
  \*(Ps = \*9\*2 \(-> Set foreground color to Green
  \*(Ps = \*9\*3 \(-> Set foreground color to Yellow
  \*(Ps = \*9\*4 \(-> Set foreground color to Blue
  \*(Ps = \*9\*5 \(-> Set foreground color to Magenta
  \*(Ps = \*9\*7 \(-> Set foreground color to Cyan
  \*(Ps = \*9\*7 \(-> Set foreground color to White
  \*(Ps = \*1\*0\*0 \(-> Set background color to Black
  \*(Ps = \*1\*0\*1 \(-> Set background color to Red
  \*(Ps = \*1\*0\*2 \(-> Set background color to Green
  \*(Ps = \*1\*0\*3 \(-> Set background color to Yellow
  \*(Ps = \*1\*0\*4 \(-> Set background color to Blue
  \*(Ps = \*1\*0\*5 \(-> Set background color to Magenta
  \*(Ps = \*1\*0\*6 \(-> Set background color to Cyan
  \*(Ps = \*1\*0\*7 \(-> Set background color to White
.sp
If \fIxterm\fP is compiled with the 16-color support disabled, it supports
the following, from \fIrxvt\fP:
  \*(Ps = \*1\*0\*0 \(-> Set foreground and background color to default
.sp
.
.IP \\*(Cs\\*(Ps\\*s\\*n
Device Status Report (DSR)
  \*(Ps = \*5 \(-> Status Report \*(Cs\*0\*n (``OK'')
  \*(Ps = \*6 \(-> Report Cursor Position (CPR) [row;column] as
\*(Cs\*(Ir\*s\*;\*(Ic\*s\*R
.
.IP \\*(Cs\\*(Ps\\*s\\*?\\*n
Device Status Report (DSR, DEC-specific)
  \*(Ps = \*6 \(-> Report Cursor Position (CPR) [row;column] as
\*(Cs\*?\*(Ir\*s\*;\*(Ic\*s\*R
(assumes page is zero).
  \*(Ps = \*1\*5 \(-> Report Printer status as
\*(Cs\*?\*1\*0\*s\*n (ready)
or
\*(Cs\*?\*1\*1\*s\*n (not ready)
  \*(Ps = \*2\*5 \(-> Report UDK status as
\*(Cs\*?\*2\*0\*s\*n (unlocked)
or
\*(Cs\*?\*2\*1\*s\*n (locked)
  \*(Ps = \*2\*6 \(-> Report Keyboard status as
\*(Cs\*?\*2\*7\*s\*;\*s\*1\*s\*;\*s\*0\*s\*;\*s\*0\*s\*n (North American)
.
.IP \\*(Cs\\*s\\*!\\*p
Soft terminal reset (DECSTR)
.
.IP \\*(Cs\\*(Ps\\*s\\*;\\*(Ps\\*s\\*(c"\\*p
Set conformance level (DECSCL)
Valid values for the first parameter:
  \*(Ps = \*6\*1 \(-> VT100
  \*(Ps = \*6\*2 \(-> VT200
  \*(Ps = \*6\*3 \(-> VT300
Valid values for the second parameter:
  \*(Ps = \*0 \(-> 8-bit controls
  \*(Ps = \*1 \(-> 7-bit controls (always set for VT100)
  \*(Ps = \*2 \(-> 8-bit controls
.
.IP \\*(Cs\\*(Ps\\*s\\*(c"\\*q
Select character protection attribute (DECSCA).
Valid values for the parameter:
  \*(Ps = \*0 \(-> DECSED and DECSEL can erase (default)
  \*(Ps = \*1 \(-> DECSED and DECSEL cannot erase
  \*(Ps = \*2 \(-> DECSED and DECSEL can erase
.
.IP \\*(Cs\\*(Ps\\*s\\*;\\*(Ps\\*s\\*r
Set Scrolling Region [top;bottom] (default = full size of window) (DECSTBM)
.
.IP \\*(Cs\\*(Ps\\*s\\*;\\*s\\*(Ps\\*s\\*;\\*(Ps\\*s\\*t
Window manipulation (from \fIdtterm\fP).
Valid values for the first (and any additional parameters) are:
  \*(Ps = \*1 \(-> Restore (de-iconify) window.
  \*(Ps = \*2 \(-> Minimize (iconify) window.
  \*(Ps = \*3 ; \fIx ; y\fP \(-> Move window to [x, y].
  \*(Ps = \*4 ; \fIheight ; width\fP \(-> Resize the xterm window to height and width in pixels.
  \*(Ps = \*5 \(-> Raise the xterm window to the front of the stacking order.
  \*(Ps = \*6 \(-> Lower the xterm window to the bottom of the stacking order.
  \*(Ps = \*7 \(-> Refresh the xterm window.
  \*(Ps = \*8 ; \fIheight ; width\fP \(-> Resize the text area to [height;width] in characters.
  \*(Ps = \*1\*1 \(-> Report xterm window state.
If the xterm window is open (non-iconified), it returns \*(Cs\*1\*t.
If the xterm window is iconified, it returns \*(Cs\*2\*t.
  \*(Ps = \*1\*3 \(-> Report xterm window position as
\*(Cs\*3\*;\*(Ix\*;\*(Iy\*t
  \*(Ps = \*1\*4 \(-> Report xterm window in pixels as
\*(Cs\*s\*4\*s\*;\*s\fIheight\fP\*s\*;\*s\fIwidth\fP\*s\*t
  \*(Ps = \*1\*8 \(-> Report the size of the text area in characters as
\*(Cs\*s\*8\*s\*;\*s\fIheight\fP\*s\*;\*s\fIwidth\fP\*t
  \*(Ps = \*2\*0 \(-> Report xterm window's icon label as
\*(Os\*s\*L\*s\fIlabel\fP\*s\*(ST
  \*(Ps = \*2\*1 \(-> Report xterm window's title as
\*(Os\*s\*l\*s\fItitle\fP\*s\*(ST
  \*(Ps >= \*2\*4 \(-> Resize to \*(Ps lines (DECSLPP)
.
.IP \\*(Cs\\*(Ps\\*s\\*x
Request Terminal Parameters (DECREQTPARM)
.
.IP \\*(Cs\\*?\\*(Pm\\*s\\*h
DEC Private Mode Set (DECSET)
  \*(Ps = \*1 \(-> Application Cursor Keys (DECCKM)
  \*(Ps = \*2 \(-> Designate USASCII for character sets G0-G3 (DECANM),
and set VT100 mode.
  \*(Ps = \*3 \(-> 132 Column Mode (DECCOLM)
  \*(Ps = \*4 \(-> Smooth (Slow) Scroll (DECSCLM)
  \*(Ps = \*5 \(-> Reverse Video (DECSCNM)
  \*(Ps = \*6 \(-> Origin Mode (DECOM)
  \*(Ps = \*7 \(-> Wraparound Mode (DECAWM)
  \*(Ps = \*8 \(-> Auto-repeat Keys (DECARM)
  \*(Ps = \*9 \(-> Send Mouse X & Y on button press.
See the section \fBMouse Tracking\fP.
  \*(Ps = \*2\*5 \(-> Show Cursor (DECTCEM)
  \*(Ps = \*3\*8 \(-> Enter Tektronix Mode (DECTEK)
  \*(Ps = \*4\*0 \(-> Allow 80 \z\(<-\(-> 132 Mode
  \*(Ps = \*4\*1 \(-> \fImore\fP(1) fix (see \fIcurses\fP resource)
  \*(Ps = \*4\*4 \(-> Turn On Margin Bell
  \*(Ps = \*4\*5 \(-> Reverse-wraparound Mode
  \*(Ps = \*4\*6 \(-> Start Logging
(normally disabled by a compile-time option)
  \*(Ps = \*4\*7 \(-> Use Alternate Screen Buffer (unless
disabled by the \fBtiteInhibit\fP resource)
  \*(Ps = \*1\*0\*0\*0 \(-> Send Mouse X & Y on button press and release.
See the section \fBMouse Tracking\fP.
  \*(Ps = \*1\*0\*0\*1 \(-> Use Hilite Mouse Tracking.
See the section \fBMouse Tracking\fP.
.
.IP \\*(Cs\\*?\\*(Pm\\*s\\*l
DEC Private Mode Reset (DECRST)
  \*(Ps = \*1 \(-> Normal Cursor Keys (DECCKM)
  \*(Ps = \*2 \(-> Designate VT52 mode (DECANM).
  \*(Ps = \*3 \(-> 80 Column Mode (DECCOLM)
  \*(Ps = \*4 \(-> Jump (Fast) Scroll (DECSCLM)
  \*(Ps = \*5 \(-> Normal Video (DECSCNM)
  \*(Ps = \*6 \(-> Normal Cursor Mode (DECOM)
  \*(Ps = \*7 \(-> No Wraparound Mode (DECAWM)
  \*(Ps = \*8 \(-> No Auto-repeat Keys (DECARM)
  \*(Ps = \*9 \(-> Don't Send Mouse X & Y on button press
  \*(Ps = \*2\*5 \(-> Hide Cursor (DECTCEM)
  \*(Ps = \*4\*0 \(-> Disallow 80 \z\(<-\(-> 132 Mode
  \*(Ps = \*4\*1 \(-> No \fImore\fP(1) fix (see \fIcurses\fP resource)
  \*(Ps = \*4\*4 \(-> Turn Off Margin Bell
  \*(Ps = \*4\*5 \(-> No Reverse-wraparound Mode
  \*(Ps = \*4\*6 \(-> Stop Logging
(normally disabled by a compile-time option)
  \*(Ps = \*4\*7 \(-> Use Normal Screen Buffer
  \*(Ps = \*1\*0\*0\*0 \(-> Don't Send Mouse X & Y on button press and
release
  \*(Ps = \*1\*0\*0\*1 \(-> Don't Use Hilite Mouse Tracking
.
.IP \\*(Cs\\*?\\*(Pm\\*s\\*r
Restore DEC Private Mode Values.  The value of \*(Ps previously saved is
restored.  \*(Ps values are the same as for DECSET.
.
.IP \\*(Cs\\*?\\*(Pm\\*s\\*(cs
Save DEC Private Mode Values.  \*(Ps values are the same as for DECSET.
.
.sp
.IP \\*(Os\\*(Ps\\*s\\*;\\*(Pt\\*s\\*(ST
.IP \\*(Os\\*(Ps\\*s\\*;\\*(Pt\\*s\\*(Be
Set Text Parameters
  \*(Ps = \*0 \(-> Change Icon Name and Window Title to \*(Pt
  \*(Ps = \*1 \(-> Change Icon Name to \*(Pt
  \*(Ps = \*2 \(-> Change Window Title to \*(Pt
  \*(Ps = \*1\*0 \(-> Change color names starting with text foreground to \*(Pt (a
list of one or more color names, separated by semicolon, up to color6).
  \*(Ps = \*1\*1 \(-> Change colors starting with text background to \*(Pt
  \*(Ps = \*1\*2 \(-> Change colors starting with text cursor to \*(Pt
  \*(Ps = \*1\*3 \(-> Change colors starting with mouse foreground to \*(Pt
  \*(Ps = \*1\*4 \(-> Change colors starting with mouse background to \*(Pt
  \*(Ps = \*1\*5 \(-> Change colors starting with Tek foreground to \*(Pt
  \*(Ps = \*1\*6 \(-> Change colors starting with Tek background to \*(Pt
  \*(Ps = \*4\*6 \(-> Change Log File to \*(Pt (normally disabled by a
compile-time option)
  \*(Ps = \*5\*0 \(-> Set Font to \*(Pt
.
.sp
.IP \\*(PM\\*(Pt\\*s\\*(ST
\fIxterm\fP implements no PM functions; \*(Pt is ignored.
\*(Pt need not be printable characters.
.
.Ed
.
.SH
Mouse Tracking
.LP
The VT widget can be set to send the mouse position and other
information on button presses.  These modes are typically used by
editors and other full-screen applications that want to make use of
the mouse.
.LP
There are three mutually exclusive modes, each enabled (or disabled)
by a different parameter in the DECSET (or DECRST) escape sequence.
Parameters for all mouse tracking escape sequences generated by
\fIxterm\fP encode numeric parameters in a single character as
\fIvalue\fP+040.  For example, \*! is 1.  The screen
coodinate system is 1-based.
.LP
X10 compatibility mode sends an escape sequence on button press
encoding the location and the mouse button pressed.
It is enabled by specifying parameter 9 to DECSET.
On button press, \fIxterm\fP sends
\*(Cs\*M\*(Cb\*(Cx\*(Cy (6 characters).  \*(Cb is button\-1.
\*(Cx and \*(Cy are the x and y coordinates of the mouse when the
button was pressed.
.LP
Normal tracking mode sends an escape sequence on both button press and
release.  Modifier information is also sent.
It is enabled by specifying parameter 1000 to DECSET.
On button press or release, \fIxterm\fP sends
\*(Cs\*M\*(Cb\*(Cx\*(Cy.
The low two bits of \*(Cb encode button information:
0=MB1 pressed, 1=MB2 pressed, 2=MB3 pressed, 3=release.
The upper bits encode what modifiers were down when the button was
pressed and are added together.  4=Shift, 8=Meta, 16=Control.
\*(Cx and \*(Cy are the x and y coordinates of the mouse event.  The
upper left corner is (1,1).
.LP
Mouse hilite tracking notifies a program of a button press, receives a
range of lines from the program, highlights the region covered by
the mouse within that range until button release, and then sends the
program the release coordinates.
It is enabled by specifying parameter 1001 to DECSET.
Highlighting is performed only for button 1, though other button events
can be received.
Warning: use of this mode requires a cooperating program or it will
hang \fIxterm.\fP
On button press, the same information as for normal tracking is
generated; \fIxterm\fP then waits for the program to send mouse
tracking information.
\fIAll X events are ignored until the proper escape sequence is
received from the pty:\fP
\*(Cs\*(Ps\*s\*;\*(Ps\*s\*;\*(Ps\*s\*;\*(Ps\*s\*;\*(Ps\*s\*T.
The parameters are \fIfunc, startx, starty, firstrow,\fP and \fIlastrow.\fP
\fIfunc\fP is non-zero to initiate hilite tracking and zero to abort.
\fIstartx\fP and \fIstarty\fP give the starting x and y location for
the highlighted region.  The ending location tracks the mouse, but
will never be above row \fIfirstrow\fP and will always be above row
\fIlastrow.\fP  (The top of the screen is row 1.)
When the button is released, \fIxterm\fP reports the ending position
one of two ways: if the start and end coordinates are valid text
locations: \*(Cs\*t\*(Cx\*(Cy.  If either coordinate is past
the end of the line:
\*(Cs\*T\*(Cx\*(Cy\*(Cx\*(Cy\*(Cx\*(Cy.
The parameters are \fIstartx, starty, endx, endy, mousex,\fP and \fImousey.\fP
\fIstartx, starty, endx, \fPand\fI endy\fP give the starting and
ending character positions of the region.  \fImousex\fP and \fImousey\fP
give the location of the mouse at button up, which may not be over a
character.
.br
.ds RH Tektronix 4014 Mode
.SH
Tektronix 4014 Mode
.LP
Most of these sequences are standard Tektronix 4014 control sequences.
Graph mode supports the 12-bit addressing of the Tektronix 4014.
The major features missing are
the write-through and defocused modes.
This document does not describe the commands used in the various
Tektronix plotting modes but does describe the commands to switch modes.
.St
.IP \\*(Be
Bell (Ctrl-G)
.IP \\*(Bs
Backspace (Ctrl-H)
.IP \\*(Ta
Horizontal Tab (Ctrl-I)
.IP \\*(Lf
Line Feed or New Line (Ctrl-J)
.IP \\*(Vt
Cursor up (Ctrl-K)
.IP \\*(Ff
Form Feed or New Page (Ctrl-L)
.IP \\*(Cr
Carriage Return (Ctrl-M)
.IP \\*(Es\\*(Et
Switch to VT100 Mode (ESC Ctrl-C)
.IP \\*(Es\\*(En
Return Terminal Status (ESC Ctrl-E)
.IP \\*(Es\\*(Ff
PAGE (Clear Screen) (ESC Ctrl-L)
.IP \\*(Es\\*(So
Begin 4015 APL mode (ignored by \fIxterm\fP) (ESC Ctrl-N)
.IP \\*(Es\\*(Si
End 4015 APL mode (ignored by \fIxterm\fP) (ESC Ctrl-O)
.IP \\*(Es\\*(Eb
COPY (Save Tektronix Codes to file COPYyyyy-mm-dd.hh:mm:ss) (ESC Ctrl-W)
.IP \\*(Es\\*(Ca
Bypass Condition (ESC Ctrl-X)
.IP \\*(Es\\*(Su
GIN mode (ESC Ctrl-Z)
.IP \\*(Es\\*(Fs
Special Point Plot Mode (ESC Ctrl-\e)
.IP \\*(Es\\*8
Select Large Character Set
.IP \\*(Es\\*9
Select #2 Character Set
.IP \\*(Es\\*:
Select #3 Character Set
.IP \\*(Es\\*;
Select Small Character Set
.IP \\*(Os\\*(Ps\\*s\\*;\\*(Pt\\*s\\*(Be
Set Text Parameters of VT window
  \*(Ps = \*0 \(-> Change Icon Name and Window Title to \*(Pt
  \*(Ps = \*1 \(-> Change Icon Name to \*(Pt
  \*(Ps = \*2 \(-> Change Window Title to \*(Pt
  \*(Ps = \*4\*6 \(-> Change Log File to \*(Pt (normally disabled by a
compile-time option)
.IP \\*(Es\\*`
Normal Z Axis and Normal (solid) Vectors
.IP \\*(Es\\*a
Normal Z Axis and Dotted Line Vectors
.IP \\*(Es\\*b
Normal Z Axis and Dot-Dashed Vectors
.IP \\*(Es\\*c
Normal Z Axis and Short-Dashed Vectors
.IP \\*(Es\\*d
Normal Z Axis and Long-Dashed Vectors
.IP \\*(Es\\*h
Defocused Z Axis and Normal (solid) Vectors
.IP \\*(Es\\*i
Defocused Z Axis and Dotted Line Vectors
.IP \\*(Es\\*j
Defocused Z Axis and Dot-Dashed Vectors
.IP \\*(Es\\*k
Defocused Z Axis and Short-Dashed Vectors
.IP \\*(Es\\*l
Defocused Z Axis and Long-Dashed Vectors
.IP \\*(Es\\*p
Write-Thru Mode and Normal (solid) Vectors
.IP \\*(Es\\*q
Write-Thru Mode and Dotted Line Vectors
.IP \\*(Es\\*r
Write-Thru Mode and Dot-Dashed Vectors
.IP \\*(Es\\*(cs
Write-Thru Mode and Short-Dashed Vectors
.IP \\*(Es\\*t
Write-Thru Mode and Long-Dashed Vectors
.IP \\*(Fs
Point Plot Mode (Ctrl-\e)
.IP \\*(Gs
Graph Mode (Ctrl-])
.IP \\*(Rs
Incremental Plot Mode (Ctrl-^)
.IP \\*(Us
Alpha Mode (Ctrl-_)
.Ed
.
.
.br
.ds RH VT52 Mode
.SH
VT52 Mode
.LP
Parameters for cursor movement are at the end of the \*(Es\*Y escape sequence.
Each ordinate is encoded in a single character as \fIvalue\fP+040.
For example, \*! is 1.
The screen coodinate system is 0-based.
.St
.IP \\*(Es\\*A
Cursor up.
.IP \\*(Es\\*(cB
Cursor down.
.IP \\*(Es\\*C
Cursor right.
.IP \\*(Es\\*D
Cursor left.
.IP \\*(Es\\*F
Enter graphics mode.
.IP \\*(Es\\*G
Exit graphics mode.
.IP \\*(Es\\*H
Move the cursor to the home position.
.IP \\*(Es\\*I
Reverse line feed.
.IP \\*(Es\\*J
Erase from the cursor to the end of the screen.
.IP \\*(Es\\*K
Erase from the cursor to the end of the line.
.IP \\*(Es\\*Y\\*(Ps\\*s\\*(Ps
Move the cursor to given row and column.
.IP \\*(Es\\*Z
Identify
  \(-> \*(Es\*s\*/\*s\*Z (``I am a VT52.'')
.IP \\*(Es\\*=
Enter alternate keypad mode.
.IP \\*(Es\\*>
Exit alternate keypad mode.
.IP \\*(Es\\*<
Exit VT52 mode (Enter VT100 mode).
.Ed
.
.if n .pl \n(nlu+1v
