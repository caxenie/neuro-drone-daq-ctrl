#include "tcp_client.h"

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


int tcp_client_connect(tcp_client_t* tcpc)
{
    int ret, error;

    socklen_t error_len = sizeof(int);

    struct sockaddr_in server_addr;    //stucture for server address, port, etc.

    // get a socket descriptor
    if((tcpc->sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        return TCP_CLIENT_SOCK_ERR;
    }

    // init TCP server address struct
    memset(&server_addr, 0x00, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(tcpc->server.port);
    server_addr.sin_addr.s_addr = inet_addr(tcpc->server.ip);

    // use timeout for connect
    struct timeval  timeout;
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;

    // set timeout
    fd_set set;
    FD_ZERO(&set);
    FD_SET(tcpc->sd, &set);


    // set non blocking mode
    tcp_client_set_non_blocking_mode(tcpc);

    printf("[%s] connect to tcp server ip = %s, port = %d ...\n",
           tcpc->name,
           tcpc->server.ip,
           tcpc->server.port);

    // connect to tcp server
    ret = connect(tcpc->sd,(struct sockaddr *) &server_addr,sizeof(struct sockaddr_in));
    if(ret < 0)
    {
        if(errno != EINPROGRESS)
        {
            printf("[%s] tcp-client connect error\n", tcpc->name);

            close(tcpc->sd);
            return TCP_CLIENT_CONN_ERR;
        }
    }

    // timeout blocking
    ret = select(tcpc->sd+1, NULL ,&set, NULL, &timeout);
    //ret = select(client->sd+1, &set ,NULL, NULL, &timeout);

    if (ret < 0)
    {
        printf("[%s] tcp-client select error\n", tcpc->name);

        // select error
        close(tcpc->sd);
        return TCP_CLIENT_SELECT_ERR;
    }

    // try to get sock option
    if(getsockopt(tcpc->sd, SOL_SOCKET, SO_ERROR, &error, &error_len) < 0)
    {
        printf("[%s] tcp-client getsockopt() error = %s\n", tcpc->name, strerror(errno));

        close(tcpc->sd);
        return TCP_CLIENT_GET_SOCKOPT_ERR;
    }

    // check if connection is good
    if(ret != 1 || error != 0)
    {
        printf("[%s] tcp-client connect timeout error\n", tcpc->name);

        close(tcpc->sd);
        return TCP_CLIENT_CONN_TIMEOUT_ERR;
    }

    tcp_client_set_blocking_mode(tcpc);
    return TCP_CLIENT_OK; //successfully connected
}


void tcp_client_set_non_blocking_mode(tcp_client_t* tcpc)
{
    // set nonblocking
    int flags = fcntl(tcpc->sd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(tcpc->sd, F_SETFL, flags);
}


void tcp_client_set_blocking_mode(tcp_client_t* tcpc)
{
    // set blocking
    int flags = fcntl(tcpc->sd, F_GETFL, 0);
    flags &= ~O_NONBLOCK;
    fcntl(tcpc->sd, F_SETFL, flags);
}


int tcp_client_write(tcp_client_t* tcpc, char* data, int bytes_to_write)
{
    int ret;

    //write packet to TCP socket
    ret = write(tcpc->sd,data,bytes_to_write);
    if (ret < 0)
    {
        return TCP_CLIENT_WRITE_ERR;
    }

    return TCP_CLIENT_OK;
}


int tcp_client_read(tcp_client_t* tcpc, char* data, int bytes_to_read)
{
    int ret;

    //write packet to TCP socket
    ret = read(tcpc->sd,data,bytes_to_read);
    if (ret < 0)
    {
        //hard read error; when not timeout or would block error
        if ((errno != EAGAIN && errno != EWOULDBLOCK))
        {
            return TCP_CLIENT_READ_ERR;
        }
    }

    return ret;
}
