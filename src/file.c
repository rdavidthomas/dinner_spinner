#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include "file.h"

#define FILE_MAGIC 0x4D45414C  //MEAL in hex

struct file_header_s {
    uint32_t magic;
    uint32_t num_of_meals;
    uint32_t offset_meals;
};

#define MOD_DEBUG 1
#define PDBG(x,...) if(MOD_DEBUG) { printf("[DEBUG] " x "\n", ##__VA_ARGS__); }


static meal_list_t _deserilize_meals(char* rawdata, size_t rawdatasize, uint32_t num_of_meals) {
    //really simple now.  Someday use json or something to expand fields, rather then a custom file format.
    meal_list_t list = (meal_list_t)rawdata;
    return list;
}

static int _read_file_header(FILE* fp, struct file_header_s* header) {
    int retval = fread(header, sizeof(struct file_header_s), 1, fp);
    if(retval != 1) {
        PDBG("Failed to read header");
        return -1;
    }
    if( header->magic != FILE_MAGIC ) {
        PDBG("File magic number is invalid.");
        return -1;
    }
    return 0;
}

meal_list_t load_meal_list(char* fname) {
    struct stat info;
    struct file_header_s header;
    char* rawdata = NULL;
    size_t data_size = 0;
    FILE* fp = NULL;
    meal_list_t list = NULL;
    int retval = 0;
    if( stat(fname, &info) < 0 ) 
        { goto errexit;}
    PDBG("File size %d", info.st_size);
    
    fp = fopen(fname, "rb");
    if(fp == NULL)
        { goto errexit; }
    
    if( _read_file_header(fp,&header) < 0)
        { goto errexit; }
    
    
    data_size = info.st_size - sizeof(struct file_header_s);
    PDBG("%d meals in %d byte file", header.num_of_meals, data_size);
    
    if((!data_size) || (!header.num_of_meals)) {
        PDBG("No meals in file.");
        goto errexit;
    }
    
    list = calloc(data_size,1);
    if(list == NULL){
        goto errexit;
    }
    
    retval = fread(list, 1, data_size, fp);
    if(retval != data_size) {
        PDBG("Failed to read raw data");
        goto errexit;
    }
    
    
    fclose(fp);
    return list;
    errexit:
        if(fp) fclose(fp);
        return NULL;
}

int save_meal_list(meal_list_t meal_list, int number_of_meals, char* fname) {
    
}

int append_meal(struct meal_s* meal, char* fname) {
    struct stat info;
    struct file_header_s header;
    FILE* fp = NULL;
    int retval = 0;
    int create_file = 0;
    errno = 0;
    if( stat(fname, &info) < 0 ) 
    { 
        if( errno == ENOENT ) {
            create_file = 1;
        }
        else {
            goto errexit;
        }
    }
    else {
        if(info.st_size == 0) {
            create_file = 1;
        }
        PDBG("File size %d", info.st_size);
    }
    
    fp = fopen(fname, "r+b");
    if(fp == NULL)
        { goto errexit; }
    
    if( create_file ){
        header.magic = FILE_MAGIC;
        header.num_of_meals = 1;
        header.offset_meals = 0;
        retval = fwrite(&header, 1, sizeof(struct file_header_s), fp);
        if(retval != sizeof(struct file_header_s)) {
            PDBG("Failed to write header to new file");
            goto errexit;
        }
    }
    else {
        if( _read_file_header(fp,&header) < 0)
            { goto errexit; }
    }
    
    fseek(fp,0,SEEK_END);
    
    retval = fwrite(meal, 1, sizeof(struct meal_s), fp);
    if( retval != sizeof(struct meal_s)) {
        PDBG("Failed to write meal to file");
        goto errexit;
    }
    
    if(!create_file) {
        fseek(fp,0,SEEK_SET);
        header.num_of_meals++;
        retval = fwrite(&header, 1, sizeof(struct file_header_s), fp);
        if(retval != sizeof(struct file_header_s)) {
            PDBG("Failed to write header");
            goto errexit;
        }
    }
    fclose(fp);
    return 0;
        errexit:
        if(fp) fclose(fp);
        return -1;
}

int release_loaded_meal_list(meal_list_t meal_list) {
    if(meal_list != NULL) {
        free( meal_list );
    }
    return 0;
}