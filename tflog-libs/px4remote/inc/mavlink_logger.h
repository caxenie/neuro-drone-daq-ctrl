#ifndef LOGGER_H
#define LOGGER_H

#include "utils_types.h"
#include "mavlink/v1.0/mavlink_types.h"

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef enum
{
    ML_LOGGER_OK              =  0,
    ML_LOGGER_HARD_FAULT      = -1

} ml_logger_error_t;



typedef struct
{
    const char* name;

    union _list
    {
        struct
        {
            utils_file_t heartbeat;
            utils_file_t sys_status;
            utils_file_t attitude;
            utils_file_t servo_output_raw;
            utils_file_t roll_pitch_yaw_thrust_setpoint;
            utils_file_t manual_control;
            utils_file_t vfr_hud;
            utils_file_t roll_pitch_yaw_rates_thrust_setpoint;
            utils_file_t highres_imu;
            utils_file_t statustext;
            utils_file_t optical_flow;
        };
        utils_file_t file[11];
    }files;
} ml_logger_t;


int ml_logger_init(ml_logger_t* mll, const char* dir_path);
void ml_logger_create_and_open_files(ml_logger_t* mll);
void ml_logger_close_files(ml_logger_t* mll);


void ml_logger_default_write_heartbeat(ml_logger_t* mll, mavlink_message_t* msg);
void ml_logger_default_write_sys_status(ml_logger_t* mll, mavlink_message_t* msg);
void ml_logger_default_write_attitude(ml_logger_t* mll, mavlink_message_t* msg);
void ml_logger_default_write_servo_output_raw(ml_logger_t* mll, mavlink_message_t* msg);
void ml_logger_default_write_roll_pitch_yaw_thrust_setpoint(ml_logger_t* mll, mavlink_message_t* msg);
void ml_logger_default_write_manual_control(ml_logger_t* mll, mavlink_message_t* msg);
void ml_logger_default_write_vfr_hud(ml_logger_t* mll, mavlink_message_t* msg);
void ml_logger_default_write_roll_pitch_yaw_rates_thrust_setpoint(ml_logger_t* mll, mavlink_message_t* msg);
void ml_logger_default_write_highres_imu(ml_logger_t* mll, mavlink_message_t* msg);
void ml_logger_default_write_statustext(ml_logger_t* mll, mavlink_message_t* msg);
void ml_logger_default_write_optical_flow(ml_logger_t* mll, mavlink_message_t* msg);

#ifdef __cplusplus
}
#endif

#endif // LOGGER_H
