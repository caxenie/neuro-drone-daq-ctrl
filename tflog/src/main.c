#include "./config.h"

#include "utils_types.h"
#include "utils.h"

#include "mavlink_logger.h"
#include "npoint_logger.h"

#include "mavlink_conn.h"
#include "npoint_conn.h"

#include "joystick.h"

#include "uio_manager.h"


#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>

#include <limits.h>

#include <pthread.h>



config_t config;


utils_config_file_param_t config_params[] =
{
    {"DEFAULT_MAV_CONN_TCP_SERVER_ADDR",  &config.mav_conn.server.ip,   "string"},
    {"DEFAULT_MAV_CONN_TCP_SERVER_PORT",  &config.mav_conn.server.port, "int"},

    {"DEFAULT_QGC_CONN_UDP_SERVER_ADDR",  &config.qgc_conn.server.ip,   "string"},
    {"DEFAULT_QGC_CONN_UDP_SERVER_PORT",  &config.qgc_conn.server.port, "int"},
    {"DEFAULT_QGC_CONN_UDP_CLIENT_ADDR",  &config.qgc_conn.client.ip,   "string"},
    {"DEFAULT_QGC_CONN_UDP_CLIENT_PORT",  &config.qgc_conn.client.port, "int"},

    {"DEFAULT_JS_DEVICE",                 &config.js.dev,               "string"},

    {"DEFAULT_TRACKER_CONN_LOCAL_ADDR",             &config.tracker_conn.local.ip ,     "string"},
    {"DEFAULT_TRACKER_CONN_LOCAL_DATA_PORT",        &config.tracker_conn.local.port,    "int"},

    {"DEFAULT_TRACKER_CONN_MULTICAST_ADDR",         &config.tracker_conn.multicast.ip,  "string"},
    {"DEFAULT_TRACKER_CONN_MULTICAST_DATA_PORT",    &config.tracker_conn.multicast.port,"int"},

    UTILS_CONFIG_FILE_PARAM_END,
};


// connection to MAV (PX4 quadrotor)
int mlc_mav_recv_func(char* buf, int len);  // read
int mlc_mav_send_func(char* buf, int len);  // write

void* mlc_mav_msg_handler(void* arg);
void* mlc_mav_log_msg_handler(void* arg);


// connection to QGroundcontrol
int mlc_qgc_recv_func(char* buf, int len);  // read
int mlc_qgc_send_func(char* buf, int len);  // write

void* mlc_qgc_msg_handler(void* arg);
void* mlc_qgc_log_msg_handler(void* arg);


// connection to tracking system
int npc_tracker_recv_func(char* buf, int len);  // read

void *npc_tracker_mocap_data_log_handler(void* arg);
void *npc_tracker_rigid_body_log_handler(void* arg);

// create and fill the manual_control mavlink message
mavlink_message_t* create_mavlink_msg_manual_control(mavlink_message_t* msg, js_data_t* jsd);

// reads joystick data and sends mavlink message and cmd to MAV
int read_and_handle_js_data();


