#include "udp_multicast_recv.h"

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

// http://publib.boulder.ibm.com/infocenter/iseries/v5r3/index.jsp?topic=%2Frzab6%2Frzab6x2multicast.htm
// http://www.linux.com/learn/docs/ldp/Multicast-HOWTO#ss1.1


int udp_mr_bind_and_join(udp_mr_t* udp)
{
    int ret;

    struct sockaddr_in local_sockaddr;    // structure for server address, port, etc.
    struct ip_mreq        group;          // structure for the mulitcast group

    // get a socket descriptor
    if((udp->sd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
    {
        return UDP_MR_SOCK_ERR;
    }

    // enable SO_REUSEADDR
    // allow multiple clients on same machine to use address/port
    int reuse=1;

    if (setsockopt(udp->sd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0)
    {
        close(udp->sd);
        return UDP_MR_SETSOCKOPT_ERR;
    }


    // create sock address for local endpoint
    udp_mr_create_sockaddr(&local_sockaddr,udp->local.ip, udp->local.port);


    // bind socket to address and port
    ret = bind(udp->sd,(struct sockaddr *)&local_sockaddr, sizeof(struct sockaddr));
    if(ret < 0)
    {
        close(udp->sd);
        return UDP_MR_BIND_ERR;
    }


    //join the multicast group
    ret = udp_mr_join_multicast_group(udp->sd,&group,udp->multicast.ip,udp->local.ip);
    if(ret < 0)
    {
        close(udp->sd);
        return UDP_MR_JOIN_MULTICAST_GROUP_ERR;
    }

    return UDP_MR_OK;

}

void udp_mr_set_non_blocking_mode(udp_mr_t* udp)
{
    // set nonblocking
    int flags = fcntl(udp->sd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(udp->sd, F_SETFL, flags);
}


void udp_mr_set_blocking_mode(udp_mr_t* udp)
{
    // set blocking
    int flags = fcntl(udp->sd, F_GETFL, 0);
    flags &= ~O_NONBLOCK;
    fcntl(udp->sd, F_SETFL, flags);
}

struct sockaddr_in* udp_mr_create_sockaddr(struct sockaddr_in* sockaddr, const char* ip, int port)
{
    // init struct
    // INADDR_ANY = 0x00000000 = "0.0.0.0"
    memset(sockaddr, 0, sizeof(struct sockaddr_in));
    sockaddr->sin_family = AF_INET;
    sockaddr->sin_addr.s_addr = inet_addr(ip);
    sockaddr->sin_port = htons(port);

    return sockaddr;
}

int udp_mr_join_multicast_group(int sd, struct ip_mreq* mg, const char* multicast_ip, const char* local_ip)
{
    mg->imr_multiaddr.s_addr = inet_addr(multicast_ip);
    mg->imr_interface.s_addr = inet_addr(local_ip);

    if (setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP,mg, sizeof(struct ip_mreq)) < 0)
    {
        printf("join multicast group error: %s\n", strerror(errno));
        close(sd);
        return UDP_MR_SETSOCKOPT_ERR;
    }

    return UDP_MR_OK;
}

int udp_mr_recv(udp_mr_t* udp, char* data, int len)
{
    int ret;

    ret = read(udp->sd,data,len);
    if (ret < 0)
    {
        //hard read error; when not timeout or would block error
        if ((errno != EAGAIN && errno != EWOULDBLOCK))
        {
            return UDP_MR_RECV_ERR;
        }
    }

    return ret;
}



