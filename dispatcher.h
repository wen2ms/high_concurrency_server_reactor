#pragma once

#include "channel.h"
#include "event_loop.h"

struct Dispatcher {
    void* (*init)();
    int (*add)(struct Channel* channel, struct EventLoop* evloop);
    int (*remove)(struct Channel* channel, struct EventLoop* evloop);
    int (*modify)(struct Channel* channel, struct EventLoop* evloop);
    int (*dispatch)(struct EventLoop* evloop, int timeout);
    int (*clear)(struct EventLoop* evloop);
};