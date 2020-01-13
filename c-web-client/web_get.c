#include "c_web_client.h"

#define TIMEOUT 5.0
#define MAX_RESPONSE_SIZE 8191

/*  Takes url, points hostname, port, and path to their
    respective starts in url, and adds respective terminators
    after each. Path does not start with a '/' */
void parse_url(char *url, char **hostname, char **port, char **path)
{
    /* Find "://" in the url, set protocol to the start of url */
    char *p = strstr(url, "://");
    char *protocol = NULL;

    /* Set the end of the protocol to null */
    if(p){
        protocol = url;
        *p = '\0';
        p += 3;
    } else {
        p = url;
    }

    /* Don't handle non-http protocols */
    if(protocol && strcmp(protocol, "http")) {
        fprintf(stderr,
            "Unknown protocol '%s'. Only 'http' is supported\n",
            protocol);
        exit(1);
    }

    *hostname = p;
    while (*p && *p != ':' && *p != '/' && *p != '#') ++p;

    /* if port is present, use that, else 80 */
    *port = "80";
    if(*p == ':'){
        *p++ = '\0';
        *port = p;
    }

    while(*p && *p != '/' && *p != '#') ++p;
    *path = p;
    if(*p == '/') {
        /* path points to char after '/' */
        *path = p + 1;
    }

    /* adds null terminator to hostname */
    *p = '\0';

    /* remove hash from path */
    while (*p && *p != '#') ++p;
    if(*p == '#') *p = '\0';
}

/* Sends get request */
void send_request(SOCKET s, char *hostname, char *port, char *path)
{
    char buffer[2048];

    sprintf(buffer,
        "GET /%s HTTP/1.1\r\n"
        "Host: %s:%s\r\n"
        "User-Agent: honpwc web_get 1.0\r\n"
        "\r\n",
        path, hostname, port);
    
    send(s, buffer, strlen(buffer), 0);
    printf("Sent Headers:\n%s", buffer);
}

/*  takes string hostname and port,
    returns socket connected to hostname */
SOCKET connect_to_host(char *hostname, char *port)
{
    /* Get remote address */
    printf("Configuring remote address...\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo *peer_address;
    if(getaddrinfo(hostname, port, &hints, &peer_address)){
        fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
        exit(1);
    }

    /* Print remote address (not actually necessary) */
    char address_buffer[100];
    char service_buffer[100];
    getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen,
        address_buffer, sizeof(address_buffer),
        service_buffer, sizeof(service_buffer),
        NI_NUMERICHOST);
    printf("Remote address: %s %s\n", address_buffer, service_buffer);

    /* Make TCP connection to remote address */
    SOCKET server = socket(peer_address->ai_family,
        peer_address->ai_socktype, peer_address->ai_protocol);
    if(!ISVALIDSOCKET(server)){
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        exit(1);
    }

    printf("Connecting...\n");
    if(connect(server, peer_address->ai_addr, peer_address->ai_addrlen)){
        fprintf(stderr, "connect() failed. (%d)\n", GETSOCKETERRNO());
        exit(1);
    }

    /* Deallocation */
    freeaddrinfo(peer_address);
    printf("Connected.\n\n");

    return server;
}

int main(int argc, char *argv[])
{
    /* Windows setup */
#ifdef _WIN32
    WSADATA d;
    if(WSAStartup(MAKEWORD(2, 2), &d)){
        fprintf(stderr, "Failed to init.\n");
        return 1;
    }
#endif

    /* Verify inputs sort of */
    if(argc < 2){
        fprintf(stderr, "usage: web_get url\n");
        return 1;
    }

    /* Point hostname, port, and path in the right place */
    char *url = argv[1];
    char *hostname, *port, *path;
    parse_url(url, &hostname, &port, &path);

    /* Send get request to server */
    SOCKET server = connect_to_host(hostname, port);
    send_request(server, hostname, port, path);

    /* get start time so we can check for timeouts */
    const clock_t start_time = clock();

    /* Buffer for response */
    char response[MAX_RESPONSE_SIZE + 1];

    /* tracks current position in response */
    char *q, *p = response;

    /* Ensures we don't try to write past reserved memory */
    char *end = response + MAX_RESPONSE_SIZE;

    /* keeps track of the start of body */
    char *body = NULL;

    enum { length, chunked, connection };
    int encoding = 0;
    int remaining = 0;

    while(1){
        /* checks for timeout */
        if((clock() - start_time) / CLOCKS_PER_SEC > TIMEOUT){
            fprintf(stderr, "timeout after %.2f seconds\n", TIMEOUT);
            return 1;
        }

        /* check for overflow */
        if(p == end) {
            fprintf(stderr, "out of buffer space\n");
            return 1;
        }

        /* Get read-ready sockets, in fd_set reads */
        fd_set reads;
        FD_ZERO(&reads);
        FD_SET(server, &reads);

        /* Set timeout */
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 200000;

        if(select(server+1, &reads, NULL, NULL, &timeout) < 0){
            fprintf(stderr, "select() failed. (%d)\n", GETSOCKETERRNO());
            return 1;
        }
        
        /* Read from server, if it is ready. */
        if(FD_ISSET(server, &reads)){
            int bytes_recieved = recv(server, p, end - p, 0);
            if(bytes_recieved < 1){
                /* Print remaining body */
                if(encoding == connection && body){
                    printf("%.*s", (int)(end-body), body);
                }
                printf("\nConnection closed by peer.\n");
                break;
            }

            p += bytes_recieved;
            *p = '\0';

            /* Look for body after end of http header,
                (encoding is set in this block) */
            if(!body && (body = strstr(response, "\r\n\r\n"))){
                /* Add null at end of header, and point body
                    to start of http body */
                *body = '\0';
                body += 4;
                printf("Recieved Headers:\n%s\n", response);

                /* Check for content-length or transfer-
                    encoding headers, and set encoding
                    appropriately */
                q = strstr(response, "\nContent-Length: ");
                if(q){
                    encoding = length;
                    q = strchr(q, ' ');
                    q += 1;
                    remaining = strtol(q, NULL, 10);
                } else {
                    q = strstr(response, "\nTransfer-Encoding: chunked");
                    if(q){
                        encoding = chunked;
                        remaining = 0;
                    } else {
                        encoding = connection;
                    }
                }

                printf("\nRecieved Body:\n");
            } // end if(!body && (body = strstr(response, "\r\n\r\n")))

            if(body){
                if(encoding == length){
                    if(p - body >= remaining){
                        printf("%.*s", remaining, body);
                        break;
                    }
                } else if (encoding == chunked) {
                    do {
                        if(remaining == 0) {
                            if((q = strstr(body, "\r\n"))) {
                                remaining = strtol(body, NULL, 16);
                                if(!remaining) goto finish;
                                body = q + 2;
                            } else {
                                break;
                            }
                        }
                        if(remaining && p - body >= remaining ){
                            printf("%.*s", remaining, body);
                            body += remaining + 2;
                            remaining = 0;
                        }
                    } while (!remaining);
                }
            } // end if(body)
        } // end if(FD_ISSET(server, &reads))
    } // end while(1)
finish: // for easy loop breakout
    
    printf("\nClosing socket...\n");

#ifdef _WIN32
    WSACleanup();
#endif

    printf("finished.\n");
    return 0;
}