#include "stack.h"


int main(void){
    int i;

    struct stack *s = stack_new();
    
    
    push(s, 0);
   
    push(s, 1);
    
    push(s, 2);
    
    stack_print(s);
    
    printf("top: %d\n", top(s));

    

    stack_destroy(s);
    return 0;
}