#ifndef UTILS_TYPES_H
#define UTILS_TYPES_H

#include <stdio.h>


#define UTILS_CONFIG_FILE_PARAM_END       {NULL,NULL,NULL}

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    UTILS_OK = 0,

    UTILS_OPEN_FILE_ERR                         = -1,
    UTILS_READ_FILE_ERR                         = -2,
    UTILS_WRITE_FILE_ERR                        = -3,
    UTILS_COPY_FILE_ERR                         = -4,
    UTILS_DELETE_FILE_ERR                       = -5,

    UTILS_OPEN_DIR_ERR                          = -10,
    UTILS_MAKE_DIR_ERR                          = -11,


    UTILS_ALLOC_LIST_ERR                        = -20,
    UTILS_FREE_LIST_ERR                         = -21,
    UTILS_FULL_LIST_ERR                         = -22,

    UTILS_ALREADY_IN_LIST_ERR                   = -30,
    UTILS_NOT_IN_LIST_ERR                       = -31

} utils_error_t;



typedef void *(*utils_generic_handler_func_t) (void *);


typedef struct
{
    char* name;
    char* file_path;
    FILE* fs;

} utils_file_t;

typedef struct
{
    utils_file_t* file;
    int index;                     // index to next free element
    int len;

} utils_file_list_t;

typedef struct
{
    utils_generic_handler_func_t* handler;
    int len;
} utils_handler_list_t;


typedef struct
{
    char** str;
    int list_len;   //number of strings in list (number of char* pointers)
    int str_len;    //number of characters in string (number of chars in char* entry)
}utils_str_list_t;


typedef struct
{
    float x;
    float y;
    float z;
}utils_3d_vec_t;

typedef struct
{
    float x;
    float y;
    float z;
    float w;
}utils_quaternion_t;


typedef struct
{
    float roll;
    float pitch;
    float yaw;
}utils_rpy_t;


typedef struct
{
    const char* name;
    void* val;
    const char* val_type;
}utils_config_file_param_t;


#ifdef __cplusplus
}
#endif




#endif // UTILS_TYPES_H
