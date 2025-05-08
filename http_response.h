#pragma once

#include "buffer.h"

enum HttpStatusCode {
    kUnknown,
    kMovedPermanently = 301,
    kMovedTemporarily = 302,
    kBadRequest = 400,
    kNotFound = 404
};

struct ResponseHeader {
    char key[32];
    char value[128];
};

typedef void (*response_body)(const char* file_name, struct Buffer* send_buf, int socket);

struct HttpResponse {
    enum HttpStatusCode status_code;
    char status_msg[128];
    char file_name[128];
    struct ResponseHeader* headers;
    int num_headers;
    response_body send_data_func;
};

struct HttpResponse* http_response_init();
void http_response_destroy(struct HttpResponse* response);
void http_response_add_header(struct HttpResponse* response, const char* key, const char* value);
void http_response_prepare_msg(struct HttpResponse* response, struct Buffer* send_buf, int socket);