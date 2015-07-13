#include "utils.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <sys/time.h>
#include <time.h>

#include <math.h>

// for directories
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

uint64_t utils_us_since_epoch()
{
    struct timeval tv;
    uint64_t micros = 0;

    gettimeofday(&tv, NULL);
    micros =  ((uint64_t)tv.tv_sec) * 1000000 + tv.tv_usec;

    return micros;
}


char* utils_get_date_time_str(char* buf)
{
    time_t t;
    struct tm* tinfo;

    time(&t);
    tinfo = localtime(&t);

    sprintf(buf,"%04d%02d%02d%02d%02d%02d",
            tinfo->tm_year+1900,
            tinfo->tm_mon+1,
            tinfo->tm_mday,
            tinfo->tm_hour,
            tinfo->tm_min,
            tinfo->tm_sec);

    return buf;
}


void utils_init_file(utils_file_t* uf,
                     const char* name,
                     const char* filename,
                     const char* dir_path)
{
    int slen;

    char* file_path;

    slen = strlen(dir_path) + strlen(filename) + 1;

    file_path = (char*) calloc(slen + 1, sizeof(char));

    strcpy(file_path,dir_path);
    strcat(file_path,"/");
    strcat(file_path,filename);

    uf->name = utils_alloc_str(name);
    uf->file_path = file_path;
}

void utils_deinit_file(utils_file_t* uf)
{
    if(uf->name != NULL)
    {
        free(uf->name);
    }

    if(uf->file_path != NULL)
    {
        free(uf->file_path);
    }
}

int utils_create_and_open_file(utils_file_t* uf)
{
    uf->fs = fopen(uf->file_path, "w");

    if(uf->fs == NULL)
    {
        return UTILS_OPEN_FILE_ERR;
    }

    return UTILS_OK;
}

void utils_close_file(utils_file_t* uf)
{
    if(uf->fs != NULL)
    {
        fclose(uf->fs);
    }
}

void utils_alloc_file_list(utils_file_list_t* list, int len)
{
    list->file = (utils_file_t*) calloc(len,sizeof(utils_file_t));
    list->len = len;
    list->index = 0;
}

void utils_free_file_list(utils_file_list_t* list)
{
    int i = 0;

    if(list->len != 0)
    {
        for(i=0; i<list->index; i++)
        {
            utils_deinit_file(&list->file[i]);
        }

        list->len = 0;
        list->index = 0;
        free(list->file);
    }
}


int utils_add_file_to_file_list(utils_file_list_t* list, utils_file_t* uf)
{
    if(list->index == list->len)
    {
        return UTILS_FULL_LIST_ERR;
    }

    memcpy(&list->file[list->index],uf,sizeof(utils_file_t));
    list->index++;

    return UTILS_OK;
}

int utils_find_file_in_file_list(utils_file_list_t *list, const char *file_name)
{
    int i = 0;

    for(i = 0; i<list->index; i++)
    {
        if(strcmp(list->file[i].name, file_name) == 0)
        {
            return i;
        }
    }

    return -1;
}

int utils_make_dir(const char *dir)
{
    if(mkdir(dir,0777) < 0)
    {
        return UTILS_MAKE_DIR_ERR;
    }

    return UTILS_OK;
}

int utils_copy_files_of_dir(const char* src_dir_path, const char* dest_dir_path)
{
    DIR* src_dir;

    // directory entry
    struct dirent * de;

    // file paths
    char src_file_path[1024];
    char dest_file_path[1024];

    // try to open src dir
    src_dir = opendir(src_dir_path);

    // open src dir failed
    if(src_dir == NULL)
    {
        printf("utils_copy_dir: open src dir failed.\n");
        return UTILS_OPEN_DIR_ERR;
    }

    // test if dest dir exists; if not try to make it
    if(utils_is_dir_accessible(dest_dir_path) == false)
    {

        if(utils_make_dir(dest_dir_path) != UTILS_OK)
        {
            closedir(src_dir);
            return UTILS_MAKE_DIR_ERR;
        }
    }

    // go throw files in src_dir and copy file to dest_dir
    // override existing files
    while((de = readdir(src_dir)) != NULL)
    {
        strncpy(src_file_path,  src_dir_path,   sizeof(src_file_path));
        strncat(src_file_path,  "/",            sizeof(src_file_path));
        strncat(src_file_path,  de->d_name,     sizeof(src_file_path));

        strncpy(dest_file_path, dest_dir_path,  sizeof(dest_file_path));
        strncat(dest_file_path, "/",            sizeof(dest_file_path));
        strncat(dest_file_path, de->d_name,     sizeof(dest_file_path));

        if(strcmp(de->d_name,"..") == 0 || strcmp(de->d_name,".") == 0)
        {
            continue;
        }

        // file status struct
        struct stat statbuf;

        // insure that directory entry is a file

        // get the file status
        if(stat(src_file_path, &statbuf) == -1)
        {
            continue;   // error while getting file status struct
        }

        // check if entry is a directory
        if(S_ISDIR(statbuf.st_mode))
        {
            continue;   // the entry is a directory
        }


        if(utils_copy_file(src_file_path,dest_file_path) != UTILS_OK)
        {
            //            printf("utils_copy_dir: copy file failed:\n");
            //            printf("    filename    = %s\n",de->d_name);
            //            printf("    src         = %s\n",src_file_path);
            //            printf("    dest        = %s\n",dest_file_path);
            closedir(src_dir);
            return UTILS_COPY_FILE_ERR;
        }
    }


    closedir(src_dir);
    return UTILS_OK;
}

