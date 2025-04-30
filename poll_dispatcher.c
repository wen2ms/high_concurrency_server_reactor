#include <poll.h>
#include <stdlib.h>
#include <stdio.h>

#include "dispatcher.h"

#define MAX 1024
struct PollData {
    int maxfd;
    struct pollfd fds[MAX];
};

static void* poll_init();
static int poll_add(struct Channel* channel, struct EventLoop* evloop);
static int poll_remove(struct Channel* channel, struct EventLoop* evloop);
static int poll_modify(struct Channel* channel, struct EventLoop* evloop);
static int poll_dispatch(struct EventLoop* evloop, int timeout);
static int poll_clear(struct EventLoop* evloop);

struct Dispatcher poll_dispatcher = {poll_init, poll_add, poll_remove, poll_modify, poll_dispatch, poll_clear};

static void* poll_init() {
    struct PollData* data = (struct PollData*)malloc(sizeof(struct PollData));
    data->maxfd = 0;
    for (int i = 0; i < MAX; ++i) {
        data->fds[i].fd = -1;
        data->fds[i].events = 0;
        data->fds[i].revents = 0;
    }

    return data;
}

static int poll_add(struct Channel* channel, struct EventLoop* evloop) {
    struct PollData* data = (struct PollData*)evloop->dispatcher_data;
    int events = 0;
    if (channel->events & kReadEvent) {
        events |= POLLIN;
    }
    if (channel->events & kWriteEvent) {
        events |= POLLOUT;
    }

    int i = 0;
    for (; i < MAX; ++i) {
        if (data->fds[i].fd == -1) {
            data->fds[i].events = events;
            data->fds[i].fd = channel->fd;
            data->maxfd = i > data->maxfd ? i : data->maxfd;
            break; 
        }
    }
    if (i >= MAX) {
        return -1;
    }
    return 0;
}

static int poll_remove(struct Channel* channel, struct EventLoop* evloop) {
    struct PollData* data = (struct PollData*)evloop->dispatcher_data;
    int i = 0;
    for (; i < MAX; ++i) {
        if (data->fds[i].fd == channel->fd) {
            data->fds[i].events = 0;
            data->fds[i].revents = 0;
            data->fds[i].fd = -1;
            break; 
        }
    }
    if (i >= MAX) {
        return -1;
    }
    return 0;
}

static int poll_modify(struct Channel* channel, struct EventLoop* evloop) {
    struct PollData* data = (struct PollData*)evloop->dispatcher_data;
    int events = 0;
    if (channel->events & kReadEvent) {
        events |= POLLIN;
    }
    if (channel->events & kWriteEvent) {
        events |= POLLOUT;
    }

    int i = 0;
    for (; i < MAX; ++i) {
        if (data->fds[i].fd == channel->fd) {
            data->fds[i].events = events;
            break; 
        }
    }
    if (i >= MAX) {
        return -1;
    }
    return 0;
}

static int poll_dispatch(struct EventLoop* evloop, int timeout) {
    struct PollData* data = (struct PollData*)evloop->dispatcher_data;
    int count = poll(data->fds, data->maxfd + 1, timeout * 1000);
    if (count == -1) {
        perror("poll");
        exit(0);
    }
    for (int i = 0; i <= data->maxfd; ++i) {
        if (data->fds[i].revents == -1) {
            continue;
        }
        if (data->fds[i].revents & POLLIN) {}
        if (data->fds[i].revents & POLLOUT) {}
    }

    return 0;
}

static int poll_clear(struct EventLoop* evloop) {
    struct PollData* data = (struct PollData*)evloop->dispatcher_data;
    free(data);
}