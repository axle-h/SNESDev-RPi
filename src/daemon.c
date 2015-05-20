#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "daemon.h"

void StartDaemon(const char *pidFile) {
    daemon(0, 0);

    // Record PID
    int lfp = open(pidFile, O_RDWR | O_CREAT, 0640);
    if (lfp < 0) {
        exit(EXIT_FAILURE);
    }

    if (lockf(lfp, F_TLOCK, 0) < 0) {
        // Can not lock
        exit(EXIT_SUCCESS);
    }

    char str[10];
    sprintf(str, "%d\n", getpid());
    write(lfp, str, strlen(str));
}