int main(int argc, char* argv[])
{
    int ret,i;

    uint64_t start, start2, stop;

    char date_time_str[64];
    char dir_path[1024];

    // build logger dir path string
    utils_get_date_time_str(date_time_str);

    strcpy(dir_path,"log-");
    strcat(dir_path,date_time_str);

    config.log_dir_path = utils_alloc_str(dir_path);

    // create directory
    utils_make_dir(dir_path);

    // standard config: is overwritten by config file
    config.mav_conn.server.ip               = DEFAULT_MAV_CONN_TCP_SERVER_ADDR;
    config.mav_conn.server.port             = DEFAULT_MAV_CONN_TCP_SERVER_PORT;

    // this program
    config.qgc_conn.server.ip               = DEFAULT_QGC_CONN_UDP_SERVER_ADDR;
    config.qgc_conn.server.port             = DEFAULT_QGC_CONN_UDP_SERVER_PORT;

    // QGroundcontrol software
    config.qgc_conn.client.ip               = DEFAULT_QGC_CONN_UDP_CLIENT_ADDR;
    config.qgc_conn.client.port             = DEFAULT_QGC_CONN_UDP_CLIENT_PORT;

    // Joystick device
    config.js.dev                           = DEFAULT_JS_DEVICE;

    // npoint tracking system data connection
    config.tracker_conn.local.ip            = DEFAULT_TRACKER_CONN_LOCAL_ADDR;
    config.tracker_conn.local.port          = DEFAULT_TRACKER_CONN_LOCAL_DATA_PORT;
    config.tracker_conn.multicast.ip        = DEFAULT_TRACKER_CONN_MULTICAST_ADDR;
    config.tracker_conn.multicast.port      = DEFAULT_TRACKER_CONN_MULTICAST_DATA_PORT;


    // assume the best ...
    config.joystick_available       = true;
    config.px4remote_available      = true;
    config.tracker_available        = true;

    // try to read config file and override default values
    utils_read_config_file(config_params,"config.cfg");


    // init the user input/output manager
    uiom_init_sig_handler();

    uiom_parse_args(argc,argv,&config);
    uiom_print_config(stdout,&config);


    /*********************************************************************************************

                initialize the modules

    *********************************************************************************************/

    // create and load joystick calibration
    js_calibration_t calib;
    config.js.calib = js_std_calib(&calib);


    // finish connection inits (config already loaded at this step)
    config.qgc_conn.name        = "qgc-conn";
    config.mav_conn.name        = "mav-conn";
    config.tracker_conn.name    = "tracker-conn";

    // init mavlink logger
    if(ml_logger_init(&config.mll,dir_path) != ML_LOGGER_OK)
    {
        printf("[%s] Mavlink logger init failed.\n", PROG_NAME);
        exit(EXIT_FAILURE);
    }
    config.mll.name = "mav-log";

    // init npoint logger
    np_logger_init(&config.npl,dir_path);
    config.npl.name = "tracker-log";


    // init the mav mavlink connection
    ml_conn_init(&config.mlc_mav);
    config.mlc_mav.name = "mlc_mav";
    config.mlc_mav.recv = mlc_mav_recv_func;
    config.mlc_mav.send = mlc_mav_send_func;

    ml_conn_add_handler(&config.mlc_mav,mlc_mav_msg_handler);
    ml_conn_add_handler(&config.mlc_mav,mlc_mav_log_msg_handler);


    // init the qgc mavlink connection
    ml_conn_init(&config.mlc_qgc);
    config.mlc_qgc.name = "mlc_qgc";
    config.mlc_qgc.recv = mlc_qgc_recv_func;
    config.mlc_qgc.send = mlc_qgc_send_func;

    ml_conn_add_handler(&config.mlc_qgc,mlc_qgc_msg_handler);
    // ml_conn_add_handler(&config.mlc_qgc,mlc_qgc_log_msg_handler);


    // init the npoint connection
    np_conn_init(&config.npc_tracker);
    config.npc_tracker.name = "npc_tracker";
    config.npc_tracker.send = NULL;
    config.npc_tracker.recv = npc_tracker_recv_func;

    np_conn_add_mocap_data_handler(&config.npc_tracker,npc_tracker_mocap_data_log_handler);
    np_conn_add_rigid_body_handler(&config.npc_tracker,npc_tracker_rigid_body_log_handler);

    // np_conn_add_marker_set_handler(&config.npc_tracker,np_conn_std_marker_set_handler);
    // np_conn_add_rigid_body_handler(&config.npc_tracker,np_conn_std_rigid_body_handler);



    /*********************************************************************************************

                try to build up connections

    *********************************************************************************************/

    //

    // joystick connection
    ret = js_open(&config.js);
    if(ret != JS_OK)
    {
        printf("[%s] Could not open joystick '%s'.\n", PROG_NAME, config.js.dev);
        config.joystick_available = false;
    }
    else
    {
        printf("[%s] joystick '%s' is opened.\n",PROG_NAME, config.js.dev);
    }

    // tcp connection to mav (px4 quadrotor)
    ret = tcp_client_connect(&config.mav_conn);
    if(ret != TCP_CLIENT_OK)
    {
        printf("[%s] error while conneting to MAV (px4 quadrotor) tcp server...\n",PROG_NAME);
        config.px4remote_available = false;
    }
    else
    {
        printf("[%s] connnection to MAV (px4 quadrotor) tcp server established\n",PROG_NAME);
        tcp_client_set_non_blocking_mode(&config.mav_conn);
    }

    // udp server for qgroundcontrol
    if(config.px4remote_available == true)
    {
        ret = udp_server_bind(&config.qgc_conn);
        if(ret != UDP_SERVER_OK)
        {
            printf("[%s] error while trying to bind udp server for qgroundcontrol connection.\n",PROG_NAME);
            exit(EXIT_FAILURE);
        }

        udp_server_set_non_blocking_mode(&config.qgc_conn);

        printf("[%s] UDP server successfully bound for qgroundcontrol connection\n",PROG_NAME);
    }

    // udp broad cast connection connection to npoint tracker
    ret = udp_mr_bind_and_join(&config.tracker_conn);
    if(ret != UDP_MR_OK)
    {
        printf("[%s] error (%d) while creating udp multicast data receiver...\n",PROG_NAME,ret);
        config.tracker_available = false;
    }
    else
    {
        printf("[%s] udp multicast data receiver link established.\n",PROG_NAME);
        udp_mr_set_non_blocking_mode(&config.tracker_conn);
    }



    /*********************************************************************************************

                start modules

    *********************************************************************************************/

    if(config.px4remote_available == true)
    {
        ml_conn_start_receiver(&config.mlc_mav);
        ml_conn_start_receiver(&config.mlc_qgc);
    }

    if(config.tracker_available == true)
    {
        np_conn_start_packet_receiver(&config.npc_tracker);
    }


    start = utils_us_since_epoch();
    start2 = start;

    while(uiom_is_prog_running())
    {
        if(uiom_is_exit_key_pressed())
        {
            break;
        }

        usleep(1000);

        stop = utils_us_since_epoch();


        if(config.joystick_available && config.px4remote_available)
        {
            if(read_and_handle_js_data() == -1)
            {
                break;
            }

        }


        if(config.px4remote_available == true)
        {
            // 5 sec
            if((stop-start) > 5000000)
            {
                double rate;

                printf("[%s] status:\n",config.mlc_mav.name);

                rate = ml_conn_get_rx_byte_rate(&config.mlc_mav);
                printf("    rx: %4.1f kBits/s", rate/1024.0f*8.0f);

                rate = ml_conn_get_tx_byte_rate(&config.mlc_mav);
                printf(", tx: %4.1f kBits/s\n", rate/1024.0f*8.0f);

                printf("\n");
                fflush(stdout);

                start = stop;
            }
        }

        if(config.tracker_available == true)
        {
            // 5 sec
            if((stop-start2) > 5000000)
            {
                double rate;

                printf("[%s] status:\n",config.npc_tracker.name);

                rate = np_conn_get_rx_byte_rate(&config.npc_tracker);
                printf("    rx: %4.1f kBits/s, ", rate/1024.0f*8.0f);

                rate = np_conn_get_rx_packet_rate(&config.npc_tracker);
                printf("%4.1f Hz\n", rate);

                printf("\n");
                fflush(stdout);

                start2 = stop;
            }
        }

    }


    ml_conn_stop_receiver(&config.mlc_mav);
    ml_conn_stop_receiver(&config.mlc_qgc);
    np_conn_stop_packet_receiver(&config.npc_tracker);

    uiom_cleanup(&config);

    exit(EXIT_SUCCESS);
}




