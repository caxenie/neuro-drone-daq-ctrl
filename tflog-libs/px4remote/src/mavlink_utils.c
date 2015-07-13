
#include "mavlink_utils.h"

#include "utils_types.h"
#include "mavlink_conn_types.h"
#include "mavlink_conn.h"

#include "utils.h"


#include "mavlink/v1.0/common/mavlink.h"


#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>




void mlu_set_initial_target_mode(ml_conn_t* mlc)
{
    printf("[%s] set initial target mode\n",mlc->name);

    uint8_t base_mode = MAV_MODE_FLAG_MANUAL_INPUT_ENABLED |
                                MAV_MODE_FLAG_STABILIZE_ENABLED |
                                MAV_MODE_FLAG_CUSTOM_MODE_ENABLED |
                                MAV_MODE_FLAG_SAFETY_ARMED;

    // uint8_t base_mode = MAV_MODE_FLAG_MANUAL_INPUT_ENABLED;

    mlu_set_target_mode(mlc,
                        base_mode & ~MAV_MODE_FLAG_SAFETY_ARMED,
                        mlc->target.custom_mode);
}

void mlu_arm_target(ml_conn_t* mlc)
{
    if(mlc->target.is_armed)
    {
        return;
    }

    printf("[%s] arming the target ...\n", mlc->name);

    mlu_set_target_mode(mlc,
                        mlc->target.base_mode | MAV_MODE_FLAG_SAFETY_ARMED,
                        mlc->target.custom_mode);
}

void mlu_disarm_target(ml_conn_t* mlc)
{
    printf("[%s] disarming the target ...\n", mlc->name);

    mlu_set_target_mode(mlc, mlc->target.base_mode & ~MAV_MODE_FLAG_SAFETY_ARMED,
                        mlc->target.custom_mode);
}


void mlu_toggle_target_arming(ml_conn_t* mlc)
{
    if (mlc->target.is_armed)
    {
        mlu_disarm_target(mlc);
    }
    else
    {
        mlu_arm_target(mlc);
    }
}

void mlu_set_target_mode(ml_conn_t* mlc, uint8_t base_mode, uint32_t custom_mode)
{
    char buf[MAVLINK_MAX_PACKET_LEN];
    mavlink_message_t msg;
    int len;

    mavlink_msg_set_mode_pack(ml_conn_std_remote_ep.system,
                              ml_conn_std_remote_ep.component,
                              &msg,
                              ml_conn_std_mav_ep.system,
                              base_mode,
                              custom_mode);

    //len = mavlink_msg_to_send_buffer((uint8_t*)buf, &msg);

    //thread save send
    ml_conn_send_mavlink_message(mlc,&msg);

//    if(mlc->send != NULL)
//    {
//        pthread_mutex_lock(&mlc->send_mutex);
//        mlc->send(buf, len);
//        pthread_mutex_unlock(&mlc->send_mutex);

//        printf("mlu: set mode msg sent\n");
//    }
}

void* mlu_default_heartbeat_handler(void* arg)
{
    ml_conn_handler_args_t* args = ML_CONN_GET_HANDLER_ARGS(arg);
    mavlink_message_t* msg = args->msg;

    if(msg->msgid == MAVLINK_MSG_ID_HEARTBEAT)
    {
        mavlink_heartbeat_t hb;
        mavlink_msg_heartbeat_decode(msg, &hb);

        printf("HEARTBEAT %d\n", msg->msgid);
        printf("    Base Mode: %u Custom Mode: %u\n", hb.base_mode, hb.custom_mode);

        printf("\n");
        printf("\n");
        fflush(stdout);
    }
}

void mlu_default_system_status_handler(void* arg)
{
    ml_conn_handler_args_t* args = ML_CONN_GET_HANDLER_ARGS(arg);
    mavlink_message_t* msg = args->msg;

    if(msg->msgid == MAVLINK_MSG_ID_SYS_STATUS)
    {
        mavlink_sys_status_t sys;
        mavlink_msg_sys_status_decode(msg, &sys);

        printf("SYS_STATUS %d\n", msg->msgid);
        printf("    Battery Voltage:            %u [mV] (%d %% remaining)\n", sys.voltage_battery, sys.battery_remaining);
        printf("    Communication drop rate:    %u [%%] \n", sys.drop_rate_comm);
        printf("    Communication errors:       %u [count] \n", sys.errors_comm);

        printf("\n");
        printf("\n");
        fflush(stdout);
    }
}


void mlu_default_highres_imu_handler(void* arg)
{
    ml_conn_handler_args_t* args = ML_CONN_GET_HANDLER_ARGS(arg);

    mavlink_message_t* msg = args->msg;

    if (msg->msgid == MAVLINK_MSG_ID_HIGHRES_IMU)
    {
        mavlink_highres_imu_t imu;
        mavlink_msg_highres_imu_decode(msg, &imu);

        printf("HIGHRES_IMU %d\n", msg->msgid);
        printf("    %12ld %9.4f %9.4f %9.4f %9.4f %9.4f %9.4f %9.4f %9.4f %9.4f\n",
               utils_us_since_epoch(),
               imu.xacc,
               imu.yacc,
               imu.zacc,
               imu.xgyro,
               imu.ygyro,
               imu.zgyro,
               imu.xmag,
               imu.ymag,
               imu.zmag);

        printf("\n");
        printf("\n");
        fflush(stdout);
    }
}


