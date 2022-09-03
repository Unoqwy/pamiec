#include "daemon.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/un.h>

#include "record.h"

#define BUF_SIZE 1024

int daemon_start(char *name) {
    int fd;

    if ((fd = socket(PF_UNIX, SOCK_DGRAM, 0)) < 0) {
        return -1;
    }

    struct sockaddr_un addr = {
        .sun_family = AF_UNIX,
    };

    char path[108];
    sprintf(path, DAEMON_SOCKET_PATH, name);
    strncpy(addr.sun_path, path, sizeof(addr.sun_path));
    addr.sun_path[sizeof(addr.sun_path) - 1] = 0;

    unlink(addr.sun_path);

    if (bind(fd, (struct sockaddr * )&addr, sizeof(addr)) < 0) {
        return -1;
    }

    int len;
    char buf[BUF_SIZE];
    struct sockaddr_un from;
    socklen_t from_len;

    printf("Listening...\n");
    while ((len = recvfrom(fd, buf, BUF_SIZE, 0, (struct sockaddr *)&from, &from_len)) > 0) {
        char op = buf[0];
        buf[strcspn(buf, "\n")] = 0;
        switch (op) {
            case OP_SAVE:
                char *filename = &buf[1];
                export_recording(filename);
                printf("Saved to '%s'\n", filename);
                break;
        }
    }

    if (fd >= 0) {
        close(fd);
    }
    return 0;
}
