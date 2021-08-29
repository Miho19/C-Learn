#include <stdio.h>
#include <stdlib.h>

typedef struct node {
    int value;
    struct node *next;
} node;

typedef struct list {
    node *first;
    int length;
} list;

void* emalloc(size_t size){
    void* result = malloc(size);
    if(!result) {
        fprintf(stderr, "Memory allocation error\n");
        exit(1);
    }
    return result;
}


list* list_new();
list* list_free(list* l);

void node_add(list* l, int value);
void list_print(list* l);


list* list_new(){
    list* result = emalloc(sizeof *result);
    result->first = 0;
    result->length = 0;
    return result;
}

list* list_free(list* l) {
    node* current;
    node* temp;
    current = l->first;
    temp = l->first;

    while(current != NULL) {
        temp = current->next;
        free(current);
        current = temp;
    }

    free(l);
    l = NULL;
    return l;
}


void node_add(list* l, int value) {
    node* new_node;
    node* current;

    new_node = emalloc(sizeof *new_node);
    new_node->next = NULL;
    new_node->value = value;

    if(l->first == NULL){
        l->first = new_node;
        return;
    }
    current = l->first;

    while(current->next != NULL){
        current = current->next;
    }

    current->next = new_node;
    l->length++;
}


int node_find(list* l, int value){
    node* current;

    current = l->first;

    while(current != NULL){
        if(current->value == value)
            return 1;
        current = current->next;
    }
    return 0;
}


void list_print(list* l){
    node* current;

    current = l->first;

    while(current != NULL){
        printf("%d\n", current->value);
        current = current->next;
    }

}


int main(void){
    int value;
    list *l;

    value = 0;
    l = list_new();

    while(1 == scanf("%d", &value)){
        node_add(l, value);
    }

    value = 6;

    printf("%d found: %d\n", value, node_find(l, value));

    list_free(l);
    return 0;
}