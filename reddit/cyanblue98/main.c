#include <stdio.h>
#include <stdlib.h>

#define N 10000

int main()
{
    int i;
    int j;

    long *p[N];

    for(i=0;i<N;i++) {
        p[i] = malloc(N * sizeof(*p[0]));
        if(!p[i]){
            printf("error alloc at p[%d] \n", i);
            exit(0);
        }
        for(j=0;j<N;j++)
            p[i][j] = 0;
    }

    
    for(i=0;i<N;i++){
        for(j=0;j<N;j++)
            p[i][j] = 1;
    }

    


    for(i=0;i<N;i++)
        free(p[i]);
  

    return 0;
}