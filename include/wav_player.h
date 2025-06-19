#ifndef WAV_PLAYER_H
#define WAV_PLAYER_H

#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <sndfile.h>

// WAV player status codes
typedef enum
{
    WAV_PLAYER_SUCCESS = 0,
    WAV_PLAYER_ERROR_OPEN_FILE,
    WAV_PLAYER_ERROR_ALSA_SETUP,
    WAV_PLAYER_ERROR_MEMORY_ALLOCATION,
    WAV_PLAYER_ERROR_READ_FILE,
    WAV_PLAYER_ERROR_ALSA_WRITE,
    WAV_PLAYER_ERROR_UNSUPPORTED_SAMPLE_RATE
} wav_player_status_t;

// Function to play a WAV file
wav_player_status_t play_wav(const char *filename);

#endif // WAV_PLAYER_H