int read_and_handle_js_data()
{
    int ret;
    uint64_t stop;

    mavlink_message_t msg;

    static js_data_t jsd;
    static uint64_t start = utils_us_since_epoch();
    static uint64_t start2 = start;

    int toggle_arming_flag = 0;


    stop = utils_us_since_epoch();



    // 1 ms
    if((stop - start) > 1000)
    {
        // read joystick data
        ret = js_update(&config.js, &jsd);
        if(ret == JS_HARD_FAULT)
        {
            printf("[%s] error while reading joystick...\n",PROG_NAME);
            return -1;
        }

        start = stop;
    }

    // 10 ms
    if((stop - start2) > 10000)
    {
        //        js_fprint_data(stdout, &jsd);
        //        fflush(stdout);

        if(jsd.bstates & JS_BUTTON1_PRESSED)
        {
            mlu_set_initial_target_mode(&config.mlc_mav);
            //printf("set initial target mode\n");
            //fflush(stdout);
        }

        if(jsd.bstates & JS_BUTTON2_PRESSED && toggle_arming_flag == 0)
        {
            mlu_toggle_target_arming(&config.mlc_mav);
            //printf("toggle arming\n");
            //fflush(stdout);
            toggle_arming_flag = 1;
        }

        if(jsd.bstates & JS_BUTTON2_RELEASED)
        {
            toggle_arming_flag = 0;
        }

        // create manual control mavlink message
        create_mavlink_msg_manual_control(&msg, &jsd);

        // send mavlink message
        ml_conn_send_mavlink_message(&config.mlc_mav,&msg);

        start2 = stop;
    }

    return 0;
}









