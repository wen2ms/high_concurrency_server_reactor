#pragma once

#include "event_loop.h"
#include "thread_pool.h"

struct Listener {
    int lfd;
    unsigned short port;
};

struct TcpServer {
    int num_threads;
    struct EventLoop* main_loop;
    struct ThreadPool* thread_pool;
    struct Listener* listener;
};

struct TcpServer* tcp_server_init(unsigned short port, int num_threads);
struct Listener* listener_init(unsigned short port);