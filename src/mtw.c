#include "mtw.h"

// Write WAV file header
void write_wav_header(FILE *file, uint32_t data_size)
{
    fwrite("RIFF", 1, 4, file); // RIFF header
    uint32_t chunk_size = 36 + data_size * 2;
    fwrite(&chunk_size, 4, 1, file); // File size after this
    fwrite("WAVE", 1, 4, file);      // WAVE header

    fwrite("fmt ", 1, 4, file); // fmt header
    uint32_t fmt_chunk_size = 16;
    fwrite(&fmt_chunk_size, 4, 1, file); // Size of fmt chunk
    uint16_t audio_format = 1;
    fwrite(&audio_format, 2, 1, file); // Linear PCM format ID
    uint16_t num_channels = 2;
    fwrite(&num_channels, 2, 1, file); // Number of channels
    uint32_t sample_rate = 44100;
    fwrite(&sample_rate, 4, 1, file); // Sample rate
    uint32_t byte_rate = 44100 * 3 * 2;
    fwrite(&byte_rate, 4, 1, file); // Bytes per second
    uint16_t block_align = 3 * 2;
    fwrite(&block_align, 2, 1, file); // Block size
    uint16_t bits_per_sample = 24;
    fwrite(&bits_per_sample, 2, 1, file); // Bits per sample

    fwrite("data", 1, 4, file);     // data header
    fwrite(&data_size, 4, 1, file); // Size of audio data
}

// Read a variable-length integer
int fread_varlen(FILE *file, uint32_t *value)
{
    uint32_t result = 0;
    uint8_t byte;
    int count = 0;
    do
    {
        if (fread(&byte, 1, 1, file) != 1)
        {
            printf("File read error\n");
            return -1;
        }
        result = (result << 7) | (byte & 0x7F);
        count++;
        if (count > 4)
        {
            printf("Invalid variable-length integer\n");
            return -1;
        }
    } while (byte & 0x80);
    *value = result;
    return count;
}

// Read a 32-bit big-endian integer
void fread_big_endian_int32(FILE *file, uint32_t *value)
{
    *value = 0;
    for (int i = 0; i < 4; i++)
    {
        uint8_t byte;
        if (fread(&byte, 1, 1, file) != 1)
        {
            printf("File read error\n");
            return;
        }
        *value = (*value << 8) | byte;
    }
}

// Read a 16-bit big-endian integer
void fread_big_endian_int16(FILE *file, uint16_t *value)
{
    *value = 0;
    for (int i = 0; i < 2; i++)
    {
        uint8_t byte;
        if (fread(&byte, 1, 1, file) != 1)
        {
            printf("File read error\n");
            return;
        }
        *value = (*value << 8) | byte;
    }
}

