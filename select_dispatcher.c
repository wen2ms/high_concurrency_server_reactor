#include <sys/select.h>
#include <stdlib.h>
#include <stdio.h>

#include "dispatcher.h"

#define MAX 1024
struct SelectData {
    fd_set read_set;
    fd_set write_set;
};

static void* select_init();
static int select_add(struct Channel* channel, struct EventLoop* ev_loop);
static int select_remove(struct Channel* channel, struct EventLoop* ev_loop);
static int select_modify(struct Channel* channel, struct EventLoop* ev_loop);
static int select_dispatch(struct EventLoop* ev_loop, int timeout);
static int select_clear(struct EventLoop* ev_loop);
static void set_fd_set(struct Channel* channel, struct SelectData* data);
static void clear_fd_set(struct Channel* channel, struct SelectData* data);


struct Dispatcher select_dispatcher = {select_init, select_add, select_remove, select_modify, select_dispatch, select_clear};

static void* select_init() {
    struct SelectData* data = (struct SelectData*)malloc(sizeof(struct SelectData));
    FD_ZERO(&data->read_set);
    FD_ZERO(&data->write_set);

    return data;
}

static void set_fd_set(struct Channel* channel, struct SelectData* data) {
    if (channel->events & kReadEvent) {
        FD_SET(channel->fd, &data->read_set);
    }
    if (channel->events & kWriteEvent) {
        FD_SET(channel->fd, &data->write_set);
    }
}

static void clear_fd_set(struct Channel* channel, struct SelectData* data) {
    if (channel->events & kReadEvent) {
        FD_CLR(channel->fd, &data->read_set);
    }
    if (channel->events & kWriteEvent) {
        FD_CLR(channel->fd, &data->write_set);
    }
}

static int select_add(struct Channel* channel, struct EventLoop* ev_loop) {
    struct SelectData* data = (struct SelectData*)ev_loop->dispatcher_data;
    if (channel >= MAX) {
        return -1;
    }
    set_fd_set(channel, data);

    return 0;
}

static int select_remove(struct Channel* channel, struct EventLoop* ev_loop) {
    struct SelectData* data = (struct SelectData*)ev_loop->dispatcher_data;
    clear_fd_set(channel, data);

    return 0;
}

static int select_modify(struct Channel* channel, struct EventLoop* ev_loop) {
    struct SelectData* data = (struct SelectData*)ev_loop->dispatcher_data;
    set_fd_set(channel, data);
    clear_fd_set(channel, data);

    return 0;
}

static int select_dispatch(struct EventLoop* ev_loop, int timeout) {
    struct SelectData* data = (struct SelectData*)ev_loop->dispatcher_data;
    struct timeval val;
    val.tv_sec = timeout;
    val.tv_usec = 0;
    fd_set rdtmp = data->read_set;
    fd_set wrtmp = data->write_set;
    int count = select(MAX, &rdtmp, &wrtmp, NULL, &val);
    if (count == -1) {
        perror("select");
        exit(0);
    }
    for (int i = 0; i < MAX; ++i) {
        if (FD_ISSET(i, &rdtmp)) {
            event_activate(ev_loop, i, kReadEvent);
        }
        if (FD_ISSET(i, &wrtmp)) {
            event_activate(ev_loop, i, kWriteEvent);

        }
    }

    return 0;
}

static int select_clear(struct EventLoop* ev_loop) {
    struct SelectData* data = (struct SelectData*)ev_loop->dispatcher_data;
    free(data);
}