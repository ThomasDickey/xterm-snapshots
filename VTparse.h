/*
 *	$Xorg: VTparse.h,v 1.3 2000/08/17 19:55:08 cpqbld Exp $
 */

/* $XFree86: xc/programs/xterm/VTparse.h,v 3.15 2001/01/17 23:46:34 dawes Exp $ */
/*
 * Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.
 *
 *                         All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Digital Equipment
 * Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 *
 *
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

#ifndef included_VTparse_h
#define included_VTparse_h 1

#include <ptyx.h>

#ifndef Const
# if defined(__STDC__) && !defined(__cplusplus)
#  define Const const
# else
#  define Const /**/
# endif
#endif

/*
 * PARSE_T has to be large enough to handle the number of cases enumerated here.
 */
typedef char PARSE_T;

extern Const PARSE_T ansi_table[];
extern Const PARSE_T csi_ex_table[];
extern Const PARSE_T csi_quo_table[];
#if OPT_DEC_LOCATOR
extern Const PARSE_T csi_tick_table[];
#endif	/* OPT_DEC_LOCATOR */
extern Const PARSE_T csi_table[];
extern Const PARSE_T dec2_table[];
extern Const PARSE_T dec3_table[];
extern Const PARSE_T dec_table[];
extern Const PARSE_T eigtable[];
extern Const PARSE_T esc_sp_table[];
extern Const PARSE_T esc_table[];
extern Const PARSE_T iestable[];
extern Const PARSE_T igntable[];
extern Const PARSE_T scrtable[];
extern Const PARSE_T scstable[];
extern Const PARSE_T sos_table[];

#if OPT_VT52_MODE
extern Const PARSE_T vt52_table[];
extern Const PARSE_T vt52_esc_table[];
#endif

/*
 * The following list of definitions is generated from VTparse.def using the
 * following command line:
 *
 *     grep '^CASE_' VTparse.def | awk '{printf "#define %s %d\n", $1, n++}'
 *
 * If you need to change something, change VTparse.def and regenerate the
 * definitions.  This would have been automatic, but since this doesn't change
 * very often, it isn't worth the makefile hassle.
 */

#define CASE_GROUND_STATE 0
#define CASE_IGNORE_STATE 1
#define CASE_IGNORE_ESC 2
#define CASE_IGNORE 3
#define CASE_BELL 4
#define CASE_BS 5
#define CASE_CR 6
#define CASE_ESC 7
#define CASE_VMOT 8
#define CASE_TAB 9
#define CASE_SI 10
#define CASE_SO 11
#define CASE_SCR_STATE 12
#define CASE_SCS0_STATE 13
#define CASE_SCS1_STATE 14
#define CASE_SCS2_STATE 15
#define CASE_SCS3_STATE 16
#define CASE_ESC_IGNORE 17
#define CASE_ESC_DIGIT 18
#define CASE_ESC_SEMI 19
#define CASE_DEC_STATE 20
#define CASE_ICH 21
#define CASE_CUU 22
#define CASE_CUD 23
#define CASE_CUF 24
#define CASE_CUB 25
#define CASE_CUP 26
#define CASE_ED 27
#define CASE_EL 28
#define CASE_IL 29
#define CASE_DL 30
#define CASE_DCH 31
#define CASE_DA1 32
#define CASE_TRACK_MOUSE 33
#define CASE_TBC 34
#define CASE_SET 35
#define CASE_RST 36
#define CASE_SGR 37
#define CASE_CPR 38
#define CASE_DECSTBM 39
#define CASE_DECREQTPARM 40
#define CASE_DECSET 41
#define CASE_DECRST 42
#define CASE_DECALN 43
#define CASE_GSETS 44
#define CASE_DECSC 45
#define CASE_DECRC 46
#define CASE_DECKPAM 47
#define CASE_DECKPNM 48
#define CASE_IND 49
#define CASE_NEL 50
#define CASE_HTS 51
#define CASE_RI 52
#define CASE_SS2 53
#define CASE_SS3 54
#define CASE_CSI_STATE 55
#define CASE_OSC 56
#define CASE_RIS 57
#define CASE_LS2 58
#define CASE_LS3 59
#define CASE_LS3R 60
#define CASE_LS2R 61
#define CASE_LS1R 62
#define CASE_PRINT 63
#define CASE_XTERM_SAVE 64
#define CASE_XTERM_RESTORE 65
#define CASE_XTERM_TITLE 66
#define CASE_DECID 67
#define CASE_HP_MEM_LOCK 68
#define CASE_HP_MEM_UNLOCK 69
#define CASE_HP_BUGGY_LL 70
#define CASE_HPA 71
#define CASE_VPA 72
#define CASE_XTERM_WINOPS 73
#define CASE_ECH 74
#define CASE_CHT 75
#define CASE_CPL 76
#define CASE_CNL 77
#define CASE_CBT 78
#define CASE_SU 79
#define CASE_SD 80
#define CASE_S7C1T 81
#define CASE_S8C1T 82
#define CASE_ESC_SP_STATE 83
#define CASE_ENQ 84
#define CASE_DECSCL 85
#define CASE_DECSCA 86
#define CASE_DECSED 87
#define CASE_DECSEL 88
#define CASE_DCS 89
#define CASE_PM 90
#define CASE_SOS 91
#define CASE_ST 92
#define CASE_APC 93
#define CASE_EPA 94
#define CASE_SPA 95
#define CASE_CSI_QUOTE_STATE 96
#define CASE_DSR 97
#define CASE_ANSI_LEVEL_1 98
#define CASE_ANSI_LEVEL_2 99
#define CASE_ANSI_LEVEL_3 100
#define CASE_MC 101
#define CASE_DEC2_STATE 102
#define CASE_DA2 103
#define CASE_DEC3_STATE 104
#define CASE_DECRPTUI 105
#define CASE_VT52_CUP 106
#define CASE_REP 107
#define CASE_CSI_EX_STATE 108
#define CASE_DECSTR 109
#define CASE_DECDHL 110
#define CASE_DECSWL 111
#define CASE_DECDWL 112
#define CASE_DEC_MC 113
#define CASE_ESC_PERCENT 114
#define CASE_UTF8 115
#define CASE_CSI_TICK_STATE 116
#define CASE_DECELR 117
#define CASE_DECRQLP 118
#define CASE_DECEFR 119
#define CASE_DECSLE 120

#endif /* included_VTparse_h */
