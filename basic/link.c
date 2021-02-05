#include <stdlib.h>
#include <stdio.h>

#include "linker.h"


int main() {
    
    list* l;
    


    l = list_new();

    

    node_add(l, 55);
   
    node_add(l, 48);
    node_add(l, 47);

    list_print(l);

    node_add(l, 46);

    list_print(l);

    node_add(l, 900);

    list_print(l);

    node_add(l, 901);

    list_print(l);

     node_add(l, 1);

    list_print(l);

     node_add(l, 49);

    list_print(l);

   l = list_free(l);


    return 0;
}