#pragma once

#include "event_loop.h"
#include "buffer.h"
#include "channel.h"
#include "http_request.h"
#include "http_response.h"

#define MSG_SEND_AUTO

struct TcpConnection {
    struct EventLoop* ev_loop;
    struct Channel* channel;
    struct Buffer* read_buf;
    struct Buffer* write_buf;
    char name[32];
    struct HttpRequest* request;
    struct HttpResponse* response;
};

struct TcpConnection* tcp_connection_init(int fd, struct EventLoop* ev_loop);
int tcp_connection_destroy(void* arg);