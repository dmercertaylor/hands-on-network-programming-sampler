#ifndef C_WEB_CLIENT_H
#define C_WEB_CLIENT_H

/*** WIN32 HEADERS ***/
#ifdef _WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <winsock2.h>
#include <ws2cpip.h>
#pragma comment(lib, "ws2_32.lib")

/*** POSIX HEADERS ***/
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#endif

/*** SYS CALL MACROS ***/

#ifdef _WIN32
#define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
#define CLOSESOCKET(s) closesocket(s)
#define GETSOCKETERRNO() (WSAGetLastError())

#else
#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)

#endif

/*** STD LIB HEADERS ***/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#endif