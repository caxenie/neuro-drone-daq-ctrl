#include "udp_server.h"


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


int udp_server_bind(udp_server_t* udps)
{
    int ret;

    struct sockaddr_in server_addr;    //stucture for server address, port, etc.

    // get a socket descriptor
    if((udps->sd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        return UDP_SERVER_SOCK_ERR;
    }


    // init UDP server address struct
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(udps->server.ip);
    server_addr.sin_port = htons(udps->server.port);


    // bind socket to address and port
    ret = bind(udps->sd,(struct sockaddr *)&server_addr, sizeof(struct sockaddr));
    if(ret < 0)
    {
        close(udps->sd);
        return UDP_SERVER_BIND_ERR;
    }

    return UDP_SERVER_OK;
}

void udp_server_set_non_blocking_mode(udp_server_t* udps)
{
    // set nonblocking
    int flags = fcntl(udps->sd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(udps->sd, F_SETFL, flags);
}


void udp_server_set_blocking_mode(udp_server_t* udps)
{
    // set blocking
    int flags = fcntl(udps->sd, F_GETFL, 0);
    flags &= ~O_NONBLOCK;
    fcntl(udps->sd, F_SETFL, flags);
}


void udp_server_create_client_sock_addr(struct sockaddr_in* client_sock_addr, const char* client_ip, int client_port)
{
    // init upd client address struct
    memset(client_sock_addr, 0, sizeof(struct sockaddr_in));
    client_sock_addr->sin_family = AF_INET;
    client_sock_addr->sin_addr.s_addr = inet_addr(client_ip);
    client_sock_addr->sin_port = htons(client_port);
}


int udp_server_send(udp_server_t* udps, char* data, int bytes_to_write)
{
    int ret;
    struct sockaddr_in client_sock_addr;

    udp_server_create_client_sock_addr(&client_sock_addr,udps->client.ip,udps->client.port);

    ret = sendto(udps->sd,data, bytes_to_write, 0, (struct sockaddr*)&client_sock_addr, sizeof(struct sockaddr_in));
    if (ret < 0)
    {
        return UDP_SERVER_SEND_ERR;
    }

    return UDP_SERVER_OK;
}


int udp_server_recv(udp_server_t* udps, char* data, int bytes_to_read)
{
    int ret;
//    struct sockaddr_in client_sock_addr;

    //udp_server_create_client_sock_addr(&client_sock_addr,server->client_ip,server->client_port);
    //ret = recvfrom(sock, (void *)buf, bytesToRead, 0, (struct sockaddr *)&client_sock_addr, &fromlen);

    //it is not checked from what client we receive data !!!
    ret = recvfrom(udps->sd, (void *)data, bytes_to_read, 0, NULL, NULL);
    if (ret < 0)
    {
        //hard read error; when not timeout or would block error
        if ((errno != EAGAIN && errno != EWOULDBLOCK))
        {
            return UDP_SERVER_RECV_ERR;
        }
    }

    return ret;
}
