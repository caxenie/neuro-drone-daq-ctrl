#ifndef UDP_SERVER_H
#define UDP_SERVER_H


#include <netinet/in.h>

#ifdef __cplusplus
extern "C" {
#endif



typedef enum
{
    UDP_SERVER_OK           = 0,
    UDP_SERVER_SOCK_ERR     = -1,
    UDP_SERVER_BIND_ERR     = -2,
    UDP_SERVER_SEND_ERR     = -3,
    UDP_SERVER_RECV_ERR     = -4

}udp_server_error_t;


typedef struct
{
    const char*   ip;
    int           port;
}udp_server_ep_t;


typedef struct
{
    const char* name;
    int sd;

    udp_server_ep_t server;
    udp_server_ep_t client;

} udp_server_t;


int udp_server_bind(udp_server_t* udps);

void udp_server_set_non_blocking_mode(udp_server_t* udps);
void udp_server_set_blocking_mode(udp_server_t* udps);

void udp_server_create_client_sock_addr(struct sockaddr_in* client_sock_addr, const char *client_ip, int client_port);

int udp_server_send(udp_server_t* udps, char* data, int bytes_to_write);
int udp_server_recv(udp_server_t* udps, char* data, int bytes_to_read);

#ifdef __cplusplus
}
#endif

#endif // UDP_SERVER_H