int utils_delete_files_from_dir(const char* dir_path)
{
    DIR* dir;

    // directory entry
    struct dirent * de;

    // file paths
    char file_path[1024];

    // try to open src dir
    dir = opendir(dir_path);

    // open src dir failed
    if(dir == NULL)
    {
        printf("utils_delete_files_from_dir: open dir failed.\n");
        return UTILS_OPEN_DIR_ERR;
    }

    // go throw files in src_dir and delete file
    while((de = readdir(dir)) != NULL)
    {
        strncpy(file_path,  dir_path,       sizeof(file_path));
        strncat(file_path,  "/",            sizeof(file_path));
        strncat(file_path,  de->d_name,     sizeof(file_path));

        // ignore directory (current folder and parent folder)
        if(strcmp(de->d_name,"..") == 0 || strcmp(de->d_name,".") == 0)
        {
            continue;
        }

        // file status struct
        struct stat statbuf;

        // insure that directory entry is a file

        // get the file status
        if(stat(file_path, &statbuf) == -1)
        {
            // printf("dir '%s': file stat error = %s\n", dir_path, strerror(errno));
            continue;   // error while getting file status struct
        }

        // check if entry is a directory
        if(S_ISDIR(statbuf.st_mode))
        {
            continue;   // the entry is a directory
        }

        if(utils_delete_file(file_path) != UTILS_OK)
        {
            //            printf("utils_copy_dir: copy file failed:\n");
            //            printf("    filename    = %s\n",de->d_name);
            //            printf("    src         = %s\n",src_file_path);
            //            printf("    dest        = %s\n",dest_file_path);
            closedir(dir);
            return UTILS_DELETE_FILE_ERR;
        }

        // printf("deleted file '%s'.\n",de->d_name);
    }

    closedir(dir);
    return UTILS_OK;
}


int utils_is_dir_accessible(const char* dir_path)
{
    DIR* dir;

    dir = opendir(dir_path);
    if(dir == NULL)
    {
        return false;
    }

    closedir(dir);
    return true;
}


int utils_copy_file(const char* src_file_path, const char* dest_file_path)
{

    //    printf("copy file:\n");
    //    printf("    src         = %s\n",src_file_path);
    //    printf("    dest        = %s\n",dest_file_path);


    FILE* src_fs;
    FILE* dest_fs;
    char buf[4*1024];           //4KB chunks

    long size;
    size_t nr;                  //bytes read
    size_t nw;                  //bytes written

    // open src file in binary read-only mode
    src_fs = fopen(src_file_path,"rb");

    // src_fs failed
    if(src_fs == NULL)
    {
        return UTILS_OPEN_FILE_ERR;
    }

    // open src file in binary write-only mode
    dest_fs = fopen(dest_file_path,"wb");

    // open failed
    if(dest_fs == NULL)
    {
        fclose(src_fs);
        return UTILS_OPEN_FILE_ERR;
    }

    //    // get file size
    //    fseek(src_fs,0,SEEK_END);
    //    size = ftell(src_fs);
    //    rewind(src_fs);


    nr = fread(buf,1,sizeof(buf),src_fs);
    if(nr < sizeof(buf))
    {
        if(ferror(src_fs))
        {
            fclose(src_fs);
            fclose(dest_fs);
            return UTILS_READ_FILE_ERR;
        }
    }

    while(nr != 0)
    {
        // write all the bytes which have been read
        nw = fwrite(buf,1,nr,dest_fs);
        if(nw < nr)
        {
            if(ferror(dest_fs))
            {
                fclose(src_fs);
                fclose(dest_fs);
                return UTILS_WRITE_FILE_ERR;
            }
        }

        nr -= nw;

        while(nr != 0)
        {
            nw = fwrite(buf,1,nr,dest_fs);
            if(nw < nr)
            {
                if(ferror(dest_fs))
                {
                    fclose(src_fs);
                    fclose(dest_fs);
                    return UTILS_WRITE_FILE_ERR;
                }
            }

            nr -= nw;
        }

        // read new chunk of bytes
        nr = fread(buf,1,sizeof(buf),src_fs);
        if(nr < sizeof(buf))
        {
            if(ferror(src_fs))
            {
                fclose(src_fs);
                fclose(dest_fs);
                return UTILS_READ_FILE_ERR;
            }
        }
    }


    fclose(src_fs);
    fclose(dest_fs);
    return UTILS_OK;
}


