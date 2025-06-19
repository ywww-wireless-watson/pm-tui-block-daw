#include "edit_synth.h"

void initialize_synth_matrix(EditorData *data)
{
    data->matrix = malloc(data->rows * sizeof(int *));
    for (int i = 0; i < data->rows; i++)
    {
        data->matrix[i] = malloc(data->cols * sizeof(int));
    }
    data->result = malloc(data->rows * sizeof(int *));
    for (int i = 0; i < data->rows; i++)
    {
        data->result[i] = malloc(data->cols / 2 * sizeof(int));
    }
}

void cleanup_synth_editor(EditorData *data)
{
    for (int i = 0; i < data->rows; i++)
    {
        free(data->matrix[i]);
        free(data->result[i]);
    }
    free(data->matrix);
    free(data->result);
}

void init_synth_editor(EditorData *synth_data)
{
    synth_data->rows = 1;
    synth_data->cols = 2;
    initialize_synth_matrix(synth_data);

    synth_data->current_row = 0;
    synth_data->current_col = 0;
}

void handle_synth_editor(EditorData *synth_data, bool synth_rewrite)
{
    handle_input(synth_data->matrix, synth_data->rows, synth_data->cols, &synth_data->current_row, &synth_data->current_col, &synth_data->flag);
    if (synth_rewrite)
    {
        synth_data->flag = 0;
    }
    switch (synth_data->flag)
    {
    case 'w':
    {
        char message[256]; // Allocate a suitable buffer size
        convert_input(synth_data->matrix, synth_data->rows, synth_data->cols, synth_data->result);
        int line = synth_data->result[0][0];
        snprintf(message, sizeof(message), "ch %d:", line);
        char *param = input_window("WRITE", message, "");
        // Write param to the specified line in preview.txt
        if (param)
        {
            FILE *file = fopen("preview.txt", "r+");
            if (file)
            {
                char buffer[256];
                int current_line = 0;
                while (fgets(buffer, sizeof(buffer), file))
                {
                    if (current_line == line)
                    {
                        fseek(file, -strlen(buffer), SEEK_CUR);
                        // Remove newline character
                        buffer[strcspn(buffer, "\r\n")] = '\0';
                        fprintf(file, "%s\n", param);
                        break;
                    }
                    current_line++;
                }
                fclose(file);
                log_message("Parameter written to preview.txt.");
            }
            else
            {
                log_message("Error opening preview.txt.");
                perror("fopen");
            }
            free(param);
        }
        else
        {
            log_message("Canceled writing parameter.");
        }
    }
    break;
    case 0:
        WINDOW *win = get_main_window();
        WINDOW *log_win = get_log_window();
        wclear(win);
        // Display the contents of preview.txt
        FILE *file = fopen("preview.txt", "r");
        if (file)
        {
            char line[256];
            const int offset = 6;
            int i = offset;
            while (fgets(line, sizeof(line), file))
            {
                line[strcspn(line, "\r\n")] = '\0'; // Remove newline character
                mvwprintw(win, i, 0, "ch %d: %s", i - offset, line);
                i++;
            }
            fclose(file);
        }
        else
        {
            log_message("Error loading preview.txt.");
            perror("fopen");
        }
        display_matrix(synth_data->matrix, synth_data->rows, synth_data->cols, synth_data->current_row, synth_data->current_col);
        display_guide();
        display_label(2, 0);
        wrefresh(win);
        display_log_messages();
        wrefresh(log_win);
        break;
    default:
        break;
    }
}
