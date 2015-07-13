#include "mavlink_logger.h"

#include "utils.h"
#include "mavlink/v1.0/common/mavlink.h"


#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <inttypes.h>


int ml_logger_init(ml_logger_t* mll, const char* dir_path)
{
    mll->name = "std-mavlink-logger";


    utils_init_file(&mll->files.heartbeat,
                        "heartbeat",
                        "heartbeat.log",
                        dir_path);


    utils_init_file(&mll->files.sys_status,
                        "sys_status",
                        "sys_status.log",
                        dir_path);

    utils_init_file(&mll->files.attitude,
                        "attitude",
                        "attitude.log",
                        dir_path);

    utils_init_file(&mll->files.servo_output_raw,
                        "servo_output_raw",
                        "servo_output_raw.log",
                        dir_path);

    utils_init_file(&mll->files.roll_pitch_yaw_thrust_setpoint,
                        "roll_pitch_yaw_thrust_setpoint",
                        "roll_pitch_yaw_thrust_setpoint.log",
                        dir_path);

    utils_init_file(&mll->files.manual_control,
                        "manual_control",
                        "manual_control.log",
                        dir_path);


    utils_init_file(&mll->files.vfr_hud,
                        "vfr_hud",
                        "vfr_hud.log",
                        dir_path);

    utils_init_file(&mll->files.roll_pitch_yaw_rates_thrust_setpoint,
                        "roll_pitch_yaw_rates_thrust_setpoint",
                        "roll_pitch_yaw_rates_thrust_setpoint.log",
                        dir_path);

    utils_init_file(&mll->files.highres_imu,
                        "highres_imu",
                        "highres_imu.log",
                        dir_path);

    utils_init_file(&mll->files.statustext,
                        "status_text",
                        "status_text.log",
                        dir_path);

    utils_init_file(&mll->files.optical_flow,
                        "optical_flow",
                        "optical_flow.log",
                        dir_path);


    ml_logger_create_and_open_files(mll);


    return ML_LOGGER_OK;

}

void ml_logger_create_and_open_files(ml_logger_t* mll)
{
    int i = 0;

    int count = sizeof(mll->files)/sizeof(utils_file_t);

    utils_file_t* uf;

    for(i=0; i<count; i++)
    {
        uf = &mll->files.file[i];

        if(utils_create_and_open_file(uf) != UTILS_OK)
        {
            printf("[%s] opening file '%s' failed\n", mll->name, uf->file_path);
        }
    }

}

void ml_logger_close_files(ml_logger_t* mll)
{
    int i = 0;

    int count = sizeof(mll->files)/sizeof(utils_file_t);

    utils_file_t* uf;

    for(i=0; i<count; i++)
    {
        uf = &mll->files.file[i];
        utils_close_file(uf);
    }

}


void ml_logger_default_write_heartbeat(ml_logger_t* mll, mavlink_message_t* msg)
{
    /**

         uint32_t custom_mode; ///< A bitfield for use for autopilot-specific flags.
         uint8_t type; ///< Type of the MAV (quadrotor, helicopter, etc., up to 15 types, defined in MAV_TYPE ENUM)
         uint8_t autopilot; ///< Autopilot type / class. defined in MAV_AUTOPILOT ENUM
         uint8_t base_mode; ///< System mode bitfield, see MAV_MODE_FLAGS ENUM in mavlink/include/mavlink_types.h
         uint8_t system_status; ///< System status flag, see MAV_STATE ENUM
         uint8_t mavlink_version; ///< MAVLink version, not writable by user, gets added by protocol because of magic data type: uint8_t_mavlink_version

    **/


    if (msg->msgid == MAVLINK_MSG_ID_HEARTBEAT)
    {
        mavlink_heartbeat_t hb;
        mavlink_msg_heartbeat_decode(msg, &hb);

        fprintf(mll->files.heartbeat.fs, "%12" PRIu64 " %10d %3d %3d %3d %3d %3d\n",
               utils_us_since_epoch(),
               hb.custom_mode,
               hb.type,
               hb.autopilot,
               hb.base_mode,
               hb.system_status,
               hb.mavlink_version
                );
        fflush(mll->files.heartbeat.fs);
    }
}

