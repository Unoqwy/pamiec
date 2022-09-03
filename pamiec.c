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

static WAVE_header new_header(pa_sample_spec spec, short bitrate, int sample_count) {
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
    };
    header.data_size = sample_count * header.fmt_byte_rate;
    return header;
}

int main(int argc, char *argv[]) {
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
        "alsa_output.usb-SteelSeries_SteelSeries_Arctis_1_Wireless-00.analog-stereo.monitor",
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
    int recordBufCapacity = byte_rate * 30;
    uint8_t *recordBuf = calloc(recordBufCapacity, sizeof(uint8_t));
    uint8_t *ptr = recordBuf;

    signal(SIGINT, sig_int_handler);
    while (running) {
        uint8_t buf[PA_BUF_SIZE];

        if (pa_simple_read(sp, buf, sizeof(buf), &error) < 0) {
            fprintf(stderr, "Unable to read audio data.\npa_simple_read() failed: %s\n", pa_strerror(error));
            goto cleanup;
        }

        memcpy(ptr, &buf, sizeof(buf));
        ptr += sizeof(buf);
    }

    int bufLen = ptr - recordBuf;
    int sample_count = bufLen / byte_rate;
    WAVE_header header = new_header(spec, spec_bitrate, sample_count);

    printf("seconds recorded: %d\n", sample_count);

    FILE *outf;

    outf = fopen("output.wav", "wb");
    if (fwrite(&header, sizeof(header), 1, outf) < 1) {
        fprintf(stderr, "Unable to write audio file header to disk\nwrite failed: %s", strerror(errno));
    }
    if (fwrite(recordBuf, sizeof(uint8_t), bufLen, outf) < bufLen) {
        fprintf(stderr, "Unable to write audio file data to disk\nwrite failed: %s", strerror(errno));
    }
    fclose(outf);

    // Exiting normally
    ret = 0;

cleanup:
    free(recordBuf);
    if (sp) {
        pa_simple_free(sp);
    }
    printf("Goodbye!\n");
    return ret;
}

