#include <stdio.h>
#include <stdlib.h>

typedef struct node {
    struct node *next;
    char *string;
} node;

typedef struct list {
     node *head;
     int length;
} list;

void list_new(list **l) {
    if((*l)->head == NULL)
        printf("hello world\n");
}


int main(void) {
    list *l;
    

    l = NULL;

    l = malloc(sizeof(*l));
    l->head = NULL;

    list_new(&l);

    free(l);

    return 0;
}