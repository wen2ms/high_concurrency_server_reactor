#include "tcp_connection.h"

#include <stdio.h>
#include <stdlib.h>

int process_read(void* arg) {
    struct TcpConnection* conn = (struct TcpConnection*)arg;
    int count = buffer_socket_read(conn->read_buf, conn->channel->fd);
    if (count > 0) {

    } else {

    }
}

struct TcpConnection* tcp_connection_init(int fd, struct EventLoop* ev_loop) {
    struct TcpConnection* conn = (struct TcpConnection*)malloc(sizeof(struct TcpConnection));
    conn->ev_loop = ev_loop;
    conn->read_buf = buffer_init(10240);
    conn->write_buf = buffer_init(10240);
    sprintf(conn->name, "connection_%d", fd);
    conn->channel = channel_init(fd, kReadEvent, process_read, NULL, conn);
    event_loop_add_task(ev_loop, conn->channel, kAdd);
    return conn;
}