#include "record.h"

#include <stdio.h>
#include <string.h>

#include <pulse/error.h>
#include <pulse/pulseaudio.h>
#include <pulse/simple.h>

#include "export.h"

#define PA_BUF_SIZE 2048

pthread_t thread_id;
pthread_mutex_t lock;

pa_simple *sp;

uint8_t *record_buf;
uint8_t *record_ptr;

void cleanup() {
    if (sp) {
        pa_simple_free(sp);
    }
    if (record_buf) {
        free(record_buf);
    }
}

void *record_loop(void *arg) {
    record_config config = *((record_config *) arg);
    free(arg);

    printf("Recorder: Retaining the %d last seconds\n", config.retain_seconds);

    int error;
    sp = pa_simple_new(
        NULL,
        config.app_name,
        PA_STREAM_RECORD,
        config.sink,
        config.record_lore,
        &spec,
        NULL,
        NULL,
        &error
    );
    if (!sp) {
        fprintf(stderr, "Cannot start recording audio.\npa_simple_new() failed: %s\n", pa_strerror(error));
        cleanup();
        return NULL;
    }

    int byte_rate = spec.rate * spec.channels * (spec_bitrate / 8);
    int record_buf_capacity = byte_rate * config.retain_seconds;

    record_buf = calloc(record_buf_capacity, sizeof(uint8_t));
    memset(record_buf, 0, record_buf_capacity);
    record_ptr = record_buf;

    int owningLock = 0;
    for (;;) {
        uint8_t read_buf[PA_BUF_SIZE];

        if (owningLock) {
            pthread_mutex_unlock(&lock);
            owningLock = 0;
        }

        if (pa_simple_read(sp, read_buf, sizeof(read_buf), &error) < 0) {
            fprintf(stderr, "Unable to read audio data.\npa_simple_read() failed: %s\n", pa_strerror(error));
            cleanup();
            return NULL;
        }

        if (!owningLock) {
            pthread_mutex_lock(&lock);
            owningLock = 1;
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

    if (owningLock) {
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

void start_recording(record_config config) {
    if (&lock == NULL) {
        if (pthread_mutex_init(&lock, NULL) != 0) {
            fprintf(stderr, "Cannot initialize mutex");
        }
    }
    if (thread_id == 0) {
        record_config *arg = malloc(sizeof(*arg));
        *arg = config;
        pthread_create(&thread_id, NULL, &record_loop, arg);
    }
}

void stop_recording() {
    if (thread_id != 0) {
        pthread_kill(thread_id, SIGTERM); // FIXME
    }
    if (&lock != NULL) {
        pthread_mutex_destroy(&lock);
    }
}

void export_recording(char *filename) {
    pthread_mutex_lock(&lock);

    int buf_len = record_ptr - record_buf;
    WAVE_header wave_header = make_wave_header(spec, spec_bitrate, buf_len);
    write_wave_file(wave_header, record_buf, buf_len, filename);

    pthread_mutex_unlock(&lock);
}

pthread_t pthread_recording() {
    return thread_id;
}

