#include "epollsocket.h"

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

static int create_and_bindsocket(char* port) {
    int sfd, status;
    struct addrinfo hints, *result, *p;
    int yes = 1;

    memset(&hints, '\0', sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((status = getaddrinfo(NULL, port, &hints, &result)) != 0) {
        perror("getaddrinfo");
        return -1;
    }
    for (p = result; p != NULL; p = p->ai_next) {
        if ((sfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        if ((status = setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))) != 0) {
            perror("setsockopt");
            return -1;
        }
        if ((status = bind(sfd, p->ai_addr, p->ai_addrlen)) != 0) {
            perror("server: bind");
            continue;
        }
        break;
    }
    if (p == NULL) {
        perror("socket null");
        return -2;
    }
    freeaddrinfo(result);
    return sfd;
}
static int set_nonblocking(int fd) {
    int flags;
    flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        perror("fcntl get error");
        return -1;
    }
    flags |= O_NONBLOCK;
    if ((fcntl(fd, F_SETFL, flags)) != 0) {
        perror("fcntl set error");
        return -1;
    }
    return 0;
}

int epollsocket(int argc, char* argv[]) {
    // 设置端口
    char* port = "9007";
    if (argc > 1)
        port = argv[1];

    int sfd, status;
    if ((sfd = create_and_bindsocket(port)) < 0) {
        return 1;
    }
    if ((status = listen(sfd, SOMAXCONN)) != 0) {
        perror("server: listen");
        return 1;
    }
    if ((status = set_nonblocking(sfd)) != 0) {
        return status;
    }
    printf("server listen at port: %s\n", port);
    return 0;
}
