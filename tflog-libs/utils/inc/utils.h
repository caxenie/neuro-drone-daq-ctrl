#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <unistd.h>

#include "utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif



uint64_t utils_us_since_epoch();
char* utils_get_date_time_str(char* buf);

// Attention: allocates memory at heap; call deinit to free memory
void utils_init_file(utils_file_t* uf,
                      const char* name,
                      const char* filename,
                      const char* dir_path);

void utils_deinit_file(utils_file_t* uf);

// creates (overrides) a text file
int utils_create_and_open_file(utils_file_t* uf);
void utils_close_file(utils_file_t* uf);


// Attention: memory is allocated at heap; must be freed when it's no longer used!
void utils_alloc_file_list(utils_file_list_t* list, int len);
void utils_free_file_list(utils_file_list_t* list);

// files are added to the list in an ordered manner
int utils_add_file_to_file_list(utils_file_list_t* list, utils_file_t* uf);
int utils_find_file_in_file_list(utils_file_list_t *list, const char *file_name);

// directory related functions
int utils_make_dir(const char *dir);
int utils_copy_files_of_dir(const char* src_dir_path, const char* dest_dir_path);
int utils_delete_files_from_dir(const char* dir_path);

int utils_is_dir_accessible(const char* dir_path);

int utils_copy_file(const char* src_file_path, const char* dest_file_path);
int utils_delete_file(const char* file_path);

// alloc/free memory for list at heap
void utils_alloc_handler_list(utils_handler_list_t* list, int len);
int utils_free_handler_list(utils_handler_list_t* list);

// handlers are added to the list in an unordered manner
//      empty slots are marked with NULL ptr
int utils_add_handler(utils_handler_list_t* list, utils_generic_handler_func_t func);
int utils_rm_handler(utils_handler_list_t* list, utils_generic_handler_func_t func);
int utils_clear_handler_list(utils_handler_list_t* list);

void utils_invoke_handlers(utils_handler_list_t* list, void* arg);

// alloc/free a string list
//      list_len:   number of strings in list (number of char* pointers)
//      str_len:    number of chars in a string (number of chars in char* entry)
void utils_alloc_str_list(utils_str_list_t* list, int list_len, int str_len);
void utils_free_str_list(utils_str_list_t* list);

//euler-y-convention (z,y',z''); rpy in [rad]
void utils_quaternion_to_rpy(utils_rpy_t* rpy, const utils_quaternion_t *q);
void utils_rpy_to_deg(utils_rpy_t* rpy_deg, const utils_rpy_t* rpy_rad);

char* utils_alloc_str(const char* str);


void utils_fprint_hex_block(FILE* fs, char* data, int len);
void utils_fprint_hex_block_with_sels(FILE* fs, char* data, int len,
                                      unsigned char* sel,
                                      int sel_len);

// reads a config file and stores the parameters into variables
//      as defined by the param list
//      param list entry:   {"<PARAM name>", <ptr to variable>, "<param-type>"},
int utils_read_config_file(utils_config_file_param_t* params,
                           const char *filename);

// get the next line from an opened text file
char* utils_get_next_file_line(char* buf, int len, FILE* pFile);

// split string into substrings
//      sl:             a pre-allocated string list
//      str:            the input string to split
//      delimiters:     the chars which split the string
//      return:         number of string parts
int utils_split_str(utils_str_list_t* sl,
                    const char* str,
                    const char* delimiters);

#ifdef __cplusplus
}
#endif

#endif // UTILS_H
