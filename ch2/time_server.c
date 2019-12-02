#include "sock_init.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

int main(int argc, char ** argv){

/* **** FIGURE OUT LOCAL ADDRESS **** */
    printf("Configuring local address...\n");

    struct addrinfo hints;
    /*
        struct addrinfo {
            int              ai_flags;
            int              ai_family;
            int              ai_socktype;
            int              ai_protocol;
            socklen_t        ai_addrlen;
            struct sockaddr *ai_addr;
            char            *ai_canonname;
            struct addrinfo *ai_next;
        };
    */
    memset(&hints, 0, sizeof(hints));
    /* Specify we want an IPv4 address
       We could use AF_INET6 for IPv6 instead.
    */
    hints.ai_family = AF_INET6;
    /* Specify we want a stream socket (good for TCP) */
    hints.ai_socktype = SOCK_STREAM;
    /* Specify we want to listen on any available network interface */
    hints.ai_flags = AI_PASSIVE;

    /* create struct to hold returned value from getaddrinfo */
    struct addrinfo *bind_address;
    /* Get an address suitable to bind() */
    if( getaddrinfo(NULL, "8080", &hints, &bind_address) != 0 ){
        perror("Failed at getaddrinfo (time_server.c::41)");
    }
    /*
        int getaddrinfo(const char *hostname, const char *service
                        const struct addrinfo *hints,
                        struct addrinfo **res);

        hostname: a string containing either a domain name,
            or an address string, like "127.0.0.1", or NULL.
            If NULL, the address 0.0.0.0 or 127.0.0.1 is assigned,
            depending on hints
        service: a string containing either a port number (like 80)
            or a service, such as echo.
        hints: Either NULL or an addrinfo structure with the type of
            service requested
        res: a pointer pointing to a new addrinfo structure.
        
        Returns 0 on success and non-0 on failure.
    */

/* *** CREATE SOCKET *** */
    printf("Creating socket...\n");
    int socket_listen;

    socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype,
                            bind_address->ai_protocol);
    /*
        int socket(int domain, int type, int protocol);
        
        domain: address family, i.e AF_INET or AF_INET6
        type: Socket type, i.e. SOCK_STREAM or SOCK_DGRAM
        protocol: Generally, just set to 0, or PF_INET for internet.
    */

    if(socket_listen == -1){
        perror("Socket() failed, failed to get socket_listen");
    }

    /* Set socket to listen on both IPv4 and IPv6 (must be IPv6 address to do this) */
    int option = 0;
    if(setsockopt(socket_listen, IPPROTO_IPV6, IPV6_V6ONLY, (void*)&option, sizeof(option))){
        perror("setsockopt() failed.");
        return 1;
    }
/* *** BIND SOCKET TO LOCAL ADDRESS *** */
    printf("Binding socket to local address...\n");
    
    if( bind(socket_listen, bind_address->ai_addr,
            bind_address->ai_addrlen))
    {
        perror("bind() failed");
        return 1;
    }
    /*
        int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
        bind() assigns an address (specified by addr) to a socket (sockfd).
    */

/* Deallocate Addresses */
    freeaddrinfo(bind_address);

/* *** START LISTENING *** */
    printf("Listening...\n");
    if(listen(socket_listen, 10) < 0){
        perror("listen() failed");
        return 1;
    }
    /*
        int listen(int sockfd, int backlog);
        listen() marks sockfd as a socket that will accept incoming
        connection requests, using accept(). Backlog defines the max
        length of pending connections.
    */

/* *** ACCEPT CONNECTIONS *** */
    printf("Waiting for connection...\n");
    struct sockaddr_storage client_address;
    /*
        sockaddr_storage: garaunteed to hold the largest
        address suppported address on system.

        struct sockaddr_storage {
            sa_family_t  ss_family; // address family

            // all this is padding, implementation specific, ignore it:
            char      __ss_pad1[_SS_PAD1SIZE];
            int64_t   __ss_align;
            char      __ss_pad2[_SS_PAD2SIZE];
        };
    */

    socklen_t client_len = sizeof(client_address);
    /* create a new socket to hold incoming clients */
    int socket_client = accept(socket_listen, (struct sockaddr*) &client_address, &client_len);
    /*
        int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
        accept() takes the new connections off the queue from listen(), and
        returns a new socket descriptor.

        sockfd: The socket we are litening on
        addr:   Either a null pointer, or a pointer to the address
            where the incoming address will be returned.
        addr_len: Pointer to a socklen_t, which on inputs specifies
            the size of addr, and on output holds the newly supplied
            address length.
    */
/* *** HANDLE CONNECTIONS *** */
    /* Print out incoming connection (optional) */
    printf("Client is connected...\n");
    char address_buffer[128];
    getnameinfo((struct sockaddr*)&client_address,
                client_len, address_buffer, sizeof(address_buffer),
                0, 0, NI_NUMERICHOST);
    /* these comments are getting long, just check the man page */
    printf("%s\n", address_buffer);

    /* Read request */
    printf("Reading request...\n");
    char request[1024];
    int bytes_recieved = recv(socket_client, request, sizeof(request), 0);
    printf("Recieved %d bytes.\n", bytes_recieved);
    /* 
        To print the get request:
        printf("%.*s\n", bytes_recieved, request);
    */

/* *** SEND BACK A RESPONSE *** */
    printf("Sending response...\n");
    /* hard coded response lol */
    const char *response =
        "HTTP/1.1 200 OK\r\n"
        "Connection: close\r\n"
        "Content-Type: text/plain\r\n\r\n"
        "Local time is: ";
    int bytes_sent = send(socket_client, response, strlen(response), 0);
    printf("Sent %d of %d bytes.\n", bytes_sent, (int)strlen(response));

    /* Actually get the time (the whole point of the program) */
    time_t timer;
    time(&timer);
    char *time_msg = ctime(&timer);
    bytes_sent = send(socket_client, time_msg, strlen(time_msg), 0);
    printf("Sent %d of %d bytes.\n", bytes_sent, (int)strlen(time_msg));

/* *** CLOSE STUFF DOWN *** */
    printf("Closing connection...\n");
    close(socket_client);
    printf("Closing server...\n");
    close(socket_listen);

    printf("Finished.\n");
    return 0;
}