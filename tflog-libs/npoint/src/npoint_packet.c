#include "npoint_packet.h"

#include "utils.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <math.h>


int np_packet_unpack_mocap_data(npoint_mocap_data_t* p, char* packet, int len)
{
    char *ptr = packet;

    //    unsigned char sels[] = {112,216,220};
    //    fprint_np_packet_raw_with_sels(packet,len,sels,3);
    //    printf("\n\n");

    p->ts = utils_us_since_epoch();

    // packet id
    memcpy(&p->id, ptr, 2);
    ptr += 2;

    // packet size
    memcpy(&p->size, ptr, 2);
    ptr += 2;

    if(p->id != NPOINT_PACKET_ID_MOCAP_DATA)
    {
        return false;
    }


    // frame number
    memcpy(&p->frame_nr, ptr, 4);
    ptr += 4;

    // marker sets
    memcpy(&p->nMarkerSets, ptr, 4);
    ptr += 4;
    p->markerSets = ptr;
    ptr = np_packet_unpack_mocap_data_skip_markerSets(ptr,p->nMarkerSets);


    // other markers
    memcpy(&p->nOtherMarkers, ptr, 4);
    ptr += 4;
    p->otherMarkers = ptr;
    ptr = np_packet_unpack_mocap_data_skip_otherMarkers(ptr,p->nOtherMarkers);

    //printf("ptr at position = %ld\n", ptr-packet);

    // rigid bodies
    memcpy(&p->nRigidBodies, ptr, 4);
    ptr += 4;
    p->rigidBodies = ptr;
    ptr = np_packet_unpack_mocap_data_skip_rigidBodies(ptr,p->nRigidBodies);

    //printf("ptr at position = %ld\n", ptr-packet);


    // skeletons (version 2.1 and later)
    if(((NPOINT_PACKET_VER_MAJOR == 2) && (NPOINT_PACKET_VER_MINOR > 0)) || (NPOINT_PACKET_VER_MAJOR > 2))
    {
        memcpy(&p->nSkeletons, ptr, 4);
        fflush(stdout);
        ptr += 4;
        p->skeletons = ptr;
        ptr = np_packet_unpack_mocap_data_skip_skeletons(ptr,p->nSkeletons);
    }


    // labeled markers (version 2.3 and later)
    if(((NPOINT_PACKET_VER_MAJOR == 2) && (NPOINT_PACKET_VER_MINOR >= 3)) || (NPOINT_PACKET_VER_MAJOR > 2))
    {
        memcpy(&p->nLabeledMarkers, ptr, 4);
        fflush(stdout);
        ptr += 4;
        p->labeledMarkers = ptr;
        ptr = np_packet_unpack_mocap_data_skip_labeledMarkers(ptr,p->nLabeledMarkers);
    }

    // latency
    memcpy(&p->latency, ptr, 4);
    ptr += 4;

    // timecode
    memcpy(&p->timecode, ptr, 4);
    ptr += 4;

    // timecode sub
    memcpy(&p->timecodeSub, ptr, 4);
    ptr += 4;

    // end of data tag
    memcpy(&p->endOfData, ptr, 4);
    ptr += 4;

    //printf("ptr at position = %ld\n", ptr-packet);
    return true;
}



/*********************************************************************************************

                                    markerSets

*********************************************************************************************/

char* np_packet_unpack_mocap_data_skip_markerSets(char* ptr, uint32_t nMarkerSets)
{
    int i = 0;

    //just to update the ptr position
    for (i=0; i < nMarkerSets; i++)
    {
        ptr = np_packet_unpack_mocap_data_skip_markerSet(ptr);
    }

    return ptr;
}

char* np_packet_unpack_mocap_data_skip_markerSet(char* ptr)
{
    uint32_t nMarkers;
    char name[256];
    int name_len = 0;

    strncpy(name, ptr, 256);
    name_len = strlen(name) + 1;
    ptr += name_len;

    memcpy(&nMarkers, ptr, 4);
    ptr += 4;

    return np_packet_unpack_mocap_data_skip_markers(ptr, nMarkers);
}

