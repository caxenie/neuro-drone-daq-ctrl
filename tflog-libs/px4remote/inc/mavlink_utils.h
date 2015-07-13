#ifndef MAVLINK_UTILS_H
#define MAVLINK_UTILS_H

#include "mavlink_conn_types.h"
#include "mavlink/v1.0/common/mavlink.h"


#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif


// some function to change the mode of the connected target (drone for example)

// remote control, system is disarmed
void mlu_set_initial_target_mode(ml_conn_t* mlc);

void mlu_arm_target(ml_conn_t* mlc);
void mlu_disarm_target(ml_conn_t* mlc);
void mlu_toggle_target_arming(ml_conn_t* mlc);

void mlu_set_target_mode(ml_conn_t* mlc, uint8_t base_mode, uint32_t custom_mode);


void* mlu_default_heartbeat_handler(void* arg);
void mlu_default_system_status_handler(void* arg);
void mlu_default_highres_imu_handler(void* arg);


const char* mlu_get_mavlink_message_name(mavlink_message_t* msg);

const char* mlu_get_mav_mode_flag_name(int mav_mode_flag);
const char* mlu_get_mav_mode_name(int mav_mode);
const char* mlu_get_mav_state_name(int mav_state);
const char* mlu_get_mav_cmd_name(int mav_cmd);

#ifdef __cplusplus
}
#endif

#endif // MAVLINK_UTILS_Hs
