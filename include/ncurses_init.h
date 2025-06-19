#ifndef NCURSES_INIT_H
#define NCURSES_INIT_H

#include <ncurses.h>

// Initialize ncurses and set up the main window
void init_ncurses();
// End ncurses mode and clean up
void end_ncurses();
// Get the main ncurses window
WINDOW *get_main_window();

#endif // NCURSES_INIT_H