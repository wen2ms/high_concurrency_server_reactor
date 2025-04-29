#include "channel_map.h"

#include <stdlib.h>
#include <string.h>

struct ChannelMap* channel_map_init(int size) {
    struct ChannelMap* map = (struct ChannelMap*)malloc(sizeof(struct ChannelMap));
    map->size = size;
    map->list = (struct Channel**)malloc(size * sizeof(struct Channel*));
    return map;
}

void channel_map_clear(struct ChannelMap* map) {
    if (map != NULL) {
        for (int i = 0; i < map->size; ++i) {
            if (map->list[i] != NULL) {
                free(map->list[i]);
            }
        }

        free(map->list);
        map->list = NULL;
    }
    map->size = 0;
}

bool make_map_room(struct ChannelMap* map, int new_size, int unit_size) {
    if (map->size < new_size) {
        int cur_size = map->size;
        while (cur_size < new_size) {
            cur_size *= 2;
        }

        struct Channel** temp = realloc(map->list, cur_size * unit_size);
        if (temp == NULL) {
            return false;
        }
        map->list = temp;
        memset(&map->list[map->size], 0, (cur_size - map->size) * unit_size);
        map->size = cur_size;
    }
    return true;
}
