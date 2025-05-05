#include "thread_pool.h"

#include <stdlib.h>

struct ThreadPool* thread_pool_init(struct EventLoop* main_loop, int count) {
    struct ThreadPool* pool = (struct ThreadPool*)malloc(sizeof(struct ThreadPool));
    pool->index = 0;
    pool->is_start = false;
    pool->main_loop = main_loop;
    pool->num_threads = count;
    pool->worker_threads = (struct WorkerThread*)malloc(count * sizeof(struct WorkerThread));
    return pool;
}