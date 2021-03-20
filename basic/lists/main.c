#include <stdlib.h>
#include <stdio.h>


typedef struct node {   
    char ch;
    struct node *next ;  
}Node;

Node *head = 0;
  
void prepend(char c)
{
    
    Node *current;
    Node *new_node; 

    if(head == 0){
        head = malloc(sizeof(*head));
        head->ch = c;
        head->next = NULL;
        return;
    }


    new_node = malloc(sizeof(*new_node));
    new_node->ch = c;
    new_node->next = NULL;
    
    
   
    
    current = head;
    
    while(current->next != NULL) {
        current = current->next;
    }
    
    current->next = new_node;
    
}

int main ()
{
	Node *cur;
	int i;


	prepend('x');
    prepend('c');


   
	i = 0;
	cur = head;

	while(cur !=NULL)
	{
		printf("%d | %c\n", i, cur->ch);
		cur = cur->next;
		i++;
		
	}

    return 0;
	
}