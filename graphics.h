extern void parse_sixel(XtermWidget xw, ANSI *params, char const *string);
extern void parse_regis(XtermWidget xw, ANSI *params, char const *string);
extern void refresh_displayed_graphics(XtermWidget xw, int leftcol, int toprow, int ncols, int nrows);
extern void scroll_displayed_graphics(int rows);
extern void erase_displayed_graphics(XtermWidget xw, int leftcol, int toprow, int ncols, int nrows);
extern void reset_displayed_graphics(void);
