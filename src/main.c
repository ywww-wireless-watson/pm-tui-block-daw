#include "edit_music.h"
#include "edit_synth.h"
#include "edit_project.h"

void handle_editor(EditorData *data, int *window_state, uint16_t num_wave_tracks, TrackW *wave_tracks, EditorData *merge_data, EditorData *synth_data, EditorData *project_data)
{
    int flag = 0;
    switch (*window_state)
    {
    case 0: // Wave Editor
        handle_wave_editor(data, wave_tracks, num_wave_tracks);
        flag = data->flag;
        break;
    case 1: // Merge Editor
        handle_merge_editor(merge_data, false);
        flag = merge_data->flag;
        break;
    case 2: // Synth Editor
        handle_synth_editor(synth_data, false);
        flag = synth_data->flag;
        break;
    case 3: // Project Editor
        handle_project_editor(project_data, false);
        flag = project_data->flag;
        break;
    default:
        break;
    }

    switch (flag)
    {
    case 'o':
        *window_state = 0;
        process_wave_editor_input(data, true);
        break;
    case 'i':
        *window_state = 1;
        ungetch(' ');
        handle_merge_editor(merge_data, true);
        break;
    case 'u':
        *window_state = 2;
        ungetch(' ');
        handle_synth_editor(synth_data, true);
        break;
    case 'y':
        *window_state = 3;
        ungetch(' ');
        handle_project_editor(project_data, true);
        break;
    case 'q':
        if (*window_state == 3)
        {
            *window_state = 4;
        }
        else
        {
            *window_state = 0;
        }
    default:
        break;
    }
}

int main()
{
    uint16_t num_wave_tracks;
    TrackW *wave_tracks;
    EditorData wave_data;
    EditorData merge_data;
    EditorData synth_data;   // Data for the synth window
    EditorData project_data; // Data for the project window
    int window_state = 0;

    init_wave_editor(&wave_data, &num_wave_tracks, &wave_tracks);
    init_merge_editor(&merge_data);
    init_synth_editor(&synth_data);     // Initialize the synth editor
    init_project_editor(&project_data); // Initialize the project editor

    process_wave_editor_input(&wave_data, true);
    while (1)
    {
        handle_editor(&wave_data, &window_state, num_wave_tracks, wave_tracks, &merge_data, &synth_data, &project_data);
        if (window_state == 4)
        {
            break;
        }
    }
    clear();
    end_ncurses();

    cleanup_wave_editor(&wave_data, wave_tracks);
    cleanup_merge_editor(&merge_data);
    cleanup_synth_editor(&synth_data);
    cleanup_project_editor(&project_data);
    cleanup_log_messages();
    return 0;
}