int utils_delete_file(const char* file_path)
{
    int ret;

    ret = unlink(file_path);
    if(ret == -1)
    {
        return UTILS_DELETE_FILE_ERR;
    }

    return UTILS_OK;
}


void utils_alloc_handler_list(utils_handler_list_t* list, int len)
{
    list->handler = (utils_generic_handler_func_t*) calloc(len,sizeof(utils_generic_handler_func_t));
    list->len = len;
}

int utils_free_handler_list(utils_handler_list_t* list)
{
    if(list->handler != NULL)
    {
        free(list->handler);
        return UTILS_OK;
    }

    return UTILS_FREE_LIST_ERR;
}

int utils_add_handler(utils_handler_list_t* list, utils_generic_handler_func_t func)
{
    int i = 0;
    for(i = 0; i<list->len; i++)
    {
        //don't add a function to list twice
        if(list->handler[i] == func)
        {
            return UTILS_ALREADY_IN_LIST_ERR;
        }

        //use an empty slot in the handler list
        if(list->handler[i] == NULL)
        {
            list->handler[i] = func;
            return UTILS_OK;
        }
    }
    return UTILS_FULL_LIST_ERR;
}

int utils_rm_handler(utils_handler_list_t* list, utils_generic_handler_func_t func)
{
    int i = 0;

    for(i = 0; i<list->len; i++)
    {
        //don't add a function to list twice
        if(list->handler[i] == func)
        {
            list->handler[i] = NULL;
            return UTILS_OK;
        }
    }

    return UTILS_NOT_IN_LIST_ERR;
}

int utils_clear_handler_list(utils_handler_list_t* list)
{
    int i=0;

    for(i=0; i<list->len; i++)
    {
        list->handler[i] = NULL;
    }

    return UTILS_OK;
}

void utils_invoke_handlers(utils_handler_list_t* list, void* arg)
{
    int i = 0;

    for(i = 0; i<list->len; i++)
    {
        if(list->handler[i] != NULL)
        {
            list->handler[i](arg);
        }
    }
}

char* utils_alloc_str(const char* str)
{
    char *ptr;
    int len = strlen(str) + 1;

    ptr = (char*) calloc(len,sizeof(char));

    strcpy(ptr,str);

    return ptr;
}


void utils_alloc_str_list(utils_str_list_t* list, int list_len, int str_len)
{
    int i=0;

    list->str = (char**) malloc (sizeof(char*) * list_len);
    list->list_len = list_len;
    list->str_len = str_len;

    for(i=0; i<list->list_len; i++)
    {
        list->str[i] = (char*) malloc(sizeof(char) * str_len);
        list->str[i][0] = '\0';
    }
}


void utils_free_str_list(utils_str_list_t* list)
{
    int i=0;

    for(i=0; i<list->list_len; i++)
    {
        free(list->str[i]);
    }

    free(list->str);
}


void utils_quaternion_to_rpy(utils_rpy_t* rpy, const utils_quaternion_t* q)
{
    rpy->roll    = atan2(2*(q->x * q->y + q->w * q->z), q->w * q->w + q->x * q->x - q->y * q->y - q->z * q->z);
    rpy->pitch   = atan2(2*(q->y * q->z + q->w * q->x), q->w * q->w - q->x * q->x - q->y * q->y + q->z * q->z);
    rpy->yaw     = asin(-2*(q->x * q->z - q->w * q->y));
}


void utils_rpy_to_deg(utils_rpy_t* rpy_deg, const utils_rpy_t *rpy_rad)
{
    rpy_deg->roll   = (rpy_rad->roll    * 180) / M_PI;
    rpy_deg->pitch  = (rpy_rad->pitch   * 180) / M_PI;
    rpy_deg->yaw    = (rpy_rad->yaw     * 180) / M_PI;
}

