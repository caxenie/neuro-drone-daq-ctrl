#ifndef UDP_LINK_H
#define UDP_LINK_H

#include <netinet/in.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UDP_LINK_OK           0
#define UDP_LINK_SOCK_ERR     -1
#define UDP_LINK_BIND_ERR     -2
#define UDP_LINK_SEND_ERR    -3
#define UDP_LINK_RECV_ERR     -4

typedef struct
{
    const char*   ip;
    int           port;
}upd_link_ep_t;

typedef struct
{
    const char* name;
    int sd;                            //socket descriptor

    upd_link_ep_t target;
    upd_link_ep_t local;

} udp_link_t;



int udp_link_bind(udp_link_t* udp);

void udp_link_set_non_blocking_mode(udp_link_t* udp);
void udp_link_set_blocking_mode(udp_link_t* udp);

struct sockaddr_in* udp_link_create_sockaddr(struct sockaddr_in* sockaddr, const char* ip, int port);

int udp_link_sendto(udp_link_t* udp, char* data, int len);
int udp_link_recvfrom(udp_link_t* udp, char* data, int len);


#ifdef __cplusplus
}
#endif



#endif // UDP_LINK_H
