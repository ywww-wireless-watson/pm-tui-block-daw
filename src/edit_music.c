#include "edit_music.h"

WINDOW *log_win;
char *log_messages[MAX_LOG_MESSAGES];
int log_message_count = 0;

#define LINES_GEN 1024

WINDOW *get_log_window()
{
    return log_win;
}

void initialize_log_window()
{
    int log_win_height = MAX_LOG_MESSAGES + 2;
    int log_win_width = 50;
    log_win = newwin(log_win_height, log_win_width, 0, COLS - log_win_width);
    box(log_win, 0, 0);
    mvwprintw(log_win, 0, 1, " Log ");
    wrefresh(log_win);
}

void display_log_messages()
{
    int log_win_height = MAX_LOG_MESSAGES + 2;
    int log_win_width = 50;
    wresize(log_win, log_win_height, log_win_width);
    mvwin(log_win, 0, COLS - log_win_width);
    werase(log_win);
    box(log_win, 0, 0);
    mvwprintw(log_win, 0, 1, " Log ");
    for (int i = 0; i < log_message_count; i++)
    {
        mvwprintw(log_win, i + 1, 1, "%s", log_messages[i]);
    }
    wrefresh(log_win);
}

void log_message(const char *message)
{
    if (log_message_count >= MAX_LOG_MESSAGES)
    {
        free(log_messages[0]);
        memmove(log_messages, log_messages + 1, (MAX_LOG_MESSAGES - 1) * sizeof(char *));
        log_message_count--;
    }
    log_messages[log_message_count] = strdup(message);
    log_message_count++;
    display_log_messages();
}

void cleanup_log_messages()
{
    for (int i = 0; i < log_message_count; i++)
    {
        free(log_messages[i]);
    }
}

void display_label(int type, int step)
{
    WINDOW *win = get_main_window();
    switch (type)
    {
    case 0:
        mvwprintw(win, LINES - 3, 0, "'p' to preview, 's' to save, 'l' to load, 't' to set time step");
        mvwprintw(win, 0, 0, "Wave Editor");
        mvwprintw(win, 0, 16, "y = sin(a1 + b1x + csin(a2 + b2x))");
        mvwprintw(win, 1, 0, " a1 = 0x");
        mvwprintw(win, 1, 10, " / 0x10 * 2pi");
        mvwprintw(win, 2, 0, " b1 = 0x");
        mvwprintw(win, 2, 10, " / 0x10 * 2pi");
        mvwprintw(win, 3, 0, " c  = 0x");
        mvwprintw(win, 3, 10, " / 0x10 * 2pi");
        mvwprintw(win, 4, 0, " a2 = 0x");
        mvwprintw(win, 4, 10, " / 0x10 * 2pi");
        mvwprintw(win, 5, 0, " b2 = 0x");
        mvwprintw(win, 5, 10, " / 0x10 * 2pi");
        mvwprintw(win, 6, 0, "steps 0x");
        for (int i = 0; i < step; i++)
        {
            mvwprintw(win, 7 + i, 0, "note%d 0x", i + 1);
        }
        break;
    case 1:
        mvwprintw(win, 0, 0, "Merge Wave Block");
        mvwprintw(win, LINES - 4, 0, "'p' to preview, 'x' to preview current column only, 'z' to resize grid");
        mvwprintw(win, LINES - 3, 0, "'s' to save, 'l' to load, 'q' to export without preview");
        for (int i = 0; i < 16; i++)
        {
            mvwprintw(win, 1 + i, 0, "ch %2d: ", i + 1);
        }
        break;
    case 2:
        mvwprintw(win, 0, 0, "Synth Editor");
        mvwprintw(win, LINES - 3, 0, "'w' to write");
        mvwprintw(win, 1, 0, " ch = 0x");
        mvwprintw(win, 3, 0, "wave volume A D S R pan(L:0 R:1)");
        mvwprintw(win, 4, 0, "wave: 0=Sine, 1=Triangle, 2=Square, 3=Sawtooth, 4=High Noise, 5=Medium Noise, 6=Low Noise");
        break;
    case 3:
        mvwprintw(win, 0, 0, "Project Editor");
        mvwprintw(win, LINES - 3, 0, "'s' to save, 'l' to load, 'q' to quit");
    default:
        break;
    }
}

void convert_input(int **matrix, int rows, int cols, int **result)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols / 2; j++)
        {
            result[i][j] = matrix[i][2 * j] * 16 + matrix[i][2 * j + 1];
        }
    }
}

