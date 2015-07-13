#include "joystick.h"


#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <fcntl.h>

#include <sys/ioctl.h>
#include <linux/joystick.h>

#include <stdbool.h>
#include <math.h>

static int parse_buttons(int lb, int rb);


int js_open(joystick_t* js)
{
    js->fd = open(js->dev, O_RDONLY);

    if (js->fd == -1)
    {
        return JS_HARD_FAULT;
    }

    //non blocking mode
    fcntl(js->fd, F_SETFL, O_NONBLOCK);
    return JS_OK;
}


js_calibration_t* js_std_calib(js_calibration_t* calib)
{
    /**     standard config

    ranges

        roll            [-1000; 1000]   * 0.3   (param: RC: RC_SCALE_ROLL)
        pitch           [-1000; 1000]   * 0.3   (param: RC: RC_SCALE_PITCH)
        yaw             [-1000; 1000]   * 2     (param: RC: RC_SCALE_YAW)
        throttle        [0;     1000]   * 1


    channel maps

        roll            1                       (param: RC: RC_MAP_ROLL)
        pitch           2                       (param: RC: RC_MAP_PTICH)
        yaw             3                       (param: RC: RC_MAP_YAW)
        throttle        4                       (param: RC: RC_MAP_THROTTLE)


    channel configs (original ones)
                        dead-zone   reversed    min         max         trim
                        (RCx_DZ)    (RCx_REV)   (RCx_MIN)   (RCx_MAX)   (RCx_TRIM)

        roll            0           1  (no)     800         2199        1524
        pitch           0           1  (no)     800         2199        1579
        yaw             0           1  (no)     800         2199        1529
        throttle        30          -1 (yes)    800         2199        2199

    **/


    calib->pitch.scale = 0.3;
    calib->pitch.rev = false;
    calib->pitch.min = SHRT_MIN;
    calib->pitch.max = SHRT_MAX;
    calib->pitch.dz = 2000;
    calib->pitch.trim = 0;

    calib->roll.scale = 0.3;
    calib->roll.rev = false;
    calib->roll.min = SHRT_MIN;
    calib->roll.max = SHRT_MAX;
    calib->roll.dz = 2000;
    calib->roll.trim = 0;

    calib->yaw.scale = 2;
    calib->yaw.rev = false;
    calib->yaw.min = SHRT_MIN;
    calib->yaw.max = SHRT_MAX;
    calib->yaw.dz = 3000;
    calib->yaw.trim = 0;

    calib->throttle.scale = 1.0;
    calib->throttle.rev = true;
    calib->throttle.min = SHRT_MIN;
    calib->throttle.max = SHRT_MAX;
    calib->throttle.dz = 1300;
    calib->throttle.trim = SHRT_MAX;

    return calib;
}


int js_update(joystick_t *js, js_data_t* js_d)
{
    int ret;
    struct js_event js_e;

    ret = read(js->fd, &js_e, sizeof(struct js_event));

    if (ret < 0)
    {
        if (errno == EAGAIN)
        {
            //timeout error; retry
            return JS_TIMEOUT_ERR;
        }

        //some serious error occurred
        return JS_HARD_FAULT;
    }


    if((js_e.type & ~JS_EVENT_INIT) == JS_EVENT_AXIS)
    {
        switch (js_e.number)
        {
        case YAW:
            js_d->yaw = (int16_t)js_e.value;
            break;
        case ROLL:
            js_d->roll = (int16_t)js_e.value;
            break;
        case PITCH:
            js_d->pitch= (int16_t)js_e.value;
            break;
        case THROTTLE:
            js_d->throttle = (int16_t)js_e.value;
            break;
        case LEFTBUT:
            js_d->bLeft = (int16_t)js_e.value;
            break;
        case RIGHTBUT:
            js_d->bRight = (int16_t)js_e.value;
            break;
        default:
            break;
        }

        js_d->bstates = parse_buttons(js_d->bLeft, js_d->bRight);
   }

    return JS_OK;
}

double js_chn_convert(const js_chn_t* chn, int16_t input)
{
    double normed, out;

    //enforce correct boarders
    if (input < chn->min)
    {
        input = chn->min;
    }

    if (input> chn->max)
    {
        input = chn->max;
    }


    //norm everthing to [-1.0, 1.0]
    if (input > (chn->trim + chn->dz))
    {
        normed = (input - chn->trim - chn->dz) / (double)(chn->max - chn->trim - chn->dz);


    }
    else if (input < (chn->trim - chn->dz))
    {
        normed = (input - chn->trim + chn->dz) / (double)(chn->trim - chn->min - chn->dz);

    }
    else
    {
        //active dead zone
        normed = 0.0f;
    }

    if(chn->rev == true)
    {
        normed = normed * -1.0;
    }


    //ensure that everthing is in valid region (throttle must be tested externally)
    if (normed < -1.0)
    {
        normed = -1.0;
    }

    if (normed > 1.0)
    {
        normed = 1.0;
    }

    // scale the output
    if (isfinite(normed) && chn->scale > 0.0)
    {
        out = normed * chn->scale;
    }
    return out;
}



int parse_buttons(int lb, int rb)
{
    static int old_lb = SHRT_MIN;
    static int old_rb = SHRT_MIN;

    int dz = 1000;      //dead zone

    int tmp1 = lb - old_lb;
    int tmp2 = rb - old_rb;

    old_lb = lb;
    old_rb = rb;

    int result = 0;

    if (tmp1 > 0 + dz)
    {
        result |= JS_BUTTON1_PRESSED;
    }
    else if (tmp1 < 0 -dz)
    {
        result |= JS_BUTTON1_RELEASED;
    }
    else
    {
        result |= JS_BUTTON1_UNCHANGED;
    }

    if (tmp2 > 0 + dz)
    {
        result |= JS_BUTTON2_PRESSED;
    }
    else if (tmp2 < 0 -dz)
    {
        result |= JS_BUTTON2_RELEASED;
    }
    else
    {
        result |= JS_BUTTON2_UNCHANGED;
    }

    return result;

}

void js_fprint_data(FILE* fs, js_data_t* jsd)
{
    fprintf(fs,"joystick data:\n");
    fprintf(fs,"   roll            = %d\n", jsd->roll);
    fprintf(fs,"   pitch           = %d\n", jsd->pitch);
    fprintf(fs,"   yaw             = %d\n", jsd->yaw);
    fprintf(fs,"   throttle        = %d\n", jsd->throttle);
    fprintf(fs,"   button left     = %d\n", jsd->bLeft);
    fprintf(fs,"   button right    = %d\n", jsd->bRight);
    fprintf(fs,"\n");

    if(jsd->bstates & JS_BUTTON1_PRESSED)
    {
        fprintf(fs,"   button left     = PRESSED\n");
    }
    else if (jsd->bstates & JS_BUTTON1_RELEASED)
    {
        fprintf(fs,"   button left     = RELEASED\n");
    }
    else if (jsd->bstates & JS_BUTTON1_UNCHANGED)
    {
        fprintf(fs,"   button left     = UNCHANGED\n");
    }


    if(jsd->bstates & JS_BUTTON2_PRESSED)
    {
        fprintf(fs,"   button right    = PRESSED\n");
    }
    else if (jsd->bstates & JS_BUTTON2_RELEASED)
    {
        fprintf(fs,"   button right    = RELEASED\n");
    }
    else if (jsd->bstates & JS_BUTTON2_UNCHANGED)
    {
        fprintf(fs,"   button right    = UNCHANGED\n");
    }

    fprintf(fs,"\n\n");
    fflush(stdout);
}


