#ifndef WRITEMIDI_H
#define WRITEMIDI_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Structure for MIDI event
typedef struct
{
    int time;
    int note_number;
    int velocity;
} EventW;

// Structure for MIDI track
typedef struct
{
    EventW *events;
    int num_events;
} TrackW;

// Function to write variable-length integer
int fwrite_varlen(FILE *file, uint32_t value);

// Function to write big-endian integers
void fwrite_big_endian_int32(FILE *file, uint32_t value);
void fwrite_big_endian_int16(FILE *file, uint16_t value);

// Function to write events to MIDI file
void write_events(FILE *file, TrackW *tracks, uint16_t num_tracks);

#endif // WRITEMIDI_H