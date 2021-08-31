#ifndef LINKED_LIST_H_
#define LINKED_LIST_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct node {
    struct node *next;
    void *data;
};

struct linked_list {
    struct node *first;
    struct node *last;
};

int list_append(struct linked_list *l, void *data, size_t size);
int list_remove(struct linked_list *l, int index);
/**int list_get(struct linked_list *l, int index); */

void list_print(struct linked_list *l, void (*fpr)(void *));

struct linked_list* list_new();
struct linked_list* list_destroy(struct linked_list *l);


#endif