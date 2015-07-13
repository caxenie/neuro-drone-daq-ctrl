#ifndef CONFIG_H
#define CONFIG_H

#include "utils_types.h"
#include "tcp_client.h"
#include "udp_server.h"
#include "udp_multicast_recv.h"

#include "joystick.h"
#include "mavlink_conn_types.h"
#include "mavlink_logger.h"
#include "mavlink_utils.h"

#include "npoint_types.h"
#include "npoint_conn_types.h"
#include "npoint_logger.h"


#define PROG_NAME                                   "tflog"

// mav connection parameters (connection to px4 quadrotor)
//       (this program is a client which connects to the
//          refered server)
#define DEFAULT_MAV_CONN_TCP_SERVER_ADDR             "10.162.177.43"
#define DEFAULT_MAV_CONN_TCP_SERVER_PORT             56001

// qgroundcontroll connection server parameters
//      (server is provided by this program)
#define DEFAULT_QGC_CONN_UDP_SERVER_ADDR            "127.0.0.1"
#define DEFAULT_QGC_CONN_UDP_SERVER_PORT             5556

// qgroundcontrol connection client parameters
//      (client is qgroundcontrol)
#define DEFAULT_QGC_CONN_UDP_CLIENT_ADDR            "127.0.0.1"
#define DEFAULT_QGC_CONN_UDP_CLIENT_PORT            5555


#define DEFAULT_JS_DEVICE                           "/dev/input/js0"


#define DEFAULT_TRACKER_CONN_LOCAL_ADDR             "0.0.0.0"
#define DEFAULT_TRACKER_CONN_MULTICAST_ADDR         "239.255.42.99"
#define DEFAULT_TRACKER_CONN_LOCAL_DATA_PORT         1511
#define DEFAULT_TRACKER_CONN_MULTICAST_DATA_PORT     1511


#ifdef __cplusplus
extern "C" {
#endif


typedef struct
{
    tcp_client_t mav_conn;              // connection to mav (px4 quadrotor)
    udp_server_t qgc_conn;              // connection to QGroundcontrol software
    udp_mr_t tracker_conn;              // connection to npoint tracking system

    joystick_t js;

    ml_conn_t mlc_mav;                  // mavlink connection to mav (px4 quadrotor)
    ml_conn_t mlc_qgc;                  // mavlink connection to QGroundcontrol software

    np_conn_t npc_tracker;              // ... connection to npoint tracking system

    int joystick_available;
    int px4remote_available;
    int tracker_available;

    ml_logger_t mll;
    np_logger_t npl;

    const char* log_dir_path;

} config_t;

extern config_t config;                                 // defined in main.c
extern utils_config_file_param_t config_params[];       // defined in main.c

#ifdef __cplusplus
}
#endif


#endif // CONFIG_H
