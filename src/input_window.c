#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include "input_window.h"

#define INPUT_LEN 50

// Internal helper function
static void update_input_field(WINDOW *win, const char *input, const char *suffix, const char *message, const char *title)
{
    werase(win);
    box(win, 0, 0);
    mvwprintw(win, 0, (getmaxx(win) - strlen(title)) / 2, "%s", title); // Update the title
    mvwprintw(win, 1, 2, "%s", message);
    mvwprintw(win, 3, 2, "> %s%s", input, suffix);
    wmove(win, 3, 4 + strlen(input)); // Move cursor to input field
    wrefresh(win);
}

char *input_window(const char *title, const char *message, const char *suffix)
{
    cbreak();
    keypad(stdscr, TRUE);
    curs_set(1); // Show cursor

    int row, col;
    getmaxyx(stdscr, row, col);

    // Window size and position
    int win_height = 5; // Adjusted height to accommodate the title
    int win_width = INPUT_LEN + 6 + strlen(suffix);
    int start_y = row / 2 - win_height / 2;
    int start_x = col / 2 - win_width / 2;

    WINDOW *input_win = newwin(win_height, win_width, start_y, start_x);
    box(input_win, 0, 0);
    keypad(input_win, TRUE);

    char input[INPUT_LEN + 1] = {0};
    int ch, pos = 0;
    update_input_field(input_win, input, suffix, message, title);

    // Input loop
    while ((ch = wgetch(input_win)) != '\n')
    {
        if (ch == KEY_BACKSPACE || ch == 127)
        {
            if (pos > 0)
            {
                input[--pos] = '\0';
            }
        }
        else if (ch >= 32 && ch <= 126)
        { // Printable ASCII characters
            if (pos < INPUT_LEN)
            {
                input[pos++] = ch;
                input[pos] = '\0';
            }
        }
        else
        {
            // If a non-printable character is entered, cancel input
            delwin(input_win);
            return NULL;
        }
        update_input_field(input_win, input, suffix, message, title);
    }

    // Dynamically allocate result string
    char *result = malloc(strlen(input) + strlen(suffix) + 1);
    if (result)
    {
        strcpy(result, input);
        strcat(result, suffix);
    }

    curs_set(0);
    delwin(input_win);
    return result;
}
