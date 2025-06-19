#include "midi_writer.h"

// Function to write a variable-length integer
int fwrite_varlen(FILE *file, uint32_t value)
{
    uint8_t buffer[4];
    int count = 0;
    buffer[count++] = value & 0x7F;
    while (value >>= 7)
    {
        buffer[count++] = 0x80 | (value & 0x7F);
    }
    for (int i = count - 1; i >= 0; i--)
    {
        fwrite(&buffer[i], 1, 1, file);
    }
    return count;
}

// Function to write a 32-bit big-endian integer
void fwrite_big_endian_int32(FILE *file, uint32_t value)
{
    for (int i = 3; i >= 0; i--)
    {
        uint8_t byte = (value >> (i * 8)) & 0xFF;
        fwrite(&byte, 1, 1, file);
    }
}

// Function to write a 16-bit big-endian integer
void fwrite_big_endian_int16(FILE *file, uint16_t value)
{
    for (int i = 1; i >= 0; i--)
    {
        uint8_t byte = (value >> (i * 8)) & 0xFF;
        fwrite(&byte, 1, 1, file);
    }
}

// Function to write events to a MIDI file
void write_events(FILE *file, TrackW *tracks, uint16_t num_tracks)
{
    // Write header
    fwrite("MThd", 1, 4, file);
    fwrite_big_endian_int32(file, 6);          // Header length
    fwrite_big_endian_int16(file, 1);          // Format type
    fwrite_big_endian_int16(file, num_tracks); // Number of tracks
    fwrite_big_endian_int16(file, 960);        // Time division

    // Process each track
    for (int track = 0; track < num_tracks; track++)
    {
        // Write track chunk header
        fwrite("MTrk", 1, 4, file);
        long track_length_pos = ftell(file);
        fwrite_big_endian_int32(file, 0); // Track chunk length (to be updated later)

        uint32_t track_length = 0; // Track chunk length (excluding the first 8 bytes)
        int current_time = 0;

        // Process each event
        for (int i = 0; i < tracks[track].num_events; i++)
        {
            EventW current_event = tracks[track].events[i];
            int delta_time = current_event.time - current_time;
            current_time = current_event.time;

            // Write delta time
            track_length += fwrite_varlen(file, delta_time);

            // Write note-on event
            if (current_event.velocity > 0)
            {
                uint8_t status_byte = 0x90;
                fwrite(&status_byte, 1, 1, file);
                fwrite(&current_event.note_number, 1, 1, file);
                fwrite(&current_event.velocity, 1, 1, file);
                track_length += 3;
            }
            // Write note-off event
            else
            {
                uint8_t status_byte = 0x80;
                fwrite(&status_byte, 1, 1, file);
                fwrite(&current_event.note_number, 1, 1, file);
                uint8_t velocity = 0;
                fwrite(&velocity, 1, 1, file);
                track_length += 3;
            }
        }

        // Update track chunk length
        long current_pos = ftell(file);
        fseek(file, track_length_pos, SEEK_SET);
        fwrite_big_endian_int32(file, track_length);
        fseek(file, current_pos, SEEK_SET);
    }
}

// int main()
// {
//     // Open the MIDI file to write
//     char filename[256];
//     printf("Enter the output MIDI file name: ");
//     fgets(filename, sizeof(filename), stdin);
//     filename[strcspn(filename, "\n")] = '\0';
//     FILE *file = fopen(filename, "wb");
//     if (!file)
//     {
//         perror("fopen");
//         return 1;
//     }
//
//     // Create tracks
//     uint16_t num_tracks = 1;
//     TrackW *tracks = (TrackW *)malloc(num_tracks * sizeof(TrackW));
//     if (tracks == NULL)
//     {
//         perror("malloc");
//         fclose(file);
//         return 1;
//     }
//
//     // Create events
//     tracks[0].num_events = 4;
//     tracks[0].events = (EventW *)malloc(tracks[0].num_events * sizeof(EventW));
//     if (tracks[0].events == NULL)
//     {
//         perror("malloc");
//         free(tracks);
//         fclose(file);
//         return 1;
//     }
//
//     // Set events
//     tracks[0].events[0] = (EventW){0, 60, 100};  // Note on (C4)
//     tracks[0].events[1] = (EventW){480, 60, 0};  // Note off (C4)
//     tracks[0].events[2] = (EventW){480, 64, 100}; // Note on (E4)
//     tracks[0].events[3] = (EventW){960, 64, 0};  // Note off (E4)
//
//     // Write events to MIDI file
//     write_events(file, tracks, num_tracks);
//
//     // Close file
//     fclose(file);
//
//     // Free memory
//     free(tracks[0].events);
//     free(tracks);
//
//     printf("MIDI file written: %s\n", filename);
//
//     return 0;
// }