// Get all events from a MIDI file
void get_events(FILE *file, Track **tracks, uint16_t *num_tracks, int debug)
{
    // Process header
    uint8_t header[4];
    if (fread(header, 1, 4, file) != 4)
    {
        printf("File read error\n");
        return;
    }
    if (memcmp(header, "MThd", 4) != 0)
    {
        // printf("Invalid MIDI file\n");
        return;
    }
    if (debug)
    {
        // printf("Header: %c%c%c%c\n", header[0], header[1], header[2], header[3]);
    }

    // Read header length
    uint32_t header_length;
    fread_big_endian_int32(file, &header_length);
    if (debug)
    {
        // printf("Header Length: %u\n", header_length);
    }

    // Read format type
    uint16_t format_type;
    fread_big_endian_int16(file, &format_type);
    if (debug)
    {
        // printf("Format Type: %u\n", format_type);
    }

    // Read number of tracks
    fread_big_endian_int16(file, num_tracks);
    if (debug)
    {
        // printf("Number of Tracks: %u\n", *num_tracks);
    }
    // Allocate memory for tracks
    *tracks = (Track *)malloc(*num_tracks * sizeof(Track));
    if (*tracks == NULL)
    {
        perror("malloc");
        return;
    }
    // Initialize tracks
    for (int i = 0; i < *num_tracks; i++)
    {
        (*tracks)[i].num_events = 0;
        (*tracks)[i].events = NULL;
        (*tracks)[i].num_notes = 0;
        (*tracks)[i].notes = NULL;
    }

    // Read time division
    uint16_t time_division;
    fread_big_endian_int16(file, &time_division);
    if (debug)
    {
        // printf("Time Division: %u\n", time_division);
    }

    // Read track chunks
    for (int track = 0; track < *num_tracks; track++)
    {
        // Read track chunk header
        uint8_t track_header[4];
        if (fread(track_header, 1, 4, file) != 4)
        {
            printf("File read error\n");
            return;
        }
        if (memcmp(track_header, "MTrk", 4) != 0)
        {
            // printf("Invalid track chunk\n");
            return;
        }
        if (debug)
        {
            // printf("\nTrack %d Header: %c%c%c%c\n", track, track_header[0], track_header[1], track_header[2], track_header[3]);
        }

        // Read track chunk length
        uint32_t track_length;
        fread_big_endian_int32(file, &track_length);
        if (debug)
        {
            // printf("Track %d Length: %u\n", track, track_length);
        }

        // Read events
        uint32_t bytes_read = 0;
        uint8_t status_byte = 0;
        while (bytes_read < track_length)
        {
            // Read event delta time
            uint32_t delta_time;
            int count = fread_varlen(file, &delta_time);
            if (count == -1)
            {
                // printf("Invalid variable-length integer\n");
                return;
            }
            bytes_read += count; // Add the return value of fread_varlen to bytes_read
            if (debug)
            {
                // printf("\nTime %u ", delta_time);
            }

            // Read event status byte
            uint8_t next_byte;
            if (fread(&next_byte, 1, 1, file) != 1)
            {
                printf("File read error\n");
                return;
            }
            bytes_read += 1;

            // Check the first bit of the next byte
            if (next_byte & 0x80)
            {
                // Treat as status byte (first bit is 1)
                status_byte = next_byte;
                if (debug)
                {
                    // printf("Byte %02X ", status_byte);
                }
            }
            else
            {
                // Use previous status byte as running status
                if (debug)
                {
                    // printf("RByte %02X ", status_byte);
                }

                // In this case, the previously read byte is treated as data byte
                // If necessary, handle here
                fseek(file, -1, SEEK_CUR); // Rewind one byte
                bytes_read -= 1;           // Adjust byte count
            }

            // Determine event type
            if (status_byte == 0xFF)
            {
                // Handle meta event
                uint8_t meta_type;
                if (fread(&meta_type, 1, 1, file) != 1)
                {
                    printf("File read error\n");
                    return;
                }
                bytes_read += 1;
                if (debug)
                {
                    // printf("MType %02X ", meta_type);
                }

                // Read meta event length
                uint32_t meta_length;
                int count = fread_varlen(file, &meta_length);
                bytes_read += count;
                if (count == -1)
                {
                    // printf("Invalid variable-length integer\n");
                    return;
                }
                if (debug)
                {
                    // printf("Length %u ", meta_length);
                }

                // If this is a track name meta event, display the track name
                if (meta_type == 0x03)
                {
                    char *track_name = (char *)malloc(meta_length + 1);
                    if (!track_name)
                    {
                        printf("Memory allocation error\n");
                        return;
                    }
                    if (fread(track_name, 1, meta_length, file) != meta_length)
                    {
                        printf("File read error\n");
                        free(track_name);
                        return;
                    }
                    track_name[meta_length] = '\0';
                    // if (debug)
                    //{
                    // printf("\nTrack Num: %d ", track);
                    // printf("Track Name: %s ", track_name);
                    //}
                    free(track_name);
                }
                else
                {
                    // Skip meta event data
                    fseek(file, meta_length, SEEK_CUR);
                }

                // Update bytes_read
                bytes_read += meta_length;
                if (debug)
                {
                    // printf("%u", bytes_read);
                }
            }
            else if ((status_byte & 0xF0) == 0x90 || (status_byte & 0xF0) == 0x80)
            {
                // Handle note on/off event
                uint8_t note_number;
                if (fread(&note_number, 1, 1, file) != 1)
                {
                    printf("File read error\n");
                    return;
                }
                bytes_read += 1;
                if (debug)
                {
                    // printf("Note Number: %u ", note_number);
                }

                uint8_t velocity;
                if (fread(&velocity, 1, 1, file) != 1)
                {
                    printf("File read error\n");
                    return;
                }
                bytes_read += 1;
                if (debug)
                {
                    // printf("Velocity: %u ", velocity);
                }

                // Add event to tracks
                (*tracks)[track].events = (Event *)realloc((*tracks)[track].events, ((*tracks)[track].num_events + 1) * sizeof(Event));
                (*tracks)[track].events[(*tracks)[track].num_events].delta_time = delta_time;
                (*tracks)[track].events[(*tracks)[track].num_events].note_number = note_number;
                (*tracks)[track].events[(*tracks)[track].num_events].velocity = velocity;
                (*tracks)[track].num_events++;

                if (debug)
                {
                    // printf("%u", bytes_read);
                }
            }
            else
            {
                // Skip other event types (e.g., control change, program change, etc.)
                // For simplicity, skip 1 or 2 bytes depending on event type
                if ((status_byte & 0xF0) == 0xC0 || (status_byte & 0xF0) == 0xD0)
                {
                    fseek(file, 1, SEEK_CUR);
                    bytes_read += 1;
                }
                else
                {
                    fseek(file, 2, SEEK_CUR);
                    bytes_read += 2;
                }
            }
        }
    }
}

