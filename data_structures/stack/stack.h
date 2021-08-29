#ifndef STACK_H_
#define STACK_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_STARTING_SIZE 10

struct stack {
    int top;
    unsigned int capacity;
    int *arr;
};

void push(struct stack *s, int item);
int pop(struct stack *s);
int top(struct stack *s);

int isEmpty(struct stack *s);


struct stack* stack_new();
struct stack* stack_destroy(struct stack *s);


void stack_print(struct stack *s);



#endif