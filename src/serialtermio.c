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

int termioRead() {
    struct pollfd rfds = {
        pipefd,
        POLLIN,
        0
    };
    char buff;

    if(poll(&rfds, 1, 5) > 0) {
        read(pipefd, &buff, 1);
        return buff;
    }

    return -1;
}

void termioClose() {
    close(pipefd);
    unlink(pipeName_);
}
