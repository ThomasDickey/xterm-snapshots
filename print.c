/*
 * $XFree86: xc/programs/xterm/print.c,v 1.3 1998/01/11 03:48:41 dawes Exp $
 */

/************************************************************

Copyright 1997,1998 by Thomas E. Dickey <dickey@clark.net>

                        All Rights Reserved

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name(s) of the above copyright
holders shall not be used in advertising or otherwise to promote the
sale, use or other dealings in this Software without prior written
authorization.

********************************************************/

#ifdef HAVE_CONFIG_H
#include <xtermcfg.h>
#endif

#include <stdio.h>

#include "ptyx.h"
#include "data.h"
#include "error.h"
#include "xterm.h"

#define isForm(c) ((c) == '\r' || (c) == '\n' || (c) == '\f')
#define Strlen(a) strlen((char *)a)
#define Strcmp(a,b) strcmp((char *)a,(char *)b)
#define Strncmp(a,b,c) strncmp((char *)a,(char *)b,c)

#define SGR_MASK (BOLD|BLINK|UNDERLINE|INVERSE)

static void charToPrinter PROTO((int chr));
static void printCursorLine PROTO((void));
static void printLine PROTO((int row, int chr));
static void send_CharSet PROTO((int row));
static void send_SGR PROTO((unsigned attr, int fg, int bg));
static void stringToPrinter PROTO((char * str));

static FILE *Printer;

static void printCursorLine()
{
	register TScreen *screen = &term->screen;
	TRACE(("printCursorLine\n"))
	printLine(screen->cur_row, '\n');
}

/*
 * DEC's manual doesn't document whether trailing blanks are removed, or what
 * happens with a line that is entirely blank.  This function prints the
 * characters that xterm would allow as a selection (which may include blanks).
 */
static void printLine(row, chr)
	int row;
	int chr;
{
	register TScreen *screen = &term->screen;
	Char *c = SCRN_BUF_CHARS(screen, row);
	Char *a = SCRN_BUF_ATTRS(screen, row);
	Char attr = *a & SGR_MASK;
	int last = screen->max_col;
	int col;
#if OPT_ISO_COLORS && OPT_PRINT_COLORS
	register Char *fb = 0;
#endif
	int fg = -1, last_fg = -1;
	int bg = -1, last_bg = -1;

	TRACE(("printLine(row=%d, chr=%d)\n", row, chr))

	if_OPT_ISO_COLORS(screen,{
		fb = SCRN_BUF_COLOR(screen, row);
	})
	while (last > 0) {
		if ((a[last-1] & CHARDRAWN) == 0)
			last--;
		else
			break;
	}
	if (last) {
		if (screen->print_attributes) {
			send_CharSet(row);
			send_SGR(0,-1,-1);
		}
		for (col = 0; col < last; col++) {
#if OPT_PRINT_COLORS
			if_OPT_ISO_COLORS(screen,{
				if (screen->print_attributes > 1) {
					fg = (a[col] & FG_COLOR)
						? extract_fg(fb[col], a[col])
						: -1;
					bg = (a[col] & BG_COLOR)
						? extract_bg(fb[col])
						: -1;
				}
			})
#endif
			if ((((a[col] & SGR_MASK) != attr)
#if OPT_PRINT_COLORS
			    || (last_fg != fg) || (last_bg != bg)
#endif
			    )
			 && c[col]) {
				attr = (a[col] & SGR_MASK);
				last_fg = fg;
				last_bg = bg;
				if (screen->print_attributes)
					send_SGR(attr, fg, bg);
			}
			charToPrinter(c[col] ? c[col] : ' ');
		}
		if (screen->print_attributes)
			send_SGR(0,-1,-1);
	}
	charToPrinter('\r');
	charToPrinter(chr);
}

void xtermPrintScreen()
{
	register TScreen *screen = &term->screen;
	int top = screen->printer_extent ? 0 : screen->top_marg;
	int bot = screen->printer_extent ? screen->max_row : screen->bot_marg;

	TRACE(("xtermPrintScreen, rows %d..%d\n", top, bot))

	while (top <= bot)
		printLine(top++, '\n');
	if (screen->printer_formfeed)
		charToPrinter('\f');
}

static void send_CharSet(row)
	int row;
{
#if OPT_DEC_CHRSET
	register TScreen *screen = &term->screen;
	char *msg = 0;

	switch (SCRN_BUF_CSETS(screen, row)[0]) {
	case CSET_SWL:
		msg = "\033#5";
		break;
	case CSET_DHL_TOP:
		msg = "\033#3";
		break;
	case CSET_DHL_BOT:
		msg = "\033#4";
		break;
	case CSET_DWL:
		msg = "\033#6";
		break;
	}
	if (msg != 0)
		stringToPrinter(msg);
#endif /* OPT_DEC_CHRSET */
}

static void send_SGR(attr, fg, bg)
	unsigned attr;
	int fg;
	int bg;
{
	char msg[80];
	strcpy(msg, "\033[0");
	if (attr & BOLD)
		strcat(msg, ";1");
	if (attr & UNDERLINE)
		strcat(msg, ";2");
	if (attr & BLINK)
		strcat(msg, ";5");
	if (attr & INVERSE)	/* typo? DEC documents this as invisible */
		strcat(msg, ";7");
#if OPT_PRINT_COLORS
	if (bg >= 0) {
		sprintf(msg + strlen(msg), ";%d", (bg < 8) ? (40 + bg) : (92 + bg));
	}
	if (fg >= 0) {
		sprintf(msg + strlen(msg), ";%d", (fg < 8) ? (30 + fg) : (82 + fg));
	}
#endif
	strcat(msg, "m");
	stringToPrinter(msg);
}

