#include<stdio.h>
#include<stdlib.h>



typedef struct queue{
    int value;
    struct queue* prev;
}queue;

queue* front = NULL;
queue* back = NULL;

void inqueue(int value){
    queue* temp=malloc(sizeof *temp);

    temp->value=value;
    temp->prev=back;

    if(front==NULL){
        front = temp;
        back  = temp;
        return;
    }

    back=temp;
    
}

void print_queue() {
    int i;
    queue* current = back;
    i = 0;
    

    while(current != NULL){
        printf("%d | %d \n",i, current->value);
        i++;
        current = current->prev;
    }
   

}

int dequeue(){

    queue* temp=back;
	int i=front->value;

    if(front==NULL){
		printf("Error");
		return 0;
	}

    if(front==back){
        free(temp);
        front = NULL;
        back = NULL;
		return i;
	}
	 
	
	while(temp!=NULL && temp->prev!=front){
		temp=temp->prev;
	}

	front=temp;
	temp=temp->prev;
	free(temp);
	return i;
}

int main(){
    inqueue(1);
    inqueue(2);
    printf("dequeue: %d\n",dequeue());
    printf("dequeue: %d\n",dequeue());
    inqueue(3);
    inqueue(4);
    inqueue(5);
    inqueue(6);
     printf("dequeue: %d\n",dequeue());
     printf("dequeue: %d\n",dequeue());
    inqueue(7);
    inqueue(8);



    print_queue();

    return 0;

}

