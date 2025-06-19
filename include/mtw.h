#ifndef MTW_H
#define MTW_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

// Structure for MIDI event
typedef struct
{
    int delta_time;
    int note_number;
    int velocity;
} Event;

// Structure for note
typedef struct
{
    int note_number;
    double start_sec;
    double end_sec;
} Note;

// Structure for MIDI track
typedef struct
{
    Event *events;
    int num_events;
    Note *notes;
    int num_notes;
} Track;

// Function declarations
void write_wav_header(FILE *file, uint32_t data_size);
int fread_varlen(FILE *file, uint32_t *value);
void fread_big_endian_int32(FILE *file, uint32_t *value);
void fread_big_endian_int16(FILE *file, uint16_t *value);
void get_events(FILE *file, Track **tracks, uint16_t *num_tracks, int debug);
double get_track_notes(Track *tracks, uint16_t num_tracks, int bpm);
double midi_note_to_freq(int note);
void add_wave(double *data_L, double *data_R, uint32_t num_samples, Note note, int wave_type, double volume, double attack_time, double decay_time, double sustain_level, double release_time, double pan);
void process_midi_to_wav(const char *midi_filename, const char *output_filename, const char *config_filename, int bpm);

#endif // MTW_H
