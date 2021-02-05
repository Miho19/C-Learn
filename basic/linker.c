#include "linker.h"

#include <stdlib.h>
#include <stdio.h>

list* list_new(){
    list* result = malloc(sizeof *result);
    result->first = NULL;
    result->length = 0;


    return result;
}


list* list_free(list *l){
    node* current = l->first;
    node* temp = l->first;


    while(current != NULL) {
       
        temp = current;
        current = current->next;
        free(temp);
        
    }

    

    free(l);
    l = NULL;
    return l;
}


void list_print(list *l) {
    node *current = l->first;
    while(current != NULL) {
        printf("%d, ", current->item);
        current = current->next;
    }
    printf("\n");
}


void node_add(list *l, int value) {
    node* current;
    node* previous;
    node* new_node = malloc(sizeof *new_node);
  

    new_node->item = value;
    new_node->next = NULL;
    l->length++;

    

    if(l->first == NULL){
        l->first = new_node;         
        return;
    }

    if(l->first->next == NULL) {
        l->first->next = new_node;
        return;
    }

    if(l->first->item < new_node->item){
        new_node->next = l->first;
        l->first = new_node;
        return;
    }

    current = l->first;
    previous = l->first;


    while(current != NULL){

        if(current->item < new_node->item){
            previous->next = new_node;
            new_node->next = current;
            return;
        }
        previous = current;
        current = current->next;
    }

   

    previous->next = new_node;
}

