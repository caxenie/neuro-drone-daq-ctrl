#ifndef MAVLINK_CONN_H
#define MAVLINK_CONN_H

#include "utils_types.h"
#include "mavlink/v1.0/mavlink_types.h"
#include "mavlink_conn_types.h"

#include <pthread.h>
#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif

#define ML_CONN_GET_HANDLER_ARGS(arg)       ((ml_conn_handler_args_t*) arg)
#define ML_CONN_TO_HANDLER_ARG(arg)        ((void*) arg)


extern const ml_conn_endpoint_t ml_conn_std_mav_ep;
extern const ml_conn_endpoint_t ml_conn_std_remote_ep;


void ml_conn_init(ml_conn_t* mlc);


int ml_conn_add_handler(ml_conn_t* mlc, utils_generic_handler_func_t func);
int ml_conn_rm_handler(ml_conn_t* mlc, utils_generic_handler_func_t func);

// thread save send
void ml_conn_send_mavlink_message(ml_conn_t* mlc, mavlink_message_t* msg);


int ml_conn_start_receiver(ml_conn_t* mlc);
int ml_conn_stop_receiver(ml_conn_t* mlc);

// tx/rx rate in bytes/s
//      don't call this function to often
//      some bytes should have been sent/received since last call
double ml_conn_get_tx_byte_rate(ml_conn_t* mlc);
double ml_conn_get_rx_byte_rate(ml_conn_t* mlc);

#ifdef __cplusplus
}
#endif



#endif // MAVLINK_CONN_H
