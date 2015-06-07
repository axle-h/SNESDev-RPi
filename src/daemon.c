#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "daemon.h"

void TryStartDaemon(SNESDevConfig *config) {
    if(!config->RunAsDaemon || config->PidFile == NULL) {
        return;
    }

    daemon(0, 0);

    // Record PID
    config->PidFilePointer = open(config->PidFile, O_RDWR | O_CREAT, 0640);
    if (config->PidFilePointer < 0) {
        exit(EXIT_FAILURE);
    }

    if (lockf(config->PidFilePointer, F_TLOCK, 0) < 0) {
        // Can not lock
        exit(EXIT_SUCCESS);
    }

    char str[10];
    sprintf(str, "%d\n", getpid());
    write(config->PidFilePointer, str, strlen(str));
}


void TryStopDaemon(SNESDevConfig *config) {
    if(config->PidFilePointer < 0) {
        return;
    }

    close(config->PidFilePointer);

    if(config->PidFile != NULL) {
        // Delete the pid file
        remove(config->PidFile);
    }
}
