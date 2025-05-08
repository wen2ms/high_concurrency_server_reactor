#include "http_request.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define HEADER_SIZE 12

struct HttpRequest* http_request_init() {
    struct HttpRequest* request = (struct HttpRequest*)malloc(sizeof(struct HttpRequest));
    http_request_reset(request);
    request->req_headers = (struct RequestHeader*)malloc(HEADER_SIZE * sizeof(struct RequestHeader));
    return request;
}

void http_request_reset(struct HttpRequest* req) {
    req->cur_state = kParseReqLine;
    req->method = NULL;
    req->url = NULL;
    req->version = NULL;
    req->num_req_headers = 0;
}

void http_request_reset_ex(struct HttpRequest* req) {
    free(req->method);
    free(req->url);
    free(req->version);
    if (req->req_headers != NULL) {
        for (int i = 0; i < req->num_req_headers; ++i) {
            free(req->req_headers[i].key);
            free(req->req_headers[i].value);
        }
        free(req->req_headers);
    }
    http_request_reset(req);
}

void http_request_destroy(struct HttpRequest* req) {
    if (req != NULL) {
        http_request_reset_ex(req);
        free(req);
    }
}

enum HttpRequestState http_request_state(struct HttpRequest* request) {
    return request->cur_state;
}

void http_request_add_header(struct HttpRequest* request, const char* key, const char* value) {
    request->req_headers[request->num_req_headers].key = key;
    request->req_headers[request->num_req_headers].value = value;
    request->num_req_headers++;
}

char* http_request_get_header(struct HttpRequest* request, const char* key) {
    if (request != NULL) {
        for (int i = 0; i < request->num_req_headers; ++i) {
            if (strncasecmp(request->req_headers[i].key, key, strlen(key)) == 0) {
                return request->req_headers[i].value;
            }
        }
    }
    return NULL;
}

char* split_request_line(const char* start, const char* end, const char* sub, char** ptr) {
    char* space = end;
    if (sub != NULL) {
        space = memmem(start, end - start, sub, strlen(sub));
        assert(space != NULL);
    }
    int length = space - start;
    char* tmp = (char*)malloc(length + 1);
    strncpy(tmp, start, length);
    tmp[length] = '\0';
    *ptr = tmp;
    return space + 1;
}

bool parse_http_request_line(struct HttpRequest* request, struct Buffer* read_buf) {
    char* end = buffer_find_crlf(read_buf);
    char* start = read_buf->data + read_buf->read_pos;
    int line_size = end - start;

    if (line_size) {
        start = split_request_line(start, end, " ", request->method);
        start = split_request_line(start, end, " ", request->url);
        split_request_line(start, end, NULL, request->version);

        read_buf->read_pos += line_size;
        read_buf->read_pos += 2;
        request->cur_state = kParseReqHeaders;
        return true;
    }

    return false;
}

bool parse_http_request_header(struct HttpRequest* request, struct Buffer* read_buf) {
    char* end = buffer_find_crlf(read_buf);
    if (end != NULL) {
        char* start = read_buf->data + read_buf->read_pos;
        int line_size = end - start;
        char* middle = memmem(start, line_size, ": ", 2);
        if (middle != NULL) {
            int key_length = middle - start;
            char* key = (char*)malloc(key_length + 1);
            strncpy(key, start, key_length);
            key[key_length] = '\0';

            int value_length = end - middle - 2;
            char* value = (char*)malloc(value_length + 1);
            strncpy(value, middle + 2, value_length);
            value[value_length] = '\0';

            http_request_add_header(request, key, value);
            read_buf->read_pos += line_size;
            read_buf->read_pos += 2;
        } else {
            read_buf->read_pos += 2;
            request->cur_state = kParseReqDone;
        }
        return true;
    }
    return false;
}
