#include "serialtermio.h"

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <poll.h>
#include <errno.h>

int pipefd;
char* pipeName_;

void termioInit(char* pipeName) {
    pipeName_ = pipeName;

    if(mkfifo(pipeName, 0666) && errno != EEXIST)
        return;

    printf("Waiting for terminal client @%s... ", pipeName);
    fflush(stdout);

    pipefd = open(pipeName, O_RDONLY);
    printf("Connected!\n");

    int flags = fcntl(pipefd, F_GETFL, 0);
    fcntl(pipefd, F_SETFL, flags | O_NONBLOCK);
}

/*
 * Read up to size bytes into buff without blocking.
 * Returns the number of bytes read, or 0 if none were available.
 */
int termioRead(char* buff, int size) {
    struct pollfd rfds = {
        pipefd,
        POLLIN,
        0
    };
    ssize_t count;

    if(poll(&rfds, 1, 0) > 0) {
        count = read(pipefd, buff, size);
        if(count > 0)
            return count;
    }

    return 0;
}

void termioClose() {
    close(pipefd);
    unlink(pipeName_);
}