char* np_packet_unpack_mocap_data_skip_markers(char* ptr, uint32_t nMarkers)
{
    int i = 0;

    for(i=0; i<nMarkers; i++)
    {
        ptr += 4;   // x
        ptr += 4;   // y
        ptr += 4;   // z
    }

    return ptr;
}


void np_packet_unpack_mocap_data_markerSet(npoint_marker_set_t* ms, npoint_mocap_data_t* md, int index)
{
    int i = 0;
    int name_len = 0;
    char* ptr = md->markerSets;

    for(i=0; i != index; i++)
    {
        ptr = np_packet_unpack_mocap_data_skip_markerSet(ptr);
    }

    // markerSet at index
    ms->index = index;

    strncpy(ms->name, ptr, 256);
    name_len = strlen(ms->name) + 1;
    ptr += name_len;

    memcpy(&ms->nMarkers, ptr, 4);
    ptr += 4;

    ms->markers = ptr;

    //markers
    //ptr = np_packet_unpack_mocap_data_skip_markers(ptr, ms->nMarkers);
}


void np_packet_unpack_mocap_data_markerSet_marker(npoint_marker_t* m, npoint_marker_set_t* ms, int index)
{
    np_packet_unpack_mocap_data_marker(m,ms->markers,index);
}

void np_packet_unpack_mocap_data_marker(npoint_marker_t* m, char* marker_list, int index)
{
    char* ptr = marker_list + index * sizeof(float) * 3;

    // x
    memcpy(&m->x, ptr, 4);
    ptr += 4;

    // y
    memcpy(&m->y, ptr, 4);
    ptr += 4;

    // z
    memcpy(&m->z, ptr, 4);
    ptr += 4;
}


int np_packet_unpack_mocap_data_find_markerSet(npoint_mocap_data_t* md, const char* name)
{
    int i = 0;

    if(strlen(name) + 1 > 256)
    {
        return -1;
    }


    npoint_marker_set_t ms;

    for(i=0; i<md->nMarkerSets; i++)
    {
        np_packet_unpack_mocap_data_markerSet(&ms,md,i);

        if(strcmp(name,ms.name) == 0)
        {
            return i;
        }

    }

    return -1;
}

int np_packet_unpack_mocap_data_get_markerSet_nameList(utils_str_list_t* nameList, npoint_mocap_data_t *md)
{
    int i =0;
    int index = 0;

    npoint_marker_set_t ms;

    for(i=0; i<md->nMarkerSets; i++)
    {
        np_packet_unpack_mocap_data_markerSet(&ms,md,i);

        strncpy(nameList->str[i],ms.name,nameList->str_len);

        index++;

        if(index == nameList->list_len)
        {
            return index;
        }
    }


    return index;
}

int np_packet_unpack_mocap_data_get_markerSet(npoint_marker_set_t* ms, npoint_mocap_data_t* md, const char *name)
{
    int ret = np_packet_unpack_mocap_data_find_markerSet(md,name);
    if(ret == -1)
    {
        false;
    }

    np_packet_unpack_mocap_data_markerSet(ms,md,ret);

    return true;
}


/*********************************************************************************************

                                    otherMarkers

*********************************************************************************************/


char* np_packet_unpack_mocap_data_skip_otherMarkers(char* ptr, uint32_t nOtherMarkers)
{
    return (ptr + 3*4*nOtherMarkers);
}


void np_packet_unpack_mocap_data_otherMarker(npoint_marker_t* m, npoint_mocap_data_t* md, int index)
{
    np_packet_unpack_mocap_data_marker(m,md->otherMarkers,index);
}


/*********************************************************************************************

                                    rigidBodies

*********************************************************************************************/


char* np_packet_unpack_mocap_data_skip_rigidBodies(char* ptr, uint32_t nRigidBodies)
{
    int j = 0;

    for (j=0; j < nRigidBodies; j++)
    {
        ptr = np_packet_unpack_mocap_data_skip_rigidBody(ptr);

    } // next rigid body

    return ptr;
}

