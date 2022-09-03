#include "export.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>

WAVE_header make_wave_header(pa_sample_spec spec, short bitrate, size_t data_size) {
    short byte_rate = bitrate / 8;
    WAVE_header header = {
        .riff_id = "RIFF",
        .riff_size = sizeof(WAVE_header) - 8,
        .riff_fmt = "WAVE",
        .fmt_id = "fmt ",
        .fmt_size = 16, // PCM
        .fmt_audio = 1, // PCM
        .fmt_channels = spec.channels,
        .fmt_sample_rate = spec.rate,
        .fmt_byte_rate = spec.rate * spec.channels * byte_rate,
        .fmt_block_align = spec.channels * byte_rate,
        .fmt_bitrate = bitrate,
        .data_id = "data",
        .data_size = data_size,
    };
    return header;
}

int write_wave_file(WAVE_header header, uint8_t *data, size_t data_size, char *filename) {
    FILE *outf;

    outf = fopen(filename, "wb");
    if (fwrite(&header, sizeof(header), 1, outf) < 1) {
        fprintf(stderr, "Unable to write audio file header to disk\nwrite failed: %s", strerror(errno));
        return -1;
    }
    if (fwrite(data, sizeof(uint8_t), data_size, outf) < data_size) {
        fprintf(stderr, "Unable to write audio file data to disk\nwrite failed: %s", strerror(errno));
        return -1;
    }
    fclose(outf);
    return 0;
}
