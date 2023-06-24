# EpollSocket

本实例指导如何简单实现Epoll + Socket 方案。

本实例目标:

+ [Part1 - 创建并bind Socket](./tech/part1.md)
+ Part2 - 设置Socket为非阻塞函数
+ Part3 - Epoll创建和错误判断
+ Part4 - Epoll监听连接请求
+ Part5 - Epoll监听客户端数据

## 环境

如果是Linux环境, 可以直接使用实例，否则推荐Mac使用Docker, Windows使用wsl2。

Mac环境的话:

        docker pull homebrew/ubuntu16.04
```yml
services:
  uu:
    container_name: uu
    image: homebrew/ubuntu16.04
    tty: true # 支持启动容器后不由于没有指令而exit的问题
    ports:
      - 9006:9006
```

        docker-compose up -d

## Ref

[Beej's Guide to Network Programming](https://beej-zhcn.netdpi.net/client-server_basic/jian_dan_de_stream_server)

[gist:epoll_server](https://gist.github.com/lidawn/90349e9260790e438a763c1e55b5b37c)