char* np_packet_unpack_mocap_data_skip_rigidBody(char* ptr)
{
    uint32_t nRigidMarkers = 0;

    ptr += 4;       // ID
    ptr += 4;       // x
    ptr += 4;       // y
    ptr += 4;       // z
    ptr += 4;       // qx
    ptr += 4;       // qy
    ptr += 4;       // qz
    ptr += 4;       // qw


    // nRigidMarkers
    memcpy(&nRigidMarkers, ptr, 4);
    ptr += 4;

    ptr = np_packet_unpack_mocap_data_skip_rigidMarkers(ptr, nRigidMarkers);

    return ptr;
}

char* np_packet_unpack_mocap_data_skip_rigidMarkers(char* ptr, uint32_t nRigidMarkers)
{
    // marker data
    ptr = np_packet_unpack_mocap_data_skip_rigidMarkers_markers(ptr,nRigidMarkers);

    if(NPOINT_PACKET_VER_MAJOR >= 2)
    {
        // associated marker IDs
        ptr = np_packet_unpack_mocap_data_skip_rigidMarkers_marker_ids(ptr,nRigidMarkers);

        // associated marker sizes
        ptr = np_packet_unpack_mocap_data_skip_rigidMarkers_marker_sizes(ptr,nRigidMarkers);
    }

    if(NPOINT_PACKET_VER_MAJOR >= 2)
    {
        ptr += 4; // Mean marker error
    }

    return ptr;
}

char* np_packet_unpack_mocap_data_skip_rigidMarkers_markers(char* ptr, uint32_t nRigidMarkers)
{
    // marker data
    ptr += nRigidMarkers * 3 *sizeof(float);

    return ptr;
}

char* np_packet_unpack_mocap_data_skip_rigidMarkers_marker_ids(char* ptr, uint32_t nRigidMarkers)
{
    // marker IDs
    ptr += nRigidMarkers * sizeof(int);

    return ptr;
}

char* np_packet_unpack_mocap_data_skip_rigidMarkers_marker_sizes(char* ptr, uint32_t nRigidMarkers)
{
    // marker sizes
    ptr += nRigidMarkers * sizeof(float);

    return ptr;
}


void np_packet_unpack_mocap_data_rigidBody(npoint_rigid_body_t* rb, npoint_mocap_data_t* md, int index)
{
    char* ptr = md->rigidBodies;
    int i = 0;

    for(i=0; i != index; i++)
    {
        ptr = np_packet_unpack_mocap_data_skip_rigidBody(ptr);
    }

    // rigidBody at index
    rb->index = index;

    // ID
    memcpy(&rb->id, ptr, 4);
    ptr += 4;

    // x
    memcpy(&rb->pos.x, ptr, 4);
    ptr += 4;

    // y
    memcpy(&rb->pos.y, ptr, 4);
    ptr += 4;

    // z
    memcpy(&rb->pos.z, ptr, 4);
    ptr += 4;

    // qx
    memcpy(&rb->q.x, ptr, 4);
    ptr += 4;

    // qy
    memcpy(&rb->q.y, ptr, 4);
    ptr += 4;

    // qz
    memcpy(&rb->q.z, ptr, 4);
    ptr += 4;

    // qw
    memcpy(&rb->q.w, ptr, 4);
    ptr += 4;

    // nRigidMarkers
    memcpy(&rb->nRigidMarkers, ptr, 4);
    ptr += 4;

    rb->rigidMarkers = ptr;


    if(NPOINT_PACKET_VER_MAJOR >= 2)
    {
        np_packet_unpack_mocap_data_rigidBody_mean_error(&rb->rigidMarkers_mean_error,rb);
    }
    else
    {
        rb->rigidMarkers_mean_error = 0.0f;
    }
}

void np_packet_unpack_mocap_data_rigidBody_mean_error(float* mean_error, npoint_rigid_body_t* rb)
{
    int i = 0;
    char* ptr = rb->rigidMarkers;

    // skip markers
    ptr = np_packet_unpack_mocap_data_skip_rigidMarkers_markers(ptr,rb->nRigidMarkers);

    // skip marker ids
    ptr = np_packet_unpack_mocap_data_skip_rigidMarkers_marker_ids(ptr,rb->nRigidMarkers);

    // skip marker sizes
    ptr = np_packet_unpack_mocap_data_skip_rigidMarkers_marker_sizes(ptr,rb->nRigidMarkers);

    // makers mean error
    memcpy(mean_error, ptr, 4);
}

