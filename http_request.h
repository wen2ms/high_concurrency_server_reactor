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