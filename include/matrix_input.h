#ifndef MATRIX_INPUT_H
#define MATRIX_INPUT_H
void init_matrix(int **matrix, int rows, int cols);
void display_matrix(int **matrix, int rows, int cols, int current_row, int current_col);
void display_guide();
int handle_input(int **matrix, int rows, int cols, int *current_row, int *current_col, int *flag);

#endif // MATRIX_INPUT_H