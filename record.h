#ifndef PAMIEC_RECORD_H
#define PAMIEC_RECORD_H

#include <pthread.h>

#include <pulse/pulseaudio.h>

typedef struct record_config {
    char *sink;
    char *app_name;
    char *record_lore;
    int retain_seconds;
} record_config;

static const pa_sample_spec spec = {
    .format = PA_SAMPLE_S16LE,
    .rate = 44100,
    .channels = 2,
};
static const short spec_bitrate = 16;

void start_recording(record_config config);

void stop_recording();

void export_recording(char *filename);

pthread_t pthread_recording();

#endif
