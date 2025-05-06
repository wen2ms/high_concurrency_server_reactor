#pragma once

struct Buffer {
    char* data;
    int capacity;
    int read_pos;
    int write_pos;
};

struct Buffer* buffer_init(int size);
void buffer_destroy(struct Buffer* buf);