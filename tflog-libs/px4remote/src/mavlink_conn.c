#include "mavlink_conn.h"

#include "utils.h"


// use the standard mavlink message set
#include "mavlink/v1.0/common/mavlink.h"




#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <stdbool.h>


// use a modified "mavlink_parse_char" function because
//		the default function uses static parsing buffers
//		and thus cannot be used by multiple threads in multiple
//		modules
#include "ml_conn_mavlink_parse_char.h"
//static uint8_t ml_conn_mavlink_parse_char(ml_conn_t* mlc,
//                                          uint8_t c,
//                                          mavlink_message_t* r_message,
//                                          mavlink_status_t* r_mavlink_status);


const ml_conn_endpoint_t ml_conn_std_mav_ep =
{
    1,      // system id
    0       // component id
};

const ml_conn_endpoint_t ml_conn_std_remote_ep =
{
    255,    // system id
    0       // component id
};


static void* ml_conn_recv_thread(void *ptr);
static void ml_conn_parse_packet(ml_conn_t* mlc, char *data, int len);
static void ml_conn_update_target_state(ml_conn_t* mlc, mavlink_message_t* msg);

// makes send thread save
static void ml_conn_send_raw_data(ml_conn_t* mlc, char* data, int len);
static int ml_conn_recv_raw_data(ml_conn_t* mlc, char* data, int len);

void ml_conn_init(ml_conn_t* mlc)
{
    mlc->name = "std-mavlink-connection";
    mlc->msg_handler = NULL;
    mlc->recv_running = false;
    mlc->send = NULL;
    mlc->recv = NULL;

    uint64_t ts = utils_us_since_epoch();

    mlc->status.rx.bytes.tcount = 0;
    mlc->status.rx.bytes.lcount = 0;
    mlc->status.rx.bytes.ts = ts;

    mlc->status.rx.packets.tcount = 0;
    mlc->status.rx.packets.lcount = 0;
    mlc->status.rx.packets.ts = ts;

    mlc->status.tx.bytes.tcount = 0;
    mlc->status.tx.bytes.lcount = 0;
    mlc->status.tx.bytes.ts = ts;

    mlc->status.tx.packets.tcount = 0;
    mlc->status.tx.packets.lcount = 0;
    mlc->status.tx.packets.ts = ts;


    pthread_mutex_init(&mlc->send_mutex,NULL);

    utils_alloc_handler_list(&mlc->handler_list,32);
    utils_clear_handler_list(&mlc->handler_list);
}


void ml_conn_deinit(ml_conn_t* mlc)
{
    utils_free_handler_list(&mlc->handler_list);
}

int ml_conn_add_handler(ml_conn_t* mlc, utils_generic_handler_func_t func)
{
   return utils_add_handler(&mlc->handler_list,func);
}


int ml_conn_rm_handler(ml_conn_t* mlc, utils_generic_handler_func_t func)
{
    return utils_rm_handler(&mlc->handler_list,func);
}


void ml_conn_send_mavlink_message(ml_conn_t* mlc, mavlink_message_t* msg)
{
    char buf[MAVLINK_MAX_PACKET_LEN];
    int len;

    len = mavlink_msg_to_send_buffer((uint8_t*)buf, msg);

    ml_conn_send_raw_data(mlc,buf,len);
}


int ml_conn_start_receiver(ml_conn_t* mlc)
{
    if(mlc->recv_running == true)
    {
        return ML_CONN_RECV_THREAD_ALREADY_EXISTS_ERR;
    }

    mlc->recv_running = true;
    pthread_create(&mlc->pth_recv, NULL, ml_conn_recv_thread, mlc);
}

int ml_conn_stop_receiver(ml_conn_t* mlc)
{
    if(mlc->recv_running == false)
    {
        return ML_CONN_RECV_THREAD_DOESNT_EXIST_ERR;
    }

    mlc->recv_running = false;
    pthread_join(mlc->pth_recv, NULL);

    return ML_CONN_OK;
}


