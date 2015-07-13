#include "npoint_logger.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <inttypes.h>


#include "npoint_packet.h"
#include "utils.h"


void np_logger_init(np_logger_t* npl, const char* dir_path)
{
    npl->name = "std-npoint-logger";
    npl->files_opened = false;
    npl->dir_path = utils_alloc_str(dir_path);
}

void np_logger_deinit(np_logger_t* npl)
{
    if(npl->dir_path != NULL)
    {
        free(npl->dir_path);
    }

    if(npl->files_opened == true)
    {
        np_logger_close_files(npl);
        utils_free_file_list(&npl->list);
    }
}

int np_logger_create_and_open_files(np_logger_t* npl, npoint_mocap_data_t* md)
{
    int i=0, ret;
    utils_file_t* uf;

    if(npl->files_opened == true)
    {
        printf("[%s] opening files failed. Files are already opened.\n", npl->name);
        return false;
    }

    ret = np_logger_alloc_and_init_file_list(&npl->list,md,npl->dir_path);
    if(ret == false)
    {
        printf("[%s] initializing file list failed.\n", npl->name);
        return false;
    }

    printf("[%s] logger file list: len = %d, index = %d\n", npl->name, npl->list.len, npl->list.index);


    for(i=0; i<npl->list.index;i++)
    {
        uf = &npl->list.file[i];

        printf("[%s] open file '%s' ...\n", npl->name, uf->file_path);

        if(utils_create_and_open_file(uf) != UTILS_OK)
        {
            printf("[%s] opening file '%s' failed\n", npl->name, uf->file_path);
        }
    }

    printf("[%s] files opend.\n", npl->name);

    npl->files_opened = true;

    return true;
}


void np_logger_close_files(np_logger_t* npl)
{
    int i = 0;

    for(i = 0; i< npl->list.index; i++)
    {
        utils_close_file(&npl->list.file[i]);
    }
}


int np_logger_alloc_and_init_file_list(utils_file_list_t* list, npoint_mocap_data_t* md, const char* dir_path)
{
    int ret,i;
    utils_str_list_t sl;
    utils_file_t uf;

    char* name; //max 256 characters
    char filename[256+10];


    // alloc file list
    utils_alloc_file_list(list,md->nRigidBodies);
    //np_logger_alloc_file_list(list,5);

    // alloc string list for rigid body name list
    utils_alloc_str_list(&sl,md->nRigidBodies,256);

    // get rigid body name list;
    ret = np_packet_unpack_mocap_data_get_rigidBody_nameList(&sl,md);

    if(ret <= 0)
    {
        return false;
    }

    for(i=0; i<ret; i++)
    {
        name = sl.str[i];

        // build filename (name + extention)
        strcpy(filename,name);
        strcat(filename,".log");

        utils_init_file(&uf, name, filename ,dir_path);
        utils_add_file_to_file_list(list,&uf);
    }

    // free string list
    utils_free_str_list(&sl);

    return true;
}


int np_logger_std_write_rigidBody(np_logger_t* npl, npoint_mocap_data_t* md, npoint_rigid_body_t* rb)
{
    int index;
    char name[256];

    utils_file_t* uf;

    // get ribid body name
    np_packet_unpack_mocap_data_get_rigidBody_name(name,rb,md);

    // get pointer to logger file
    index = utils_find_file_in_file_list(&npl->list,name);
    if(index == -1)
    {
        return -1;
    }

    uf = &npl->list.file[index];

    utils_rpy_t rpy_rad, rpy_deg;

    utils_quaternion_to_rpy(&rpy_rad,&rb->q);
    utils_rpy_to_deg(&rpy_deg,&rpy_rad);


    // print values to file
    fprintf(uf->fs, "%12" PRIu64 " ",   md->ts);

    fprintf(uf->fs, "%9.6f ",           md->latency);
    fprintf(uf->fs, "%10u ",            md->frame_nr);
    fprintf(uf->fs, "%4u ",             md->id);

    fprintf(uf->fs, "%9.6f ",           rb->pos.x);
    fprintf(uf->fs, "%9.6f ",           rb->pos.y);
    fprintf(uf->fs, "%9.6f ",           rb->pos.z);

    fprintf(uf->fs, "%9.6f ",           rpy_rad.roll);
    fprintf(uf->fs, "%9.6f ",           rpy_rad.pitch);
    fprintf(uf->fs, "%9.6f ",           rpy_rad.yaw);

    fprintf(uf->fs, "%9.6f ",           rpy_deg.roll);
    fprintf(uf->fs, "%9.6f ",           rpy_deg.pitch);
    fprintf(uf->fs, "%9.6f ",           rpy_deg.yaw);

    fprintf(uf->fs, "%9.6f ",           rb->q.x);
    fprintf(uf->fs, "%9.6f ",           rb->q.y);
    fprintf(uf->fs, "%9.6f ",           rb->q.z);
    fprintf(uf->fs, "%9.6f ",           rb->q.w);

    fprintf(uf->fs, "%9.6f ",           rb->rigidMarkers_mean_error);

    fprintf(uf->fs, "%4u ",             rb->nRigidMarkers);


    npoint_rigid_marker_t rm;
    int j;

    for(j=0; j<rb->nRigidMarkers; j++)
    {
        np_packet_unpack_mocap_data_rigidMarker(&rm,rb,j);

        fprintf(uf->fs, "%4u ",             j);
        fprintf(uf->fs, "%4u ",             rm.id);
        fprintf(uf->fs, "%9.6f ",           rm.size);

        fprintf(uf->fs, "%9.6f ",           rm.m.x);
        fprintf(uf->fs, "%9.6f ",           rm.m.y);
        fprintf(uf->fs, "%9.6f ",           rm.m.z);
    }

    fprintf(uf->fs, "\n");

    // enforce that everything is written to file
    fflush(uf->fs);

    return false;
}




