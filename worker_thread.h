#pragma once

#include <pthread.h>

#include "event_loop.h"

struct WorkerThread {
    pthread_t thread_id;
    char name[24];
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    struct EventLoop* ev_loop;
};

int worker_thread_init(struct WorkerThread* thread, int index);
void worker_thread_run(struct WorkerThread* thread);