const char* mlu_get_mavlink_message_name(mavlink_message_t* msg)
{

    switch(msg->msgid)
    {
    case MAVLINK_MSG_ID_COMMAND_LONG:
        return "COMMAND_LONG";

    case MAVLINK_MSG_ID_HEARTBEAT:
        return "HEARTBEAT";

    case MAVLINK_MSG_ID_COMMAND_ACK:
        return "COMMAND_ACK";

    case MAVLINK_MSG_ID_PING:
        return "PING";

    case MAVLINK_MSG_ID_PARAM_REQUEST_READ:
        return "PARAM_REQUEST_READ";

    case MAVLINK_MSG_ID_PARAM_VALUE:
        return "PARAM_VALUE";

    case MAVLINK_MSG_ID_PARAM_REQUEST_LIST:
        return "PARAM_REQUEST_LIST";

    case MAVLINK_MSG_ID_PARAM_SET:
        return "PARAM_SET";

    case MAVLINK_MSG_ID_MISSION_REQUEST_LIST:
        return "MISSION_REQUEST_LIST";

    default:
        return NULL;

    }

}


const char* mlu_get_mav_mode_flag_name(int mav_mode_flag)
{

    switch(mav_mode_flag)
    {
    case MAV_MODE_FLAG_CUSTOM_MODE_ENABLED:
        return "CUSTOM_MODE_ENABLED";

    case MAV_MODE_FLAG_TEST_ENABLED:
        return "TEST_ENABLED";

    case MAV_MODE_FLAG_AUTO_ENABLED:
        return "AUTO_ENABLED";

    case MAV_MODE_FLAG_GUIDED_ENABLED:
        return "GUIDED_ENABLED";

    case MAV_MODE_FLAG_STABILIZE_ENABLED:
        return "STABILIZE_ENABLED";

    case MAV_MODE_FLAG_HIL_ENABLED:
        return "HIL_ENABLED";

    case MAV_MODE_FLAG_MANUAL_INPUT_ENABLED:
        return "MANUAL_INPUT_ENABLED";

    case MAV_MODE_FLAG_SAFETY_ARMED:
        return "FLAG_SAFETY_ARMED";
    default:
        return NULL;

    }
}


const char* mlu_get_mav_mode_name(int mav_mode)
{

    switch(mav_mode)
    {
    case MAV_MODE_PREFLIGHT:
        return "PREFLIGHT";

    case MAV_MODE_MANUAL_DISARMED:
        return "MANUAL_DISARMED";

    case MAV_MODE_TEST_DISARMED:
        return "TEST_DISARMED";

    case MAV_MODE_STABILIZE_DISARMED:
        return "STABILIZE_DISARMED";

    case MAV_MODE_GUIDED_DISARMED:
        return "GUIDED_DISARMED";

    case MAV_MODE_AUTO_DISARMED:
        return "AUTO_DISARMED";

    case MAV_MODE_MANUAL_ARMED:
        return "MANUAL_ARMED";

    case MAV_MODE_TEST_ARMED:
        return "TEST_ARMED";

    case MAV_MODE_STABILIZE_ARMED:
        return "STABILIZE_ARMED";

    case MAV_MODE_GUIDED_ARMED:
        return "GUIDED_ARMED";

    case MAV_MODE_AUTO_ARMED:
        return "AUTO_ARMED";

    default:
        return NULL;

    }
}


const char* mlu_get_mav_state_name(int mav_state)
{

    switch(mav_state)
    {
    case MAV_STATE_UNINIT:
        return "UNINIT";

    case MAV_STATE_BOOT:
        return "BOOT";

    case MAV_STATE_CALIBRATING:
        return "CALIBRATING";

    case MAV_STATE_STANDBY:
        return "STANDBY";

    case MAV_STATE_ACTIVE:
        return "ACTIVE";

    case MAV_STATE_CRITICAL:
        return "CRITICAL";

    case MAV_STATE_EMERGENCY:
        return "EMERGENCY";

    case MAV_STATE_POWEROFF:
        return "POWEROFF";

    default:
        return NULL;

    }
}

const char* mlu_get_mav_cmd_name(int mav_cmd)
{
    switch (mav_cmd)
    {
    case MAV_CMD_DO_SET_MODE:
        return "MAV_CMD_DO_SET_MODE";

    case MAV_CMD_DO_SET_PARAMETER:
        return "MAV_CMD_DO_SET_PARAMETER";

    case MAV_CMD_PREFLIGHT_CALIBRATION:
        return "MAV_CMD_PREFLIGHT_CALIBRATION";

    case MAV_CMD_PREFLIGHT_SET_SENSOR_OFFSETS:
        return "MAV_CMD_PREFLIGHT_SET_SENSOR_OFFSETS";

    case MAV_CMD_PREFLIGHT_STORAGE:
        return "MAV_CMD_PREFLIGHT_STORAGE";

    case MAV_CMD_PREFLIGHT_REBOOT_SHUTDOWN:
        return "MAV_CMD_PREFLIGHT_REBOOT_SHUTDOWN";

    case MAV_CMD_COMPONENT_ARM_DISARM:
        return "MAV_CMD_COMPONENT_ARM_DISARM";


    default:
        return NULL;
    }

}
