#ifndef INPUT_WINDOW_H
#define INPUT_WINDOW_H

// Opens an ncurses input window and returns the entered string with the given suffix appended.
char *input_window(const char *title, const char *message, const char *suffix);

#endif
