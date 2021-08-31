#include "linked_list.h"

/**
 * int list_get(struct linked_list *l, int index);
*/

int list_append(struct linked_list *l, void *data, size_t size){
    struct node *new_node = 0;

    if(!l)
        return 0;

    new_node = malloc(sizeof(*new_node));

    if(!new_node){
        fprintf(stderr, "Error allocating memory for new_node\n");
        return 0;
    }

    new_node->data = 0;
    new_node->next = 0;

    new_node->data = malloc(size);
    if(!new_node->data){
        fprintf(stderr, "Error allocating memory for new_node data\n");
        free(new_node);
        return 0;
    }

    memcpy(new_node->data, data, size);

    if(l->first == 0) {
        l->first = l->last = new_node;
        return 1;
    }

    l->last->next = new_node;
    l->last = new_node;
    return 1;
}


int list_remove(struct linked_list *l, int index){
    
    struct node *curr = l->first;
    struct node *prev = l->first;

    int i = 0;

    if(!l)
        return 0;


    if(index == 0){
        curr = l->first;
        l->first = curr->next;
        free(curr->data);
        free(curr);
        return 1;
    }
    
    while(i != index && curr->next != 0){
        prev = curr;
        curr = curr->next;
        i++;
    }

    if(curr == 0 || index != i)
        return 0;

    prev->next = curr->next;

    free(curr->data);
    free(curr);
    return 1;    
}





struct linked_list* list_new(){
    struct linked_list *list = malloc(sizeof(struct linked_list));

    if(!list){
        fprintf(stderr, "Memory allocation error list\n");
        return 0;
    }

    list->first     = 0;
    list->last      = 0;
    return list;
}
struct linked_list* list_destroy(struct linked_list *l){
    
    struct node *curr;
    struct node *temp;

    if(!l)
        return 0;
    
    curr = l->first;

    while(curr != 0){
        temp = curr->next;
        free(curr->data);
        free(curr);
        curr = temp;
    }
    free(l);
    l = 0;
    return l;
}


void list_print(struct linked_list *l, void (*fptr)(void *)){
    struct node *curr = l->first;

    while(curr != 0){
        fptr(curr->data);
        curr = curr->next;
    }
}