void np_packet_unpack_mocap_data_rigidMarker(npoint_rigid_marker_t* rm, npoint_rigid_body_t* rb, int index)
{
    // marker
    np_packet_unpack_mocap_data_rigidMarker_marker(&rm->m,rb,index);

    if(NPOINT_PACKET_VER_MAJOR >= 2)
    {
        // marker ID
        np_packet_unpack_mocap_data_rigidMarker_marker_id(&rm->id,rb,index);

        // marker size
        np_packet_unpack_mocap_data_rigidMarker_marker_size(&rm->size,rb,index);
    }
    else
    {
        rm->id = 0;
        rm->size = 0.0f;
    }

}

void np_packet_unpack_mocap_data_rigidMarker_marker(npoint_marker_t* m, npoint_rigid_body_t* rb, int index)
{
    np_packet_unpack_mocap_data_marker(m,rb->rigidMarkers,index);
}

void np_packet_unpack_mocap_data_rigidMarker_marker_id(uint32_t* id, npoint_rigid_body_t* rb, int index)
{
    int i = 0;
    char* ptr = rb->rigidMarkers;

    // marker data
    ptr = np_packet_unpack_mocap_data_skip_rigidMarkers_markers(ptr,rb->nRigidMarkers);

    // skip ids
    for(i=0; i != index; i++)
    {
        ptr += 4;
    }

    // id at index
    memcpy(id, ptr, 4);
}

void np_packet_unpack_mocap_data_rigidMarker_marker_size(float* size, npoint_rigid_body_t* rb, int index)
{
    int i = 0;
    char* ptr = rb->rigidMarkers;

    // skip markers
    ptr = np_packet_unpack_mocap_data_skip_rigidMarkers_markers(ptr,rb->nRigidMarkers);

    // skip marker ids
    ptr = np_packet_unpack_mocap_data_skip_rigidMarkers_marker_ids(ptr,rb->nRigidMarkers);

    // skip sizes
    for(i=0; i != index; i++)
    {
        ptr += 4;
    }

    // id at index
    memcpy(size, ptr, 4);
}


int np_packet_unpack_mocap_data_find_rigidBody(npoint_mocap_data_t* md, const char *name)
{
    int ret;

    if(strcmp(name,"all") == 0)
    {
        return -1;
    }

    ret = np_packet_unpack_mocap_data_find_markerSet(md,name);


    return ret;
}

int np_packet_unpack_mocap_data_get_rigidBody_nameList(utils_str_list_t* nameList, npoint_mocap_data_t *md)
{
    int i =0;
    int index = 0;

    npoint_marker_set_t ms;

    for(i=0; i<md->nMarkerSets; i++)
    {
        np_packet_unpack_mocap_data_markerSet(&ms,md,i);

        if(strcmp(ms.name,"all") == 0)
        {
            continue;
        }

        strncpy(nameList->str[i],ms.name,nameList->str_len);

        index++;

        if(index == nameList->list_len)
        {
            return index;
        }
    }

    return index;
}

int np_packet_unpack_mocap_data_get_rigidBody(npoint_rigid_body_t* rb, npoint_mocap_data_t* md, const char *name)
{
    int ret = np_packet_unpack_mocap_data_find_markerSet(md,name);
    if(ret == -1)
    {
        return false;
    }

    np_packet_unpack_mocap_data_rigidBody(rb,md,ret);

    return true;
}

void np_packet_unpack_mocap_data_get_rigidBody_name(char* name, npoint_rigid_body_t* rb, npoint_mocap_data_t* md)
{
    int index = rb->index;
    npoint_marker_set_t ms;

    np_packet_unpack_mocap_data_markerSet(&ms,md,index);
    strncpy(name,ms.name,256);
}

/*********************************************************************************************

                                    skeletons

*********************************************************************************************/



