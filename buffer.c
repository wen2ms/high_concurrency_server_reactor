#include "buffer.h"

#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <unistd.h>

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

int buffer_socket_read(struct Buffer* buffer, int fd) {
    struct iovec vec[2];
    int writable = buffer_writable_size(buffer);
    vec[0].iov_base = buffer->data + buffer->write_pos;
    vec[0].iov_len = writable;
    char* tmpbuf = (char*)malloc(40960);
    vec[1].iov_base = tmpbuf;
    vec[1].iov_len = 40960;
    int result = readv(fd, vec, 2);
    if (result == -1) {
        return -1;
    } else if (result <= writable) {
        buffer->write_pos += result;
    } else {
        buffer->write_pos = buffer->capacity;
        buffer_append_data(buffer, tmpbuf, result - writable);
    }
    free(tmpbuf);
    return result;
}

char* buffer_find_crlf(struct Buffer* buffer) {
    char* ptr = memmem(buffer->data + buffer->read_pos, buffer_readable_size(buffer), "\r\n", 2);
    return ptr;
}

int buffer_send_data(struct Buffer* buffer, int socket) {
    int readable = buffer_readable_size(buffer);
    if (readable > 0) {
        int count = send(socket, buffer->data + buffer->read_pos, readable, MSG_NOSIGNAL);
        if (count > 0) {
            buffer->read_pos += count;
            usleep(1);
        }
        return count;
    }
    return 0;
}
