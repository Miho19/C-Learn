#ifndef LINKER_H_
#define LINKER_H_




typedef struct node {
    int item;
     struct node* next;
}node;

typedef struct list {
    node* first;
    int length;
}list;

extern list* list_new();
extern list* list_free();

extern void node_add(list* l, int value);


extern void list_print(list* l);



#endif