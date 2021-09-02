#include "bst.h"

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
    int i;

    struct bst *b = bst_new(print_func, compare_func, sizeof(int));
    i = 10;
    bst_add(b, &i);

    i = 1000;
    bst_add(b, &i);

    i = 9;
    bst_add(b, &i);

    i = 1002;
    bst_add(b, &i);

    i = 99;
    bst_add(b, &i);

    i = 1000;
    
    bst_delete(b, &i);
    
    bst_print(b);

    bst_destroy(b);
    
    return 0;
}