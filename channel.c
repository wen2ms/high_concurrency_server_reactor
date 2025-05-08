#include "channel.h"

#include <stdlib.h>

struct Channel* channel_init(int fd, int events, handle_func read_func, handle_func write_func, handle_func destroy_func,
                             void* arg) {
    struct Channel* channel = (struct Channel*)malloc(sizeof(struct Channel));
    channel->fd = fd;
    channel->events = events;
    channel->read_callback = read_func;
    channel->write_callback = write_func;
    channel->destroy_callback = destroy_func;
    channel->arg = arg;
    return channel;
}

void write_event_enable(struct Channel* channel, bool flag) {
    if (flag) {
        channel->events |= kWriteEvent;
    } else {
        channel->events &= ~kWriteEvent;
    }
}

bool is_write_event_enable(struct Channel* channel) {
    return channel->events & kWriteEvent;
}
