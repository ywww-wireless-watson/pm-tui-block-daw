#include <stdlib.h>
#include "matrix_input.h"
#include "ncurses_init.h"

void init_matrix(int **matrix, int rows, int cols)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            matrix[i][j] = 0;
        }
    }
}

void display_matrix(int **matrix, int rows, int cols, int current_row, int current_col)
{
    WINDOW *win = get_main_window();
    // werase(win);
    int start_y = 1; // Change the starting y position
    int start_x = 8; // Change the starting x position
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            int display_col = j + (j / 2); // Adjust column position for the pattern
            if (i == current_row && j == current_col)
            {
                wattron(win, A_REVERSE);
            }
            else
            {
                wattron(win, A_BOLD);
            }
            mvwprintw(win, start_y + i, start_x + display_col * 1, "%1X", matrix[i][j]);
            wattroff(win, A_REVERSE | A_BOLD);
        }
    }
}

void display_guide()
{
    WINDOW *win = get_main_window();
    mvwprintw(win, LINES - 1, 1, "'o' to wave editor, 'i' to merge editor, 'u' to synth editor, 'y' to project editor");
    mvwprintw(win, LINES - 2, 0, "arrow keys to move, hex keys (0-9, a-f) to input.");
}

int handle_input(int **matrix, int rows, int cols, int *current_row, int *current_col, int *flag)
{
    int ch = getch();
    *flag = ch;
    bool updated = false;
    switch (ch)
    {
    case KEY_UP:
        if (*current_row > 0)
        {
            (*current_row)--;
            updated = true;
        }
        *flag = 0;
        break;
    case KEY_DOWN:
        if (*current_row < rows - 1)
        {
            (*current_row)++;
            updated = true;
        }
        *flag = 0;
        break;
    case KEY_LEFT:
        if (*current_col > 0)
        {
            (*current_col)--;
            updated = true;
        }
        *flag = 0;
        break;
    case KEY_RIGHT:
        if (*current_col < cols - 1)
        {
            (*current_col)++;
            updated = true;
        }
        *flag = 0;
        break;
    default:
        if ((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F'))
        {
            if (ch >= '0' && ch <= '9')
            {
                matrix[*current_row][*current_col] = ch - '0';
            }
            else if (ch >= 'a' && ch <= 'f')
            {
                matrix[*current_row][*current_col] = ch - 'a' + 10;
            }
            else if (ch >= 'A' && ch <= 'F')
            {
                matrix[*current_row][*current_col] = ch - 'A' + 10;
            }
            updated = true;
            *flag = 0;
        }
        break;
    }
    return updated ? 1 : 0;
}