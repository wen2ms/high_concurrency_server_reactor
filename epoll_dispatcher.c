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

static int epoll_add(struct Channel* channel, struct EventLoop* evloop) {}

static int epoll_remove(struct Channel* channel, struct EventLoop* evloop) {}

static int epoll_modify(struct Channel* channel, struct EventLoop* evloop) {}

static int epoll_dispatch(struct EventLoop* evloop, int timeout) {}

static int epoll_clear(struct EventLoop* evloop) {}