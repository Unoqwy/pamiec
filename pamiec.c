#include <stdio.h>

#include <pthread.h>
#include <unistd.h>
#include <signal.h>

#include "daemon.h"
#include "record.h"

static void sig_int_handler(int code) {
    stop_recording();
}

int main(int argc, char *argv[]) {
    signal(SIGINT, sig_int_handler);

    record_config config = {
        .sink = "alsa_output.usb-SteelSeries_SteelSeries_Arctis_1_Wireless-00.analog-stereo.monitor",
        .app_name = "Pamiec",
        .record_lore = "Safekeeping the best moments",
        .retain_seconds = 30,
    };
    start_recording(config);

    daemon_start("main");

    printf("Goodbye!\n");
    return 0;
}

