#!/bin/bash
# $XTermId: unascii.sh,v 1.2 2013/09/02 21:54:06 tom Exp $
# -----------------------------------------------------------------------------
# this file is part of xterm
#
# Copyright 2013 by Thomas E. Dickey
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
# display the characters recognized by xterm in AsciiEquivs
export PATH=$(dirname $(readlink -f $0)):$PATH
vxt-utf8 0x2010
vxt-utf8 0x2011
vxt-utf8 0x2012
vxt-utf8 0x2013
vxt-utf8 0x2014
vxt-utf8 0x2015
vxt-utf8 0x2212
vxt-utf8 0x2018
vxt-utf8 0x2019
vxt-utf8 0x201C
vxt-utf8 0x201D
vxt-utf8 0x2329
vxt-utf8 0x232a
