#include "http_response.h"

#include <stdlib.h>
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

    response->send_data_func = NULL;

    return response;
}