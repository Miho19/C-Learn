#ifndef _TEST_FILE_SYSTEM_H_
#define _TEST_FILE_SYSTEM_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define BLOCK_SIZE 512
#define MAX_BUFFER_SIZE (BLOCK_SIZE * BLOCK_SIZE)
#define MASK (BLOCK_SIZE - 1)

#define BASE_OFFSET BLOCK_SIZE
#define MAX_FILE_AMOUNT 30

extern int BUFFER_SIZE;
extern int fd_counter;

#define FILE_APPEND 0
#define FILE_OVERWRITE 1


/**
 *  first 24 bytes for superblock
 */

typedef struct superblock {
    int type;
    int size;
    int number_of_blocks;
    int index_inodes;
    int number_inodes_max;
    int index_data;
    unsigned char data_block_free_list;
} superblock;

/**  24 */
/** size -> 24*/

typedef struct direntry {
    int inode_number;
    char name[14];
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

typedef struct mem_inode {
    int fd;
    user_file *file;
    inode *user_inode;
} mem_inode;

direntry *current_dir;
char *path;


FILE *f;

void init(int size);
void destroy(void);
int file_write(int fd, const void *data, int mode);
int file_read(int fd);
void file_print_superblock(void);
void file_print_all_files(void);

int file_open(char *name);

int ls(void);



#endif