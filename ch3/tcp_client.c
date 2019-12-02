#include "tcp_client.h"

int main(int argc, char **argv){
/* *** STARTUP / SETUP FOR WINDOWS *** */
#ifdef _WIN32
    WSADATA d;
    if(WSAStartup(MAKEWORD(2,2),&2)){
        fprintf(stderr, "Failed to init\n");
        return 1;
    }
#endif
    /* validate input args */
    if(argc < 3){
        fprintf(stderr, "usage: tcp_client hostname port\n");
        return 1;
    }
/* *** CONFIGURE REMOTE ADDRESS *** */
    printf("Configuring remote address...\n");

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    /* specify we want a TCP address */
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo *peer_address;

    /* See ch2 for details on this */
    if(getaddrinfo(argv[1], argv[2], &hints, &peer_address)){
        fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    /* (optional) print out address */
    char address_buffer[128];
    char service_buffer[128];
    getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen,
                address_buffer, sizeof(address_buffer),
                service_buffer, sizeof(service_buffer),
                NI_NUMERICHOST);
    printf("Remote address is: %s %s\n", address_buffer, service_buffer);

/* *** CREATE SOCKET *** */
    printf("Creating socket...\n");
    SOCKET socket_peer;
    socket_peer = socket(peer_address->ai_family, peer_address->ai_socktype,
                        peer_address->ai_protocol);
    if(!ISVALIDSOCKET(socket_peer)){
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

/* *** CONNECT TO SERVER *** */
    printf("Connecting...\n");
    if(connect(socket_peer, peer_address->ai_addr, peer_address->ai_addrlen)){
        fprintf(stderr, "connect() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }
    /*
        connect(int socket, const struct sockaddr *address, socklen_t address_len);
        socket: socket to make connection from
        address: peer address to connect to
        address_len: length of sockaddr pointed to by address
    */
    /* Deallocate addresses */
    freeaddrinfo(peer_address);

/* *** BEGIN select() LOOP *** */
    printf("Connected.\n");
    printf("To send data, enter text followed by enter.\n");
    
    /* the luup */
    while(1){
    /* CREATE fd_set */
        /* fd_set: set of file descriptors to be used
            with select() */
        fd_set reads;
        /* fd_set MACROS:
            FD_ZERO(fd_set *)           : clears the set
            FD_SET(int fd, fd_set *)    : adds a file descriptor to the set 
            FD_CLR(int fd, fd_set *)    : removes a file descriptor from the set
            FD_ISSET(int fd, fd_set *)  : checks if file descriptor is in set.
        */
        FD_ZERO(&reads);
        FD_SET(socket_peer, &reads);
#ifndef _WIN32
        FD_SET(0, &reads);
#endif
    /* RUN select() */
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;
        /*
            struct timeval {
                time_t   tv_sec; // whole seconds
                long int tv_usec; // microseconds, always less than 1 million
            }
        */
        if(select(socket_peer+1, &reads, NULL, NULL, &timeout) < 0){
            fprintf(stderr, "select() failed. (%d)\n", GETSOCKETERRNO());
            return 1;
        }
        /*
            int select(int nfds, fd_set *readfds, fd_set *writefds,
                        struct timeval *timeout);
            nfds: integer one more than the maximum of any file descriptor.
            readfds: fd_set holding the file descriptors to be checked for
                being ready to read, and on output indicates which file
                descriptors are ready to read. Can be null.
            writefds: fd_set holding file descriptors being checked for being
                ready to write, and on output indicates which file descriptors are
                ready to write. Can be null.
            errorfds: fd_set holding file descriptors to be checked for error
                conditions pending, and on output which file descriptors have
                error conditions pending. Can be NULL.
            timeout: struct timeval that specifies the maximum interval to wait for
                the select to complete. If the timeout argument points to an object
                of type struct timeval whose members are 0, selcect() does not block.
                If set to NULL, select() blocks until an event is read. Linux will
                update timeout in place to indicate elapsed time, but this is not
                POSIX standard.
        */
    /* RECIEVE DATA */
        if(FD_ISSET(socket_peer, &reads)){
            char read[4096];
            int bytes_recieved = recv(socket_peer, read, sizeof(read), 0);
            /* ssize_t recv(int sockfd, void *buf, size_t len, int flags); */
            if(bytes_recieved < 1){
                printf("Connection closed by peer.\n");
                break;
            }
            printf("Recieved (%d bytes): %.*s", bytes_recieved, bytes_recieved, read);
        }
    /* SEND DATA */
#ifdef _WIN32
        if(_kbhit()){
#else
        if(FD_ISSET(0, &reads)){
#endif
            char read[4096];
            if(!fgets(read, 4096, stdin)) break;
            printf("Sending: %s", read);
            int bytes_sent = send(socket_peer, read, strlen(read), 0);
            printf("Sent %d bytes.\n", bytes_sent);
        }
    }

/* *** CLOSE SOCKET *** */
    printf("Closing socket...\n");
    CLOSESOCKET(socket_peer);

#ifdef _WIN32
    WSACleanup();
#endif

    printf("Finished.\n");
    return 0;
}