#include "worker_thread.h"

#include <stdio.h>

int worker_thread_init(struct WorkerThread* thread, int index) {
    thread->ev_loop = NULL;
    thread->thread_id = 0;
    sprintf(thread->name, "sub_thread_%d", index);
    pthread_mutex_init(&thread->mutex, NULL);
    pthread_cond_init(&thread->cond, NULL);
    return 0;
}

void* sub_thread_running(void* arg) {
    struct WorkerThread* thread = (struct WorkerThread*)arg;

    pthread_mutex_lock(&thread->mutex);
    thread->ev_loop = event_loop_init_ex(thread->name);
    pthread_cond_signal(&thread->cond);
    pthread_mutex_unlock(&thread->mutex);
    event_loop_run(thread->ev_loop);
    return NULL;
}

void worker_thread_run(struct WorkerThread* thread) {
    pthread_create(&thread->thread_id, NULL, sub_thread_running, thread);
    pthread_mutex_lock(&thread->mutex);
    while (thread->ev_loop == NULL) {
        pthread_cond_wait(&thread->cond, &thread->mutex);
    }
    pthread_mutex_unlock(&thread->mutex);
}
