#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

extern int errno; // From errno.h

int main(int argc, char ** argv){
    struct ifaddrs *addresses;

    // struct ifaddrs {
    //     struct ifaddrs  *ifa_next;    /* Next item in list */
    //     char            *ifa_name;    /* Name of interface */
    //     unsigned int     ifa_flags;   /* Flags from SIOCGIFFLAGS */
    //     struct sockaddr *ifa_addr;    /* Address of interface */
    //     struct sockaddr *ifa_netmask; /* Netmask of interface */
    //     union {
    //         struct sockaddr *ifu_broadaddr;
    //                         /* Broadcast address of interface */
    //         struct sockaddr *ifu_dstaddr;
    //                         /* Point-to-point destination address */
    //     } ifa_ifu;
    // #define              ifa_broadaddr ifa_ifu.ifu_broadaddr
    // #define              ifa_dstaddr   ifa_ifu.ifu_dstaddr
    //     void            *ifa_data;    /* Address-specific data */
    // };

    // getifaddrs: create linked list of network addresses
    if(getifaddrs(&addresses) == -1){
        perror("getifaddrs() failed\n");
        return -1;
    }
    struct ifaddrs *address = addresses;

    // walk through addresses
    while(address){
        int family = address->ifa_addr->sa_family;
        // ifa_addr is a sockaddr. NOT sockaddr_in (though the pointers can be cast interchangeably).

        if(family == AF_INET || family == AF_INET6){
            printf("%s\t", address->ifa_name);
            printf("%s\t", (family == AF_INET) ? "IPv4" : "IPv6");

            char ap[100];
            const size_t family_size = (family == AF_INET) ?
                sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
            // sockaddr_in6 is bigger than sockaddr_in

            // getnameinfo: converts socket address into corresponding host/service
            getnameinfo(address->ifa_addr, family_size, ap, sizeof(ap), 0, 0, NI_NUMERICHOST);
            
            //    int getnameinfo(const struct sockaddr *addr, socklen_t addrlen,
            //        char *host, socklen_t hostlen,
            //        char *serv, socklen_t servlen, int flags);

            printf("\t%s\n", ap);
        }
        address = address->ifa_next;
    }

    // DEALLOCATION
    freeifaddrs(addresses);
    return 0;
}