#ifndef PAMIEC_DAEMON_H
#define PAMIEC_DAEMON_H

#define DAEMON_SOCKET_PATH "/tmp/pamiec.%s.sock"

/* Operation Codes */
#define OP_SAVE 40

/** Start the daemon as a blocking process, listening for commands on a UNIX socket.
 * Giving the daemon a NAME allows to have several daemons running in parallel.
 * Note: It can only start if no other daemon already uses this name*/
int daemon_start(char *name);

#endif
