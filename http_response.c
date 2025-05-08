#include "http_response.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define RES_HEADER_SIZE 16

struct HttpResponse* http_response_init() {
    struct HttpResponse* response = (struct HttpResponse*)malloc(sizeof(struct HttpResponse));
    response->num_headers = 0;
    int size = sizeof(struct ResponseHeader) * RES_HEADER_SIZE;
    response->headers = (struct ResponseHeader*)malloc(size);
    response->status_code = kUnknown;
    bzero(response->headers, size);
    bzero(response->status_msg, sizeof(response->status_msg));
    bzero(response->file_name, sizeof(response->file_name));

    response->send_data_func = NULL;

    return response;
}

void http_response_destroy(struct HttpResponse* response) {
    if (response != NULL) {
        free(response->headers);
        free(response);
    }
}

void http_response_add_header(struct HttpResponse* response, const char* key, const char* value) {
    if (response == NULL || key == NULL || value == NULL) {
        return;
    }
    strcpy(response->headers[response->num_headers].key, key);
    strcpy(response->headers[response->num_headers].value, value);
    response->num_headers++;
}

void http_response_prepare_msg(struct HttpResponse* response, struct Buffer* send_buf, int socket) {
    char tmp[1024] = {0};
    sprintf(tmp, "HTTP/1.1 %d %s\r\n", response->status_code, response->status_msg);
    buffer_append_string(send_buf, tmp);
    for (int i = 0; i < response->num_headers; ++i) {
        sprintf(tmp, "%s: %s\r\n", response->headers[i].key, response->headers[i].value);
        buffer_append_string(send_buf, tmp);
    }
    buffer_append_string(send_buf, "\r\n");
#ifndef MSG_SEND_AUTO
    buffer_send_data(send_buf, socket);
#endif

    response->send_data_func(response->file_name, send_buf, socket);
}