#include "stack.h"


static void *emalloc(size_t s){
    void *result = 0;
    result = malloc(s);
    if(!result){
        fprintf(stderr, "Memory allocation error\n");
        exit(1);
    }
    return result;
}

struct stack* stack_new(){
    struct stack *s = 0;
    s = emalloc(sizeof(struct stack));
    s->top = 0;
    s->arr = 0;
    s->capacity = 0;

    s->arr = emalloc(DEFAULT_STARTING_SIZE * sizeof(s->arr[0]));
    s->capacity = DEFAULT_STARTING_SIZE;
    return s;
}

struct stack* stack_destroy(struct stack *s){
    if(!s)
        return 0;
    if(s->arr)
        free(s->arr);
    free(s);
    s = 0;
    return s;
}


void push(struct stack *s, int item) {
    
    if(!s)
        return;
    
    if(s->capacity == s->top){
        s->arr = realloc(s->arr, (s->capacity + DEFAULT_STARTING_SIZE) * sizeof(s->arr[0]));
        s->capacity += DEFAULT_STARTING_SIZE;       
    }
    
    s->arr[s->top] = item;
    s->top++;
}

int pop(struct stack *s){
    
    int item = 0;

    if(!s)
        return 0;

    item = s->arr[s->top];

    s->top--;

    return item;
}


void stack_print(struct stack* s){
    int i;
    int cursor;

    if(!s)
        return;
    
    for(i=0, cursor = 0;i < s->top; i++, cursor++){
        
        if(cursor == 10) {
            printf("\n");
            cursor = 0;
        }

        printf("%d ", s->arr[i]);        
    }

    printf("\n");
}


int isEmpty(struct stack *s){
    return s->top > 0 ? 0 : 1;
}


int top(struct stack *s){
    int item = s->arr[s->top - 1];
    return item;
}