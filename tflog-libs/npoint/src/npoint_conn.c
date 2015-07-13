#include "npoint_conn.h"

#include "utils.h"
#include "npoint_packet.h"


static void* np_conn_recv_thread(void *ptr);

static void np_conn_parse_packet(np_conn_t* npc, char *data, int len);

static void np_conn_invoke_mocap_data_handlers(np_conn_t* npc,
                                               npoint_mocap_data_t* md);

static void np_conn_invoke_marker_set_handlers(np_conn_t* npc,
                                               npoint_mocap_data_t* md,
                                               npoint_marker_set_t* ms);

static void np_conn_invoke_rigid_body_handlers(np_conn_t* npc,
                                               npoint_mocap_data_t* md,
                                               npoint_rigid_body_t* rb);


static int np_conn_recv_raw_data(np_conn_t* npc, char* data, int len);



void np_conn_init(np_conn_t* npc)
{
    npc->name = "std-npoint-connection";
    npc->mocap_data_handler = NULL;
    npc->marker_set_handler = NULL;
    npc->rigid_body_handler = NULL;

    npc->recv_running = false;

    npc->send = NULL;
    npc->recv = NULL;

    uint64_t ts = utils_us_since_epoch();

    npc->status.rx.bytes.tcount = 0;
    npc->status.rx.bytes.lcount = 0;
    npc->status.rx.bytes.ts = ts;

    npc->status.rx.packets.tcount = 0;
    npc->status.rx.packets.lcount = 0;
    npc->status.rx.packets.ts = ts;

    utils_alloc_handler_list(&npc->mocap_data_handler_list,NP_CONN_HANDLER_LIST_LEN);
    utils_alloc_handler_list(&npc->marker_set_handler_list,NP_CONN_HANDLER_LIST_LEN);
    utils_alloc_handler_list(&npc->rigid_body_handler_list,NP_CONN_HANDLER_LIST_LEN);

    utils_clear_handler_list(&npc->mocap_data_handler_list);
    utils_clear_handler_list(&npc->marker_set_handler_list);
    utils_clear_handler_list(&npc->rigid_body_handler_list);
}

void np_conn_deinit(np_conn_t* npc)
{
    utils_free_handler_list(&npc->mocap_data_handler_list);
    utils_free_handler_list(&npc->marker_set_handler_list);
    utils_free_handler_list(&npc->rigid_body_handler_list);
}

int np_conn_add_mocap_data_handler(np_conn_t* npc, utils_generic_handler_func_t func)
{
    return utils_add_handler(&npc->mocap_data_handler_list,func);
}

int np_conn_add_marker_set_handler(np_conn_t* npc, utils_generic_handler_func_t func)
{
    return utils_add_handler(&npc->marker_set_handler_list,func);
}

int np_conn_add_rigid_body_handler(np_conn_t* npc, utils_generic_handler_func_t func)
{
    return utils_add_handler(&npc->rigid_body_handler_list,func);
}

int np_conn_rm_mocap_data_handler(np_conn_t* npc, utils_generic_handler_func_t func)
{
    return utils_rm_handler(&npc->mocap_data_handler_list,func);
}

int np_conn_rm_marker_set_handler(np_conn_t* npc, utils_generic_handler_func_t func)
{
    return utils_rm_handler(&npc->marker_set_handler_list,func);
}

int np_conn_rm_rigid_body_handler(np_conn_t* npc, utils_generic_handler_func_t func)
{
    return utils_rm_handler(&npc->rigid_body_handler_list,func);
}


int np_conn_start_packet_receiver(np_conn_t* npc)
{
    if(npc->recv_running == true)
    {
        return NP_CONN_RECV_THREAD_ALREADY_EXISTS_ERR;
    }

    npc->recv_running = true;
    pthread_create(&npc->pth_recv, NULL, np_conn_recv_thread, npc);

    return NP_CONN_OK;
}

int np_conn_stop_packet_receiver(np_conn_t* npc)
{
    if(npc->recv_running == false)
    {
        return NP_CONN_RECV_THREAD_DOESNT_EXIST_ERR;
    }

    npc->recv_running = false;
    pthread_join(npc->pth_recv, NULL);

    return NP_CONN_OK;
}


void* np_conn_recv_thread(void *ptr)
{
    np_conn_t* npc = (np_conn_t*)ptr;

    char buf[NP_CONN_MAX_PACKET_LEN];
    int len;

    if(npc->recv == NULL)
    {
        printf("[%s] ERROR: NO receiver callback specified!\n", npc->name);
        return NULL;
    }

    printf("[%s] recv thread started ...\n", npc->name);
    while (npc->recv_running)
    {
        len = np_conn_recv_raw_data(npc, buf, sizeof(buf));

        if(len<=0)
        {
            usleep(1000);
            continue;
        }

        np_conn_parse_packet(npc, buf, len);
    }

    printf("[%s] recv thread stopped ...\n", npc->name);

    return NULL;
}

