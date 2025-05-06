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

void buffer_extend_room(struct Buffer* buffer, int size) {
    if (buffer_writable_size(buffer) >= size) {
        return;
    } else if (buffer->read_pos + buffer_writable_size(buffer) >= size) {
        int readable = buffer_readable_size(buffer);
        memcpy(buffer->data, buffer->data + buffer->read_pos, readable);
        buffer->read_pos = 0;
        buffer->write_pos = readable;
    } else {
        void* temp = realloc(buffer->data, buffer->capacity + size);
        if (temp == NULL) {
            return;
        }
        memset(temp + buffer->capacity, 0, size);
        buffer->data = temp;
        buffer->capacity += size;
    }
}

int buffer_readable_size(struct Buffer* buffer) {
    return buffer->write_pos - buffer->read_pos;
}

int buffer_writable_size(struct Buffer* buffer) {
    return buffer->capacity - buffer->write_pos;
}

int buffer_append_data(struct Buffer* buffer, const char* data, int size) {
    if (buffer == NULL || data == NULL || size <= 0) {
        return -1;
    }
    buffer_extend_room(buffer, size);
    memcpy(buffer->data + buffer->write_pos, data, size);
    buffer->write_pos += size;
    return 0;
}

int buffer_append_string(struct Buffer* buffer, const char* data) {
    int size = strlen(data);
    int ret = buffer_append_data(buffer, data, size);
    return ret;
}