// Function to extract all note pitches, start times, and end times for each track from an array of tracks
// (formerly: トラックの配列から，トラックごとの全てのノートの音程，開始時刻，終了時刻を取得する関数)
double get_track_notes(Track *tracks, uint16_t num_tracks, int bpm)
{
    // Process each track (formerly: トラックごとに処理)
    for (int track = 0; track < num_tracks; track++)
    {
        // Process each event
        int current_time = 0;
        for (int i = 0; i < tracks[track].num_events; i++)
        {
            Event current_event = tracks[track].events[i];
            current_time += current_event.delta_time;

            if (current_event.velocity > 0)
            {
                // Add new note
                Note new_note;
                new_note.note_number = current_event.note_number;
                new_note.start_sec = (double)current_time * 60.0 / (bpm * 960.0); // 4分音符＝960ティック．
                new_note.end_sec = -1.0;
                tracks[track].notes = (Note *)realloc(tracks[track].notes, (tracks[track].num_notes + 1) * sizeof(Note));
                tracks[track].notes[tracks[track].num_notes] = new_note;
                tracks[track].num_notes++;
            }
            else if (current_event.velocity == 0)
            {
                // Set the end time of the note
                for (int j = tracks[track].num_notes - 1; j >= 0; j--)
                {
                    Note *note = &tracks[track].notes[j];
                    if (note->note_number == current_event.note_number && note->end_sec == -1.0)
                    {
                        note->end_sec = (double)current_time * 60.0 / (bpm * 960.0); // 4分音符＝960ティック．
                        break;
                    }
                }
            }
        }
    }
    // Get the end time of each track (formerly: トラックの終了時刻を取得)
    double endtime = 0;
    for (int track = 0; track < num_tracks; track++)
    {
        for (int i = 0; i < tracks[track].num_notes; i++)
        {
            Note note = tracks[track].notes[i];
            if (note.end_sec > endtime)
            {
                endtime = note.end_sec;
            }
        }
    }
    return endtime;
}

// Convert MIDI note number to frequency (formerly: MIDIノート番号を周波数に変換する関数)
double midi_note_to_freq(int note)
{
    return 440.0 * pow(2.0, (note - 69) / 12.0); // A4 = 440Hz
}

