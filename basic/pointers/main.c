#include <stdio.h>
#include <stdlib.h>

#include "test_file_system.h"


int main(void){

    int i;
    int quit;
    char input[20];
    int fd;
    memset(input, 0, sizeof(input));
    fd = 0;

    init(4096);

    quit = 0;
    while(!quit) {
        printf("file_system:%s", path);
        fgets(input, sizeof(input), stdin);
        for(i=0;i<(int)sizeof(input);i++){
            if(input[i] == '-'){
                if(input[i+1] == 'q' || input[i+1] == 'Q') {
                    quit = 1;         
                }
                if(input[i+1] == 'p' || input[i+1] == 'P')
                    file_print_superblock();
                if(input[i+1] == 'l' && input[i+2] == 's')
                    ls();
                if(input[i+1] == 'a')
                    file_print_all_files();
            }
        }

        fd = file_open("example.txt");
        file_write(fd, "This text is going to disk", FILE_OVERWRITE);
        file_read(fd);
        
    }

    destroy();
    return 0;
}


/*** file_open("example.txt", "This is an example file which is being write to disk\n");*/