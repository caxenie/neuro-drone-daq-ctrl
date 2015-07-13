#ifndef NPOINT_PACKET_H
#define NPOINT_PACKET_H


#define NPOINT_PACKET_ID_MOCAP_DATA             0x07
#define NPOINT_PACKET_ID_DATA_DESCRIPTIONS      0x05

#define NPOINT_PACKET_VER_MAJOR                     3
#define NPOINT_PACKET_VER_MINOR                     0

#include <stdint.h>
#include <stdio.h>

#include "npoint_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int np_packet_unpack_mocap_data(npoint_mocap_data_t* p, char* packet, int len);

// markerSets
char* np_packet_unpack_mocap_data_skip_markerSets(char* ptr, uint32_t nMarkerSets);
char* np_packet_unpack_mocap_data_skip_markerSet(char* ptr);
char* np_packet_unpack_mocap_data_skip_markers(char* ptr, uint32_t nMarkers);

void np_packet_unpack_mocap_data_markerSet(npoint_marker_set_t* ms, npoint_mocap_data_t* md, int index);
void np_packet_unpack_mocap_data_markerSet_marker(npoint_marker_t* m, npoint_marker_set_t* ms, int index);
void np_packet_unpack_mocap_data_marker(npoint_marker_t* m, char* marker_list, int index);

int np_packet_unpack_mocap_data_find_markerSet(npoint_mocap_data_t* md, const char *name);
int np_packet_unpack_mocap_data_get_markerSet_nameList(utils_str_list_t* nameList, npoint_mocap_data_t *md);
int np_packet_unpack_mocap_data_get_markerSet(npoint_marker_set_t* ms, npoint_mocap_data_t* md, const char *name);

// otherMarkers
char* np_packet_unpack_mocap_data_skip_otherMarkers(char* ptr, uint32_t nOtherMarkers);
void np_packet_unpack_mocap_data_otherMarker(npoint_marker_t* m, npoint_mocap_data_t* md, int index);

// rigidBodies
char* np_packet_unpack_mocap_data_skip_rigidBodies(char* ptr, uint32_t nRigidBodies);
char* np_packet_unpack_mocap_data_skip_rigidBody(char* ptr);
char* np_packet_unpack_mocap_data_skip_rigidMarkers(char* ptr, uint32_t nRigidMarkers);
char* np_packet_unpack_mocap_data_skip_rigidMarkers_markers(char* ptr, uint32_t nRigidMarkers);
char* np_packet_unpack_mocap_data_skip_rigidMarkers_marker_ids(char* ptr, uint32_t nRigidMarkers);
char* np_packet_unpack_mocap_data_skip_rigidMarkers_marker_sizes(char* ptr, uint32_t nRigidMarkers);

void np_packet_unpack_mocap_data_rigidBody(npoint_rigid_body_t* rb, npoint_mocap_data_t* md, int index);
void np_packet_unpack_mocap_data_rigidBody_mean_error(float* mean_error, npoint_rigid_body_t* rb);

void np_packet_unpack_mocap_data_rigidMarker(npoint_rigid_marker_t* rm, npoint_rigid_body_t* rb, int index);
void np_packet_unpack_mocap_data_rigidMarker_marker(npoint_marker_t* m, npoint_rigid_body_t* rb, int index);
void np_packet_unpack_mocap_data_rigidMarker_marker_id(uint32_t* id, npoint_rigid_body_t* rb, int index);
void np_packet_unpack_mocap_data_rigidMarker_marker_size(float* size, npoint_rigid_body_t* rb, int index);

int np_packet_unpack_mocap_data_get_rigidBody_nameList(utils_str_list_t* nameList, npoint_mocap_data_t *md);
int np_packet_unpack_mocap_data_get_rigidBody(npoint_rigid_body_t* rb, npoint_mocap_data_t* md, const char *name);
void np_packet_unpack_mocap_data_get_rigidBody_name(char* name, npoint_rigid_body_t* rb, npoint_mocap_data_t* md);

// skeletons
char* np_packet_unpack_mocap_data_skip_skeletons(char* ptr, uint32_t nSkeletons);

// labeledMarkers
char* np_packet_unpack_mocap_data_skip_labeledMarkers(char* ptr, uint32_t nLabeledMarkers);
char* np_packet_unpack_mocap_data_skip_labeledMarker(char* ptr);

void np_packet_unpack_mocap_data_labeledMarker(npoint_labeled_marker_t* lm, npoint_mocap_data_t* md, int index);


// fprintf functions
void np_packet_printf_mocap_data(FILE* fs, npoint_mocap_data_t* md);
void np_packet_printf_mocap_data_markerSet(FILE* fs, npoint_mocap_data_t *md, npoint_marker_set_t *ms);
void np_packet_printf_mocap_data_markerSet_i(FILE* fs, npoint_mocap_data_t* md, int index);
void np_packet_printf_mocap_data_markerSet_n(FILE* fs, npoint_mocap_data_t* md, const char* name);

void np_packet_printf_mocap_data_rigidBody(FILE* fs, npoint_mocap_data_t* md, npoint_rigid_body_t *rb);
void np_packet_printf_mocap_data_rigidBody_i(FILE* fs, npoint_mocap_data_t* md, int index);
void np_packet_printf_mocap_data_rigidBody_n(FILE* fs, npoint_mocap_data_t* md, const char* name);

void np_packet_printf_packet_raw(FILE* fs, char* packet, int len);
void np_packet_printf_raw_with_sels(FILE* fs, char* packet, int len, unsigned char *sel, int sel_len);


#ifdef __cplusplus
}
#endif

#endif // NPOINT_PACKET_H
