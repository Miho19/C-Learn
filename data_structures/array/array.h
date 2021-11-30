#ifndef _ARRAY_
#define _ARRAY_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct array {

    void **data;
    size_t element_size;
    int (*compare_function)(void *, void*);
    int length;
    void (*print_function)(void*);
};

struct array* array_new(int length, int(*compare_function)(void*, void*), void(*print_function)(void*), size_t element_size);
struct array * array_destroy(struct array *arr);

int array_print(struct array* arr);


#endif