void decode_input(int **result, int rows, int cols, int **matrix)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols / 2; j++)
        {
            matrix[i][2 * j] = result[i][j] / 16;
            matrix[i][2 * j + 1] = result[i][j] % 16;
        }
    }
}

void process_wave_editor_input(EditorData *wave_data, bool wave_rewrite)
{
    if (wave_rewrite)
    {
        // Simulate a key press. Notify the system that the space key was pressed.
        ungetch(' ');
    }
    if (handle_input(wave_data->matrix, wave_data->rows, wave_data->cols, &wave_data->current_row, &wave_data->current_col, &wave_data->flag) == 1 || wave_rewrite)
    {
        WINDOW *win = get_main_window();
        wclear(win);
        mvwprintw(win, 1, 32, "Time Step: %d", wave_data->time_step);

        if (wave_data->flag != 0 && !wave_rewrite)
        {
            return;
        }

        convert_input(wave_data->matrix, wave_data->rows, wave_data->cols, wave_data->result);

        double a1 = wave_data->result[0][0] / 16.0 * 2 * M_PI;
        double b1 = wave_data->result[1][0] / 16.0 * 2 * M_PI;
        double c = wave_data->result[2][0] / 16.0 * 2 * M_PI;
        double a2 = wave_data->result[3][0] / 16.0 * 2 * M_PI;
        double b2 = wave_data->result[4][0] / 16.0 * 2 * M_PI;
        int step = wave_data->result[5][0];
        int length = UNIT_TIME;

        double prev_fx = sin(a1 + b1 * 0 + c * sin(a2 + b2 * 0));
        int prev_y = (int)((LINES / 2 - 1) * (1 - prev_fx)) + 1;

        for (int i = 1; i < length; i++)
        {
            double x = (double)i / length;
            double fx = sin(a1 + b1 * x + c * sin(a2 + b2 * x));
            int displayy = (int)((LINES / 2 - 1) * (1 - fx)) + 1;
            int displayx = (int)(x * COLS);
            int color_pair = 1 + (displayy * step / LINES) % 2;

            if (displayy >= 0 && displayy < LINES)
            {
                wattron(win, COLOR_PAIR(color_pair));
                mvwaddch(win, displayy, displayx, '*');
                wattroff(win, COLOR_PAIR(color_pair));
            }

            if (abs(displayy - prev_y) > 1)
            {
                int min_y = displayy < prev_y ? displayy : prev_y;
                int max_y = displayy > prev_y ? displayy : prev_y;
                for (int j = min_y + 1; j < max_y; j++)
                {
                    color_pair = 1 + (j * step / LINES) % 2;
                    wattron(win, COLOR_PAIR(color_pair));
                    mvwaddch(win, j, displayx, '*');
                    wattroff(win, COLOR_PAIR(color_pair));
                }
            }

            prev_y = displayy;
        }

        display_matrix(wave_data->matrix, wave_data->rows, wave_data->cols, wave_data->current_row, wave_data->current_col);
        display_guide();
        display_label(0, step);
        wrefresh(win);
        display_log_messages();
        wrefresh(log_win);
    }
}

void initialize_wave_tracks(TrackW **wave_tracks, uint16_t num_wave_tracks)
{
    *wave_tracks = (TrackW *)malloc(num_wave_tracks * sizeof(TrackW));
    if (*wave_tracks == NULL)
    {
        perror("malloc");
        exit(1);
    }

    for (uint16_t i = 0; i < num_wave_tracks; i++)
    {
        (*wave_tracks)[i].num_events = MAX_EVENTS;
        (*wave_tracks)[i].events = (EventW *)malloc((*wave_tracks)[i].num_events * sizeof(EventW));
        if ((*wave_tracks)[i].events == NULL)
        {
            perror("malloc");
            for (uint16_t j = 0; j < i; j++)
            {
                free((*wave_tracks)[j].events);
            }
            free(*wave_tracks);
            exit(1);
        }
    }
}

void initialize_wave_matrix(EditorData *data)
{
    data->matrix = malloc(data->rows * sizeof(int *));
    for (int i = 0; i < data->rows; i++)
    {
        data->matrix[i] = malloc(data->cols * sizeof(int));
        memset(data->matrix[i], 0, data->cols * sizeof(int));
    }
    data->result = malloc(data->rows * sizeof(int *));
    for (int i = 0; i < data->rows; i++)
    {
        data->result[i] = malloc(data->cols / 2 * sizeof(int));
        memset(data->result[i], 0, data->cols / 2 * sizeof(int));
    }
}

