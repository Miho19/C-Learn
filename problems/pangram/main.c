#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARR 10
#define STR 51
#define LETTERS 26

#define BASE 97

#define toLower 32


int main(void) {

    int numberOfWords = 0;
    int i;
    int j;
    
    
    char words[ARR][STR];

    char letters[LETTERS];

    int total = 0;
    
    char temp;
    int pan = 0;

   for(i = 0;i<ARR;i++) {
       for(j=0;j<STR;j++) {
            words[i][j] = 0;
       }
   }

   for(i=0;i<LETTERS;i++){
       letters[i] = 0;
   }
    

    while(numberOfWords < ARR && scanf("%51s", words[numberOfWords])) {
        numberOfWords++;
    }

    for(i=0;i<ARR;i++){
        for(j=0;j<STR && words[i][j] != '\0';j++){
            if(words[i][j] >= 'A' && words[i][j] <= 'Z'){
                words[i][j] = words[i][j] + toLower;
            }
        }
    }
        
    
    temp = 0;
    
    for(i=0;i<numberOfWords;i++){
        
        for(j=0;j<STR && words[i][j] != '\0';j++){
            temp = words[i][j];
            if(temp >= 'a' && temp <= 'z'){
                
                if(letters[(int)temp - BASE] == 0){
                    total++;
                    letters[(int)temp - BASE] = 1;
                }
            }
            
            if(total >= LETTERS) {
                pan = 1;
                break;
            }
                
        } 

    }

    if(pan) {
        printf("That was a pangram\n");
    }else{
        printf("That was not a pangram\n");
    }       
        


    return 0;
}