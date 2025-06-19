#ifndef EDIT_SYNTH_H
#define EDIT_SYNTH_H

#include <stdlib.h>
#include <stdbool.h>
#include <ncurses.h>
#include "edit_music.h"

// Synth editor function declarations
void initialize_synth_matrix(EditorData *data);
void cleanup_synth_editor(EditorData *data);
void init_synth_editor(EditorData *synth_data);
void handle_synth_editor(EditorData *synth_data, bool synth_rewrite);

#endif // EDIT_SYNTH_H