void cleanup_wave_editor(EditorData *data, TrackW *wave_tracks)
{
    for (int i = 0; i < data->rows; i++)
    {
        free(data->matrix[i]);
        free(data->result[i]);
    }
    free(data->matrix);
    free(data->result);
    free(wave_tracks[0].events);
    free(wave_tracks);
}

void cleanup_merge_editor(EditorData *data)
{
    for (int i = 0; i < data->rows; i++)
    {
        free(data->matrix[i]);
    }
    free(data->matrix);
}

int generate_wave_events(int **wave_result, TrackW *wave_tracks, int wave_length, int wave_step, int time_step)
{
    wave_tracks[0].num_events = MAX_EVENTS;
    double a1 = wave_result[0][0] / 16.0 * 2 * M_PI;
    double b1 = wave_result[1][0] / 16.0 * 2 * M_PI;
    double c = wave_result[2][0] / 16.0 * 2 * M_PI;
    double a2 = wave_result[3][0] / 16.0 * 2 * M_PI;
    double b2 = wave_result[4][0] / 16.0 * 2 * M_PI;

    double prev_fx = sin(a1 + b1 * 0 + c * sin(a2 + b2 * 0));
    int prev_y = LINES_GEN - (int)((LINES_GEN / 2 - 1) * (1 - prev_fx)) - 1;
    int event_index = 0;
    int init_step = prev_y * wave_step / LINES_GEN;
    wave_tracks[0].events[event_index].time = 0;
    if (wave_result[6 + init_step][0] == 0)
    {
        wave_tracks[0].events[event_index].velocity = 0x00;
    }
    else
    {
        wave_tracks[0].events[event_index].velocity = 0xFF;
    }
    wave_tracks[0].events[event_index].note_number = wave_result[6 + init_step][0];
    event_index++;

    for (int i = time_step; i <= wave_length; i += time_step)
    {
        double x = (double)i / wave_length;
        double fx = sin(a1 + b1 * x + c * sin(a2 + b2 * x));
        int displayy = LINES_GEN - (int)((LINES_GEN / 2 - 1) * (1 - fx)) - 1;

        if (i > wave_length - time_step)
        {
            log_message("Wave length exceeded.");
            int prev_step = prev_y * wave_step / LINES_GEN;
            if (event_index < wave_tracks[0].num_events)
            {
                wave_tracks[0].events[event_index].time = i;
                wave_tracks[0].events[event_index].velocity = 0x00;
                wave_tracks[0].events[event_index].note_number = wave_result[6 + prev_step][0];
                event_index++;
            }
        }
        else if (prev_y * wave_step / LINES_GEN != displayy * wave_step / LINES_GEN)
        {
            int prev_step = prev_y * wave_step / LINES_GEN;
            int next_step = displayy * wave_step / LINES_GEN;
            if (event_index < wave_tracks[0].num_events - 1)
            {
                wave_tracks[0].events[event_index].time = i;
                wave_tracks[0].events[event_index].velocity = 0x00;
                wave_tracks[0].events[event_index].note_number = wave_result[6 + prev_step][0];
                event_index++;
                wave_tracks[0].events[event_index].time = i;
                if (wave_result[6 + next_step][0] == 0)
                {
                    wave_tracks[0].events[event_index].velocity = 0x00;
                }
                else
                {
                    wave_tracks[0].events[event_index].velocity = 0xFF;
                }
                wave_tracks[0].events[event_index].note_number = wave_result[6 + next_step][0];
                event_index++;
            }
        }
        prev_y = displayy;
    }
    wave_tracks[0].num_events = event_index;
    return event_index;
}

void write_wave_midi_file(TrackW *wave_tracks, uint16_t num_wave_tracks)
{
    FILE *file = fopen("preview.mid", "wb");
    if (file == NULL)
    {
        perror("fopen");
        exit(1);
    }

    write_events(file, wave_tracks, num_wave_tracks);
    fclose(file);
}

void init_wave_editor(EditorData *wave_data, uint16_t *num_wave_tracks, TrackW **wave_tracks)
{
    *num_wave_tracks = 1;
    initialize_wave_tracks(wave_tracks, *num_wave_tracks);

    wave_data->rows = 32;
    wave_data->cols = 2;
    initialize_wave_matrix(wave_data);

    wave_data->flag = 0;
    wave_data->current_row = 0;
    wave_data->current_col = 0;

    wave_data->time_step = 1;

    init_ncurses();
    init_matrix(wave_data->matrix, wave_data->rows, wave_data->cols);
    initialize_log_window();
}

void init_merge_editor(EditorData *merge_data)
{
    merge_data->rows = 4;
    merge_data->cols = 8;
    initialize_wave_matrix(merge_data);

    merge_data->current_row = 0;
    merge_data->current_col = 0;
}

