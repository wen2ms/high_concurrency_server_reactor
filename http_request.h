#pragma once

#include <stdbool.h>

#include "buffer.h"
#include "http_response.h"

struct RequestHeader {
    char* key;
    char* value;
};

enum HttpRequestState {
    kParseReqLine,
    kParseReqHeaders,
    kParseReqBody,
    kParseReqDone
};

struct HttpRequest {
    char* method;
    char* url;
    char* version;
    struct RequestHeader* req_headers;
    int num_req_headers;
    enum HttpRequestState cur_state;
};

struct HttpRequest* http_request_init();
void http_request_reset(struct HttpRequest* req);
void http_request_reset_ex(struct HttpRequest* req);
void http_request_destroy(struct HttpRequest* req);
enum HttpRequestState http_request_state(struct HttpRequest* request);
void http_request_add_header(struct HttpRequest* request, const char* key, const char* value);
char* http_request_get_header(struct HttpRequest* request, const char* key);
bool parse_http_request_line(struct HttpRequest* request, struct Buffer* read_buf);
bool parse_http_request_header(struct HttpRequest* request, struct Buffer* read_buf);
bool parse_http_request(struct HttpRequest* request, struct Buffer* read_buf, struct HttpResponse* response,
                        struct Buffer* send_buf, int socket);
bool process_http_request(struct HttpRequest* request);
void decode_msg(char* to, char* from);