#pragma once

#include "channel.h"
#include "event_loop.h"

struct Dispatcher {
    void* (*init)();
    int (*add)(struct Channel* channel, struct EventLoop* ev_loop);
    int (*remove)(struct Channel* channel, struct EventLoop* ev_loop);
    int (*modify)(struct Channel* channel, struct EventLoop* ev_loop);
    int (*dispatch)(struct EventLoop* ev_loop, int timeout);
    int (*clear)(struct EventLoop* ev_loop);
};