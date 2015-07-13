#ifndef NPOINT_CONN_H
#define NPOINT_CONN_H

#include "npoint_types.h"
#include "utils_types.h"
#include "npoint_conn_types.h"

#include <stdbool.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NP_CONN_MAX_PACKET_LEN              20000
#define NP_CONN_HANDLER_LIST_LEN            32


#define NP_CONN_GET_MOCAP_DATA_HANDLER_ARGS(arg)       ((np_conn_mocap_data_handler_args_t*) arg)
#define NP_CONN_GET_MARKER_SET_HANDLER_ARGS(arg)       ((np_conn_marker_set_handler_args_t*) arg)
#define NP_CONN_GET_RIGID_BODY_HANDLER_ARGS(arg)       ((np_conn_rigid_body_handler_args_t*) arg)
#define NP_CONN_TO_HANDLER_ARG(arg)                    ((void*) arg)


// function to init with standard settings
void np_conn_init(np_conn_t* np);
void np_conn_deinit(np_conn_t* np);


// handler list related support functions

// add/remove handler
int np_conn_add_mocap_data_handler(np_conn_t* npc, utils_generic_handler_func_t func);
int np_conn_add_marker_set_handler(np_conn_t* npc, utils_generic_handler_func_t func);
int np_conn_add_rigid_body_handler(np_conn_t* npc, utils_generic_handler_func_t func);

int np_conn_rm_mocap_data_handler(np_conn_t* npc, utils_generic_handler_func_t func);
int np_conn_rm_marker_set_handler(np_conn_t* npc, utils_generic_handler_func_t func);
int np_conn_rm_rigid_body_handler(np_conn_t* npc, utils_generic_handler_func_t func);


// receiver thread related functions
int np_conn_start_packet_receiver(np_conn_t* npc);
int np_conn_stop_packet_receiver(np_conn_t* npc);


// some std handler functions
void* np_conn_std_mocap_data_handler(void* arg);
void* np_conn_std_marker_set_handler(void* arg);
void* np_conn_std_rigid_body_handler(void* arg);


// tx/rx rate in bytes/s
//      don't call this function to often
//      some bytes should have been sent/received since last call
double np_conn_get_rx_byte_rate(np_conn_t* npc);

// packet rate in Hz
double np_conn_get_rx_packet_rate(np_conn_t* npc);

#ifdef __cplusplus
}
#endif


#endif // NPOINT_CONN_H
