# Epoll监听客户端数据

在上一节中我们将cfd 加入到了epoll中，如果客户端对该连接发出数据，那么epoll_wait 返回的数据events中同样包含该客户端请求数据:

```diff
+} else {
+ int done = 0;
+ while (true) {
+   int count;
+   char buf[512];
+   errno = 0;
+   if ((count = read(events[i].data.fd, buf, sizeof(buf))) == -1) {
+     if (errno != EAGAIN) {
+       perror("read");
+       done = 1;
+     }
+     break;
+   } else if (0 == count) { // 当客户端断开连接时，会收到`0 == count`
+     done = 1;
+     break;
+   }
+   if ((status = write(events[i].data.fd, buf, count)) < 0) {
+     perror("write");
+     return 1;
+   }
+ }
+ if (done) {
+   printf("cfd: %d connection going to close!\n", events[i].data.fd);
+   close(events[i].data.fd);
+ }
}
```

以上对客户端请求的数据进行读取，并且回复相同数据。当收到客户端断开连接时, 关闭连接。