// mutex implemented in mavlink_conn and mavlink_utils modules
int mlc_mav_send_func(char* buf, int len)
{
    int ret;

    ret = tcp_client_write(&config.mav_conn,buf,len);
    if(ret != TCP_CLIENT_OK)
    {
        printf("[%s] send MAVLINK packet to MAV failed...\n", PROG_NAME);
    }

    return ret;
}


int mlc_mav_recv_func(char* buf, int len)
{
    int ret;

    ret = tcp_client_read(&config.mav_conn,buf,len);
    if(ret == TCP_CLIENT_READ_ERR)
    {
        printf("[%s] receive MAVLINK packet from MAV failed!\n",  PROG_NAME);
        printf("[%s] read error: %s\n", config.mav_conn.name, strerror(errno));

        printf("[%s] stop programm...\n", PROG_NAME);
        uiom_stop_running();

        // give thread some time to join
        usleep(100000); //100 ms
        exit(EXIT_SUCCESS);
    }

    if(ret < 0)
    {
        return -1;
    }

    return ret;
}

// forward message to QGroundcontrol software
void* mlc_mav_msg_handler(void* arg)
{
    ml_conn_handler_args_t* args = ML_CONN_GET_HANDLER_ARGS(arg);
    mavlink_message_t* msg = args->msg;

    ml_conn_send_mavlink_message(&config.mlc_qgc,msg);

    return NULL;
}

// write message to log file
void* mlc_mav_log_msg_handler(void* arg)
{
    ml_conn_handler_args_t* args = ML_CONN_GET_HANDLER_ARGS(arg);
    mavlink_message_t* msg = args->msg;

    ml_logger_default_write_heartbeat(&config.mll,msg);
    ml_logger_default_write_sys_status(&config.mll,msg);

    ml_logger_default_write_attitude(&config.mll,msg);
    ml_logger_default_write_servo_output_raw(&config.mll,msg);
    ml_logger_default_write_roll_pitch_yaw_thrust_setpoint(&config.mll,msg);
    ml_logger_default_write_manual_control(&config.mll,msg);
    ml_logger_default_write_vfr_hud(&config.mll,msg);

    ml_logger_default_write_roll_pitch_yaw_rates_thrust_setpoint(&config.mll,msg);
    ml_logger_default_write_highres_imu(&config.mll,msg);

    ml_logger_default_write_statustext(&config.mll,msg);

    ml_logger_default_write_optical_flow(&config.mll,msg);

    return NULL;
}

// mutex implemented in mavlink_conn and mavlink_utils modules
int mlc_qgc_send_func(char* buf, int len)
{
    int ret;

    ret = udp_server_send(&config.qgc_conn,buf,len);
    if(ret != UDP_SERVER_OK)
    {
        printf("[%s] send MAVLINK packet to QGroundcontrol failed...\n", PROG_NAME);
    }

    return ret;
}


