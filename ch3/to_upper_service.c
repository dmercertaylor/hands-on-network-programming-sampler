#include "service_utils.h"
#include <ctype.h>

int main(int argc, char ** argv)
{
    if(argc < 2){
        printf("Usage: to_upper_service port\n");
        return 1;
    }
    if(start_sockets()){
        fprintf(stderr, "Failed to start sockets API : %d\n", GETSOCKETERRNO());
        return 1;
    }

    char * local_addr_str;
    SOCKET local_socket = get_local_socket(AF_INET6, argv[1], &local_addr_str, AI_PASSIVE);
    if( !ISVALIDSOCKET(local_socket) ) fprintf(stderr, "Failed to create local socket\n");

    printf("Listening at [%s]:%s...\n", local_addr_str, argv[1]);
    if( listen( local_socket, 10 ) < 0 ){
        fprintf(stderr, "listen() failed : %d\n", GETSOCKETERRNO());
        return 1;
    }

    /* Add local_socket to fd_set so we can select() it */
    fd_set master;
    FD_ZERO(&master);
    FD_SET(local_socket, &master);
    SOCKET max_socket = local_socket;

    printf("Waiting for connections...\n");
    
    while(1){
        /* copy master to reads, as select modifies the set */
        fd_set reads = master;
        /* block until something in reads is ready to be read from */
        if( select(max_socket+1, &reads, NULL, NULL, NULL) < 0) {
            /*
                int select(int nfds, fd_set *readfds, fd_set *writefds,
                            fd_set *exceptfds, struct timeval *timeout);
            */
            fprintf(stderr, "select() failed : %d\n", GETSOCKETERRNO());
            return 1;
        }
        SOCKET i;
        for(i = 1; i <= max_socket; ++i){
            if(FD_ISSET(i, &reads)){
                if (i == local_socket) {
                    struct sockaddr_storage client_address;
                    socklen_t client_len = sizeof(client_address);
                    SOCKET socket_client = accept(local_socket,
                        (struct sockaddr*) &client_address, &client_len);
                    if(!ISVALIDSOCKET(socket_client)){
                        fprintf(stderr, "accept() failed : %d\n", GETSOCKETERRNO());
                        return 1;
                    }

                    FD_SET(socket_client, &master);
                    if(socket_client > max_socket) max_socket = socket_client;

                    char address_buffer[INET6_ADDRSTRLEN];
                    getnameinfo((struct sockaddr*)&client_address, client_len,
                        address_buffer, sizeof(address_buffer), 0, 0, NI_NUMERICHOST);
                    printf("New connection from %s\n", address_buffer);
                } else {
                    char read[1024];
                    int bytes_recieved = recv(i, read, 1024, 0);
                    if(bytes_recieved < 1) {
                        printf("Closing socket %d\n", i);
                        FD_CLR(i, &master);
                        CLOSESOCKET(i);
                        continue;
                    }

                    int j;
                    for(j = 0; j < bytes_recieved; ++j){
                        read[j] = toupper(read[j]);
                    }
                    send(i, read, bytes_recieved, 0);
                }
            } // End if(FD_ISSET)
        } // End for i to max_socket
    } // End while(1)

    printf("Closing listening socket...\n");

#if defined(_WIN32)
    WSACleanup();
#endif

    CLOSESOCKET(local_socket);
    return 0;
}