#include "sock_init.h"
#include <stdio.h>

extern int errno;

int main(int argc, char ** argv){
#ifdef _WIN32
    WSADATA d;
    if (WSAStartup(MAKEWORD(2, 2), &d)){
        perror("Failed to initialize.\n");
        return 1;
    }
#endif
    // POSIX sockets need no setup lol
    printf("Socket API ready for use.\n");

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;

    // isn't windows a pain?
}