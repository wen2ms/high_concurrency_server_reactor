#pragma once

#include <stdbool.h>

struct ChannelMap {
    int size;
    struct Channel** list;
};

struct ChannelMap* channel_map_init(int size);
void channel_map_clear(struct ChannelMap* map);
bool make_map_room(struct ChannelMap* map, int new_size, int unit_size);