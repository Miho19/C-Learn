#include <stdlib.h>
#include <stdio.h>
#include <string.h>


int main(void){

    char letter;
    char sentence[500];

  
  


    while(  scanf(" %c %500s", &letter, sentence) == 2 ) {
        
       
        printf("Letter:%c\n", letter);
        printf("sentence:%s\n", sentence);

    
        
    }


    return 0;
}