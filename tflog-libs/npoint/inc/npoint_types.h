#ifndef NPOINT_TYPES_H
#define NPOINT_TYPES_H

#include "utils_types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef utils_3d_vec_t npoint_marker_t;

typedef struct
{
    npoint_marker_t m;

    uint32_t id;
    float size;

}npoint_rigid_marker_t;


typedef struct
{
    uint32_t id;
    npoint_marker_t m;
    float size;

}npoint_labeled_marker_t;


typedef struct
{
    int index;
    char name[256];

    uint32_t nMarkers;
    char* markers;

} npoint_marker_set_t;


typedef struct
{
    int index;
    uint32_t id;

    utils_3d_vec_t pos;
    utils_quaternion_t q;

    uint32_t nRigidMarkers;
    char* rigidMarkers;
    float rigidMarkers_mean_error;

} npoint_rigid_body_t;

typedef struct
{
    uint64_t ts;

    uint16_t id;
    uint16_t size;
    uint32_t frame_nr;
    uint32_t nMarkerSets;
    uint32_t nOtherMarkers;
    uint32_t nRigidBodies;
    uint32_t nSkeletons;
    uint32_t nLabeledMarkers;

    char* markerSets;
    char* otherMarkers;
    char* rigidBodies;
    char* skeletons;
    char* labeledMarkers;

    float latency;
    uint32_t timecode;
    uint32_t timecodeSub;
    uint32_t endOfData;

} npoint_mocap_data_t;


#ifdef __cplusplus
}
#endif


#endif // NPOINT_TYPES_H
