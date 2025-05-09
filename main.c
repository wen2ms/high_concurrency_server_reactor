#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: %s port path\n", argv[0]);
        return -1;
    }

    unsigned short port = atoi(argv[1]);

    chdir(argv[2]);

    return 0;
}