void handle_wave_editor(EditorData *wave_data, TrackW *wave_tracks, uint16_t num_wave_tracks)
{
    process_wave_editor_input(wave_data, false);
    switch (wave_data->flag)
    {
    case 'p':
    {
        int step = wave_data->result[5][0];
        int length = UNIT_TIME;

        generate_wave_events(wave_data->result, wave_tracks, length, step, wave_data->time_step);

        write_wave_midi_file(wave_tracks, num_wave_tracks);

        const char *midi_filename = "preview.mid";
        const char *output_filename = "preview.wav";
        const char *config_filename = "preview.txt";
        int bpm = 120;
        const char *supplement = "supplement.txt";
        FILE *supplement_file = fopen(supplement, "rb");
        if (supplement_file)
        {
            if (fscanf(supplement_file, "%d", &bpm) != 1)
            {
                log_message("Error reading BPM from config file. Using default BPM 120.");
                bpm = 120;
            }
            fclose(supplement_file);
        }
        else
        {
            log_message("Config file not found. Using default BPM 120.");
        }

        process_midi_to_wav(midi_filename, output_filename, config_filename, bpm);
        play_wav(output_filename);
        log_message("Preview played.");
        wave_data->flag = 0;
    }
    break;
    case 's':
    {
        char *filename = input_window("SAVE", "input wave block name", ".wblk");
        if (filename)
        {
            FILE *file = fopen(filename, "wb");
            if (file)
            {
                fwrite(&wave_data->rows, sizeof(int), 1, file);
                fwrite(&wave_data->cols, sizeof(int), 1, file);
                for (int i = 0; i < wave_data->rows; i++)
                {
                    for (int j = 0; j < wave_data->cols / 2; j++)
                    {
                        fwrite(&wave_data->result[i][j], sizeof(int), 1, file);
                    }
                }
                fclose(file);
                log_message("Wave block saved.");

                char supplement[256];
                int n = snprintf(supplement, sizeof(supplement), "%s.wbsp", filename);
                if (n < 0 || n >= (int)sizeof(supplement))
                {
                    log_message("Supplement filename truncated.");
                }

                FILE *file = fopen(supplement, "wb");
                if (file)
                {
                    fwrite(&wave_data->time_step, sizeof(int), 1, file);
                    fclose(file);
                    log_message("Wave block supplement saved.");
                }
                else
                {
                    log_message("Error saving wave block supplement.");
                    perror("fopen");
                }
            }
            else
            {
                log_message("Error saving wave block.");
                perror("fopen");
            }
            free(filename);
        }
        else
        {
            log_message("Canceled saving wave block.");
        }
        process_wave_editor_input(wave_data, true);
    }
    break;
    case 'l':
    {
        char *filename = input_window("LOAD", "input wave block name", ".wblk");
        if (filename)
        {
            FILE *file = fopen(filename, "rb");
            if (file)
            {
                int saved_rows, saved_cols;
                if (fread(&saved_rows, sizeof(int), 1, file) != 1 ||
                    fread(&saved_cols, sizeof(int), 1, file) != 1)
                {
                    log_message("Error reading wave block file header.");
                    fclose(file);
                    free(filename);
                    break;
                }

                // Free existing matrix and result
                for (int i = 0; i < wave_data->rows; i++)
                {
                    free(wave_data->matrix[i]);
                    free(wave_data->result[i]);
                }
                free(wave_data->matrix);
                free(wave_data->result);

                // Allocate new matrix and result with the loaded size
                wave_data->rows = saved_rows;
                wave_data->cols = saved_cols;
                wave_data->matrix = malloc(wave_data->rows * sizeof(int *));
                wave_data->result = malloc(wave_data->rows * sizeof(int *));
                for (int i = 0; i < wave_data->rows; i++)
                {
                    wave_data->matrix[i] = malloc(wave_data->cols * sizeof(int));
                    wave_data->result[i] = malloc(wave_data->cols / 2 * sizeof(int));
                }

                // Load data into the new matrix and result
                for (int i = 0; i < wave_data->rows; i++)
                {
                    for (int j = 0; j < wave_data->cols / 2; j++)
                    {
                        if (fread(&wave_data->result[i][j], sizeof(int), 1, file) != 1)
                        {
                            log_message("Error reading wave block data.");
                            fclose(file);
                            free(filename);
                            break;
                        }
                    }
                }
                decode_input(wave_data->result, wave_data->rows, wave_data->cols, wave_data->matrix);
                log_message("Wave block loaded with new size.");
                fclose(file);

                char supplement[256];
                snprintf(supplement, sizeof(supplement), "%s.wbsp", filename);
                FILE *file = fopen(supplement, "rb");
                if (file)
                {
                    if (fread(&wave_data->time_step, sizeof(int), 1, file) != 1)
                    {
                        log_message("Error reading wave block supplement file.");
                    }
                    else
                    {
                        log_message("Wave block supplement loaded.");
                    }
                    fclose(file);
                }
                else
                {
                    log_message("No wave block supplement file found.");
                    wave_data->time_step = 1;
                }
            }
            else
            {
                log_message("Error loading wave block.");
                perror("fopen");
            }
            free(filename);
        }
        else
        {
            log_message("Canceled loading wave block.");
        }
        process_wave_editor_input(wave_data, true);
    }
    break;
    case 't':
    {
        char *time_step_str = input_window("TIME STEP", "input time step", "");
        if (time_step_str)
        {
            wave_data->time_step = atoi(time_step_str);
            free(time_step_str);
        }
    }
    default:
        break;
    }
}

