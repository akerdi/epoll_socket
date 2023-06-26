# Epoll监听连接请求

由于之前我们将`sfd`加入到了epoll中，所以现在外部有连接请求时，epoll_wait 会在返回的数据events中带有`sfd == events[i].data.fd`，所以本章是对连接的处理:

```diff
    fprintf(stderr, "close unspecify status for fd: %d\n", events[i].data.fd);
    close(events[i].data.fd);
    continue;
+} else if (sfd == events[i].data.fd) {
+   while (true) {
+       struct sockaddr sockaddr;
+       socklen_t sock_len = sizeof(sockaddr);
+       int cfd;
        // 获取客户端信息: man getnameinfo
+       char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
+       if ((cfd = accept(events[i].data.fd, &sockaddr, &sock_len)) == -1) {
            // 如果出现错误，如果是EAGAIN或者EWOULDBLOCK说明数据读取完毕，结束此次读取
+           if (errno == EAGAIN || errno == EWOULDBLOCK) {
+               break;
+           }
            // 否则可能是系统级出错
+           perror("server: accept");
+           return 1;
+       }
+       if ((status = set_nonblocking(cfd)) != 0) {
+           perror("cfd set non blocking");
+           return 1;
+       }
        // 打印客户端连接信息
+       if ((status = getnameinfo(&sockaddr, sock_len, hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV)) == 0) {
+           printf("cfd connection success: %d(host: %s, port: %s)\n", cfd, hbuf, sbuf);
+       }
+       event.events = EPOLLIN | EPOLLET;
+       event.data.fd = cfd;
        // 将该客户端fd的EPOLLIN和EPOLLET也写入epoll I/O事件中
+       if ((status = epoll_ctl(efd, EPOLL_CTL_ADD, cfd, &event)) != 0) {
+           perror("cfd epoll add");
+           return -1;
+       }
+   }
}
```

以上将连接上来的客户端信息写入epoll I/O事件中，以便监听连接的客户端发出的事件。