char* np_packet_unpack_mocap_data_skip_skeletons(char* ptr, uint32_t nSkeletons)
{
    uint32_t nRigidBodies = 0;
    uint32_t nRigidMarkers = 0;

    int nBytes = 0;
    int j = 0;
    int k = 0;

    for (j=0; j < nSkeletons; j++)
    {
        // skeleton id
        ptr += 4;

        // number of rigid bodies
        memcpy(&nRigidBodies, ptr, 4);
        ptr += 4;

        for (k=0; k < nRigidBodies; k++)
        {
            ptr += 4;       // ID
            ptr += 4;       // x
            ptr += 4;       // y
            ptr += 4;       // z
            ptr += 4;       // qx
            ptr += 4;       // qy
            ptr += 4;       // qz
            ptr += 4;       // qw

            // associated marker positions
            memcpy(&nRigidMarkers, ptr, 4);
            ptr += 4;

            // marker data
            nBytes = nRigidMarkers * 3 *sizeof(float);
            ptr += nBytes;


            // associated marker IDs
            nBytes = nRigidMarkers * sizeof(int);
            ptr += nBytes;

            // associated marker sizes
            nBytes = nRigidMarkers * sizeof(float);
            ptr += nBytes;

            ptr += 4; // Mean marker error



        } // next rigid body

    } // next skeleton

    return ptr;
}


/*********************************************************************************************

                                    labeledMarkers

*********************************************************************************************/


char* np_packet_unpack_mocap_data_skip_labeledMarkers(char* ptr, uint32_t nLabeledMarkers)
{
    int j = 0;

    for (j=0; j < nLabeledMarkers; j++)
    {
        ptr = np_packet_unpack_mocap_data_skip_labeledMarker(ptr);
    }

    return ptr;
}

char* np_packet_unpack_mocap_data_skip_labeledMarker(char* ptr)
{
    ptr += 4;   // id
    ptr += 4;   // x
    ptr += 4;   // y
    ptr += 4;   // z
    ptr += 4;   // size

    return ptr;
}

void np_packet_unpack_mocap_data_labeledMarker(npoint_labeled_marker_t* lm, npoint_mocap_data_t* md, int index)
{
    char* ptr = md->labeledMarkers;
    int i =0;

    for(i=0; i != index; i++)
    {
        ptr = np_packet_unpack_mocap_data_skip_labeledMarker(ptr);
    }

    //labeled marker at index

    // id
    memcpy(&lm->id, ptr, 4);
    ptr += 4;

    // x
    memcpy(&lm->m.x, ptr, 4);
    ptr += 4;

    // y
    memcpy(&lm->m.y, ptr, 4);
    ptr += 4;

    // z
    memcpy(&lm->m.z, ptr, 4);
    ptr += 4;

    // size
    memcpy(&lm->size, ptr, 4);
    ptr += 4;

}


/*********************************************************************************************

                                    fprintf

*********************************************************************************************/


