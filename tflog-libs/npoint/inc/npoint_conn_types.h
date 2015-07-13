#ifndef NPOINT_CONN_TYPES_H
#define NPOINT_CONN_TYPES_H


#include "npoint_types.h"
#include "utils_types.h"

#include <stdbool.h>
#include <pthread.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    NP_CONN_OK   = 0,

    NP_CONN_FULL_HANDLER_LIST_ERR               = UTILS_FULL_LIST_ERR,
    NP_CONN_FUNC_ALREADY_IN_HANDLER_LIST_ERR    = UTILS_ALREADY_IN_LIST_ERR,
    NP_CONN_FUNC_NOT_IN_HANDLER_LIST_ERR        = UTILS_NOT_IN_LIST_ERR,


    NP_CONN_RECV_THREAD_ALREADY_EXISTS_ERR      = -100,
    NP_CONN_RECV_THREAD_DOESNT_EXIST_ERR        = -101

} np_conn_error_t;


typedef struct
{
    npoint_mocap_data_t* md;
} np_conn_mocap_data_handler_args_t;

typedef struct
{
    npoint_mocap_data_t* md;
    npoint_marker_set_t* ms;
} np_conn_marker_set_handler_args_t;


typedef struct
{
    npoint_mocap_data_t* md;
    npoint_rigid_body_t* rb;
} np_conn_rigid_body_handler_args_t;


typedef void (*np_conn_mocap_data_handler_func_t)(npoint_mocap_data_t* md);
typedef void (*np_conn_marker_set_handler_func_t)(npoint_mocap_data_t* md, npoint_marker_set_t* ms);
typedef void (*np_conn_rigid_body_handler_func_t)(npoint_mocap_data_t* md, npoint_rigid_body_t* rb);

typedef int (*np_conn_send_func_t)(char* buf, int len); //read
typedef int (*np_conn_recv_func_t)(char* buf, int len); //write


typedef struct
{
    unsigned int tcount;        // total count
    unsigned int lcount;        // last count
    uint64_t ts;                // last timestamp
}np_conn_status_entry_t;


typedef struct
{
    np_conn_status_entry_t bytes;
    np_conn_status_entry_t packets;
}np_conn_link_status;

typedef struct
{
    np_conn_link_status rx;
    np_conn_link_status tx;
}np_conn_status_t;


typedef struct
{
    const char* name;

    //receiver thread
    pthread_t pth_recv;

    //determines if the thread is running and if it should continue
    bool recv_running;


    utils_handler_list_t mocap_data_handler_list;
    utils_handler_list_t marker_set_handler_list;
    utils_handler_list_t rigid_body_handler_list;

    //list of handler functions; handler functions are called if message was received
    np_conn_mocap_data_handler_func_t mocap_data_handler;

    // is called when a marker set was parsed
    np_conn_marker_set_handler_func_t marker_set_handler;

    // is called when a rigid body was parsed
    np_conn_rigid_body_handler_func_t rigid_body_handler;


    //this function is called if there is a package to send
    //if no function (NULL pointer) is defined: no data is sent
    np_conn_send_func_t send;

    //this function is called if the receiver thread awaits data
    //this function pointer must be set! Error otherwise
    np_conn_recv_func_t recv;

    np_conn_status_t status;


} np_conn_t;

#ifdef __cplusplus
}
#endif

#endif // NPOINT_CONN_TYPES_H