// Add a sine wave of a specific frequency and length to the data array at the specified position according to the note
void add_wave(double *data_L, double *data_R, uint32_t num_samples, Note note, int wave_type, double volume, double attack_time, double decay_time, double sustain_level, double release_time, double pan)
{
    (void)num_samples; // suppress unused parameter warning
    uint32_t sample_rate = 44100;
    double frequency = midi_note_to_freq(note.note_number);
    uint32_t start_sample = note.start_sec * sample_rate;
    uint32_t end_sample = note.end_sec * sample_rate;
    double attack_samples = attack_time * sample_rate;
    double decay_samples = decay_time * sample_rate;
    double release_samples = release_time * sample_rate;
    double sample = 0.0; // initialize
    for (uint32_t i = start_sample; i < end_sample + release_samples; ++i)
    {
        double t = (double)i / sample_rate; // Current time in seconds
        switch (wave_type)
        {
        case 0:
            sample = sin(2.0 * M_PI * frequency * t); // Sine wave sample value
            break;
        case 1:
            sample = 2.0 * fabs(2.0 * (t * frequency - floor(t * frequency + 0.5))) - 1.0; // Triangle wave sample value
            break;
        case 2:
            sample = (fmod(t * frequency, 1.0) < 0.5) ? 1.0 : -1.0; // Square wave sample value
            break;
        case 3:
            // sawtooth wave
            sample = 2.0 * (t * frequency - floor(t * frequency + 0.5)); // Sawtooth wave sample value
            break;
        case 4:
            // 音程が高いノイズ
            sample = 2.0 * ((double)rand() / RAND_MAX) - 1.0; // Noise wave sample value
            break;
        case 5:
            // 音程が中くらいのノイズ
            if (i % 4 == 0)
                sample = 2.0 * ((double)rand() / RAND_MAX) - 1.0; // Noise wave sample value
            break;
        case 6:
            // 音程が低いノイズ
            if (i % 16 == 0)
                sample = 2.0 * ((double)rand() / RAND_MAX) - 1.0; // Noise wave sample value
            break;
        default:
            sample = 0.0; // Default to 0 if wave_type is invalid
            break;
        }
        sample *= volume; // Apply volume

        // Apply ADSR envelope
        if (i < start_sample + attack_samples)
        {
            double attack_factor = (double)(i - start_sample) / attack_samples;
            sample *= attack_factor;
        }
        else if (i < start_sample + attack_samples + decay_samples)
        {
            double decay_factor = (double)(i - start_sample - attack_samples) / decay_samples;
            sample *= (1.0 - sustain_level) * (1.0 - decay_factor) + sustain_level;
        }
        else if (i < end_sample)
        {
            sample *= sustain_level;
        }
        else
        {
            // Apply release envelope
            double release_factor = (double)(i - end_sample) / release_samples;
            if (release_factor > 1.0)
            {
                release_factor = 1.0;
            }
            sample *= (1.0 - release_factor) * sustain_level;
        }

        double left_sample = sample * (1.0 - pan);
        double right_sample = sample * pan;
        data_L[i] += left_sample;
        data_R[i] += right_sample;
    }
    // Display the written note
    // // printf("Note: Note Number: %d, Start Sec: %lf, End Sec: %lf\n", note.note_number, note.start_sec, note.end_sec);
}

