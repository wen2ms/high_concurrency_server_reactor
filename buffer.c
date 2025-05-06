#include "buffer.h"

#include <stdlib.h>
#include <string.h>

struct Buffer* buffer_init(int size) {
    struct Buffer* buffer = (struct Buffer*)malloc(sizeof(struct Buffer));
    if (buffer != NULL) {
        buffer->data = (char*)malloc(size);
        buffer->capacity = size;
        buffer->read_pos = buffer->write_pos = 0;
        memset(buffer->data, 0, size);
    }

    return buffer;
}

void buffer_destroy(struct Buffer* buf) {
    if (buf != NULL) {
        if (buf->data != NULL) {
            free(buf->data);
        }
    }
    free(buf);
}
