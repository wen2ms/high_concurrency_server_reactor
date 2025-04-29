#pragma once

#include "dispatcher.h"

extern struct Dispatcher epoll_dispatcher;
struct EventLoop {
    struct Dispatcher* dispatcher;
    void* dispatcher_data;
};