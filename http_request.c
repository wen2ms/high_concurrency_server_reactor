#include "http_request.h"

#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/sendfile.h>

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

bool parse_http_request(struct HttpRequest* request, struct Buffer* read_buf, struct HttpResponse* response,
                        struct Buffer* send_buf, int socket) {
    bool flag = true;
    while (request->cur_state != kParseReqDone) {
        switch (request->cur_state) {
            case kParseReqLine:
                flag = parse_http_request_line(request, read_buf);
                break;
            case kParseReqHeaders:
                flag = parse_http_request_header(request, read_buf);
                break;
            case kParseReqBody:
                break;
            default:
                break;
        }

        if (!flag) {
            return flag;
        }
        if (request->cur_state == kParseReqDone) {
            process_http_request(request, response);

            http_response_prepare_msg(response, send_buf, socket);
        }
    }
    request->cur_state = kParseReqLine;
    return flag;
}

bool process_http_request(struct HttpRequest* request, struct HttpResponse* response) {
    if (strcasecmp(request->method, "get") != 0) {
        return false;
    }

    decode_msg(request->url, request->url);

    char* file = NULL;
    if (strcmp(request->url, "/") == 0) {
        file = "./";
    } else {
        file = request->url + 1;
    }

    struct stat st;
    int ret = stat(file, &st);
    if (ret == -1) {
        strcpy(response->file_name, "404.html");
        response->status_code = kNotFound;
        strcpy(response->status_msg, "Not Found");
        http_response_add_header(response, "Content-type", get_content_type(".html"));
        response->send_data_func = send_file;
        return true;
    }

    strcpy(response->file_name, file);
    response->status_code = kOK;
    strcpy(response->status_msg, "OK");
    if (S_ISDIR(st.st_mode)) {
        http_response_add_header(response, "Content-type", get_content_type(".html"));
        response->send_data_func = send_dir;
    } else {
        char tmp[12] = {0};
        sprintf(tmp, "%ld", st.st_size);
        http_request_add_header(response, "Content-type", get_content_type(file));
        http_request_add_header(response, "Content-length", tmp);
        response->send_data_func = send_file;
    }

    return true;
}

void decode_msg(char* to, char* from) {
    for (; *from != '\0'; ++from, ++to) {
        if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2])) {
            *to = hex_to_dec(from[1]) * 16 + hex_to_dec(from[2]);
            from += 2;
        } else {
            *to = *from;
        }
    }
    *to = '\0';
}

int hex_to_dec(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }

    return 0;
}

const char* get_content_type(const char* file_name) {
    const char* dot = strrchr(file_name, '.');
    if (dot == NULL) {
        return "text/plain; charset=utf-8";
    } else if (strcasecmp(dot, ".html") == 0 || strcasecmp(dot, ".htm") == 0) {
        return "text/html; charset=utf-8";
    } else if (strcasecmp(dot, ".jpg") == 0 || strcasecmp(dot, ".jpeg") == 0) {
        return "image/jpeg";
    } else if (strcasecmp(dot, ".gif") == 0) {
        return "image/gif";
    } else if (strcasecmp(dot, ".png") == 0) {
        return "image/png";
    } else if (strcasecmp(dot, ".css") == 0) {
        return "text/css";
    } else if (strcasecmp(dot, ".au") == 0) {
        return "audio/basic";
    } else if (strcasecmp(dot, ".wav") == 0) {
        return "audio/wav";
    } else if (strcasecmp(dot, ".avi") == 0) {
        return "video/x-msvideo";
    } else if (strcasecmp(dot, ".midi") == 0 || strcasecmp(dot, ".mid") == 0) {
        return "audio/midi";
    } else if (strcasecmp(dot, ".mp3") == 0) {
        return "audio/mpeg";
    } else if (strcasecmp(dot, ".mov") == 0 || strcasecmp(dot, ".qt") == 0) {
        return "video/quicktime";
    } else if (strcasecmp(dot, ".mpeg") == 0 || strcasecmp(dot, ".mpe") == 0) {
        return "video/mpeg";
    } else if (strcasecmp(dot, ".vrml") == 0 || strcasecmp(dot, ".vrl") == 0) {
        return "model/vrml";
    } else if (strcasecmp(dot, ".ogg") == 0) {
        return "application/ogg";
    } else if (strcasecmp(dot, ".pac") == 0) {
        return "application/x-ns-proxy-autoconfig";
    } else {
        return "text/plain; charset=utf-8";
    }
}

void send_dir(const char* dir_name, struct Buffer* send_buf, int cfd) {
    char buf[4096] = {0};
    sprintf(buf, "<html><head><title>%s</title></head><body><table>", dir_name);

    struct dirent** namelist;
    int num = scandir(dir_name, &namelist, NULL, alphasort);
    for (int i = 0; i < num; ++i) {
        char* name = namelist[i]->d_name;
        struct stat st;
        char sub_path[1024] = {0};
        sprintf(sub_path, "%s/%s", dir_name, name);
        stat(sub_path, &st);
        if (S_ISDIR(st.st_mode)) {
            sprintf(buf + strlen(buf), "<tr><td><a href=\"%s/\">%s</a></td><td>%ld</td></tr>", name, name, st.st_size);
        } else {
            sprintf(buf + strlen(buf), "<tr><td><a href=\"%s\">%s</a></td><td>%ld</td></tr>", name, name, st.st_size);
        }
    
        buffer_append_string(send_buf, buf);
        memset(buf, 0, sizeof(buf));
        free(namelist[i]);
    }

    sprintf(buf, "</table></body></html>");
    buffer_append_string(send_buf, buf);
    free(namelist);

    return 0;
}

void send_file(const char* file_name, struct Buffer* send_buf, int cfd) {
    int fd = open(file_name, O_RDONLY);
    assert(fd > 0);

    while (1) {
        char buf[1024];
        int len = read(fd, buf, sizeof(buf));
        if (len > 0) {
            buffer_append_data(send_buf, buf, len);
            usleep(10);
        } else if (len == 0) {
            break;
        } else {
            perror("read");
        }
    }
    close(fd);
    return 0;
}