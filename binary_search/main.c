#include <stdio.h>

#define ARRAY_SIZE 1000


void array_print(int *arr, int n){
   int i;
   for(i=0;i<n;i++)
    printf("%d\n", arr[i]);
}

void insertion_sort(int *arr, int n) {
    int i;
    int j;
    int key;
    key = 0;
    for(i=1;i<n;i++){
        key = arr[i];
        for(j=i - 1; j >= 0 && key < arr[j];j--){
            arr[j+1] = arr[j];
        }
        arr[j+1] = key;
    }
}

int binsearch(int *arr, int lower, int upper, int query) {
    int mid;
    mid = 0;

    if(upper < lower)
        return 0;
    mid = (int)(lower + (upper-lower)/2);
    if(arr[mid] == query)
        return 1;
    return arr[mid] > query ? binsearch(arr, lower, mid - 1, query) : binsearch(arr, mid + 1, upper, query);

}


int main(void) {
    int my_array[ARRAY_SIZE];
    int count;

    count = 0;

    while(count < ARRAY_SIZE && 1 == scanf("%d", &my_array[count]))
        count++;

    insertion_sort(my_array, count);

    printf("%d: %d\n", 1, binsearch(my_array, 0, count, 1));


    return 0;
}