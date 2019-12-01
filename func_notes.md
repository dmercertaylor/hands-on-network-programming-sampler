# Common Socket functions
## socket()

    Creates and initializes a new socket.

## bind()

    Associates a socket with a particular local IP address and port number.

## listen()

    Used on the server to cause a TCP socket to listen for new connections.

## connect()

    Used on the client to set the remote address and port. In the case of TCP, it also establishes a connection.

## accept()

    Used on the server to create a new socket for an incoming TCP connection.

## send() and recv()

    Used to send and receive data with a socket.

## sendto() and recvfrom()

    Used to send and receive data from sockets without a bound remote address.

## close() (Berkeley sockets) and closesocket() (Winsock sockets) 

    Used to close a socket. In the case of TCP, this also terminates the connection.

## shutdown()

    Used to close one side of a TCP connection. It is useful to ensure an orderly connection teardown.

## select()

    Used to wait for an event on one or more sockets.

## getnameinfo() and getaddrinfo()

    Provide a protocol-independent manner of working with hostnames and addresses.

## setsockopt()

    Used to change some socket options.

## fcntl() (Berkeley sockets) and ioctlsocket() (Winsock sockets)

    Also used to get and set some socket options.