#pragma once

#include <stdbool.h>
#include <pthread.h>

#include "dispatcher.h"
#include "channel.h"
#include "channel_map.h"

extern struct Dispatcher epoll_dispatcher;
extern struct Dispatcher poll_dispatcher;
extern struct Dispatcher select_dispatcher;

enum ElemType {
    kAdd,
    kDelete,
    kModify
};

struct ChannelElement {
    int type;
    struct Channel* channel;
    struct ChannelElement* next;
};

struct EventLoop {
    bool is_quit;
    struct Dispatcher* dispatcher;
    void* dispatcher_data;
    struct ChannelElement* head;
    struct ChannelElement* tail;
    struct ChannelMap* channel_map;
    pthread_t thread_id;
    char thread_name[32];
    pthread_mutex_t mutex;
    int socket_pair[2];
};

struct EventLoop* event_loop_init();
struct EventLoop* event_loop_init_ex(const char* thread_name);
int event_loop_run(struct EventLoop* ev_loop);
int event_activate(struct EventLoop* ev_loop, int fd, int event);
int event_loop_add_task(struct EventLoop* ev_loop, struct Channel* channel, int type);
int event_loop_process_task(struct EventLoop* ev_loop);
