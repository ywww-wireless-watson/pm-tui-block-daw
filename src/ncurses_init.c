#include "ncurses_init.h"

static WINDOW *main_window = NULL;

void init_ncurses()
{
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    curs_set(0);                             // Hide the cursor
    main_window = newwin(LINES, COLS, 0, 0); // Use full terminal size

    if (has_colors())
    {
        start_color();
        init_pair(1, COLOR_RED, COLOR_BLACK);
        init_pair(2, COLOR_GREEN, COLOR_BLACK);
        init_pair(3, COLOR_YELLOW, COLOR_BLACK);
        init_pair(4, COLOR_BLUE, COLOR_BLACK);
        init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(6, COLOR_CYAN, COLOR_BLACK);
        init_pair(7, COLOR_WHITE, COLOR_BLACK);
        init_pair(8, COLOR_BLACK, COLOR_WHITE);
    }
}

void end_ncurses()
{
    if (main_window)
    {
        delwin(main_window);
        main_window = NULL;
    }
    endwin();
}

WINDOW *get_main_window()
{
    return main_window;
}