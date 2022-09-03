#include <string.h>
#include <stdio.h>

#include <signal.h>
#include <errno.h>

#include <pulse/error.h>
#include <pulse/pulseaudio.h>
#include <pulse/simple.h>

#define PA_BUF_SIZE 1024

static volatile int running = 1;

static void sig_int_handler(int code) {
    running = 0;
}

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

static WAVE_header new_header(pa_sample_spec spec, short bitrate, int data_size) {
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

int main(int argc, char *argv[]) {
    char *sink = "alsa_output.usb-SteelSeries_SteelSeries_Arctis_1_Wireless-00.analog-stereo.monitor";
    int keep_seconds = 30;

    static const pa_sample_spec spec = {
        .format = PA_SAMPLE_S16LE,
        .rate = 44100,
        .channels = 2,
    };
    static const short spec_bitrate = 16;

    pa_simple *sp;
    int ret = 1;
    int error;

    sp = pa_simple_new(
        NULL,
        "Pamiec",
        PA_STREAM_RECORD,
        sink,
        "Safekeeping the best moments",
        &spec,
        NULL,
        NULL,
        &error
    );
    if (!sp) {
        fprintf(stderr, "Cannot start recording audio.\npa_simple_new() failed: %s\n", pa_strerror(error));
        goto cleanup;
    }

    int byte_rate = spec.rate * spec.channels * (spec_bitrate / 8);
    int record_buf_capacity = byte_rate * keep_seconds;
    uint8_t *record_buf = calloc(record_buf_capacity, sizeof(uint8_t));
    uint8_t *record_ptr = record_buf;

    signal(SIGINT, sig_int_handler);
    while (running) {
        uint8_t read_buf[PA_BUF_SIZE];

        if (pa_simple_read(sp, read_buf, sizeof(read_buf), &error) < 0) {
            fprintf(stderr, "Unable to read audio data.\npa_simple_read() failed: %s\n", pa_strerror(error));
            goto cleanup;
        }

        int read_buf_len = sizeof(read_buf);
        if (record_ptr + read_buf_len >= record_buf + record_buf_capacity) {
            /* Shift the buffer to discard the oldest second of data and free enough space for a new second.
             * An actual second may not have elapsed between two movements as PA may return more
             * than a second at a time (in "packets" of PA_BUF_SIZE length) */
            uint8_t *shift_from = record_buf + byte_rate; // first byte to keep
            size_t data_len = (record_buf + record_buf_capacity) - shift_from;
            memmove(record_buf, shift_from, data_len);
            record_ptr = record_buf + data_len;
        }

        memcpy(record_ptr, &read_buf, read_buf_len);
        record_ptr += read_buf_len;
    }

    FILE *outf;
    int buf_len = record_ptr - record_buf;
    WAVE_header header = new_header(spec, spec_bitrate, buf_len);

    outf = fopen("output.wav", "wb");
    if (fwrite(&header, sizeof(header), 1, outf) < 1) {
        fprintf(stderr, "Unable to write audio file header to disk\nwrite failed: %s", strerror(errno));
    }
    if (fwrite(record_buf, sizeof(uint8_t), buf_len, outf) < buf_len) {
        fprintf(stderr, "Unable to write audio file data to disk\nwrite failed: %s", strerror(errno));
    }
    fclose(outf);

    // Exiting normally
    ret = 0;

cleanup:
    free(record_buf);
    if (sp) {
        pa_simple_free(sp);
    }
    printf("Goodbye!\n");
    return ret;
}

