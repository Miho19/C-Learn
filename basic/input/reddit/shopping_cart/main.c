#include <stdio.h>

int main(void){
    char input;
    input = ' ';
    while(input != 'q') {

        
      while ((input != 'a' && input != 'r') && (input != 'c' && input != 'i') && (input != 'o' && input != 'q')){

          
          printf("Choose an option:");
          scanf(" %c", &input);
        }




        
        
    }
    return 0;
}

/*
  do {
            
            printf("Choose an option:\n");
            scanf(" %c", &input);
        } while((input != 'a' && input != 'r') && (input != 'c' && input != 'i') && (input != 'o' && input != 'q'));

*/