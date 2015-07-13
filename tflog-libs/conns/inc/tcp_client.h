#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H


#ifdef __cplusplus
extern "C" {
#endif



typedef enum
{
    TCP_CLIENT_OK                   = 0,
    TCP_CLIENT_SOCK_ERR             = -1,
    TCP_CLIENT_CONN_ERR             = -2,
    TCP_CLIENT_WRITE_ERR            = -3,
    TCP_CLIENT_READ_ERR             = -4,


    TCP_CLIENT_SELECT_ERR           = -10,
    TCP_CLIENT_GET_SOCKOPT_ERR      = -11,
    TCP_CLIENT_CONN_TIMEOUT_ERR     = -12

} tcp_client_error_t;


typedef struct
{
    const char*   ip;
    int           port;

}tcp_client_ep_t;

typedef struct
{
    const char* name;
    int sd;

    tcp_client_ep_t server;

} tcp_client_t;


int tcp_client_connect(tcp_client_t* tcpc);

void tcp_client_set_non_blocking_mode(tcp_client_t* tcpc);
void tcp_client_set_blocking_mode(tcp_client_t* tcpc);

int tcp_client_write(tcp_client_t* tcpc, char* data, int bytes_to_write);
int tcp_client_read(tcp_client_t* tcpc, char* data, int bytes_to_read);

#ifdef __cplusplus
}
#endif

#endif // TCP_CLIENT_H