void process_midi_to_wav(const char *midi_filename, const char *output_filename, const char *config_filename, int bpm)
{
    int debug = 1;

    // Read the MIDI file
    FILE *midi_file = fopen(midi_filename, "rb");
    if (!midi_file)
    {
        perror("fopen");
        return;
    }

    // Open the WAV file to write
    FILE *file = fopen(output_filename, "wb");
    if (!file)
    {
        perror("fopen");
        fclose(midi_file); // Close the MIDI file
        return;
    }

    // Get events from the MIDI file
    Track *tracks;
    uint16_t num_tracks;
    get_events(midi_file, &tracks, &num_tracks, debug);
    fclose(midi_file);

    if (debug)
    {
        // トラックの内容を表示
        for (int i = 0; i < num_tracks; i++)
        {
            // printf("Track %d\n", i + 1);
            for (int j = 0; j < tracks[i].num_events; j++)
            {
                // printf("  Event %d: Delta Time: %d, Note Number: %d, Velocity: %d\n", j + 1, tracks[i].events[j].delta_time, tracks[i].events[j].note_number, tracks[i].events[j].velocity);
                // FILE *log_file = fopen("log.txt", "a");
                // if (log_file != NULL)
                // {
                //     fprintf(log_file, "Event %d: Delta Time: %d, Note Number: %d, Velocity: %d\n", j + 1, tracks[i].events[j].delta_time, tracks[i].events[j].note_number, tracks[i].events[j].velocity);
                //     fclose(log_file);
                // }
                // else
                // {
                //     perror("fopen");
                // }
            }
        }
    }

    // Get note pitches, start times, and end times from the track contents
    double endtime = get_track_notes(tracks, num_tracks, bpm) + 1; // Add extra time for release

    // Display the contents of each track
    if (debug)
    {
        for (int i = 0; i < num_tracks; i++)
        {
            if (i == 7)
            {
                // printf("Track %d\n", i);
                for (int j = 0; j < tracks[i].num_notes; j++)
                {
                    // printf("  Note %d: Note Number: %d, Start Sec: %lf, End Sec: %lf\n", j + 1, tracks[i].notes[j].note_number, tracks[i].notes[j].start_sec, tracks[i].notes[j].end_sec);
                }
            }
        }
    }

    // Warn if there are notes with the same start time and note number
    for (int i = 0; i < num_tracks; i++)
    {
        for (int j = 0; j < tracks[i].num_notes; j++)
        {
            for (int k = j + 1; k < tracks[i].num_notes; k++)
            {
                if (tracks[i].notes[j].note_number == tracks[i].notes[k].note_number && tracks[i].notes[j].start_sec == tracks[i].notes[k].start_sec)
                {
                    // printf("\nWarning: Track %d, Note %d and Note %d have the same start time and note number\n", i + 1, j, k);
                    // printf("Note Number: %d, Start Sec: %lf\n", tracks[i].notes[j].note_number, tracks[i].notes[j].start_sec);
                }
            }
        }
    }

    // Warn if there are notes with length 0
    for (int i = 0; i < num_tracks; i++)
    {
        for (int j = 0; j < tracks[i].num_notes; j++)
        {
            if (tracks[i].notes[j].start_sec == tracks[i].notes[j].end_sec)
            {
                // printf("\nWarning: Track %d, Note %d has zero length\n", i, j);
            }
        }
    }

    // Display the end time
    // printf("\nEnd Time: %lf\n", endtime);

    uint32_t num_samples = 44100 * endtime;
    uint32_t data_size = num_samples * 2 * 3;

    // Write the WAV header
    write_wav_header(file, data_size);

    // Declare data arrays
    double *data_L = (double *)calloc(num_samples, sizeof(double));
    double *data_R = (double *)calloc(num_samples, sizeof(double));
    if (data_L == NULL || data_R == NULL)
    {
        perror("calloc");
        fclose(file); // Close the WAV file
        free(data_L); // Free the left channel data array
        free(data_R); // Free the right channel data array
        return;
    }

    // Add waves according to notes
    // Read the track configuration file
    FILE *config_file = fopen(config_filename, "r");
    if (!config_file)
    {
        perror("fopen");
        fclose(file); // Close the WAV file
        free(data_L); // Free the left channel data array
        free(data_R); // Free the right channel data array
        return;
    }

    for (int i = 0; i < num_tracks; i++)
    {
        int wave_type;
        double volume;
        double attack_time;
        double decay_time;
        double sustain_level;
        double release_time;
        double pan;
        if (fscanf(config_file, "%d %lf %lf %lf %lf %lf %lf", &wave_type, &volume, &attack_time, &decay_time, &sustain_level, &release_time, &pan) != 7)
        {
            printf("Config read error\n");
            wave_type = 0;
            volume = 0.5;
            attack_time = 0.01;
            decay_time = 0.1;
            sustain_level = 0.7;
            release_time = 0.2;
            pan = 0.5;
        }
        char buf[256];
        if (!fgets(buf, sizeof(buf), config_file))
        {
            // ignore error, not fatal
        }

        // printf("Track %d, Wave Type %d, Volume %lf, Attack Time %lf, Decay Time %lf, Sustain Level %lf, Release Time %lf, Pan %lf\n", i, wave_type, volume, attack_time, decay_time, sustain_level, release_time, pan);

        for (int j = 0; j < tracks[i].num_notes; j++)
        {
            add_wave(data_L, data_R, num_samples, tracks[i].notes[j], wave_type, volume, attack_time, decay_time, sustain_level, release_time, pan);
        }
    }
    fclose(config_file);

    // Allocate memory for the interleaved stereo data array
    int32_t *data = (int32_t *)malloc(num_samples * 2 * sizeof(int32_t));
    if (data == NULL)
    {
        perror("malloc");
        fclose(file);
        free(data_L);
        free(data_R);
        return;
    }

    // Combine left and right channels into the interleaved stereo data array
    for (uint32_t i = 0; i < num_samples; ++i)
    {
        data[2 * i] = (int32_t)(data_L[i] * (1 << (24 - 1)));     // Left channel
        data[2 * i + 1] = (int32_t)(data_R[i] * (1 << (24 - 1))); // Right channel
    }

    free(data_L);
    free(data_R);

    // Normalize the data to 0.6
    double max = 0.0;
    for (uint32_t i = 0; i < num_samples * 2; ++i)
    {
        double abs_val = (data[i] >= 0) ? (double)data[i] : (double)-data[i];
        if (abs_val > max)
        {
            max = abs_val;
        }
    }
    for (uint32_t i = 0; i < num_samples * 2; ++i)
    {
        data[i] = (int32_t)((data[i] / max) * 0.6 * (1 << (24 - 1)));
    }

    // Write the interleaved stereo data to the file
    for (uint32_t i = 0; i < num_samples * 2; ++i)
    {
        int32_t sample_int = data[i];
        fwrite(&sample_int, 3, 1, file); // Write 3 bytes (24 bits)
    }

    fclose(file);
    // printf("output: %s\n", output_filename);

    // Free allocated memory
    free(data);
    for (int i = 0; i < num_tracks; i++)
    {
        free(tracks[i].events);
        free(tracks[i].notes);
    }
    free(tracks);
}