void utils_fprint_hex_block(FILE* fs, char* data, int len)
{
    unsigned int i=0;

    unsigned char* d = data;

    for(i=0; i<len; i++)
    {
        fprintf(fs, " %02x",d[i]);

        if(((i+1) % 16) == 0 && i !=0)
        {
            fprintf(fs,"\n");
        }
    }

    if(((i+1) % 16) != 0)
    {
        fprintf(fs,"\n");
    }

    fflush(fs);
}

void utils_fprint_hex_block_with_sels(FILE* fs, char* data, int len,
                                      unsigned char* sel,
                                      int sel_len)
{
    unsigned int i=0 , j=0;

    int sel_flag = 0;
    unsigned char* d = data;

    for(i=0; i<len; i++)
    {
        sel_flag = 0;
        for(j=0;j<sel_len;j++)
        {
            if(sel[j] == i)
            {
                sel_flag = 1;
            }
        }

        if(sel_flag == 1)
        {
            fprintf(fs, " [%02x]",d[i]);
        }
        else
        {
            fprintf(fs, "  %02x ",d[i]);
        }

        if(((i+1) % 16) == 0 && i !=0)
        {
            fprintf(fs,"\n");
        }
    }

    if(((i+1) % 16) != 0)
    {
        fprintf(fs,"\n");
    }

    fflush(fs);
}

int utils_read_config_file(utils_config_file_param_t* params,
                           const char *filename)
{
    const int buf_len = 1024;

    utils_str_list_t sl;
    FILE* fs;
    int ret,i;
    char buf[buf_len];

    fs = fopen(filename,"r");

    if(fs == NULL)
    {
        //return open config file error
        return UTILS_OPEN_FILE_ERR;
    }

    utils_alloc_str_list(&sl,10,buf_len);



    // go through the file and read lines
    while(utils_get_next_file_line(buf,sizeof(buf),fs) != NULL)
    {
        // split line into parts using a set of delimiters
        ret = utils_split_str(&sl, buf, " \n\r\t");

        //empty line or incomplete line: continue
        if(ret == 0 || ret == 1)
        {
            continue;
        }

        //remove '"' of string arguments
        if(sl.str[1][0] == '"')
        {
            // allocate string because overlapping copy
            //      with strncpy is not allowed
            char* s = utils_alloc_str(sl.str[1]);
            char* ptr;

            // split the string and get the first non-empty
            //      string part
            ptr = strtok(s,"\"");

            // copy the string without '"' into the string list
            strncpy(sl.str[1],ptr,sl.str_len);

            free(s);
        }


        // parse the config params
        i = 0;
        while(params[i].name != NULL)       // as long as not END-OF-PARAM entry
        {
            //check if it is an valid parameter
            if(strcmp(params[i].name, sl.str[0]) == 0)
            {
                //printf("[i = %d]\n",i);

                // parse argument and write it to the refered param value
                if(strcmp(params[i].val_type,"string") == 0)
                {
                    // pointer a param string value
                    char** s =  ((char**)params[i].val);

                    // allocate memory and override refered str ptr
                    *s = utils_alloc_str(sl.str[1]);
                }
                else
                {
                    // pointer to a param int value
                    int* val =  ((int*)params[i].val);

                    // write new value to refered int value
                    *val = atoi(sl.str[1]);
                }
            }

            // go to next param entry
            i++;
        }



    }

    utils_free_str_list(&sl);
}

char* utils_get_next_file_line(char* buf, int len, FILE* fs)
{
    int end;

    // test for end of file (EOF) sign
    if(feof(fs) == true)
    {
        return NULL;
    }

    // try to get next line
    if(fgets(buf, len, fs) == NULL)
    {
        return NULL;
    }

    end = strlen(buf)-1;

    // if the last character of the line is '\n' then remove it
    if(buf[end] == '\n')
    {
        buf[end] = '\0';
    }

    return buf;
}

int utils_split_str(utils_str_list_t* sl,
                    const char* str,
                    const char* delimiters)
{
    int i=0;
    char* ptr;

    // needed because strtok modifies the input string
    char* buf = utils_alloc_str(str);

    strcpy(buf,str);

    // split string
    ptr = strtok (buf,delimiters);
    while (ptr != NULL)
    {
        if(i < sl->list_len)
        {
            // copy str part to string list
            strncpy(sl->str[i++],ptr,sl->str_len);
        }

        // get next pointer
        ptr = strtok (NULL, delimiters);
    }

    free(buf);

    return i;
}