void np_packet_printf_mocap_data(FILE* fs, npoint_mocap_data_t* md)
{
    int i;

    fprintf(fs, "    id                         = %d\n", md->id);
    fprintf(fs, "    size                       = %d\n", md->size);
    fprintf(fs, "    frame                      = %d\n", md->frame_nr);
    fprintf(fs, "    nMarkerSets                = %d\n", md->nMarkerSets);
    fprintf(fs, "    nOtherMarkers              = %d\n", md->nOtherMarkers);
    fprintf(fs, "    nRigidBodies               = %d\n", md->nRigidBodies);
    fprintf(fs, "    nSkeletons                 = %d\n", md->nSkeletons);
    fprintf(fs, "    nLabeledMarkers            = %d\n", md->nLabeledMarkers);
    fprintf(fs, "    <timeSinceMotiveStart?>    = %9.6f [s]\n", md->latency);
    fprintf(fs, "    timecode                   = %d\n", md->timecode);
    fprintf(fs, "    timecodeSub                = %d\n", md->timecodeSub);

    npoint_marker_set_t ms;
    for(i=0; i<md->nMarkerSets; i++)
    {
        np_packet_unpack_mocap_data_markerSet(&ms,md,i);

        fprintf(fs, "   markerSet[%d]:\n", i);
        fprintf(fs, "       name        = %s\n", ms.name);
        fprintf(fs, "       nMarkers    = %d\n", ms.nMarkers);

        int j;
        npoint_marker_t m;

        for(j=0; j< ms.nMarkers; j++)
        {
            np_packet_unpack_mocap_data_markerSet_marker(&m,&ms,j);

            fprintf(fs, "        marker[%d]  = [% 3.6f, % 3.6f, % 3.6f] [m]\n",j,m.x,m.y,m.z);
        }
    }

    for(i=0; i<md->nOtherMarkers; i++)
    {
        npoint_marker_t m;
        np_packet_unpack_mocap_data_otherMarker(&m,md,i);

        fprintf(fs, "    otherMarker[%d]  = [% 3.6f, % 3.6f, % 3.6f] [m]\n",i,m.x,m.y,m.z);
    }

    npoint_rigid_body_t rb;
    for(i=0; i<md->nRigidBodies; i++)
    {
        np_packet_unpack_mocap_data_rigidBody(&rb,md,i);

        utils_rpy_t rpy_rad, rpy_deg;
        utils_quaternion_to_rpy(&rpy_rad,&rb.q);
        utils_rpy_to_deg(&rpy_deg,&rpy_rad);


        fprintf(fs, "    rigidBody[%d]:\n", i);
        fprintf(fs, "        id                  = %d\n", rb.id);
        fprintf(fs, "        [x, y, z]           = [% 3.6f, % 3.6f, % 3.6f] [m]\n", rb.pos.x, rb.pos.y, rb.pos.z);
        fprintf(fs, "        [roll, pitch, yaw]  = [% 3.6f, % 3.6f, % 3.6f] [rad]\n", rpy_rad.roll, rpy_rad.pitch, rpy_rad.yaw);
        fprintf(fs, "        [roll, pitch, yaw]  = [% 3.6f, % 3.6f, % 3.6f] [deg]\n", rpy_deg.roll, rpy_deg.pitch, rpy_deg.yaw);
        fprintf(fs, "        [qx, qy, qz, qw]    = [% 3.6f, % 3.6f, % 3.6f, % 3.6f] [quaternion]\n", rb.q.x, rb.q.y, rb.q.z, rb.q.w);
        fprintf(fs, "        nRigidMarkers       = %d\n", rb.nRigidMarkers);
        fprintf(fs, "        mean error          = % 3.6f [m]\n", rb.rigidMarkers_mean_error);

        npoint_rigid_marker_t rm;
        int j;

        for(j=0; j<rb.nRigidMarkers; j++)
        {
            np_packet_unpack_mocap_data_rigidMarker(&rm,&rb,j);

            fprintf(fs, "        rigidMarker[%d]:\n", j);
            fprintf(fs, "            id          = %d\n", rm.id);
            fprintf(fs, "            size        = %3.6f\n", rm.size);
            fprintf(fs, "            [x, y, z]   = [% 3.6f, % 3.6f, % 3.6f] [m]\n", rm.m.x, rm.m.y, rm.m.z);
        }

    }

    npoint_labeled_marker_t lm;

    for(i=0; i<md->nLabeledMarkers; i++)
    {
        np_packet_unpack_mocap_data_labeledMarker(&lm,md,i);

        fprintf(fs, "   labeledMarker[%d]:\n",i);
        fprintf(fs, "       id          =   %d\n", lm.id);
        fprintf(fs, "       size        =   %f\n", lm.size);
        fprintf(fs, "       [x, y, z]   =   [% 3.6f, % 3.6f, % 3.6f] [m]\n", lm.m.x, lm.m.y, lm.m.z);
    }

    fprintf(fs, "\n\n");
    fflush(fs);
}

void np_packet_printf_mocap_data_markerSet(FILE* fs, npoint_mocap_data_t* md, npoint_marker_set_t* ms)
{
    fprintf(fs, "markerSet: frame = %d, t_target = %f, t_local = %lu\n", md->frame_nr, md->latency, md->ts);
    fprintf(fs, "   name        = %s\n", ms->name);
    fprintf(fs, "   nMarkers    = %d\n", ms->nMarkers);

    int j;
    npoint_marker_t m;

    for(j=0; j< ms->nMarkers; j++)
    {
        np_packet_unpack_mocap_data_markerSet_marker(&m,ms,j);

        fprintf(fs, "       marker[%d]  = [% 3.6f, % 3.6f, % 3.6f] [m]\n",j,m.x,m.y,m.z);
    }
}

