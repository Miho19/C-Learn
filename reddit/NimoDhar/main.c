#include <stdio.h>
#include <stdlib.h>

void question1(int *a, int a_length, int *b, int b_length) {
    int i;

    int a_count;
    int b_count;

    a_count = 0;
    b_count = 0;

    for(i=0;i<a_length;i++){
        if(a[i] % 2 == 0)
            a_count++;
    }

    for(i=0;i<b_length;i++){
        if(b[i] % 2 == 0)
            b_count++;
    }

    if(a_count > b_count) {
        for(i=0;i<a_length;i++){
            printf("%d ", a[i]);
        }
    } else {
        for(i=0;i<b_length;i++){
            printf("%d ", b[i]);
        }
    }
    printf("\n");

}

int main(){
    int a[] = {0, 2, 54, 55, 3, 4, 6, 34};
    int b[] = {2, 4, 6, 8, 10, 12, 14, 16};

    question1(a, sizeof(a) / sizeof(a[0]), b, sizeof(b) / sizeof(b[0]));

    return 0;
}