#include "thread_pool.h"

#include <assert.h>
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

void thread_pool_run(struct ThreadPool* pool) {
    assert(pool && !pool->is_start);
    if (pool->main_loop->thread_id != pthread_self()) {
        exit(0);
    }
    pool->is_start = true;
    if (pool->num_threads > 0) {
        for (int i = 0; i < pool->num_threads; ++i) {
            worker_thread_init(&pool->worker_threads[i], i);
            worker_thread_run(&pool->worker_threads[i]);
        }
    }
}

struct EventLoop* take_worker_event_loop(struct ThreadPool* pool) {
    assert(pool && !pool->is_start);
    if (pool->main_loop->thread_id != pthread_self()) {
        exit(0);
    }
    struct EventLoop* ev_loop = pool->main_loop;
    if (pool->num_threads > 0) {
        ev_loop = pool->worker_threads[pool->index].ev_loop;
        pool->index = ++pool->index % pool->num_threads;
    }

    return ev_loop;
}
