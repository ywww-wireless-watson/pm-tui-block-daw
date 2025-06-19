#ifndef EDIT_PROJECT_H
#define EDIT_PROJECT_H

#include "edit_music.h"

// Project editor function declarations
void initialize_project_matrix(EditorData *data);
void cleanup_project_editor(EditorData *data);
void init_project_editor(EditorData *project_data);
void handle_project_editor(EditorData *project_data, bool project_rewrite);

#endif // EDIT_PROJECT_H