void ml_logger_default_write_sys_status(ml_logger_t* mll, mavlink_message_t* msg)
{
    /**

        uint32_t onboard_control_sensors_present; ///< Bitmask showing which onboard controllers and sensors are present. Value of 0: not present. Value of 1: present. Indices: 0: 3D gyro, 1: 3D acc, 2: 3D mag, 3: absolute pressure, 4: differential pressure, 5: GPS, 6: optical flow, 7: computer vision position, 8: laser based position, 9: external ground-truth (Vicon or Leica). Controllers: 10: 3D angular rate control 11: attitude stabilization, 12: yaw position, 13: z/altitude control, 14: x/y position control, 15: motor outputs / control
        uint32_t onboard_control_sensors_enabled; ///< Bitmask showing which onboard controllers and sensors are enabled:  Value of 0: not enabled. Value of 1: enabled. Indices: 0: 3D gyro, 1: 3D acc, 2: 3D mag, 3: absolute pressure, 4: differential pressure, 5: GPS, 6: optical flow, 7: computer vision position, 8: laser based position, 9: external ground-truth (Vicon or Leica). Controllers: 10: 3D angular rate control 11: attitude stabilization, 12: yaw position, 13: z/altitude control, 14: x/y position control, 15: motor outputs / control
        uint32_t onboard_control_sensors_health; ///< Bitmask showing which onboard controllers and sensors are operational or have an error:  Value of 0: not enabled. Value of 1: enabled. Indices: 0: 3D gyro, 1: 3D acc, 2: 3D mag, 3: absolute pressure, 4: differential pressure, 5: GPS, 6: optical flow, 7: computer vision position, 8: laser based position, 9: external ground-truth (Vicon or Leica). Controllers: 10: 3D angular rate control 11: attitude stabilization, 12: yaw position, 13: z/altitude control, 14: x/y position control, 15: motor outputs / control
        uint16_t load; ///< Maximum usage in percent of the mainloop time, (0%: 0, 100%: 1000) should be always below 1000
        uint16_t voltage_battery; ///< Battery voltage, in millivolts (1 = 1 millivolt)
        int16_t current_battery; ///< Battery current, in 10*milliamperes (1 = 10 milliampere), -1: autopilot does not measure the current
        uint16_t drop_rate_comm; ///< Communication drops in percent, (0%: 0, 100%: 10'000), (UART, I2C, SPI, CAN), dropped packets on all links (packets that were corrupted on reception on the MAV)
        uint16_t errors_comm; ///< Communication errors (UART, I2C, SPI, CAN), dropped packets on all links (packets that were corrupted on reception on the MAV)
        uint16_t errors_count1; ///< Autopilot-specific errors
        uint16_t errors_count2; ///< Autopilot-specific errors
        uint16_t errors_count3; ///< Autopilot-specific errors
        uint16_t errors_count4; ///< Autopilot-specific errors
        int8_t battery_remaining; ///< Remaining battery energy: (0%: 0, 100%: 100), -1: autopilot estimate the remaining battery

    **/


    if (msg->msgid == MAVLINK_MSG_ID_SYS_STATUS)
    {
        mavlink_sys_status_t stat;
        mavlink_msg_sys_status_decode(msg, &stat);

        fprintf(mll->files.sys_status.fs, "%12" PRIu64 " %10d %10d %10d %5d %5d %5d %5d %5d %5d %5d %5d %5d %3d\n",
               utils_us_since_epoch(),
               stat.onboard_control_sensors_present,
               stat.onboard_control_sensors_enabled,
               stat.onboard_control_sensors_health,
               stat.load,
               stat.voltage_battery,
               stat.current_battery,
               stat.drop_rate_comm,
               stat.errors_comm,
               stat.errors_count1,
               stat.errors_count2,
               stat.errors_count3,
               stat.errors_count4,
               stat.battery_remaining
                );
        fflush(mll->files.sys_status.fs);
    }
}

