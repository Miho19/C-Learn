#ifndef _TEST_FILE_SYSTEM_H_
#define _TEST_FILE_SYSTEM_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file_system_error.h"


#define BLOCK_SIZE 512
#define MAX_BUFFER_SIZE (BLOCK_SIZE * BLOCK_SIZE)
#define MASK (BLOCK_SIZE - 1)

#define BASE_OFFSET BLOCK_SIZE
#define MAX_FILE_AMOUNT 30

extern int BUFFER_SIZE;
extern int fd_counter;

#define FILE_APPEND 0
#define FILE_OVERWRITE 1

#define MAX_FILE_NAME 14
#define BITS_IN_A_BYTE 8

/**
 *  first 24 bytes for superblock
 */

typedef struct superblock {
    int type;
    int size;
    int MAX_INODE_NUMBER;
    int index_inode;
    int index_bitmap;
    int index_data;
    
} superblock;

/**  24 */
/** size -> 24*/

typedef struct direntry {
    int inode_number;
    char name[MAX_FILE_NAME];
} direntry;


typedef enum {
    text_file, directory
} file_type;

/** 16 */
typedef struct inode {
    file_type type;
    int inode_number;
    int used;
    int file_size;
    int data_index;    
} inode;

typedef struct user_file {

    int off;
    int inode_number;
    int data_length;
    char *name;
    char *data;
    
} user_file;

char *path;

typedef struct sequence {
    char *string;
    int length;
} sequence;

typedef struct command_only {
    char *command_name;
    int (*command_function)(void);
} command_only;


typedef struct file_request {
    char *command_name;
    int (*file_function)(char **path_split, int items_split);
} file_request;

typedef struct data_request {
    char *command_name;
    int (*data_function)(char **path_split, int items_split, sequence *file_data, int mode);
} data_request;

typedef enum {
    COMMAND_ONLY,
    FILE_REQUEST,
    DATA_REQUEST,
    PENDING
} request_type;

typedef struct request {
    int status;
    int command_index;
    request_type type;
    sequence *file_command;
    sequence *file_name;
    sequence *file_data;
} request;

request *rq;

FILE *f;

void init(int size);
void destroy(void);



int run(void);

/** User Commands */

/** Command only */

int ls(void);
int superblock_print(void);
int open_files_print_all(void);


/** file request */

int file_create(char **path_split, int items_split);
int cat(char **path_split, int items_split);


/** Dir request */
int mkdir(char **path_split, int items_split);
int cd(char **path_split, int items_split);


/** data request */
int file_write(char **path_split, int items_split, sequence *file_data, int mode);



int path_convert_to_full_path(sequence *user_path, char **path_split);



#endif