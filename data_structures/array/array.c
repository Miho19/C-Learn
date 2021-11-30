
#include "array.h"



struct array* array_new(int length, int(*compare_function)(void*, void*), void(*print_function)(void*), size_t element_size){
    struct array *newArr = 0;


    if(!print_function || !compare_function || !element_size){
        fprintf(stderr, "Must supply a print function, compare function and element size\n");
        exit(1);
    }
 
    newArr = malloc(sizeof(*newArr));

    
 
    newArr->length = length;
    newArr->data = 0;
    newArr->compare_function = compare_function;
    newArr->print_function = print_function;
    newArr->element_size = element_size;
   

    return newArr;

}

struct array * array_destroy(struct array *arr){
    int i;

    if(!arr) return 0;

    if(arr->data){
        for(i=0;i<arr->length;i++)
            free(arr->data[i]);

        free(arr->data);
    }
    

    free(arr);
    arr = 0;
    return arr;
}


static int _array_print(void ** data, int length, void (*print_function)(void *)){
    int i;

    for(i=0;i<length;i++)
        print_function(data[i]);

    return 0;
}

int array_print(struct array* arr){
   
   if(!arr) return 1;
    
   if(!arr->data) return 1;

    return _array_print(arr->data, arr->length, arr->print_function);
}