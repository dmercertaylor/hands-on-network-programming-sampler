#include "c_web_client.h"

void parse_url(char *url, char **hostname, char **port, char **path);

int main(int argc, char *argv[]){
    char url[] = "http://www.google.com/login";
    char *hostname, *port, *path;
    parse_url(url, &hostname, &port, &path);
    printf("%s, %s, %s\n", hostname, port, path);
    return 0;
}