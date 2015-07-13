#ifndef NPOINT_LOGGER_H
#define NPOINT_LOGGER_H


#include <stdio.h>

#include "npoint_types.h"
#include "utils_types.h"


#ifdef __cplusplus
extern "C" {
#endif




typedef struct
{
    const char* name;
    char* dir_path;
    int files_opened;
    utils_file_list_t list;

} np_logger_t;


/**

   ATTENTION:

        As of now: np_logger can only log rigid body sets


  */

// Attention: allocates memory at heap; call deinit to free memory
void np_logger_init(np_logger_t* npl, const char *dir_path);
void np_logger_deinit(np_logger_t* npl);

// initializes, creates and open files according to the content of the mocap data packet
//      - a file list is allocated and initialized
//      - all the files of the file list are created and opened
int np_logger_create_and_open_files(np_logger_t* npl, npoint_mocap_data_t* md);

// closes all files of the file list
void np_logger_close_files(np_logger_t* npl);

// Attention: allocates memory at heap; free it if you no longer need the list
int np_logger_alloc_and_init_file_list(utils_file_list_t* list, npoint_mocap_data_t* md, const char* dir_path);


// writes rigid body data to the right logging file
int np_logger_std_write_rigidBody(np_logger_t* npl, npoint_mocap_data_t* md, npoint_rigid_body_t* rb);

#ifdef __cplusplus
}
#endif


#endif // NPOINT_LOGGER_H
