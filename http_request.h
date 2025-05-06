#pragma once

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
