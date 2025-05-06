#pragma once

#include "event_loop.h"
#include "buffer.h"
#include "channel.h"

struct TcpConnection {
    struct EventLoop* ev_loop;
    struct Channel* channel;
    struct Buffer* read_buf;
    struct Buffer* write_buf;
    char name[12];
};

struct TcpConnection* tcp_connection_init(int fd, struct EventLoop* ev_loop);