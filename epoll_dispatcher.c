#include <sys/epoll.h>
#include <stdlib.h>
#include <stdio.h>

#include "dispatcher.h"

#define Max 520
struct EpollData {
    int epfd;
    struct epoll_event* events;
};

static void* epoll_init();
static int epoll_add(struct Channel* channel, struct EventLoop* evloop);
static int epoll_remove(struct Channel* channel, struct EventLoop* evloop);
static int epoll_modify(struct Channel* channel, struct EventLoop* evloop);
static int epoll_dispatch(struct EventLoop* evloop, int timeout);
static int epoll_clear(struct EventLoop* evloop);
static int epoll_op(struct Channel* channel, struct EventLoop* evloop, int op);

struct Dispatcher epoll_dispatcher = {epoll_init, epoll_add, epoll_remove, epoll_modify, epoll_dispatch, epoll_clear};

static void* epoll_init() {
    struct EpollData* data = (struct EpollData*)malloc(sizeof(struct EpollData));
    data->epfd = epoll_create(10);
    if (data->epfd == -1) {
        perror("epoll_create");
        exit(0);
    }
    data->events = (struct epoll_event*)calloc(Max, sizeof(struct epoll_event));

    return data;
}

static int epoll_op(struct Channel* channel, struct EventLoop* evloop, int op) {
    struct EpollData* data = (struct EpollData*)evloop->dispatcher_data;
    struct epoll_event ev;
    ev.data.fd = channel->fd;

    int events = 0;
    if (channel->events & kReadEvent) {
        events |= EPOLLIN;
    }
    if (channel->events & kWriteEvent) {
        events |= EPOLLOUT;
    }
    ev.events = events;
    int ret = epoll_ctl(data->epfd, op, channel->fd, &ev);
    return ret;
}

static int epoll_add(struct Channel* channel, struct EventLoop* evloop) {
    int ret = epoll_op(channel, evloop, EPOLL_CTL_ADD);
    if (ret == -1) {
        perror("epoll_add");
        exit(0);
    }
    return ret;
}

static int epoll_remove(struct Channel* channel, struct EventLoop* evloop) {
    int ret = epoll_op(channel, evloop, EPOLL_CTL_DEL);
    if (ret == -1) {
        perror("epoll_remove");
        exit(0);
    }
    return ret;
}

static int epoll_modify(struct Channel* channel, struct EventLoop* evloop) {
    int ret = epoll_op(channel, evloop, EPOLL_CTL_MOD);
    if (ret == -1) {
        perror("epoll_modify");
        exit(0);
    }
    return ret;
}

static int epoll_dispatch(struct EventLoop* evloop, int timeout) {}

static int epoll_clear(struct EventLoop* evloop) {}