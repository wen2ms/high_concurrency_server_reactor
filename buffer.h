#pragma once

struct Buffer {
    char* data;
    int capacity;
    int read_pos;
    int write_pos;
};

struct Buffer* buffer_init(int size);
void buffer_destroy(struct Buffer* buf);
void buffer_extend_room(struct Buffer* buffer, int size);
int buffer_readable_size(struct Buffer* buffer);
int buffer_writable_size(struct Buffer* buffer);
int buffer_append_data(struct Buffer* buffer, const char* data, int size);
int buffer_append_string(struct Buffer* buffer, const char* data);
int buffer_socket_read(struct Buffer* buffer, int fd);
char* buffer_find_crlf(struct Buffer* buffer);