void ml_logger_default_write_attitude(ml_logger_t* mll, mavlink_message_t* msg)
{
    /**

        uint32_t time_boot_ms; ///< Timestamp (milliseconds since system boot)
        float roll; ///< Roll angle (rad, -pi..+pi)
        float pitch; ///< Pitch angle (rad, -pi..+pi)
        float yaw; ///< Yaw angle (rad, -pi..+pi)
        float rollspeed; ///< Roll angular speed (rad/s)
        float pitchspeed; ///< Pitch angular speed (rad/s)
        float yawspeed; ///< Yaw angular speed (rad/s)

    **/


    if (msg->msgid == MAVLINK_MSG_ID_ATTITUDE)
    {
        mavlink_attitude_t att;
        mavlink_msg_attitude_decode(msg, &att);

        fprintf(mll->files.attitude.fs, "%12" PRIu64 " %10d %9.6f %9.6f %9.6f %9.6f %9.6f %9.6f\n",
               utils_us_since_epoch(),
               att.time_boot_ms,
               att.roll,
               att.pitch,
               att.yaw,
               att.rollspeed,
               att.pitchspeed,
               att.yawspeed
                );
        fflush(mll->files.attitude.fs);
    }
}


void ml_logger_default_write_servo_output_raw(ml_logger_t* mll, mavlink_message_t* msg)
{
    /**

        uint32_t time_usec; ///< Timestamp (microseconds since system boot)
        uint16_t servo1_raw; ///< Servo output 1 value, in microseconds
        uint16_t servo2_raw; ///< Servo output 2 value, in microseconds
        uint16_t servo3_raw; ///< Servo output 3 value, in microseconds
        uint16_t servo4_raw; ///< Servo output 4 value, in microseconds
        uint16_t servo5_raw; ///< Servo output 5 value, in microseconds
        uint16_t servo6_raw; ///< Servo output 6 value, in microseconds
        uint16_t servo7_raw; ///< Servo output 7 value, in microseconds
        uint16_t servo8_raw; ///< Servo output 8 value, in microseconds
        uint8_t port; ///< Servo output port (set of 8 outputs = 1 port). Most MAVs will just use one, but this allows to encode more than 8 servos.

    **/


    if (msg->msgid == MAVLINK_MSG_ID_SERVO_OUTPUT_RAW)
    {
        mavlink_servo_output_raw_t sor;
        mavlink_msg_servo_output_raw_decode(msg, &sor);

        fprintf(mll->files.servo_output_raw.fs, "%12" PRIu64 " %10d %5d %5d %5d %5d %5d %5d %5d %5d %3d\n",
               utils_us_since_epoch(),
                sor.time_usec,
                sor.servo1_raw,
                sor.servo2_raw,
                sor.servo3_raw,
                sor.servo4_raw,
                sor.servo5_raw,
                sor.servo6_raw,
                sor.servo7_raw,
                sor.servo8_raw,
                sor.port
                );
        fflush(mll->files.servo_output_raw.fs);
    }
}


void ml_logger_default_write_roll_pitch_yaw_thrust_setpoint(ml_logger_t* mll, mavlink_message_t* msg)
{
    /**

        uint32_t time_boot_ms; ///< Timestamp in milliseconds since system boot
        float roll; ///< Desired roll angle in radians
        float pitch; ///< Desired pitch angle in radians
        float yaw; ///< Desired yaw angle in radians
        float thrust; ///< Collective thrust, normalized to 0 .. 1

    **/


    if (msg->msgid == MAVLINK_MSG_ID_ROLL_PITCH_YAW_THRUST_SETPOINT)
    {
        mavlink_roll_pitch_yaw_thrust_setpoint_t rpyts;
        mavlink_msg_roll_pitch_yaw_thrust_setpoint_decode(msg, &rpyts);

        fprintf(mll->files.roll_pitch_yaw_thrust_setpoint.fs, "%12" PRIu64 " %10d %9.6f %9.6f %9.6f %9.6f\n",
               utils_us_since_epoch(),
               rpyts.time_boot_ms,
                rpyts.roll,
                rpyts.pitch,
                rpyts.yaw,
                rpyts.thrust
                );
        fflush(mll->files.roll_pitch_yaw_thrust_setpoint.fs);
    }
}

