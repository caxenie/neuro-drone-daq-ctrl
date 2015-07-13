#ifndef JOYSTICK_H
#define JOYSTICK_H


#include <stdio.h>
#include <stdint.h>
#include <unistd.h>



#ifdef __cplusplus
extern "C" {
#endif

#define JS_OK                0
#define JS_HARD_FAULT       -1
#define JS_TIMEOUT_ERR      -2


#define JS_BUTTON1_UNCHANGED (1 << 0)
#define JS_BUTTON1_PRESSED   (1 << 1)
#define JS_BUTTON1_RELEASED  (1 << 2)
#define JS_BUTTON2_UNCHANGED (1 << 3)
#define JS_BUTTON2_PRESSED   (1 << 4)
#define JS_BUTTON2_RELEASED  (1 << 5)


typedef enum
{
    YAW=0,
    THROTTLE,
    ROLL,
    LEFTBUT,
    RIGHTBUT,
    PITCH
} axis_index_t;




typedef struct
{
    int16_t roll;
    int16_t pitch;
    int16_t yaw;
    int16_t throttle;
    int16_t bLeft;
    int16_t bRight;
    int bstates;             //state of the buttons
}js_data_t;


typedef struct
{
    double scale;
    int rev;
    int16_t min;
    int16_t max;
    int16_t trim;
    int16_t dz;
}js_chn_t;

typedef struct
{
    js_chn_t roll;
    js_chn_t pitch;
    js_chn_t yaw;
    js_chn_t throttle;
}js_calibration_t;


typedef struct
{
    const char* dev;                        //path to joystick device file
    int fd;                                 //file descriptor of joystick device

    const js_calibration_t* calib;         //joystick calibration settings

} joystick_t;

// open a joystick device
int js_open(joystick_t *js);

// loades the standard joystick configuration into the struct
js_calibration_t* js_std_calib(js_calibration_t* calib);

// tries to read joystick raw data and updates joystick data
int js_update(joystick_t* js, js_data_t* js_d);

// used a joystick channel calibration and convertes
//      the raw joystick value to calibrated joystick value
double js_chn_convert(const js_chn_t* chn, int16_t input);

// prints the raw joystick data
void js_fprint_data(FILE* fs, js_data_t* jsd);

#ifdef __cplusplus
}
#endif

#endif // JOYSTICK_H
