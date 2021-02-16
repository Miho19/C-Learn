#ifndef _TEST_FILE_SYSTEM_H_
#define _TEST_FILE_SYSTEM_H_


#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 4096
#define BLOCK_SIZE 512
#define MASK (BLOCK_SIZE - 1)

typedef struct file {
    int off;
} file;

file *user_file;

FILE *f;

char *buffer_space;

void init(void);
void destroy(void);

int buffer_write(file *f, char *buf, int num);
int buffer_read(file *f, char *buf, int num);
int file_set_position(file *f, int n);

void buffer_to_file(void);



#endif