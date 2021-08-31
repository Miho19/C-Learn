#include "queue.h"

void print_int(void *data){
    printf("%d\n", *(int*)(data));
}


int main(void){
    struct queue *q = queue_new(print_int, sizeof(int));

    int i;

    for(i=0;i<3;i++){
        enqueue(q, &i);
    }


    queue_print(q);

    q = queue_destroy(q);


    return 0;
}