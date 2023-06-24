# Epoll创建和错误判断

本章节创建了Epoll，等待IO更新的判断.

## EPOLL

对于IO量不大的情况，可以使用select 的IO多路复用机制，但是如果IO订阅量达到上万时则推荐Epoll。Epoll相比select/poll的优势如下:

1. 相比将fd列表从用户态拷贝到内核态，当fd数据很多时，这会造成低效。
2. epoll只会返回具体活跃的句柄，而select、poll则每次都会返回全部的句柄再加以判断

下面我们来加入EPOLL:

```diff
int epollsocket(int argc, char* argv[]) {
    ...
    int sfd, status;
    // efd为epoll的句柄
+   int efd;
    // event为写入`epoll_ctl`做准备，*events为从等待队列做准备
+   struct epoll_event event, *events;
    ...
    printf("server listen at port: %s\n", port);
    // epoll_create 只要创建大于0的大小即可
+   if ((efd = epoll_create1(0)) == -1) {
+       perror("epoll create");
+       return -1;
+   }
    // event.events 支持多种事件类型，EPOLLIN为监听输入事件，EPOLLET为边缘触发(即该次触发需要将全部数据都获取结束，否则不会再次触发返回)
+   event.events = EPOLLIN | EPOLLET;
    // event.data.fd 设置目标句柄，在epoll_wait 返回的数据中可以获取到来源信息
+   event.data.fd = sfd;
    // epoll_ctl支持新增、删除、修改
+   if ((status = epoll_ctl(efd, EPOLL_CTL_ADD, sfd, &event)) != 0) {
+       perror("epoll_ctl add sfd");
+       return 1;
+   }
+   events = (struct epoll_event*)calloc(MAXEVENTS, sizeof(event));
+   while (true) {
+       int n, i;
+       n = epoll_wait(efd, events, MAXEVENTS, -1);
+       for (i = 0; i < n; i++) {
+           if ((events[i].events & EPOLLERR) ||
+               (events[i].events & EPOLLHUP) ||
+               (!(events[i].events & EPOLLIN))
+           ) {
+               fprintf(stderr, "close unspecify status for fd: %d\n", events[i].data.fd);
+               close(events[i].data.fd);
+               continue;
+           }
+       }
+   }
+   free(events);
+   close(sfd);
+   close(efd);
}
```