void handle_merge_editor(EditorData *merge_data, bool merge_rewrite)
{
    handle_input(merge_data->matrix, merge_data->rows, merge_data->cols, &merge_data->current_row, &merge_data->current_col, &merge_data->flag);
    if (merge_rewrite)
    {
        merge_data->flag = 0;
    }
    switch (merge_data->flag)
    {
    case 'p':
    case 'q':
    {
        convert_input(merge_data->matrix, merge_data->rows, merge_data->cols, merge_data->result);
        TrackW *merge_tracks;
        uint16_t num_merge_tracks = merge_data->rows;
        initialize_wave_tracks(&merge_tracks, num_merge_tracks);

        for (int row = 0; row < merge_data->rows; row++)
        {
            merge_tracks[row].num_events = MAX_EVENTS;
            merge_tracks[row].events = (EventW *)malloc(merge_tracks[row].num_events * sizeof(EventW));
            if (merge_tracks[row].events == NULL)
            {
                perror("malloc");
                exit(1);
            }
            int event_offset = 0;
            TrackW *temp_tracks;
            initialize_wave_tracks(&temp_tracks, 1);
            for (int i = 0; i < merge_data->cols / 2; i++)
            {
                int filenum = merge_data->result[row][i];
                char filename[256];
                sprintf(filename, "%d.wblk", filenum);
                FILE *file = fopen(filename, "rb");
                if (filenum > 0)
                {
                    if (file)
                    {
                        int saved_rows, saved_cols;
                        if (fread(&saved_rows, sizeof(int), 1, file) != 1 ||
                            fread(&saved_cols, sizeof(int), 1, file) != 1)
                        {
                            log_message("Error reading file.");
                            fclose(file);
                            continue;
                        }
                        int **temp_wave_result = malloc(saved_rows * sizeof(int *));
                        if (!temp_wave_result)
                        {
                            log_message("Memory allocation failed.");
                            fclose(file);
                            continue;
                        }
                        for (int j = 0; j < saved_rows; j++)
                        {
                            temp_wave_result[j] = malloc(saved_cols / 2 * sizeof(int));
                            size_t nread = fread(temp_wave_result[j], sizeof(int), saved_cols / 2, file);
                            if (!temp_wave_result[j] || nread != (size_t)(saved_cols / 2))
                            {
                                log_message("Error reading file or memory allocation failed.");
                                for (int k = 0; k <= j; k++)
                                {
                                    free(temp_wave_result[k]);
                                }
                                free(temp_wave_result);
                                fclose(file);
                                continue;
                            }
                        }
                        fclose(file);

                        char supplement[256];
                        int n = snprintf(supplement, sizeof(supplement), "%s.wbsp", filename);
                        if (n < 0 || n >= (int)sizeof(supplement))
                        {
                            log_message("Supplement filename truncated.");
                        }

                        FILE *file = fopen(supplement, "rb");
                        int time_step = 1;
                        if (file)
                        {
                            if (fread(&time_step, sizeof(int), 1, file) != 1)
                            {
                                log_message("Error reading wave block supplement file.");
                                fclose(file);
                            }
                            fclose(file);
                        }
                        else
                        {
                            log_message("No wave block supplement file found.");
                        }

                        int step = temp_wave_result[5][0];
                        int length = UNIT_TIME;
                        int event_count = generate_wave_events(temp_wave_result, temp_tracks, length, step, time_step);
                        for (int j = 0; j < event_count; j++)
                        {
                            temp_tracks[0].events[j].time += i * UNIT_TIME;
                            merge_tracks[row].events[event_offset + j] = temp_tracks[0].events[j];
                        }
                        event_offset += event_count;

                        for (int j = 0; j < saved_rows; j++)
                        {
                            free(temp_wave_result[j]);
                        }
                        free(temp_wave_result);
                    }
                    else
                    {
                        log_message("Error opening wave block file.");
                        perror("fopen");
                    }
                }
            }
            merge_tracks[row].num_events = event_offset;
            free(temp_tracks[0].events);
            free(temp_tracks);
        }

        FILE *file = fopen("merge.mid", "wb");
        if (file == NULL)
        {
            perror("fopen");
            exit(1);
        }

        write_events(file, merge_tracks, num_merge_tracks);
        fclose(file);

        const char *midi_filename = "merge.mid";
        const char *output_filename = "merge.wav";
        const char *config_filename = "merge.txt";
        int bpm = 120;
        const char *supplement = "supplement.txt";
        FILE *supplement_file = fopen(supplement, "rb");
        if (supplement_file)
        {
            if (fscanf(supplement_file, "%d", &bpm) != 1)
            {
                log_message("Error reading BPM from config file. Using default BPM 120.");
                bpm = 120;
            }
            fclose(supplement_file);
        }
        else
        {
            log_message("Config file not found. Using default BPM 120.");
        }

        process_midi_to_wav(midi_filename, output_filename, config_filename, bpm);
        if (merge_data->flag == 'p')
        {
            play_wav(output_filename);
            log_message("Preview played.");
        }
        else
        {
            log_message("Exported without preview.");
        }
        merge_data->flag = 0;

        for (int row = 0; row < merge_data->rows; row++)
        {
            free(merge_tracks[row].events);
        }
        free(merge_tracks);
    }
    break;
    case 'x': // current columnのみに対してpと同じ動作を行う
    {
        convert_input(merge_data->matrix, merge_data->rows, merge_data->cols, merge_data->result);
        TrackW *merge_tracks;
        uint16_t num_merge_tracks = merge_data->rows;
        initialize_wave_tracks(&merge_tracks, num_merge_tracks);

        for (int row = 0; row < merge_data->rows; row++)
        {
            merge_tracks[row].num_events = MAX_EVENTS;
            merge_tracks[row].events = (EventW *)malloc(merge_tracks[row].num_events * sizeof(EventW));
            if (merge_tracks[row].events == NULL)
            {
                perror("malloc");
                exit(1);
            }
            int event_offset = 0;
            TrackW *temp_tracks;
            initialize_wave_tracks(&temp_tracks, 1);
            for (int i = 0; i < merge_data->cols / 2; i++)
            {
                if (i != merge_data->current_col / 2)
                {
                    continue;
                }
                int filenum = merge_data->result[row][i];
                char filename[256];
                sprintf(filename, "%d.wblk", filenum);
                FILE *file = fopen(filename, "rb");
                if (filenum > 0)
                {
                    if (file)
                    {
                        int saved_rows, saved_cols;
                        if (fread(&saved_rows, sizeof(int), 1, file) != 1 ||
                            fread(&saved_cols, sizeof(int), 1, file) != 1)
                        {
                            log_message("Error reading file.");
                            fclose(file);
                            continue;
                        }
                        int **temp_wave_result = malloc(saved_rows * sizeof(int *));
                        if (!temp_wave_result)
                        {
                            log_message("Memory allocation failed.");
                            fclose(file);
                            continue;
                        }
                        for (int j = 0; j < saved_rows; j++)
                        {
                            temp_wave_result[j] = malloc(saved_cols / 2 * sizeof(int));
                            size_t nread = fread(temp_wave_result[j], sizeof(int), saved_cols / 2, file);
                            if (!temp_wave_result[j] || nread != (size_t)(saved_cols / 2))
                            {
                                log_message("Error reading file or memory allocation failed.");
                                for (int k = 0; k <= j; k++)
                                {
                                    free(temp_wave_result[k]);
                                }
                                free(temp_wave_result);
                                fclose(file);
                                continue;
                            }
                        }
                        fclose(file);

                        char supplement[256];
                        int n = snprintf(supplement, sizeof(supplement), "%s.wbsp", filename);
                        if (n < 0 || n >= (int)sizeof(supplement))
                        {
                            log_message("Supplement filename truncated.");
                        }

                        FILE *file = fopen(supplement, "rb");
                        int time_step = 1;
                        if (file)
                        {
                            if (fread(&time_step, sizeof(int), 1, file) != 1)
                            {
                                log_message("Error reading wave block supplement file.");
                            }
                            fclose(file);
                        }
                        else
                        {
                            log_message("No wave block supplement file found.");
                            log_message(supplement);
                        }

                        int step = temp_wave_result[5][0];
                        int length = UNIT_TIME;
                        int event_count = generate_wave_events(temp_wave_result, temp_tracks, length, step, time_step);
                        for (int j = 0; j < event_count; j++)
                        {
                            // temp_tracks[0].events[j].time += i * UNIT_TIME;
                            merge_tracks[row].events[event_offset + j] = temp_tracks[0].events[j];
                        }
                        event_offset += event_count;

                        for (int j = 0; j < saved_rows; j++)
                        {
                            free(temp_wave_result[j]);
                        }
                        free(temp_wave_result);
                    }
                    else
                    {
                        log_message("Error loading wave block.");
                        perror("fopen");
                    }
                }
            }
            merge_tracks[row].num_events = event_offset;
            free(temp_tracks[0].events);
            free(temp_tracks);
        }

        FILE *file = fopen("merge.mid", "wb");
        if (file == NULL)
        {
            perror("fopen");
            exit(1);
        }

        write_events(file, merge_tracks, num_merge_tracks);
        fclose(file);

        const char *midi_filename = "merge.mid";
        const char *output_filename = "merge.wav";
        const char *config_filename = "merge.txt";
        int bpm = 120;
        const char *supplement = "supplement.txt";
        FILE *supplement_file = fopen(supplement, "rb");
        if (supplement_file)
        {
            if (fscanf(supplement_file, "%d", &bpm) != 1)
            {
                log_message("Error reading BPM from config file. Using default BPM 120.");
                bpm = 120;
            }
            fclose(supplement_file);
        }
        else
        {
            log_message("Config file not found. Using default BPM 120.");
        }

        process_midi_to_wav(midi_filename, output_filename, config_filename, bpm);
        play_wav(output_filename);
        log_message("Preview played.");
        merge_data->flag = 0;

        for (int row = 0; row < merge_data->rows; row++)
        {
            free(merge_tracks[row].events);
        }
        free(merge_tracks);
    }
    break;
    case 's':
    {
        char *filename = input_window("SAVE", "input Merge data name", ".mdat");
        if (filename)
        {
            FILE *file = fopen(filename, "wb");
            if (file)
            {
                if (merge_data->matrix == NULL || merge_data->result == NULL)
                {
                    log_message("Error: merge_data matrix or result is NULL.");
                    fclose(file);
                    free(filename);
                    return;
                }

                fwrite(&merge_data->rows, sizeof(int), 1, file);
                fwrite(&merge_data->cols, sizeof(int), 1, file);
                convert_input(merge_data->matrix, merge_data->rows, merge_data->cols, merge_data->result);
                for (int i = 0; i < merge_data->rows; i++)
                {
                    if (merge_data->result[i] == NULL)
                    {
                        log_message("Error: merge_data result row is NULL.");
                        fclose(file);
                        free(filename);
                        return;
                    }
                    for (int j = 0; j < merge_data->cols / 2; j++)
                    {
                        fwrite(&merge_data->result[i][j], sizeof(int), 1, file);
                    }
                }
                fclose(file);
                log_message("Merge data saved.");
            }
            else
            {
                log_message("Error saving Merge data.");
                perror("fopen");
            }
            free(filename);
        }
        else
        {
            log_message("Canceled saving Merge data.");
        }
    }
    break;
    case 'l':
    {
        char *filename = input_window("LOAD", "input merge data name", ".mdat");
        if (filename)
        {
            FILE *file = fopen(filename, "rb");
            if (file)
            {
                int saved_rows, saved_cols;
                if (fread(&saved_rows, sizeof(int), 1, file) != 1 || fread(&saved_cols, sizeof(int), 1, file) != 1)
                {
                    log_message("Error reading wave block file header.");
                    fclose(file);
                    free(filename);
                    break;
                }

                // Free existing matrix
                for (int i = 0; i < merge_data->rows; i++)
                {
                    free(merge_data->matrix[i]);
                }
                free(merge_data->matrix);

                // Allocate new matrix and result with the loaded size
                merge_data->rows = saved_rows;
                merge_data->cols = saved_cols;
                merge_data->matrix = malloc(merge_data->rows * sizeof(int *));
                merge_data->result = malloc(merge_data->rows * sizeof(int *));
                if (merge_data->matrix == NULL || merge_data->result == NULL)
                {
                    log_message("Memory allocation failed.");
                    fclose(file);
                    free(filename);
                    return;
                }
                for (int i = 0; i < merge_data->rows; i++)
                {
                    merge_data->matrix[i] = malloc(merge_data->cols * sizeof(int));
                    merge_data->result[i] = malloc((merge_data->cols / 2) * sizeof(int));
                    if (merge_data->matrix[i] == NULL || merge_data->result[i] == NULL)
                    {
                        log_message("Memory allocation failed.");
                        for (int j = 0; j <= i; j++)
                        {
                            free(merge_data->matrix[j]);
                            free(merge_data->result[j]);
                        }
                        free(merge_data->matrix);
                        free(merge_data->result);
                        fclose(file);
                        free(filename);
                        return;
                    }
                }

                // Load data into the new result
                for (int i = 0; i < merge_data->rows; i++)
                {
                    for (int j = 0; j < merge_data->cols / 2; j++)
                    {
                        if (fread(&merge_data->result[i][j], sizeof(int), 1, file) != 1)
                        {
                            log_message("Error reading wave block data.");
                            for (int k = 0; k < merge_data->rows; k++)
                            {
                                free(merge_data->matrix[k]);
                                free(merge_data->result[k]);
                            }
                            free(merge_data->matrix);
                            free(merge_data->result);
                            fclose(file);
                            free(filename);
                            return;
                        }
                    }
                }
                decode_input(merge_data->result, merge_data->rows, merge_data->cols, merge_data->matrix);
                log_message("Merge data loaded with new size.");
                fclose(file);
            }
            else
            {
                log_message("Error loading Merge data.");
                perror("fopen");
            }
            free(filename);
        }
        else
        {
            log_message("Canceled loading Merge data.");
        }
    }
    break;
    case 'z':
    {
        int new_rows = atoi(input_window("Resize", "Enter new number of rows:", ""));
        int new_cols = atoi(input_window("Resize", "Enter new number of columns:", ""));
        if (new_rows > 0 && new_cols > 0)
        {
            int **new_matrix = malloc(new_rows * sizeof(int *));
            int **new_result = malloc(new_rows * sizeof(int *));
            if (new_matrix == NULL || new_result == NULL)
            {
                log_message("Memory allocation failed.");
                if (new_matrix)
                    free(new_matrix);
                if (new_result)
                    free(new_result);
                return;
            }

            for (int i = 0; i < new_rows; i++)
            {
                new_matrix[i] = malloc(new_cols * sizeof(int));
                new_result[i] = malloc((new_cols / 2) * sizeof(int));
                if (new_matrix[i] == NULL || new_result[i] == NULL)
                {
                    log_message("Memory allocation failed.");
                    for (int j = 0; j <= i; j++)
                    {
                        if (new_matrix[j])
                            free(new_matrix[j]);
                        if (new_result[j])
                            free(new_result[j]);
                    }
                    free(new_matrix);
                    free(new_result);
                    return;
                }

                if (i < merge_data->rows)
                {
                    memcpy(new_matrix[i], merge_data->matrix[i], (new_cols < merge_data->cols ? new_cols : merge_data->cols) * sizeof(int));
                    if (new_cols > merge_data->cols)
                    {
                        memset(new_matrix[i] + merge_data->cols, 0, (new_cols - merge_data->cols) * sizeof(int));
                    }
                    memcpy(new_result[i], merge_data->result[i], (new_cols / 2 < merge_data->cols / 2 ? new_cols / 2 : merge_data->cols / 2) * sizeof(int));
                    if (new_cols / 2 > merge_data->cols / 2)
                    {
                        memset(new_result[i] + merge_data->cols / 2, 0, (new_cols / 2 - merge_data->cols / 2) * sizeof(int));
                    }
                }
                else
                {
                    memset(new_matrix[i], 0, new_cols * sizeof(int));
                    memset(new_result[i], 0, (new_cols / 2) * sizeof(int));
                }
            }

            for (int i = 0; i < merge_data->rows; i++)
            {
                free(merge_data->matrix[i]);
                free(merge_data->result[i]);
            }
            free(merge_data->matrix);
            free(merge_data->result);

            merge_data->matrix = new_matrix;
            merge_data->result = new_result;
            merge_data->rows = new_rows;
            merge_data->cols = new_cols;
            log_message("Matrix resized.");
        }
        else
        {
            log_message("Invalid matrix size.");
        }
    }
    break;
    case 0:
        WINDOW *win = get_main_window();
        wclear(win);
        display_matrix(merge_data->matrix, merge_data->rows, merge_data->cols, merge_data->current_row, merge_data->current_col);
        display_guide();
        display_label(1, 0);
        wrefresh(win);
        display_log_messages();
        wrefresh(log_win);
        break;
    default:
        break;
    }
}
