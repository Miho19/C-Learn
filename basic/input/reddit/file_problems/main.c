#include <stdio.h>
#include <stdlib.h>


int main(void){
    int players;
    char game_file[] = "game_file.txt";
    
    FILE *save = fopen(game_file, "r");
    if(save == NULL) {
        return 1;
    }
    
    players = 0;
    fscanf(save, "%d ", &players);
    fclose(save);
    return 0;
}