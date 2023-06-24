# 设置Socket为非阻塞函数

如果你是一个server，而你想要listen正在进来的连接，或者同时读或者写。

如果你在`Accept`时或者`send`/`recv`时发生了blocking该怎么办？

你需要先将程序从默认的blocking mode改为non-block, 接着使用`select`(`poll`)或者`epoll` 这类IO多路复用功能(`epoll`是本实例讲解内容):

```cpp
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
```

注意，设置non-block 可设置服务器的句柄(sfd), 也可设置连上来的客户端句柄(cfd, 之后用到).
