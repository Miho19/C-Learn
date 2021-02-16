#include <stdio.h>

#define MASK (512 -1)

int main(void) {
    int i;
    i=0;
    
    if( (i & MASK) != 0) {
        printf("i: %d\n", i);
    } else {
        printf("not\n");
    }
    return 0;
}