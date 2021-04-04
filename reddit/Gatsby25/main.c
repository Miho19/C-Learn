#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX(a,b) (((a)>(b))?(a):(b))


void printResult(int, int, char**);
char** performConversion(int, char**);
char** decimalToHex(int, char**);
char** decimalToHexHelper(int, int, char**, char**);
void freeHex(int, int, char**);

char** decimalToBinary(int, char**);




int main(int argc, char *argv[]) {
    
    
    char **result;
    
    result = 0;
   
   



    if (argc < 4) {
        printf("\nError: You need to provide more arguments.\n");
        return(-1);
    }
    else if (argc > 3) {

       

        
        result = performConversion(argc - 1, argv + 1);
    }

    

    
    freeHex(0, argc - 3, result);


    return 0;
}

void freeHex(int i, int size, char** hex) {
    if(i == size) {
        free(hex);
        return;
    }
        
    free(hex[i]);
    freeHex(i + 1, size, hex);
}

void printResult(int i, int size, char** hex) {
    if(i == size)
        return;
    printf("0x%s\n", hex[i]);
    printResult(i + 1, size, hex);
}



char** performConversion(int size, char** argv) {
    

    if(size < 1)
        return 0;
    
    if(strcmp(argv[0], "decimal") == 0 && strcmp(argv[1], "hex") == 0) {
        return decimalToHex(size - 2, argv + 2);
    }
    if(strcmp(argv[0], "decimal") == 0 && strcmp(argv[1], "binary") == 0) {
        return decimalToBinary(size - 2, argv + 2);
    }

    return 0;
}

char* strrev(char * str) {
    int j;
    int i;
    char c;
    c = 0;
    i = 0;
    j = 0;

    

    if(!str || !*str)
        return str;

    i = strlen(str) -1;
    
    while(i > j) {
        c = str[i];
        str[i] = str[j];
        str[j] = c;
        i--;
        j++;
    }

    return str;

}

char** decimalToBinaryHelper(int i, int size, char **input, char **result) {
    int num;
    int j;
    char binary[64 + 1];
    num = 0;
    j = 0;

    if(i == size)
        return result;

    num = strtol(input[i], NULL, 10);

    result[i] = malloc( 64 + 1);

    while(num) {
        binary[j] = (num % 2) + '0';
        num /= 2;
        j++;
    }

    binary[j] = '\0';

    strcpy(result[i], strrev(binary)); 
    return decimalToBinaryHelper(i + 1,size, input, result);
}


char **decimalToBinary(int size, char **input) {
    char **result;
    result = 0;

    result = malloc(sizeof(char*) * size);
    result = decimalToBinaryHelper(0, size, input, result);
    return result;
}



char** decimalToHex(int size, char** argv) {
    char** result;
    result = 0;

    if(size < 1)
        return 0;

    result = malloc(sizeof(result[0]) * size);
    return decimalToHexHelper(0, size, argv, result);
}    

char** decimalToHexHelper(int i, int size, char** argv, char** result) {
    int num;
    int s;

    num = 0;
    s = 0;

    if(i == size)
        return result;

    num = (int)strtol(argv[i], NULL, 10);
    

    s = ceil(log10(num) + 1/log10(16));


    

    result[i] = malloc(s + 1);
    

    sprintf(result[i], "%x", num);

    return decimalToHexHelper(i + 1, size, argv, result);
}