#include "tcp_server.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>

#include "tcp_connection.h"

struct TcpServer* tcp_server_init(unsigned short port, int num_threads) {
    struct TcpServer* tcp = (struct TcpServer*)malloc(sizeof(struct TcpServer));
    tcp->listener = listener_init(port);
    tcp->main_loop = event_loop_init();
    tcp->num_threads = num_threads;
    tcp->thread_pool = thread_pool_init(tcp->main_loop, num_threads); 
    return tcp;
}

struct Listener* listener_init(unsigned short port) {
    struct Listener* listener = (struct Listener*)malloc(sizeof(struct Listener));
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd == -1) {
        perror("socket");
        return NULL;
    }
    int opt = 1;
    int ret = setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (ret == -1) {
        perror("setsockopt");
        return NULL;
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    ret = bind(lfd, (struct sockaddr*)&addr, sizeof(addr));
    if (ret == -1) {
        perror("bind");
        return NULL;
    }
    ret = listen(lfd, 128);
    if (ret == -1) {
        perror("listen");
        return NULL;
    }
    listener->lfd = lfd;
    listener->port = port;
    return listener;
}

int accept_connection(void* arg) {
    struct TcpServer* server =(struct TcpServer*)arg;
    int cfd = accept(server->listener->lfd, NULL, NULL);
    struct EventLoop* ev_loop = take_worker_event_loop(server->thread_pool);
    tcp_connection_init(cfd, ev_loop);
}

void tcp_server_run(struct TcpServer* server) {
    thread_pool_run(server->thread_pool);
    struct Channel* channel = channel_init(server->listener->lfd, kReadEvent, accept_connection, NULL, NULL, server);
    event_loop_add_task(server->main_loop, channel, kAdd);
    event_loop_run(server->main_loop);
}
