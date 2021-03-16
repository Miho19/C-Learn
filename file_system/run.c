#include "test_file_system.h"

#define EOT 4
#define MAX_LENGTH_COMMANDS 20

#define LIST_LENGTH(arr) sizeof(arr) / sizeof(arr[0])

#define TOTAL_COMMAND_AMOUNT LIST_LENGTH(list_file_command_only) + LIST_LENGTH(list_file_command_name) + LIST_LENGTH(list_file_command_name_data)

static int quit = 0;

int input();
int execute();
int display();

int quit_file_system(void);
int command_find(const char *command, int length);
int help(void);


command_only list_file_command_only[] = {
    {"ls", ls},
    {"help", help},
    {"quit", quit_file_system},
    {"print_superblock", superblock_print},
    {"print_open_files", open_files_print_all}
};

file_request list_file_command_name[] = {
    {"open", file_open},
    {"close", NULL},
    {"cat", cat}
};

data_request list_file_command_name_data[] =  {
    {"write", file_write}
};

int execute(){
    unsigned int i;

    if(rq->type == PENDING)
        return -1;

    if(rq->command_index == -1)
        return -1;
    
    if(rq->type == COMMAND_ONLY) {
        list_file_command_only[rq->command_index].command_function();
        return 0;
    }

    if(rq->type == FILE_REQUEST) {
        list_file_command_name[rq->command_index].file_function(rq->file_name);
        return 0;
    }

    if(rq->type == DATA_REQUEST) {
        list_file_command_name_data[rq->command_index].data_function(rq->file_name, rq->file_data, FILE_OVERWRITE);
        return 0;
    }


    return 0;
}


int run(){
    

    init(4096);

    printf("%s:", path);


    while(!quit){
        input();
        execute();
        display();
    }


    destroy();

    return 1;
}


int input() {
    char input[512];
    char c;
    int i;
    i = 0;


    if(!rq->file_command){
        rq->file_command = malloc(sizeof(sequence));
        rq->file_command->string = malloc(sizeof(char) * MAX_LENGTH_COMMANDS);
        rq->file_command->length = 0;
    }

    if(!rq->file_name){
        rq->file_name = malloc(sizeof(sequence));
        rq->file_name->string = malloc(sizeof(char) * MAX_FILE_NAME + 3);
        rq->file_name->length = 0;
    }

    if(!rq->file_data){
        rq->file_data= malloc(sizeof(sequence));
        rq->file_data->string = malloc(sizeof(char) * BLOCK_SIZE);
        rq->file_data->length = 0;
    }

    rq->status = 0;
    rq->type = PENDING;
    rq->command_index = -1;



    memset(input, 0 , sizeof(input));


    while((c = getchar()) != EOF ) {
        
        if(c == ' ' || c == 10 || c == 4) {
            input[i] = '\0';
            break;
        }
        input[i] = c;
        i++;
    }

    

    if(rq->file_command->length < i) {
        rq->file_command->string = realloc(rq->file_command->string, i);
        if(!rq->file_command->string){
            printf("mem alloc failure file command \n");
            return -10;
        }
    }

    rq->file_command->length = i;
    strcpy(rq->file_command->string, input);


    if(!command_find(rq->file_command->string, rq->file_command->length)){
        printf("\"%s\" is not a command\n", rq->file_command->string);
        return 1;
    }

    

    if(rq->type == COMMAND_ONLY)
        return 1;

    i = 0;
    memset(input, 0 , sizeof(input));

    

    while(c != 10)  {
        c = getchar();
        if(c == ' ' || c == 4 || c == -1 || c == 10) {    
            input[i] = '\0';
            break;
        }
        input[i] = c;
        i++;
    }

    


    if(i > MAX_FILE_NAME) {
        printf("Filename can only be 14 characters long: %d\n", i);
        rq->status = FILE_NAME_TOO_LONG;
        rq->command_index = -1;
        return -1;
    }

    if(i <= 0){
        printf("No filename specified\n");
        rq->status = NO_FILE_NAME;
        rq->command_index = -1;
        return -1;
    }


    if(rq->file_name->length < i) {
        rq->file_name->string = realloc(rq->file_name->string, (sizeof(char) * i) );
    }

    rq->file_name->length = i;
    strcpy(rq->file_name->string, input);




    if(rq->type == FILE_REQUEST)
        return 0;

    i = 0;
    memset(input, 0 , sizeof(input));

    if(c == 10) {    
        printf("Nothing to write\n");
        rq->status = NO_FILE_DATA;
        rq->command_index = -1;
        return -1;
    }

    while( i < BLOCK_SIZE)  {
        c = getchar();
        
        if(c == EOT || c == 26 || c == 4 || c == EOF || c == 10) {
            input[i] = '\0';
            i++;
            break;
        }

        if(c == '\\') {
            switch( (c = getchar()) ) {
                case 'n':
                    input[i] = '\n';
                    i++;
                    continue;
                case 't':
                    input[i] = '\t';
                    i++;
                    continue;
                case 'r':
                   input[i] = '\r';
                    i++;
                    continue;
                default:
                    input[i] = '\\';
                    i++;
                    break;
            }
        }

        input[i] = c;
        i++;
    }

    if(i <= 0){
        printf("Nothing to write\n");
        rq->status = NO_FILE_DATA;
        rq->command_index = -1;
    }

    rq->file_data->length = i;
    strcpy(rq->file_data->string, input);


    return 1;
}

int display(){
    printf("%s:", path);

    if(rq->status){
        rq->status = 0;
        return -1;
    }

    

    return 1;
}


int command_find(const char *command, int length) {
    unsigned int i;

    if(length <= 0)
        return -1;
    
    for(i=0;i<LIST_LENGTH(list_file_command_only);i++){
        if(strcmp(list_file_command_only[i].command_name, command) == 0 ) {
            rq->type = COMMAND_ONLY;
            rq->command_index = i;
            return 1;
        }
    }
    
    for(i=0;i<LIST_LENGTH(list_file_command_name);i++) {
        if(strcmp(list_file_command_name[i].command_name, command) == 0 ) {
            rq->type = FILE_REQUEST;
            rq->command_index = i;
            return 1;
        }
    }
        

    for(i=0;i<LIST_LENGTH(list_file_command_name_data);i++) {
        if(strcmp(list_file_command_name_data[i].command_name, command) == 0 ) {
            rq->type = DATA_REQUEST;
            rq->command_index = i;
            return 1;
        }
    }

    rq->type = PENDING;
    rq->command_index = -1;
    
    return 0;
}


int help(){
    unsigned int i;
    printf("\nhelp\n");
    printf("usuage:file_command file_name file_data\n");
    printf("command list\n");

    for(i=0;i<LIST_LENGTH(list_file_command_only);i++)
        printf("%s\n", list_file_command_only[i].command_name);

    for(i=0;i<LIST_LENGTH(list_file_command_name);i++)
        printf("%s <name>\n", list_file_command_name[i].command_name);

    for(i=0;i<LIST_LENGTH(list_file_command_name_data);i++)
        printf("%s <name> <data>\n", list_file_command_name_data[i].command_name);

    return 0;
}

int quit_file_system(){
    quit = 1;
    return 0;
}


/** 
 * character 10 represents null value or something in windows for getchar()
 * 
 * 
 * 
 * 
 * 
*/