void np_conn_parse_packet(np_conn_t* npc, char *data, int len)
{
    int i=0, ret;
    npoint_mocap_data_t md;
    npoint_marker_set_t ms;
    npoint_rigid_body_t rb;

    ret = np_packet_unpack_mocap_data(&md,data,len);

    // invalid package id
    if(ret == false)
    {
        return;
    }

    // invoke mocap data handlers
    np_conn_invoke_mocap_data_handlers(npc, &md);

    // invoke marker set handlers
    for(i=0; i<md.nMarkerSets; i++)
    {
        np_packet_unpack_mocap_data_markerSet(&ms,&md,i);
        np_conn_invoke_marker_set_handlers(npc, &md, &ms);
    }

    // invoke rigid body handlers
    for(i=0; i<md.nRigidBodies; i++)
    {
        np_packet_unpack_mocap_data_rigidBody(&rb,&md,i);
        np_conn_invoke_rigid_body_handlers(npc, &md, &rb);
    }
}

static void np_conn_invoke_mocap_data_handlers(np_conn_t* npc,
                                               npoint_mocap_data_t* md)
{
    np_conn_mocap_data_handler_args_t mda;
    mda.md = md;

    utils_invoke_handlers(&npc->mocap_data_handler_list,
                          NP_CONN_TO_HANDLER_ARG(&mda));
}

static void np_conn_invoke_marker_set_handlers(np_conn_t* npc,
                                               npoint_mocap_data_t* md,
                                               npoint_marker_set_t* ms)
{
    np_conn_marker_set_handler_args_t msa;
    msa.md = md;
    msa.ms = ms;

    utils_invoke_handlers(&npc->marker_set_handler_list,
                          NP_CONN_TO_HANDLER_ARG(&msa));
}


static void np_conn_invoke_rigid_body_handlers(np_conn_t* npc,
                                               npoint_mocap_data_t* md,
                                               npoint_rigid_body_t* rb)
{
    np_conn_rigid_body_handler_args_t rba;
    rba.md = md;
    rba.rb = rb;

    utils_invoke_handlers(&npc->rigid_body_handler_list,
                          NP_CONN_TO_HANDLER_ARG(&rba));
}

int np_conn_recv_raw_data(np_conn_t* npc, char* data, int len)
{
    int ret;

    ret = npc->recv(data,len);
    if(ret > 0)
    {
        npc->status.rx.bytes.tcount += ret;
        npc->status.rx.packets.tcount += 1;
    }

    return ret;
}


void* np_conn_std_mocap_data_handler(void* arg)
{
    npoint_mocap_data_t* md = NP_CONN_GET_MOCAP_DATA_HANDLER_ARGS(arg)->md;

    np_packet_printf_mocap_data(stdout,md);
    printf("\n");
}

void* np_conn_std_marker_set_handler(void* arg)
{
    npoint_mocap_data_t* md = NP_CONN_GET_MARKER_SET_HANDLER_ARGS(arg)->md;
    npoint_marker_set_t* ms = NP_CONN_GET_MARKER_SET_HANDLER_ARGS(arg)->ms;

    np_packet_printf_mocap_data_markerSet(stdout,md,ms);
    printf("\n");
}

void* np_conn_std_rigid_body_handler(void* arg)
{
    npoint_mocap_data_t* md = NP_CONN_GET_RIGID_BODY_HANDLER_ARGS(arg)->md;
    npoint_rigid_body_t* rb = NP_CONN_GET_RIGID_BODY_HANDLER_ARGS(arg)->rb;

    np_packet_printf_mocap_data_rigidBody(stdout,md,rb);
    printf("\n");
}

double np_conn_get_rx_byte_rate(np_conn_t* npc)
{
    uint64_t start, stop;
    unsigned int diff, b, lb;
    double rate;

    stop = utils_us_since_epoch();

    // read rate related values
    start   = npc->status.rx.bytes.ts;
    b       = npc->status.rx.bytes.tcount;
    lb      = npc->status.rx.bytes.lcount;

    // store new rate related values;
    npc->status.rx.bytes.lcount = b;
    npc->status.rx.bytes.ts = stop;

    // difference of bytes since last call;
    diff = b - lb;

    // bytes per s
    rate = 1e6f *((double)diff) / ((double)(stop - start));

    return rate;
}

double np_conn_get_rx_packet_rate(np_conn_t* npc)
{
    uint64_t start, stop;
    unsigned int diff, b, lb;
    double rate;

    stop = utils_us_since_epoch();

    // read rate related values
    start   = npc->status.rx.packets.ts;
    b       = npc->status.rx.packets.tcount;
    lb      = npc->status.rx.packets.lcount;

    // store new rate related values;
    npc->status.rx.packets.lcount = b;
    npc->status.rx.packets.ts = stop;

    // difference of packets since last call;
    diff = b - lb;

    // bytes per s
    rate = 1e6f *((double)diff) / ((double)(stop - start));

    return rate;
}