void ml_logger_default_write_manual_control(ml_logger_t* mll, mavlink_message_t* msg)
{
    /**

        int16_t x; ///< X-axis, normalized to the range [-1000,1000]. A value of INT16_MAX indicates that this axis is invalid. Generally corresponds to forward(1000)-backward(-1000) movement on a joystick and the pitch of a vehicle.
        int16_t y; ///< Y-axis, normalized to the range [-1000,1000]. A value of INT16_MAX indicates that this axis is invalid. Generally corresponds to left(-1000)-right(1000) movement on a joystick and the roll of a vehicle.
        int16_t z; ///< Z-axis, normalized to the range [-1000,1000]. A value of INT16_MAX indicates that this axis is invalid. Generally corresponds to a separate slider movement with maximum being 1000 and minimum being -1000 on a joystick and the thrust of a vehicle.
        int16_t r; ///< R-axis, normalized to the range [-1000,1000]. A value of INT16_MAX indicates that this axis is invalid. Generally corresponds to a twisting of the joystick, with counter-clockwise being 1000 and clockwise being -1000, and the yaw of a vehicle.
        uint16_t buttons; ///< A bitfield corresponding to the joystick buttons' current state, 1 for pressed, 0 for released. The lowest bit corresponds to Button 1.
        uint8_t target; ///< The system to be controlled.

    **/
    /**
            x = roll        [-600,600]
            y = pitch       [-600,600]
            z = yaw         [-2000,2000]
            r = thrust      [0,1000]
    **/
    if (msg->msgid == MAVLINK_MSG_ID_MANUAL_CONTROL)
    {
        mavlink_manual_control_t mc;
        mavlink_msg_manual_control_decode(msg, &mc);

        fprintf(mll->files.manual_control.fs, "%12" PRIu64 " %5d %5d %5d %5d %5d %3d\n",
                utils_us_since_epoch(),
                mc.x,
                mc.y,
                mc.z,
                mc.r,
                mc.buttons,
                mc.target
                );
        fflush(mll->files.manual_control.fs);
    }
}

void ml_logger_default_write_vfr_hud(ml_logger_t* mll, mavlink_message_t* msg)
{
    /**

        float airspeed; ///< Current airspeed in m/s
        float groundspeed; ///< Current ground speed in m/s
        float alt; ///< Current altitude (MSL), in meters
        float climb; ///< Current climb rate in meters/second
        int16_t heading; ///< Current heading in degrees, in compass units (0..360, 0=north)
        uint16_t throttle; ///< Current throttle setting in integer percent, 0 to 100

    **/


    if (msg->msgid == MAVLINK_MSG_ID_VFR_HUD)
    {
        mavlink_vfr_hud_t vh;
        mavlink_msg_vfr_hud_decode(msg, &vh);

        fprintf(mll->files.vfr_hud.fs, "%12" PRIu64 " %9.6f %9.6f %9.6f %9.6f %5d %5d\n",
               utils_us_since_epoch(),
                vh.airspeed,
                vh.groundspeed,
                vh.alt,
                vh.climb,
                vh.heading,
                vh.throttle
                );
        fflush(mll->files.vfr_hud.fs);
    }
}

void ml_logger_default_write_roll_pitch_yaw_rates_thrust_setpoint(ml_logger_t* mll, mavlink_message_t* msg)
{
    /**

        uint32_t time_boot_ms; ///< Timestamp in milliseconds since system boot
        float roll_rate; ///< Desired roll rate in radians per second
        float pitch_rate; ///< Desired pitch rate in radians per second
        float yaw_rate; ///< Desired yaw rate in radians per second
        float thrust; ///< Collective thrust, normalized to 0 .. 1

    **/


    if (msg->msgid == MAVLINK_MSG_ID_ROLL_PITCH_YAW_RATES_THRUST_SETPOINT)
    {
        mavlink_roll_pitch_yaw_rates_thrust_setpoint_t rpyrts;
        mavlink_msg_roll_pitch_yaw_rates_thrust_setpoint_decode(msg, &rpyrts);

        fprintf(mll->files.roll_pitch_yaw_rates_thrust_setpoint.fs, "%12" PRIu64 " %10d %9.6f %9.6f %9.6f %9.6f\n",
               utils_us_since_epoch(),
                rpyrts.time_boot_ms,
                rpyrts.roll_rate,
                rpyrts.pitch_rate,
                rpyrts.yaw_rate,
                rpyrts.thrust
                );
        fflush(mll->files.roll_pitch_yaw_rates_thrust_setpoint.fs);
    }
}

