#include "tcp_connection.h"

#include <stdio.h>
#include <stdlib.h>

int process_read(void* arg) {
    struct TcpConnection* conn = (struct TcpConnection*)arg;
    int count = buffer_socket_read(conn->read_buf, conn->channel->fd);
    if (count > 0) {
        int socket = conn->channel->fd;
#ifdef MSG_SEND_AUTO
        write_event_enable(conn->channel, true);
        event_loop_add_task(conn->ev_loop, conn->channel, kModify);
#endif
        bool flag = parse_http_request(conn->request, conn->read_buf, conn->response, conn->write_buf, socket);
        if (!flag) {
            char* err_msg = "HTTP/1.1 400 Bad Request\r\n\r\n";
            buffer_append_string(conn->write_buf, err_msg);
        }
    }
#ifndef MSG_SEND_AUTO
    event_loop_add_task(conn->ev_loop, conn->channel, kDelete);
#endif
}

int process_write(void* arg) {
    struct TcpConnection* conn = (struct TcpConnection*)arg;
    int count = buffer_send_data(conn->write_buf, conn->channel->fd);
    if (count > 0) {
        if (buffer_readable_size(conn->write_buf) == 0) {
            write_event_enable(conn->channel, false);
            event_loop_add_task(conn->ev_loop, conn->channel, kModify);
            event_loop_add_task(conn->ev_loop, conn->channel, kDelete);
        }
    }
}

struct TcpConnection* tcp_connection_init(int fd, struct EventLoop* ev_loop) {
    struct TcpConnection* conn = (struct TcpConnection*)malloc(sizeof(struct TcpConnection));
    conn->ev_loop = ev_loop;
    conn->read_buf = buffer_init(10240);
    conn->write_buf = buffer_init(10240);
    conn->request = http_request_init();
    conn->response = http_response_init();
    sprintf(conn->name, "connection_%d", fd);
    conn->channel = channel_init(fd, kReadEvent, process_read, process_write, conn);
    event_loop_add_task(ev_loop, conn->channel, kAdd);
    return conn;
}