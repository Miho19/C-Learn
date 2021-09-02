#ifndef BST_H_
#define BST_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct node {

    void *data;
    struct node* left;
    struct node* right;

};

struct bst {
    struct node *root;
    void (*print_function)(void *);
    int (*compare_function)(void *, void *);
    size_t element_size;
};

struct bst* bst_new(void (*print_func)(void *), int (*compare_func)(void *, void *), size_t element_size);
struct bst* bst_destroy(struct bst* b);

int bst_add(struct bst *b, void *data);
int bst_delete(struct bst *b, void *data);
int bst_find(struct bst *b, void *data);
int bst_print(struct bst *b);


#endif