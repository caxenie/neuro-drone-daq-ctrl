#ifndef UDP_MULTICAST_RECV_H
#define UDP_MULTICAST_RECV_H


#include <netinet/in.h>

#ifdef __cplusplus
extern "C" {
#endif




typedef enum
{
    UDP_MR_OK                           = 0,
    UDP_MR_SOCK_ERR                     = -1,
    UDP_MR_BIND_ERR                     = -2,
    UDP_MR_SEND_ERR                     = -3,
    UDP_MR_RECV_ERR                     = -4,

    UDP_MR_SETSOCKOPT_ERR               = -10,
    UDP_MR_JOIN_MULTICAST_GROUP_ERR     = -11

}udp_mr_error_t;


typedef struct
{
    const char*   ip;
    int           port;
}udp_mr_ep_t;


typedef struct
{
    const char* name;
    int sd;                            //socket descriptor

    udp_mr_ep_t multicast;
    udp_mr_ep_t local;

} udp_mr_t;


int udp_mr_bind_and_join(udp_mr_t* udp);

void udp_mr_set_non_blocking_mode(udp_mr_t* udp);
void udp_mr_set_blocking_mode(udp_mr_t* udp);

struct sockaddr_in* udp_mr_create_sockaddr(struct sockaddr_in* sockaddr, const char* ip, int port);
int udp_mr_join_multicast_group(int sd, struct ip_mreq* mg, const char* multicast_ip, const char* local_ip);

int udp_mr_recv(udp_mr_t* udp, char* data, int len);


#ifdef __cplusplus
}
#endif


#endif // UDP_MULTICAST_RECV_H
