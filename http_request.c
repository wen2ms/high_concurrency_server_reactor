#include "http_request.h"

#include <stdlib.h>

#define HEADER_SIZE 12

struct HttpRequest* http_request_init() {
    struct HttpRequest* request = (struct HttpRequest*)malloc(sizeof(struct HttpRequest));
    request->cur_state = kParseReqLine;
    request->method = NULL;
    request->url = NULL;
    request->version = NULL;
    request->req_headers = (struct RequestHeader*)malloc(HEADER_SIZE * sizeof(struct RequestHeader));
    request->num_req_headers = 0;
    return request;
}