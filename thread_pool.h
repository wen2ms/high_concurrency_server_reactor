#pragma once

#include <stdbool.h>

#include "event_loop.h"
#include "worker_thread.h"

struct ThreadPool {
    struct EventLoop* main_loop;
    bool is_start;
    int num_threads;
    struct WorkerThread* worker_threads;
    int index;
};

struct ThreadPool* thread_pool_init(struct EventLoop* main_loop, int count);
void thread_pool_run(struct ThreadPool* pool);
struct EventLoop* take_worker_event_loop(struct ThreadPool* pool);