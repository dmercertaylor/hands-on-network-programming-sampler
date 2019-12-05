#include "service_utils.h"

SOCKET get_local_socket(int addr_family, const char * port, char ** addr, int flags)
{
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = addr_family;
    hints.ai_socktype = SOCK_STREAM;
    if(flags != 0) hints.ai_flags = flags;

    struct addrinfo *bind_address_head;
    /* Get local address at port */
    if(getaddrinfo(NULL, port, &hints, &bind_address_head) != 0){
        fprintf(stderr, "Failed to get local address info in get_local_socket. errno: %d\n", GETSOCKETERRNO());
        return -1;
    }
    struct addrinfo *bind_address = bind_address_head;
    /* Make socket at bind_address */
    int socket_local = -1;
    /* Run through all addresses in bind_address */
    for(;bind_address != NULL; bind_address = bind_address->ai_next){
        socket_local = socket(bind_address->ai_family, bind_address->ai_socktype,
                                bind_address->ai_protocol);
        if(socket_local != -1) break; 
        CLOSESOCKET(socket_local);
    }

    if(!ISVALIDSOCKET(socket_local)){
        fprintf(stderr, "socket() failed in get_local address. errno : %d\n", GETSOCKETERRNO());
        return -1;
    }

#ifdef IPV6_V6ONLY
    if(addr_family == AF_INET6){
        /* set socket to dual listen mode */
        int option = 0;
        if(setsockopt(socket_local, IPPROTO_IPV6, IPV6_V6ONLY,
                        (void*) &option, sizeof(option))){
            fprintf(stderr, "setsockopt() failed in get_local_socket. errno: %d\n", GETSOCKETERRNO());
            return -1;
        }
    }
#endif

    /* Bind socket to local address */
    if( bind(socket_local, bind_address->ai_addr, bind_address->ai_addrlen)){
        fprintf(stderr, "bind() failed in get_local_socket. errno: %d\n", GETSOCKETERRNO());
        return -1;
    }

    static char buffer[INET6_ADDRSTRLEN + 3];
    getnameinfo((struct sockaddr*)bind_address->ai_addr, sizeof(*bind_address->ai_addr),
                buffer, sizeof(buffer), NULL, 0, NI_NUMERICHOST);
    *addr = buffer;

    freeaddrinfo(bind_address_head);
    return socket_local;
}

int start_sockets(void)
{
#ifdef _WIN32
    WSADATA d;
    if(WSAStartup(MAKEWORD(2,2), &2)){
        fprintf(stderr, "Failed to init\n");
        return 1;
    }
#endif
    return 0;
}