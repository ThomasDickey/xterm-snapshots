typedef unsigned short RegisterNum;

extern void parse_sixel(XtermWidget xw, ANSI *params, char const *string);
extern void parse_regis(XtermWidget xw, ANSI *params, char const *string);
extern void refresh_displayed_graphics(TScreen const *screen, int leftcol, int toprow, int ncols, int nrows);
extern void refresh_modified_displayed_graphics(TScreen const *screen);
extern void update_displayed_graphics_color_registers(TScreen const *screen, RegisterNum color, short r, short g, short b);
extern void scroll_displayed_graphics(int rows);
extern void erase_displayed_graphics(TScreen const *screen, int leftcol, int toprow, int ncols, int nrows);
extern void reset_displayed_graphics(TScreen const *screen);
