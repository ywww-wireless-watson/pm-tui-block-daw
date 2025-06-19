#ifndef EDIT_MUSIC_H
#define EDIT_MUSIC_H

#include <ncurses.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "ncurses_init.h"
#include "matrix_input.h"
#include "midi_writer.h"
#include "mtw.h"
#include "wav_player.h"
#include "input_window.h"

#define MAX_LOG_MESSAGES 10
#define UNIT_TIME 480 * 32
#define MAX_EVENTS 4096

// Data structure for the music editor
typedef struct
{
    int **matrix;    // Pointer to the matrix data
    int rows;        // Number of rows in the matrix
    int cols;        // Number of columns in the matrix
    int **result;    // Pointer to the result matrix after conversion
    int current_row; // Currently selected row in the editor
    int current_col; // Currently selected column in the editor
    int flag;        // General-purpose flag for various uses
    int time_step;   // Time step for the editor, in ticks
} EditorData;

WINDOW *get_log_window();
void initialize_log_window();
void display_log_messages();
void log_message(const char *message);
void cleanup_log_messages();
void display_label(int type, int step);
void convert_input(int **matrix, int rows, int cols, int **result);
void decode_input(int **result, int rows, int cols, int **matrix);
void process_wave_editor_input(EditorData *wave_data, bool wave_rewrite);
void initialize_wave_tracks(TrackW **wave_tracks, uint16_t num_wave_tracks);
void initialize_wave_matrix(EditorData *data);
void cleanup_wave_editor(EditorData *data, TrackW *wave_tracks);
void cleanup_merge_editor(EditorData *data);
int generate_wave_events(int **wave_result, TrackW *wave_tracks, int wave_length, int wave_step, int time_step);
void write_wave_midi_file(TrackW *wave_tracks, uint16_t num_wave_tracks);
void init_wave_editor(EditorData *wave_data, uint16_t *num_wave_tracks, TrackW **wave_tracks);
void init_merge_editor(EditorData *merge_data);
void handle_wave_editor(EditorData *wave_data, TrackW *wave_tracks, uint16_t num_wave_tracks);
void handle_merge_editor(EditorData *merge_data, bool merge_rewrite);

#endif // EDIT_MUSIC_H