int mlc_qgc_recv_func(char* buf, int len)
{
    int ret = udp_server_recv(&config.qgc_conn,buf,len);

    if(ret == UDP_SERVER_SEND_ERR)
    {
        printf("[%s] receive MAVLINK packet from QGroundcontrol failed!\n", PROG_NAME);
        printf("[%s] read error: %s\n", config.qgc_conn.name, strerror(errno));

        printf("[%s] stop programm...\n", PROG_NAME);
        uiom_stop_running();

        // give thread some time to join
        usleep(100000); //100 ms
        exit(EXIT_SUCCESS);
    }

    if(ret < 0)
    {
        return -1;
    }

    return ret;
}

// forward message to MAV (px4 quadrotor)
void* mlc_qgc_msg_handler(void* arg)
{
    ml_conn_handler_args_t* args = ML_CONN_GET_HANDLER_ARGS(arg);
    mavlink_message_t* msg = args->msg;

    ml_conn_send_mavlink_message(&config.mlc_mav, msg);

    return NULL;
}

// dummy; nothing to log so far
void* mlc_qgc_log_msg_handler(void* arg)
{
    ml_conn_handler_args_t* args = ML_CONN_GET_HANDLER_ARGS(arg);
    mavlink_message_t* msg = args->msg;

    return NULL;
}

int npc_tracker_recv_func(char* buf, int len)
{
    int ret;

    ret = udp_mr_recv(&config.tracker_conn,buf,len);
    if(ret == UDP_MR_RECV_ERR)
    {
        printf("[%s] receive NPOINT tracker packet failed!\n", PROG_NAME);
        printf("[%s] read error: %s\n", config.tracker_conn.name, strerror(errno));

        printf("[%s] stop programm...\n", PROG_NAME);
        uiom_stop_running();

        // give thread some time to join
        usleep(100000); //100 ms
        exit(EXIT_SUCCESS);
    }

    if(ret < 0)
    {
        return -1;
    }

    return ret;
}

void *npc_tracker_mocap_data_log_handler(void* arg)
{
    npoint_mocap_data_t* md = NP_CONN_GET_MOCAP_DATA_HANDLER_ARGS(arg)->md;

    // open files only once
    if(config.npl.files_opened == false)
    {
        printf("[%s] create and open npoint tracker log files ...\n", PROG_NAME);
        np_logger_create_and_open_files(&config.npl,md);
    }
}

void *npc_tracker_rigid_body_log_handler(void* arg)
{
    npoint_mocap_data_t* md = NP_CONN_GET_RIGID_BODY_HANDLER_ARGS(arg)->md;
    npoint_rigid_body_t* rb = NP_CONN_GET_RIGID_BODY_HANDLER_ARGS(arg)->rb;

    np_logger_std_write_rigidBody(&config.npl,md,rb);
}

// create and fill the manual_control mavlink message
mavlink_message_t* create_mavlink_msg_manual_control(mavlink_message_t* msg, js_data_t* jsd)
{
    mavlink_manual_control_t mc;

    // get the conversion results for each channel using a calibration setting
    double roll     = js_chn_convert(&config.js.calib->roll,    jsd->roll);
    double pitch    = js_chn_convert(&config.js.calib->pitch,   jsd->pitch);
    double yaw      = js_chn_convert(&config.js.calib->yaw,     jsd->yaw);
    double throttle = js_chn_convert(&config.js.calib->throttle,jsd->throttle);

    if(throttle < 0.0)
    {
        throttle = 0.0;
    }

    mc.x = roll     * 1000;     // js.calib: range -0.3 ... 0.3
    mc.y = pitch    * 1000;     // js.calib: range -0.3 ... 0.3
    mc.z = yaw      * 1000;     // js.calib: range -2.0 ... 2.0
    mc.r = throttle * 1000;     // js.calib: range  0.0 ... 1.0

    mavlink_msg_manual_control_encode(ml_conn_std_remote_ep.system,
                                      ml_conn_std_remote_ep.component,
                                      msg,
                                      &mc);

    return msg;
}