double ml_conn_get_tx_byte_rate(ml_conn_t* mlc)
{
    uint64_t start, stop;
    unsigned int diff, b, lb;
    double rate;

    stop = utils_us_since_epoch();

    // read rate related values
    start   = mlc->status.tx.bytes.ts;
    b       = mlc->status.tx.bytes.tcount;
    lb      = mlc->status.tx.bytes.lcount;

    // store new rate related values;
    mlc->status.tx.bytes.lcount = b;
    mlc->status.tx.bytes.ts = stop;

    // difference of bytes since last call;
    diff = b - lb;

    // bytes per s
    rate = 1e6f *((double)diff) / ((double)(stop - start));

    return rate;
}

double ml_conn_get_rx_byte_rate(ml_conn_t* mlc)
{
    uint64_t start, stop;
    unsigned int diff, b, lb;
    double rate;

    stop = utils_us_since_epoch();

    // read rate related values
    start   = mlc->status.rx.bytes.ts;
    b       = mlc->status.rx.bytes.tcount;
    lb      = mlc->status.rx.bytes.lcount;

    // store new rate related values;
    mlc->status.rx.bytes.lcount = b;
    mlc->status.rx.bytes.ts = stop;

    // difference of bytes since last call;
    diff = b - lb;

    // bytes per s
    rate = 1e6f *((double)diff) / ((double)(stop - start));

    return rate;
}



void* ml_conn_recv_thread(void *ptr)
{
    ml_conn_t* mlc = (ml_conn_t*)ptr;

    char buf[MAVLINK_MAX_PACKET_LEN];
    int len;

    if(mlc->recv == NULL)
    {
        printf("[%s] ERROR: NO receiver callback specified!\n", mlc->name);
        return NULL;
    }

    printf("[%s] recv thread started ...\n", mlc->name);

    while (mlc->recv_running)
    {
        len = ml_conn_recv_raw_data(mlc,buf,MAVLINK_MAX_PACKET_LEN);

        if(len<=0)
        {
            usleep(1000);
            continue;
        }

        ml_conn_parse_packet(mlc, buf, len);

    }

    printf("[%s] recv thread stopped ...\n", mlc->name);

    return NULL;
}

void ml_conn_parse_packet(ml_conn_t* mlc, char *data, int len)
{
    //uint8_t link_id = 0;

    ml_conn_handler_args_t ha;

    mavlink_message_t msg;
    mavlink_status_t status;
    unsigned int decode_state;

    //ha.mlc = mlc;
    ha.msg = &msg;

    int i = 0;

    for (i = 0; i < len; i++)
    {
        //decode_state = mavlink_parse_char(link_id, (uint8_t)data[i], &msg, &status);
        decode_state = ml_conn_mavlink_parse_char(mlc,(uint8_t)data[i], &msg, &status);

        if (decode_state == 1)
        {
            if(mlc->msg_handler != NULL)
            {
                mlc->msg_handler(&msg);
            }

            utils_invoke_handlers(&mlc->handler_list, ML_CONN_TO_HANDLER_ARG(&ha));

            ml_conn_update_target_state(mlc,&msg);

            // printf("[%s] packet parsed.\n",mlc->name);
        }
    }
}

void ml_conn_update_target_state(ml_conn_t* mlc, mavlink_message_t* msg)
{
    if(msg->msgid == MAVLINK_MSG_ID_HEARTBEAT)
    {
        mavlink_heartbeat_t hb;
        mavlink_msg_heartbeat_decode(msg, &hb);

        mlc->target.base_mode = hb.base_mode;
        mlc->target.custom_mode = hb.custom_mode;

        if(mlc->target.base_mode & MAV_MODE_FLAG_SAFETY_ARMED)
        {
            mlc->target.is_armed = true;
        }
        else
        {
            mlc->target.is_armed = false;
        }

    }

}

void ml_conn_send_raw_data(ml_conn_t* mlc, char* data, int len)
{
    if(mlc->send != NULL)
    {
        pthread_mutex_lock(&mlc->send_mutex);
        mlc->send(data, len);
        pthread_mutex_unlock(&mlc->send_mutex);

        mlc->status.tx.bytes.tcount += len;
    }
}

int ml_conn_recv_raw_data(ml_conn_t* mlc, char* data, int len)
{
    int ret;

    ret = mlc->recv(data,len);
    if(ret > 0)
    {
        mlc->status.rx.bytes.tcount += ret;
    }

    return ret;
}
