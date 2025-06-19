#include "wav_player.h"

#define BUFFER_SIZE 4096 // Define a buffer size that is a multiple of the period size

wav_player_status_t play_wav(const char *filename)
{
    SF_INFO sfinfo;
    SNDFILE *sndfile = sf_open(filename, SFM_READ, &sfinfo);
    if (!sndfile)
    {
        fprintf(stderr, "Error opening file: %s\n", sf_strerror(NULL));
        return WAV_PLAYER_ERROR_OPEN_FILE;
    }

    if (sfinfo.samplerate != 44100)
    {
        fprintf(stderr, "Unsupported sample rate: %d\n", sfinfo.samplerate);
        sf_close(sndfile);
        return WAV_PLAYER_ERROR_UNSUPPORTED_SAMPLE_RATE;
    }

    snd_pcm_t *handle;
    if (snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0)
    {
        sf_close(sndfile);
        return WAV_PLAYER_ERROR_ALSA_SETUP;
    }

    if (snd_pcm_set_params(handle, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED,
                           sfinfo.channels, sfinfo.samplerate, 1, 1000000) < 0)
    {
        sf_close(sndfile);
        snd_pcm_close(handle);
        return WAV_PLAYER_ERROR_ALSA_SETUP;
    }

    short buffer1[BUFFER_SIZE];
    short buffer2[BUFFER_SIZE];
    short *buffers[2] = {buffer1, buffer2};
    int current_buffer = 0;

    sf_count_t frames_read;
    while ((frames_read = sf_readf_short(sndfile, buffers[current_buffer], BUFFER_SIZE / sfinfo.channels)) > 0)
    {
        snd_pcm_sframes_t frames_written = snd_pcm_writei(handle, buffers[current_buffer], frames_read);
        if (frames_written < 0)
        {
            fprintf(stderr, "Error writing to PCM device: %s\n", snd_strerror(frames_written));
            sf_close(sndfile);
            snd_pcm_close(handle);
            return WAV_PLAYER_ERROR_ALSA_WRITE;
        }
        current_buffer = 1 - current_buffer; // Switch to the other buffer
    }

    snd_pcm_drain(handle);
    snd_pcm_close(handle);
    sf_close(sndfile);

    return WAV_PLAYER_SUCCESS;
}
