#include "udp_link.h"


#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>



int udp_link_bind(udp_link_t* udp)
{
    int ret;

    struct sockaddr_in local_sockaddr;    //stucture for server address, port, etc.

    // get a socket descriptor
    if((udp->sd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        return UDP_LINK_SOCK_ERR;
    }


    // create sock address for local endpoint
    udp_link_create_sockaddr(&local_sockaddr,udp->local.ip, udp->local.port);


    // bind socket to address and port
    ret = bind(udp->sd, (struct sockaddr *)&local_sockaddr, sizeof(struct sockaddr));
    if(ret < 0)
    {
        close(udp->sd);
        return UDP_LINK_BIND_ERR;
    }

    return UDP_LINK_OK;
}

void udp_link_set_non_blocking_mode(udp_link_t* udp)
{
    // set nonblocking
    int flags = fcntl(udp->sd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(udp->sd, F_SETFL, flags);
}


void udp_link_set_blocking_mode(udp_link_t* udp)
{
    // set blocking
    int flags = fcntl(udp->sd, F_GETFL, 0);
    flags &= ~O_NONBLOCK;
    fcntl(udp->sd, F_SETFL, flags);
}

struct sockaddr_in* udp_link_create_sockaddr(struct sockaddr_in* sockaddr, const char* ip, int port)
{
    // init struct
    // INADDR_ANY = 0x00000000 = "0.0.0.0"
    memset(sockaddr, 0, sizeof(struct sockaddr_in));
    sockaddr->sin_family = AF_INET;
    sockaddr->sin_addr.s_addr = inet_addr(ip);
    sockaddr->sin_port = htons(port);

    return sockaddr;
}


int udp_link_sendto(udp_link_t* udp, char* data, int len)
{
    int ret;
    struct sockaddr_in target_sockaddr;

    udp_link_create_sockaddr(&target_sockaddr,udp->target.ip,udp->target.port);

    ret = sendto(udp->sd,data, len, 0, (struct sockaddr*)&target_sockaddr, sizeof(target_sockaddr));
    if (ret < 0)
    {
        return UDP_LINK_SEND_ERR;
    }

    return UDP_LINK_OK;
}


int udp_link_recvfrom(udp_link_t* udp, char* data, int len)
{
    int ret;
    struct sockaddr_in target_sockaddr;
    socklen_t target_socklen;

    //it is not checked from what target we receive data !!!
    ret = recvfrom(udp->sd, (void *)data, len, 0,(struct sockaddr*)&target_sockaddr, &target_socklen);
    if (ret < 0)
    {
        //hard read error; when not timeout or would block error
        if ((errno != EAGAIN && errno != EWOULDBLOCK))
        {
            return UDP_LINK_RECV_ERR;
        }
    }

    return ret;
}
