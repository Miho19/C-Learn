#include "bst.h"

struct bst* bst_new(void (*print_function)(void *), int (*compare_function)(void *, void *), size_t element_size){
    struct bst *b = 0;

    if(!print_function || !compare_function || !element_size){
        fprintf(stderr, "Must supply print func, compare func and element size\n");
        exit(1);
    }

    b = malloc(sizeof(*b));

    if(!b){
        fprintf(stderr, "Memory allocatino error b\n");
        exit(1);
    }

    b->element_size = element_size;
    b->print_function = print_function;
    b->compare_function = compare_function;
    b->root = 0;

    return b;
}

static void node_free(struct node *curr){
    
    if(curr->left)
        node_free(curr->left);
    if(curr->right)
        node_free(curr->right);

    free(curr->data);
    free(curr);
}




struct bst* bst_destroy(struct bst* b){

    if(b->root)
        node_free(b->root);
    
    free(b);
    b = 0;
    return b;
}

static void node_add(struct node **curr, struct node **new_node, int (*compare_function)(void *, void *)){
    
    int compare = 0;

    if(*(curr) == 0){
        *(curr) = *(new_node);
        return;
    }

    compare = compare_function((*(new_node))->data, (*(curr))->data);
    

    if(compare == 0){
        free((*(new_node))->data);
        free(*(new_node));
        return;
    }

    if(compare > 0){
        node_add(&(*(curr))->right, new_node, compare_function);
        return;
    }

    if(compare < 0) {
        node_add(&(*(curr))->left, new_node, compare_function);
        return;
    }
}


int bst_add(struct bst *b, void *data) {
    struct node *new_node = 0;

    if(!data || !b)
        return 0;

    new_node = malloc(sizeof(*new_node));

    if(!new_node){
        fprintf(stderr, "Memory allocation failure: new node\n");
        return 0;
    }

    new_node->left  = 0;
    new_node->right = 0;
    new_node->data  = 0;

    new_node->data = malloc(b->element_size);

    if(!new_node->data){
        fprintf(stderr, "Memory allocation failure: new_node->data\n");
        free(new_node);
        return 0;
    }

    memcpy(new_node->data, data, b->element_size);

    node_add(&b->root, &new_node, b->compare_function);

    return 1;
}

static void node_print(struct node *curr, void (*print_function)(void *)){
    if(curr == 0)
        return;

    node_print(curr->left, print_function);

    print_function(curr->data);

    node_print(curr->right, print_function); 
    
}

int bst_print(struct bst *b){
    
    if(!b)
        return 0;

    node_print(b->root, b->print_function);
    return 1;
}


static struct node* node_find(struct node *curr, void *data, int (*compare_function)(void *, void *)){
    
    int compare = 0;

    if(curr == 0){
        return 0;
    }

    compare = compare_function(data, curr->data);
    
    if(compare == 0){
        return curr;
    }
        
    if(compare > 0 && curr->right){
        return node_find(curr->right, data, compare_function);
    }

    if(compare < 0 && curr->left){
        return node_find(curr->left, data, compare_function);
    }


    return 0;
}

int bst_find(struct bst *b, void *data){
    
    if(!data)
        return 0;
    
    return node_find(b->root, data, b->compare_function) ? 1 : 0;
}


static struct node* node_parent(struct node *child, struct node *parent, int (*compare_function)(void *, void*)){

    int compare = 0;
    
    

    if(parent->left == child || parent->right == child){    
        return parent;   
    }

    compare = compare_function(child->data, parent->data);

    if(compare == 0)
        return 0;
    
    if(compare > 0){
        return node_parent(child, parent->right, compare_function);
    }

    if(compare < 0){
        return node_parent(child, parent->left, compare_function);
    }

    return 0;

}

static void* node_insuccesor_data(struct node *curr){
    
    while(curr->left != 0){
        curr = curr->left;
    }

    return curr->data;
}

static int node_delete(struct node *curr, struct node *parent, int (*compare_function)(void *, void *), size_t element_size){

    void *data = 0;
    struct node *successor = 0;

    
    
    if(!curr->left && !curr->right){
        
        if(parent) {
            
            if(parent->left == curr){
                
                parent->left = 0;
            } else {
                
                parent->right = 0;
            }
        }

        free(curr->data);
        free(curr);
        
        return 1;    
    }

    if(curr->left && !curr->right){
        memcpy(curr->data, curr->left->data, element_size);
        return node_delete(curr->left, curr, compare_function, element_size);
    }

    if(curr->right && !curr->left){
         
        memcpy(curr->data, curr->right->data, element_size);
        return node_delete(curr->right, curr, compare_function, element_size);
    }

    if(curr->right && curr->left){
        data = node_insuccesor_data(curr);
        successor = node_find(curr, data, compare_function);

        memcpy(curr->data, data, element_size);
        
        return node_delete(successor, curr, compare_function, element_size);
    }

    return -1;
}

int bst_delete(struct bst *b, void *data){
    struct node *delete_node = 0;
    struct node *parent = 0;

    delete_node = node_find(b->root, data, b->compare_function);
    
    if(!delete_node)
        return 0;


    parent = node_parent(delete_node, b->root, b->compare_function);

    return node_delete(delete_node, parent, b->compare_function, b->element_size) > 0 ? 1 : 0;
}