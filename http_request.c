#include "http_request.h"

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