/*
 * This implementation only knows how to write to a pipe.
 */
static void charToPrinter(chr)
	int chr;
{
	static int initialized;
	if (!initialized) {
		FILE	*input;
		int	my_pipe[2];
		int	my_pid;
		int	c;
		register TScreen *screen = &term->screen;

	    	if (pipe(my_pipe))
			SysError (ERROR_FORK);
		if ((my_pid = fork()) < 0)
			SysError (ERROR_FORK);

		if (my_pid == 0) {
			close(my_pipe[1]);	/* printer is silent */
			setgid (screen->gid);
			setuid (screen->uid);
			Printer = popen(screen->printer_command, "w");
			input = fdopen(my_pipe[0], "r");
			while ((c = fgetc(input)) != EOF) {
				fputc(c, Printer);
				if (isForm(c))
					fflush(Printer);
			}
			exit(0);
		} else {
			close(my_pipe[0]);	/* won't read from printer */
			Printer = fdopen(my_pipe[1], "w");
		}
		initialized++;
	}
	if (Printer != 0) {
		fputc(chr, Printer);
		if (isForm(chr))
			fflush(Printer);
	}
}

static void stringToPrinter(str)
	char *str;
{
	while (*str)
		charToPrinter(*str++);
}

/*
 * This module implements the MC (Media Copy) and related printing control
 * sequences for VTxxx emulation.  This is based on the description in the
 * VT330/VT340 Programmer Reference Manual EK-VT3XX-TP-001 (Digital Equipment
 * Corp., March 1987).
 */
void xtermMediaControl (param, private)
	int param;
	int private;
{
	register TScreen *screen = &term->screen;

	TRACE(("MediaCopy param=%d, private=%d\n", param, private))

	if (private) {
		switch (param) {
		case  1:
			printCursorLine();
			break;
		case  4:
			screen->printer_controlmode = 0;
			TRACE(("Reset autoprint mode\n"))
			break;
		case  5:
			screen->printer_controlmode = 1;
			TRACE(("Set autoprint mode\n"))
			break;
		}
	} else {
		switch (param) {
		case -1:
		case  0:
			xtermPrintScreen();
			break;
		case  4:
			screen->printer_controlmode = 0;
			TRACE(("Reset printer controller mode\n"))
			break;
		case  5:
			screen->printer_controlmode = 2;
			TRACE(("Set printer controller mode\n"))
			break;
		}
	}
}

/*
 * When in autoprint mode, the printer prints a line from the screen when you
 * move the cursor off that line with an LF, FF, or VT character, or an
 * autowrap occurs.  The printed line ends with a CR and the character (LF, FF
 * or VT) that moved the cursor off the previous line.
 */
void xtermAutoPrint(chr)
	int chr;
{
	register TScreen *screen = &term->screen;

	if (screen->printer_controlmode == 1) {
		TRACE(("AutoPrint %d\n", chr))
		printLine(screen->cursor_row, chr);
		if (Printer != 0)
			fflush(Printer);
	}
}

/*
 * When in printer controller mode, the terminal send received characters to
 * the printer without displaying them on the screen. The terminal sends all
 * characters and control sequences to the printer, except NUL, XON, XOFF, and
 * the printer controller sequences.
 *
 * This function eats characters, returning 0 as long as it must buffer or
 * divert to the printer.  We're only invoked here when in printer controller
 * mode, and handle the exit from that mode.
 */
#define LB '['

int xtermPrinterControl(chr)
	int chr;
{
	register TScreen *screen = &term->screen;

	static struct {
		Char seq[5];
		int active;
	} tbl[] = {
		{ { CSI,     '5', 'i' }, 2 },
		{ { CSI,     '4', 'i' }, 0 },
		{ { ESC, LB, '5', 'i' }, 2 },
		{ { ESC, LB, '4', 'i' }, 0 },
	};

	static Char bfr[10];
	static size_t length;
	size_t n;

	TRACE(("In printer:%d\n", chr))

	switch (chr) {
	case 0:
	case 'Q' & 0x1f:
	case 'S' & 0x1f:
		return 0;	/* ignored by application */

	case CSI:
	case ESC:
	case '[':
	case '4':
	case '5':
	case 'i':
		bfr[length++] = chr;
		for (n = 0; n < sizeof(tbl)/sizeof(tbl[0]); n++) {
			size_t len = Strlen(tbl[n].seq);

			if (length == len
			 && Strcmp(bfr, tbl[n].seq) == 0) {
				screen->printer_controlmode = tbl[n].active;
				length = 0;
				return 0;
			} else if (len > length
			 && Strncmp(bfr, tbl[n].seq, length) == 0) {
				return 0;
			}
		}
		length--;

		/* FALLTHRU */

	default:
		for (n = 0; n < length; n++)
			charToPrinter(bfr[n]);
		bfr[0] = chr;
		length = 1;
		return 0;
	}
}
