#ifndef QUEUE_H_
#define QUEUE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "linked_list.h"


struct queue {
    int length;
    size_t element_size;
    void (*print_function)(void *);
    struct linked_list *list;
};

void enqueue(struct queue *q, void *data);
void dequeue(struct queue *q);

void queue_print(struct queue *q);

struct queue* queue_new();
struct queue* queue_destroy(struct queue* q);

#endif