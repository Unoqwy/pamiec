#include <stdio.h>
#include <string.h>

#include <ctype.h>
#include <unistd.h>
#include <signal.h>

#include "daemon.h"
#include "record.h"

static void sig_int_handler(int code) {
    stop_recording();
}

void send_usage(char *bin) {
    printf("%s daemon <sink> [-n <name>] [-d <duration>]\n", bin);
}

int main_daemon(int argc, char **argv) {
    char sink[256];

    char name[32] = "main";
    int retain_seconds = 30;

    int vararg_count = 0;
    for (int arg = 2; arg < argc; arg++) {
        if (argv[arg][0] == '-') {
            char *opt = argv[arg];
            if (strcmp(opt, "-n") == 0 || strcmp(opt, "--name") == 0) {
                if (++arg >= argc) {
                    fprintf(stderr, "Missing value for option %s\n", opt);
                    return 1;
                }
                strncpy(name, &argv[arg][0], 32);
            } else if (strcmp(opt, "-d") == 0 || strcmp(opt, "--duration") == 0) {
                if (++arg >= argc) {
                    fprintf(stderr, "Missing value for option %s\n", opt);
                    return 1;
                }
                char *val = argv[arg];
                int val_len = strlen(val);
                for (int ch = 0; ch < val_len; ch++) {
                    if (!isdigit(val[ch])) {
                        fprintf(stderr, "Option %s expects a number, but got \"%s\"\n", opt, val);
                        return 1;
                    }
                }
                retain_seconds = atoi(val);
            } else {
                fprintf(stderr, "Unknown option \"%s\"\n", opt);
                return 1;
            }
        } else {
            vararg_count++;
            if (vararg_count == 1) {
                strncpy(sink, argv[arg], 256);
            } else {
                fprintf(stderr, "Too many arguments! Did not expect \"%s\"\n", argv[arg]);
                return 1;
            }
        }
    }

    if (vararg_count == 0) {
        fprintf(stderr, "Missing <sink> argument\n");
        return 1;
    }

    record_config config = {
        .sink = sink,
        .app_name = "Pamiec",
        .record_lore = "Safekeeping the best moments",
        .retain_seconds = retain_seconds,
    };
    start_recording(config); // starts in another thread

    daemon_start(name);
    return 0;
}

int main(int argc, char **argv) {
    signal(SIGINT, sig_int_handler);

    if (argc < 2) {
        send_usage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "daemon") == 0) {
        return main_daemon(argc, argv);
    } else {
        send_usage(argv[0]);
    }

    return 0;
}

