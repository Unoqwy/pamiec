#ifndef PAMIEC_EXPORT_H
#define PAMIEC_EXPORT_H

#include <pulse/pulseaudio.h>

typedef struct WAVE_header {
    /* RIFF chunk descriptor */
    char riff_id[4];
    int riff_size;
    char riff_fmt[4];

    /* "fmt" subchunk */
    char fmt_id[4];
    int fmt_size;
    short fmt_audio;
    short fmt_channels;
    int fmt_sample_rate;
    int fmt_byte_rate;
    short fmt_block_align;
    short fmt_bitrate;

    /* "data" subchunk */
    char data_id[4];
    int data_size;
} WAVE_header;

WAVE_header make_wave_header(pa_sample_spec spec, short birate, size_t data_size);

int write_wave_file(WAVE_header header, uint8_t *data, size_t data_size, char *filename);

#endif
