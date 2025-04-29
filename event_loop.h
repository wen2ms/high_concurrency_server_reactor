#pragma once

#include "dispatcher.h"

struct EventLoop {
    Dispatcher* dispatcher;
    void* dispatcher_data;
};