#ifndef MAVLINK_CONN_TYPES_H
#define MAVLINK_CONN_TYPES_H

#include "utils_types.h"
#include "mavlink/v1.0/mavlink_types.h"

#include <stdint.h>
#include <inttypes.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef enum
{
    ML_CONN_OK                                   = 0,

    ML_CONN_FULL_HANDLER_LIST_ERR               = UTILS_FULL_LIST_ERR,
    ML_CONN_FUNC_ALREADY_IN_HANDLER_LIST_ERR    = UTILS_ALREADY_IN_LIST_ERR,
    ML_CONN_FUNC_NOT_IN_HANDLER_LIST_ERR        = UTILS_NOT_IN_LIST_ERR,


    ML_CONN_RECV_THREAD_ALREADY_EXISTS_ERR      = -100,
    ML_CONN_RECV_THREAD_DOESNT_EXIST_ERR        = -101

} ml_conn_error_t;

typedef void (*ml_conn_msg_handler_func_t)(mavlink_message_t* msg);
typedef int (*ml_conn_send_func_t)(char* buf, int len); //read
typedef int (*ml_conn_recv_func_t)(char* buf, int len); //write


typedef struct
{
    uint8_t base_mode;
    uint32_t custom_mode;
    int is_armed;
}ml_conn_mav_mode_t;        // mode of the MAV, updated through
                            //      heartbeat message

typedef struct
{
    uint8_t system;         // system id of the endpoint
    uint8_t component;      // system component id of the endpoint;
}ml_conn_endpoint_t;


typedef struct
{
    unsigned int tcount;        // total count
    unsigned int lcount;        // last count
    uint64_t ts;                // last timestamp
}ml_conn_status_entry_t;


typedef struct
{
    ml_conn_status_entry_t bytes;
    ml_conn_status_entry_t packets;
}ml_conn_link_status;

typedef struct
{
    ml_conn_link_status rx;
    ml_conn_link_status tx;
}ml_conn_status_t;

typedef struct
{
    const char* name;

    // buffers for the modified mavlink package parsing function
    mavlink_message_t msg_buf;
    mavlink_status_t status_buf;

    // if heartbeat measage is received from the target (MAV)
    //      then the system status of the target is stored to this
    //      struct
    ml_conn_mav_mode_t target;

    utils_handler_list_t handler_list;

    ml_conn_msg_handler_func_t msg_handler;

    // receiver thread
    pthread_t pth_recv;

    // determines if the thread is running and if it should continue
    int recv_running;

    // this function is called if there is a mavlink package to send
    //      if no function (NULL pointer) is defined: no data is sent
    ml_conn_send_func_t send;

    // to make send operation thread save only ONE thread must call
    //      the send function
    //      so try to lock this mutex before send function is called
    pthread_mutex_t send_mutex;

    // this function is called if the receiver thread awaits data
    //      this function pointer must be set! Error otherwise
    ml_conn_recv_func_t recv;

    // rx/tx status info
    ml_conn_status_t status;

} ml_conn_t;

typedef struct
{
//    ml_conn_t* mlcml_conn_rxtx_stat_t
    mavlink_message_t* msg;
} ml_conn_handler_args_t;


#ifdef __cplusplus
}
#endif


#endif // MAVLINK_CONN_TYPES_H