void np_packet_printf_mocap_data_markerSet_i(FILE* fs, npoint_mocap_data_t* md, int index)
{
    npoint_marker_set_t ms;
    np_packet_unpack_mocap_data_markerSet(&ms,md,index);
    np_packet_printf_mocap_data_markerSet(fs,md,&ms);
}

void np_packet_printf_mocap_data_markerSet_n(FILE* fs, npoint_mocap_data_t* md, const char* name)
{
    npoint_marker_set_t ms;

    if(np_packet_unpack_mocap_data_get_markerSet(&ms,md,name) == false)
    {
        fprintf(fs, "markerSet: Error, couldn't find markerSet with name '%s'\n",name);
    }

    np_packet_printf_mocap_data_markerSet(fs,md,&ms);
}

void np_packet_printf_mocap_data_rigidBody(FILE* fs, npoint_mocap_data_t* md, npoint_rigid_body_t *rb)
{
    utils_rpy_t rpy_rad, rpy_deg;

    utils_quaternion_to_rpy(&rpy_rad,&rb->q);
    utils_rpy_to_deg(&rpy_deg,&rpy_rad);

    char n[256];
    np_packet_unpack_mocap_data_get_rigidBody_name(n,rb,md);

    fprintf(fs, "rigidBody: frame = %d, t_target = %f, t_local = %lu\n", md->frame_nr, md->latency, md->ts);
    fprintf(fs, "   name                = %s\n", n);
    fprintf(fs, "   id                  = %d\n", rb->id);
    fprintf(fs, "   [x, y, z]           = [% 3.6f, % 3.6f, % 3.6f] [m]\n", rb->pos.x, rb->pos.y, rb->pos.z);
    fprintf(fs, "   [roll, pitch, yaw]  = [% 3.6f, % 3.6f, % 3.6f] [rad]\n", rpy_rad.roll, rpy_rad.pitch, rpy_rad.yaw);
    fprintf(fs, "   [roll, pitch, yaw]  = [% 3.6f, % 3.6f, % 3.6f] [deg]\n", rpy_deg.roll, rpy_deg.pitch, rpy_deg.yaw);
    fprintf(fs, "   [qx, qy, qz, qw]    = [% 3.6f, % 3.6f, % 3.6f, % 3.6f] [quaternion]\n", rb->q.x, rb->q.y, rb->q.z, rb->q.w);
    fprintf(fs, "   nRigidMarkers       = %d\n", rb->nRigidMarkers);
    fprintf(fs, "   mean error          = % 3.6f [m]\n", rb->rigidMarkers_mean_error);


    npoint_rigid_marker_t rm;
    int j;

    for(j=0; j<rb->nRigidMarkers; j++)
    {
        np_packet_unpack_mocap_data_rigidMarker(&rm,rb,j);

        fprintf(fs, "       rigidMarker[%d]: id = %d, size = %3.6f, [% 3.6f, % 3.6f, % 3.6f] [m]\n",
                j,
                rm.id,
                rm.size,
                rm.m.x,
                rm.m.y,
                rm.m.z
                );

    }
}

void np_packet_printf_mocap_data_rigidBody_i(FILE* fs, npoint_mocap_data_t* md, int index)
{
    npoint_rigid_body_t rb;
    np_packet_unpack_mocap_data_rigidBody(&rb,md,index);
    np_packet_printf_mocap_data_rigidBody(fs,md,&rb);
}

void np_packet_printf_mocap_data_rigidBody_n(FILE* fs, npoint_mocap_data_t* md, const char* name)
{
    npoint_rigid_body_t rb;

    if(np_packet_unpack_mocap_data_get_rigidBody(&rb,md,name) == false)
    {
        fprintf(fs, "rigidBody: Error, couldn't find rigidBody with name '%s'\n",name);
    }

    np_packet_printf_mocap_data_rigidBody(fs,md,&rb);
}


void np_packet_printf_packet_raw(FILE* fs, char* packet, int len)
{
    utils_fprint_hex_block(fs,packet,len);
}


void np_packet_printf_raw_with_sels(FILE *fs, char *packet,
                                        int len,
                                        unsigned char *sel, int sel_len)
{
    utils_fprint_hex_block_with_sels(fs,packet,len,sel,sel_len);
}

