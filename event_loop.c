#include "event_loop.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct EventLoop* event_loop_init() {
    return event_loop_init_ex(NULL);
}

struct EventLoop* event_loop_init_ex(const char* thread_name) {
    struct EventLoop* ev_loop = (struct EventLoop*)malloc(sizeof(struct EventLoop));
    ev_loop->is_quit = false;
    ev_loop->thread_id = pthread_self();
    pthread_mutex_init(&ev_loop->mutex, NULL);
    strcpy(ev_loop->thread_name, thread_name == NULL ? "main_thread" : thread_name);
    ev_loop->dispatcher = &epoll_dispatcher;
    ev_loop->dispatcher_data = ev_loop->dispatcher->init();
    ev_loop->head = ev_loop->tail = NULL;
    ev_loop->channel_map = channel_map_init(128);

    return ev_loop;
}

int event_loop_run(struct EventLoop* ev_loop) {
    assert(ev_loop != NULL);
    struct Dispatcher* dispatcher = ev_loop->dispatcher;
    if (ev_loop->thread_id != pthread_self()) {
        return -1;
    }

    while (!ev_loop->is_quit) {
        dispatcher->dispatch(ev_loop, 2);
    }

    return 0;
}

int event_activate(struct EventLoop* ev_loop, int fd, int event) {
    if (fd < 0 || ev_loop == NULL) {
        return -1;
    }

    struct Channel* channel = ev_loop->channel_map->list[fd];
    assert(channel->fd == fd);
    if (event & kReadEvent && channel->read_callback) {
        channel->read_callback(channel->arg);
    }
    if (event & kWriteEvent && channel->write_callback) {
        channel->write_callback(channel->arg);
    }

    return 0;
}
