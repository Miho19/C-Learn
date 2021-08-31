#include "queue.h"


struct queue* queue_new(void (*print_func)(void *), size_t element_size) {
    
    
    struct queue *q = 0;

    if(!print_func || !element_size){
        fprintf(stderr, "Must supply print function and element size\n");
        exit(1);
    }

    q = malloc(sizeof(struct queue));

    if(!q) {
        fprintf(stderr, "Memory allocation error q\n");
        exit(1);
    }

    q->length   = 0;
    q->list     = list_new();
    q->element_size = element_size;
    q->print_function = print_func;

    if(!q->list){
        free(q);
        fprintf(stderr, "Memory allocation error q->list\n");
        exit(1);
    }

    return q;
}

struct queue* queue_destroy(struct queue* q){
    
    if(!q)
        return 0;
    list_destroy(q->list);
    free(q);
    q = 0;
    return q;
}

void enqueue(struct queue *q, void *data) {
    list_append(q->list, data, q->element_size);
    q->length++;
}

void dequeue(struct queue *q){
    list_remove(q->list, 0);
    q->length--;
}

void queue_print(struct queue *q){
    list_print(q->list, q->print_function);
}