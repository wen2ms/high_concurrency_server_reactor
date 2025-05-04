#include "event_loop.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

struct EventLoop* event_loop_init() {
    return event_loop_init_ex(NULL);
}

void task_wakeup(struct EventLoop* ev_loop) {
    const char msg = "Wake Up!!!";
    write(ev_loop->socket_pair[0], msg, strlen(msg));
}

int read_local_message(void* arg) {
    struct EventLoop* ev_loop = (struct EventLoop*)arg;
    char buf[256];
    read(ev_loop->socket_pair[1], buf, sizeof(buf));
    return 0;
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
    int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, ev_loop->socket_pair);
    if (ret == -1) {
        perror("socketpair");
        exit(0);
    }
    struct Channel* channel = channel_init(ev_loop->socket_pair[1], kReadEvent, read_local_message, NULL, ev_loop);
    event_loop_add_task(ev_loop, channel, kAdd);

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
        event_loop_process_task(ev_loop);
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

int event_loop_add_task(struct EventLoop* ev_loop, struct Channel* channel, int type) {
    pthread_mutex_lock(&ev_loop->mutex);
    struct ChannelElement* node = (struct ChannelElement*)malloc(sizeof(struct ChannelElement));
    node->channel = channel;
    node->type = type;
    node->next = NULL;
    if (ev_loop->head = NULL) {
        ev_loop->head = ev_loop->tail = node;
    } else {
        ev_loop->tail->next = node;
        ev_loop->tail = node;
    }
    pthread_mutex_unlock(&ev_loop->mutex);

    if (ev_loop->thread_id == pthread_self()) {
        event_loop_process_task(ev_loop);
    } else {
        task_wakeup(ev_loop);
    }

    return 0;
}

int event_loop_process_task(struct EventLoop* ev_loop) {
    pthread_mutex_lock(&ev_loop->mutex);
    struct ChannelElement* head = ev_loop->head;
    while (head != NULL) {
        struct Channel* channel = head->channel;
        if (head->type == kAdd) {
            event_loop_add(ev_loop, channel);
        } else if (head->type == kDelete) {
            event_loop_remove(ev_loop, channel);
        } else if (head->type == kModify) {
            event_loop_modify(ev_loop, channel);
        }
        struct ChannelElement* tmp = head;
        head = head->next;
        free(tmp);
    }
    ev_loop->head = ev_loop->tail = NULL;
    pthread_mutex_unlock(&ev_loop->mutex);
    return 0;
}

int event_loop_add(struct EventLoop* ev_loop, struct Channel* channel) {
    int fd = channel->fd;
    struct ChannelMap* channel_map = ev_loop->channel_map;
    if (fd >= channel_map->size) {
        if (!make_map_room(channel_map, fd, sizeof(struct Channel*))) {
            return -1;
        }
    }

    if (channel_map->list[fd] == NULL) {
        channel_map->list[fd] = channel;
        ev_loop->dispatcher->add(channel, ev_loop);
    }

    return 0;
}

int event_loop_remove(struct EventLoop* ev_loop, struct Channel* channel) {
    int fd = channel->fd;
    struct ChannelMap* channel_map = ev_loop->channel_map;
    if (fd >= channel_map->size || channel_map->list[fd] == NULL) {
        return -1;
    }
    int ret = ev_loop->dispatcher->remove(channel, ev_loop);
    return ret;
}

int event_loop_modify(struct EventLoop* ev_loop, struct Channel* channel) {
    int fd = channel->fd;
    struct ChannelMap* channel_map = ev_loop->channel_map;
    if (fd >= channel_map->size || channel_map->list[fd] == NULL) {
        return -1;
    }
    int ret = ev_loop->dispatcher->modify(channel, ev_loop);
    return ret;
}

int destroy_channel(struct EventLoop* ev_loop, struct Channel* channel) {
    ev_loop->channel_map->list[channel->fd] = NULL;
    close(channel->fd);
    free(channel);
    return 0;
}
