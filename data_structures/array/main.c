#include "array.h"



int compare_func(void *a, void *b){

    if(*(int*)a == *(int*)b){
        return 0;
    }
    if(*(int*)a > *(int*)b){
        return 1;
    }
    if(*(int*)a < *(int*)b){
        return -1;
    }

    return 0;
}

void print_func(void *data){
    printf("%d\n", *(int*)data);
}


int main(void){

struct array *arr = 0;

    
    arr = array_new(1, compare_func, print_func, sizeof(int));

    array_destroy(arr);

    return 0;
}