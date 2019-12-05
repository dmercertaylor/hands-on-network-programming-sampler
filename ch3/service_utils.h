/* Check if we're on windows
   This is from the book, I haven't tested it
   
    NOTE: I am only writing POSIX code, these
    macros are just here in case I change my
    mind about that in the future.
*/
#ifndef SOCKS_SERVICE_UTILS_H
#define SOCKS_SERVICE_UTILS_H

#ifdef _WIN32
    #ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0600
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <conio.h>
    #pragma comment(lib, "ws2_32.lib")

    #define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
    #define CLOSESOCKET(s) closesocket(s)
    #define GETSOCKETERRNO() (WSAGetLastError())
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
    #include <errno.h>

    #define ISVALIDSOCKET(s) ((s) >= 0)
    #define CLOSESOCKET(s) close(s)
    #define SOCKET int
    #define GETSOCKETERRNO() (errno)
#endif

#include <stdio.h>
#include <string.h>

int start_sockets(void);
/* Start up the socket API on windows */

SOCKET get_local_socket(int addr_family, const char *port, char ** addr, int addr_flags);
/*
    Returns socket descriptor for local socket, on port "port".
    *addr will be pointed to a statically allocated string representation 
    of the local IP address. Be aware this value could change after multiple calls.
    addr_flags: if non-0, flags will be added to hints in the getaddrinfo call.
*/
#endif