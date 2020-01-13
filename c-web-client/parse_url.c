#include "c_web_client.h"

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