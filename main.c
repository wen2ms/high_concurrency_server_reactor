#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "tcp_server.h"

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: %s port path\n", argv[0]);
        return -1;
    }

    unsigned short port = atoi(argv[1]);

    chdir(argv[2]);

    struct TcpServer* server = tcp_server_init(port, 4);
    tcp_server_run(server);

    return 0;
}