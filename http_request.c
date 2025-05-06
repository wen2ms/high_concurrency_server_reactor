#include "http_request.h"

#include <stdlib.h>

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
    http_request_reset(req);
}

void http_request_destroy(struct HttpRequest* req) {
    if (req != NULL) {
        http_request_reset_ex(req);
        if (req->req_headers != NULL) {
            for (int i = 0; i < req->num_req_headers; ++i) {
                free(req->req_headers[i].key);
                free(req->req_headers[i].value);
            }
            free(req->req_headers);
        }
        free(req);
    }
}