void ml_logger_default_write_highres_imu(ml_logger_t* mll, mavlink_message_t* msg)
{
    /**

         uint64_t time_usec; ///< Timestamp (microseconds, synced to UNIX time or since system boot)
         float xacc; ///< X acceleration (m/s^2)
         float yacc; ///< Y acceleration (m/s^2)
         float zacc; ///< Z acceleration (m/s^2)
         float xgyro; ///< Angular speed around X axis (rad / sec)
         float ygyro; ///< Angular speed around Y axis (rad / sec)
         float zgyro; ///< Angular speed around Z axis (rad / sec)
         float xmag; ///< X Magnetic field (Gauss)
         float ymag; ///< Y Magnetic field (Gauss)
         float zmag; ///< Z Magnetic field (Gauss)
         float abs_pressure; ///< Absolute pressure in millibar
         float diff_pressure; ///< Differential pressure in millibar
         float pressure_alt; ///< Altitude calculated from pressure
         float temperature; ///< Temperature in degrees celsius

    **/


    if (msg->msgid == MAVLINK_MSG_ID_HIGHRES_IMU)
    {
        mavlink_highres_imu_t imu;
        mavlink_msg_highres_imu_decode(msg, &imu);


        fprintf(mll->files.highres_imu.fs, "%12" PRIu64 " %12" PRIu64 " %9.6f %9.6f %9.6f %9.6f %9.6f %9.6f %9.6f %9.6f %9.6f %9.6f %9.6f\n",
               utils_us_since_epoch(),
               imu.time_usec,
               imu.xacc,
               imu.yacc,
               imu.zacc,
               imu.xgyro,
               imu.ygyro,
               imu.zgyro,
               imu.xmag,
               imu.ymag,
               imu.zmag,
               imu.abs_pressure,
               imu.pressure_alt
                );
        fflush(mll->files.highres_imu.fs);
    }
}

void ml_logger_default_write_statustext(ml_logger_t* mll, mavlink_message_t* msg)
{
    /**
        uint8_t severity; ///< Severity of status. Relies on the definitions within RFC-5424. See enum MAV_SEVERITY.
        char text[50]; ///< Status text message, without null termination character
    **/


    if (msg->msgid == MAVLINK_MSG_ID_STATUSTEXT)
    {
        mavlink_statustext_t st;
        mavlink_msg_statustext_decode(msg, &st);

        char text[51];
        memcpy(text,st.text,50);
        text[51] = '\0';

        fprintf(mll->files.statustext.fs, "%12" PRIu64 " %3d %s\n",
               utils_us_since_epoch(),
                st.severity,
                text
                );
        fflush(mll->files.statustext.fs);
    }
}

void ml_logger_default_write_optical_flow(ml_logger_t* mll, mavlink_message_t* msg)
{
    /**
         uint64_t time_usec; ///< Timestamp (UNIX)
         float flow_comp_m_x; ///< Flow in meters in x-sensor direction, angular-speed compensated
         float flow_comp_m_y; ///< Flow in meters in y-sensor direction, angular-speed compensated
         float ground_distance; ///< Ground distance in meters. Positive value: distance known. Negative value: Unknown distance
         int16_t flow_x; ///< Flow in pixels in x-sensor direction
         int16_t flow_y; ///< Flow in pixels in y-sensor direction
         uint8_t sensor_id; ///< Sensor ID
         uint8_t quality; ///< Optical flow quality / confidence. 0: bad, 255: maximum quality
    **/

    if (msg->msgid == MAVLINK_MSG_ID_OPTICAL_FLOW)
    {
        mavlink_optical_flow_t of;
        mavlink_msg_optical_flow_decode(msg, &of);

        fprintf(mll->files.optical_flow.fs, "%12" PRIu64 " %12" PRIu64 " %9.6f %9.6f %9.6f %5d %5d %3d %3d\n",
                utils_us_since_epoch(),
                of.time_usec,
                of.flow_comp_m_x,
                of.flow_comp_m_y,
                of.ground_distance,
                of.flow_x,
                of.flow_y,
                of.sensor_id,
                of.quality
                );
        fflush(mll->files.optical_flow.fs);
    }
}

