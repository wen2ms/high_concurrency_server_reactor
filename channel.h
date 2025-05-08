#pragma once

#include <stdbool.h>

typedef int (*handle_func)(void* arg);

enum FDEvent {
    kTimeout = 0x01,
    kReadEvent = 0x02,
    kWriteEvent = 0x04
};

struct Channel {
    int fd;
    int events;
    handle_func read_callback;
    handle_func write_callback;
    handle_func destroy_callback;
    void* arg;
};

struct Channel* channel_init(int fd, int events, handle_func read_func, handle_func write_func, handle_func destroy_func,
                             void* arg);
void write_event_enable(struct Channel* channel, bool flag);
bool is_write_event_enable(struct Channel* channel);
