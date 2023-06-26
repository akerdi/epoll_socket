#include "epollsocket.h"

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>

#define MAXEVENTS 64

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
    int efd;
    struct epoll_event event, *events;
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
    if ((efd = epoll_create1(0)) == -1) {
        perror("epoll create");
        return -1;
    }
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = sfd;
    if ((status = epoll_ctl(efd, EPOLL_CTL_ADD, sfd, &event)) != 0) {
        perror("epoll_ctl add sfd");
        return 1;
    }
    events = (struct epoll_event*)calloc(MAXEVENTS, sizeof(event));
    while (true) {
        int n, i;
        n = epoll_wait(efd, events, MAXEVENTS, -1);
        for (i = 0; i < n; i++) {
            if ((events[i].events & EPOLLERR) ||
                (events[i].events & EPOLLHUP) ||
                (!(events[i].events & EPOLLIN))
            ) {
                fprintf(stderr, "close unspecify status for fd: %d\n", events[i].data.fd);
                close(events[i].data.fd);
                continue;
            }  else if (sfd == events[i].data.fd) {
                while (true) {
                        struct sockaddr sockaddr;
                        socklen_t sock_len = sizeof(sockaddr);
                        int cfd;
                        char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
                        if ((cfd = accept(events[i].data.fd, &sockaddr, &sock_len)) == -1) {
                                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                                        break;
                                }
                                perror("server: accept");
                                return 1;
                        }
                        if ((status = set_nonblocking(cfd)) != 0) {
                                perror("cfd set non blocking");
                                return 1;
                        }
                        if ((status = getnameinfo(&sockaddr, sock_len, hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV)) == 0) {
                                printf("cfd connection success: %d(host: %s, port: %s)\n", cfd, hbuf, sbuf);
                        }
                        event.events = EPOLLIN | EPOLLET;
                        event.data.fd = cfd;
                        if ((status = epoll_ctl(efd, EPOLL_CTL_ADD, cfd, &event)) != 0) {
                                perror("cfd epoll add");
                                return -1;
                        }
                }
            }  else {
                int done = 0;
                while (true) {
                        int count;
                        char buf[512];
                        errno = 0;
                        if ((count = read(events[i].data.fd, buf, sizeof(buf))) == -1) {
                                if (errno != EAGAIN) {
                                        perror("read");
                                        done = 1;
                                }
                                break;
                        } else if (0 == count) {
                                done = 1;
                                break;
                        }
                        if ((status = write(events[i].data.fd, buf, count)) < 0) {
                                perror("write");
                                return 1;
                        }
                }
                if (done) {
                        printf("cfd: %d connection going to close!\n", events[i].data.fd);
                        close(events[i].data.fd);
                }
            }
        }
    }
    free(events);
    close(sfd);
    close(efd);
    return 0;
}
