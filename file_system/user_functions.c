#include "test_file_system.h"

extern void superblock_get(superblock *s);

extern int inode_update(inode *l);
extern int inode_create_dir(superblock *s, inode *parent, int parent_num_files, const char *name);
extern int split_path(char **dest, const char *source, int length);
extern int root_inode_get(superblock *s, inode *root, direntry **file_list);
extern int inode_fetch(superblock *s, inode *l, int inode_number);
extern int print_list_inodes(direntry *dir, int length);
extern int path_get_inode(superblock *s, char **path_split, int items_split, char **path_returned, int *path_number_items, inode *l);
extern int mem_inode_list_get_free_index(int inode_number);
extern int inode_create_text_file(superblock *s, inode *parent, int parent_num_files, const char *name);


/** Command only */

int ls() {

    superblock *s;
    inode root_inode;
    direntry *dir;

    
    int i;

    char *path_split[MAX_FILE_AMOUNT];
    int path_split_index;
    int items_split;




    inode current_inode;
    int current_inode_number_of_files;

    
    items_split = 0;
    path_split_index = 0;
    current_inode_number_of_files = 0;
    dir = 0;
    s = 0;



    s = malloc(sizeof(*s));
    memset(s, 0, sizeof(*s));
    memset(&root_inode, 0, sizeof(root_inode));


    superblock_get(s);
    current_inode_number_of_files = root_inode_get(s, &root_inode, &dir);

    if(strcmp(path, "/") == 0) {
        print_list_inodes(dir + 2, current_inode_number_of_files - 2);
        goto exit;
    }

    items_split = split_path(path_split, path, strlen(path));



    /** ignore root / */
    path_split_index = 1;

    current_inode = root_inode;

    while(path_split_index < items_split) {
        

        for(i=0;i<current_inode_number_of_files;i++){
            if(strcmp(path_split[path_split_index], dir[i].name) == 0) {
                break;
            }
        }

        inode_fetch(s, &current_inode, dir[i].inode_number);
        current_inode_number_of_files = current_inode.file_size / sizeof(direntry);
        fseek(f, current_inode.data_index * BLOCK_SIZE, 0);
        fread(dir, sizeof(direntry), current_inode_number_of_files, f);
        path_split_index++;

    }

    memset(dir, 0 ,sizeof(direntry) * MAX_FILE_AMOUNT);

    current_inode_number_of_files = current_inode.file_size / sizeof(direntry);
    fseek(f, current_inode.data_index * BLOCK_SIZE, 0);
    fread(dir, sizeof(direntry), current_inode_number_of_files, f);

    
    print_list_inodes(dir + 2, current_inode_number_of_files - 2);


    exit:
    free(dir);
    free(s);
    for(i=0;i<items_split;i++)
        free(path_split[i]);

    return 0;
}


int superblock_print(){
    superblock *s;

    s = 0;
    s = malloc(sizeof(*s));
    memset(s, 0, sizeof(*s));
    

    superblock_get(s);

    printf("\n\nsuperblock stats\n\n");

    printf("version %d\n", s->type);
    printf("size %d bytes\n", s->size);
    printf("max number of inodes %d\n", s->MAX_INODE_NUMBER);
    printf("starting index of inode %d\n", s->index_inode);
    printf("starting index of data %d\n", s->index_data);
    free(s);

    return 1;    
}



/** File requests */

int file_create(char **path_split, int items_split){

    int i;

    superblock *s;
    direntry *dir;

    int current_inode_number_of_files;
    inode current_inode;
    

    char *path_returned_split[MAX_FILE_AMOUNT];
    int items_returned_split;

    char temp_path[512];


    memset(temp_path, 0 , sizeof(temp_path));
    items_returned_split = 0;
    current_inode_number_of_files = 0;
   
    dir = 0;
    s = 0;

    s = malloc(sizeof(*s));
    memset(s, 0, sizeof(*s));
   

    superblock_get(s);

   

    current_inode_number_of_files = root_inode_get(s, &current_inode, &dir);

    if( (items_split - 1) > 1 ) {

        free(dir);
        dir = 0;

        current_inode_number_of_files = path_get_inode(s, path_split, items_split - 1, path_returned_split, &items_returned_split, &current_inode);
        if(current_inode_number_of_files == -404)
            goto exit;

        current_inode_number_of_files = current_inode.file_size / sizeof(direntry);
        dir = malloc(sizeof(direntry) * MAX_FILE_AMOUNT);

        fseek(f, current_inode.data_index * BLOCK_SIZE, 0);
        fread(dir, sizeof(direntry), current_inode_number_of_files, f);

    }

    if(strcmp(path_split[items_split - 1], ".") == 0 || strcmp(path_split[items_split - 1], "..") == 0) {
       goto exit;
    }

    
    for(i=0;i<current_inode_number_of_files;i++){
        if(strcmp(dir[i].name, path_split[items_split - 1]) == 0)
            break;
    }

    if(i != current_inode_number_of_files){
        printf("%s already exists\n", path_split[items_split - 1]);
        goto exit;
    }

    
    inode_create_text_file(s, &current_inode, current_inode_number_of_files, path_split[items_split - 1]);


    exit:

    if(dir)
        free(dir);
    free(s);

    for(i=0;i<items_returned_split;i++)
        free(path_returned_split[i]);
    

    return 0;
}




int cat(char **path_split, int items_split) {
    int i;
    superblock *s;
    
    direntry *dir;

    char *path_returned_split[MAX_FILE_AMOUNT];
    int items_returned_split;

    inode current_inode;
    int current_inode_file_number;

    char temp_path[512];

    items_returned_split = 0;
    current_inode_file_number = 0;
    

    
    memset(temp_path, 0, sizeof(temp_path));
    memset(&current_inode, 0, sizeof(current_inode));

    dir = 0;
    s = 0;

    s = malloc(sizeof(*s));
    memset(s, 0, sizeof(*s));
    

    superblock_get(s);

    if(items_split > 2) {

        current_inode_file_number = path_get_inode(s, path_split, items_split - 1, path_returned_split,&items_returned_split, &current_inode);
        if(current_inode_file_number == -404)
            goto exit;
        current_inode_file_number = current_inode.file_size / sizeof(direntry);
        
        
        dir = malloc(sizeof(direntry) * MAX_FILE_AMOUNT);
        fseek(f, current_inode.data_index * BLOCK_SIZE, 0);
        fread(dir, sizeof(direntry), current_inode_file_number, f);

        

    } else {
        current_inode_file_number = root_inode_get(s, &current_inode, &dir);
    }

    if(strcmp(path_split[items_split - 1], ".") == 0 || strcmp(path_split[items_split - 1], "..") == 0) {
       goto exit;
    }

    for(i=0;i<current_inode_file_number;i++){
        if(strcmp(dir[i].name, path_split[items_split - 1]) == 0) {
            break;
        }   
    }

    if(i == current_inode_file_number){
        printf("%s is not a file: 1\n", path_split[items_split - 1]);
        goto exit;
    }

    inode_fetch(s, &current_inode, dir[i].inode_number);

    if(current_inode.type != text_file){
        printf("%s is not a file: 2\n", path_split[items_split - 1]);
        goto exit;
    }

    memset(temp_path, 0, sizeof(temp_path));

    fseek(f, current_inode.data_index * BLOCK_SIZE, 0);

    fread(temp_path, sizeof(char), current_inode.file_size,f);


    for(i=0;i<current_inode.file_size && temp_path[i] != '\0';i++){
        printf("%c", temp_path[i]);
    }
    
    printf("\n");

    exit:
    

    for(i=0;i<items_returned_split;i++)
        free(path_returned_split[i]);
    

    free(dir);
    free(s);

    
    return 0;
}

/** data request */

int file_write(char **path_split, int items_split, sequence *file_data, int mode){
    
    int i;

    superblock s;
    inode current_inode;
    int current_inode_number_files;
    direntry *dir;

    char *path_returned_split[MAX_FILE_AMOUNT];
    int items_returned_split;

    int result;

    char temp[BLOCK_SIZE];
    int temp_index;

    dir = 0;
    items_returned_split = 0;
    current_inode_number_files = 0;
    memset(&current_inode, 0, sizeof(current_inode));
    memset(&s, 0, sizeof(s));
    result = 0;

    superblock_get(&s);

    for(i=0;i<MAX_FILE_AMOUNT;i++)
        path_returned_split[i] = 0;

    if(items_split - 1 > 1) {
        result = path_get_inode(&s, path_split, items_split - 1, path_returned_split, &items_returned_split, &current_inode);
        if(result == -404)
            goto exit;
        current_inode_number_files = current_inode.file_size / sizeof(direntry);
        dir = malloc(sizeof(direntry) * current_inode_number_files);
        fseek(f, current_inode.data_index * BLOCK_SIZE, 0);
        fread(dir, sizeof(direntry), current_inode_number_files, f);
    } else {
        current_inode_number_files = root_inode_get(&s, &current_inode, &dir);
    }

    if(strcmp(path_split[items_split - 1], ".") == 0 || strcmp(path_split[items_split - 1], "..") == 0)
        goto exit;

    for(i=0;i<current_inode_number_files;i++){
        if(strcmp(path_split[items_split - 1], dir[i].name) == 0)
            break;
    }

    if(i == current_inode_number_files){
        for(i=1;i<items_split;i++)
            printf("/%s", path_split[i]);
        printf("%s is not a text file: 1\n", path_split[items_split - 1]);
        result = -1;
        goto exit;
    }

    inode_fetch(&s, &current_inode, dir[i].inode_number);

    if(current_inode.type != text_file){
        for(i=1;i<items_split;i++)
            printf("/%s", path_split[i]);
        printf("%s is not a text file: 2\n", path_split[items_split - 1]);
        result = -1;
        goto exit;
    }

    memset(temp, 0, sizeof(temp));

    if(mode == FILE_OVERWRITE){
        strcpy(temp, file_data->string);
        temp[file_data->length] = '\0';
        fseek(f, (current_inode.data_index * BLOCK_SIZE), 0);
        result = fwrite(temp, sizeof(char), BLOCK_SIZE, f);
        current_inode.file_size = file_data->length;
        inode_update(&current_inode);
        goto exit;
    }

    if((current_inode.file_size + file_data->length) > BLOCK_SIZE) {
        file_data->length = (BLOCK_SIZE - file_data->length);
    }

    fseek(f, current_inode.data_index * BLOCK_SIZE, 0);

    fread(temp, sizeof(char), current_inode.file_size, f);

    for(i=0, temp_index = current_inode.data_index;temp_index<BLOCK_SIZE && i < file_data->length;i++, temp_index++){
        temp[temp_index] = file_data->string[i]; 
    }

    temp[temp_index] = '\0';
    fseek(f, current_inode.data_index * BLOCK_SIZE, 0);
    result = fwrite(temp, sizeof(char), BLOCK_SIZE, f);

    inode_update(&current_inode);
        

    exit:
    if(dir)
        free(dir);
    for(i=0;i<MAX_FILE_AMOUNT;i++){
        if(path_returned_split[i])
            free(path_returned_split[i]);
    }


    return result;
}



int cd(char **path_split, int items_split){

    int i;
    int j;

    superblock *s;
    int current_inode_file_number;

    char *path_returned_split[MAX_FILE_AMOUNT];
    inode current_inode;
    direntry *dir;
    int items_returned_split;

    char temp[512];
    int temp_index;
    int item_length;

    current_inode_file_number = 0;
    items_returned_split = -1;
    item_length = 0;
    temp_index = 0;
    dir = 0;
    s = 0;


    s = malloc(sizeof(*s));
    memset(s, 0, sizeof(*s));
    

    memset(temp, 0, sizeof(temp));
    memset(&current_inode, 0, sizeof(current_inode));

    for(i=0;i<MAX_FILE_AMOUNT;i++)
        path_returned_split[i] = 0;

    superblock_get(s);


    if(items_split > 2) {
        current_inode_file_number = path_get_inode(s, path_split, items_split, path_returned_split,&items_returned_split, &current_inode);
        
        if(current_inode_file_number == -404)
            goto exit;

        dir = malloc(sizeof(direntry) * MAX_FILE_AMOUNT);
        current_inode_file_number = current_inode.file_size / sizeof(direntry);
        

        fseek(f, (current_inode.data_index * BLOCK_SIZE) , 0);
        fread(dir, sizeof(direntry), current_inode_file_number, f);

        

    } else {
        current_inode_file_number = root_inode_get(s, &current_inode, &dir);
    }

    if((strcmp(".", path_split[items_split -1]) == 0 || strcmp("..", path_split[items_split -1]) == 0) && items_returned_split == -1) {
        strcpy(path, "/");
        goto exit;
    }

    

    temp[temp_index++] = '/';

    if(items_returned_split == -1) {
        for(i=0;i<current_inode_file_number;i++){
            if(strcmp(dir[i].name, path_split[items_split -1]) == 0)
            break;
        }
        if(i == current_inode_file_number){
            printf("%s is not a directory\n", path_split[items_split -1]);
            goto exit;
        }
        inode_fetch(s, &current_inode, dir[i].inode_number);

        if(current_inode.type != directory) {
            printf("%s is not a directory\n", path_split[items_split -1]);
            goto exit;
        }

        strcpy(temp + temp_index, path_split[items_split -1]);
        temp_index += strlen(path_split[items_split -1]);
        temp[temp_index++] = '/';
        temp[temp_index] = '\0';
        strcpy(path, temp);
        goto exit;
    }

   

    for(i=1;i<items_returned_split;i++) {
        item_length = strlen(path_returned_split[i]);
        for(j=0;j<item_length;j++, temp_index++){
            temp[temp_index] = path_returned_split[i][j];
        }
        temp[temp_index++] = '/';
    }

   

    temp[temp_index] = '\0';

    strcpy(path, temp);


    exit:

    if(dir)
        free(dir);
    free(s);

    for(i=0;i<MAX_FILE_AMOUNT;i++){
        if(path_returned_split[i])
            free(path_returned_split[i]);
    }

    return 0;
}

int mkdir(char **path_split, int items_split) {
    int i;

    superblock s;

    inode current_inode;
    direntry *dir;
    int current_inode_file_number;

    char *path_returned_split[MAX_FILE_AMOUNT];
    int items_returned_split;

    
    memset(&current_inode, 0, sizeof(current_inode));
    current_inode_file_number = 0;
    items_returned_split = 0;
    dir = 0;

    memset(&s, 0, sizeof(s));
    superblock_get(&s);

    for(i=0;i<MAX_FILE_AMOUNT;i++)
        path_returned_split[i] = 0;


    if((items_split - 1) > 1) {
        current_inode_file_number = path_get_inode(&s, path_split, items_split - 1, path_returned_split, &items_returned_split, &current_inode);
        if(current_inode_file_number == -404)
            goto exit;
        current_inode_file_number = current_inode.file_size / sizeof(direntry);
        dir = malloc(sizeof(direntry) * current_inode_file_number);
        fseek(f, current_inode.data_index * BLOCK_SIZE, 0);
        fread(dir, sizeof(direntry), current_inode_file_number, f);

    } else {
        current_inode_file_number = root_inode_get(&s, &current_inode, &dir);
    }

    if(strcmp(".",path_split[items_split - 1]) == 0 || strcmp("..",path_split[items_split - 1]) == 0)
        goto exit;
    
    for(i=0;i<current_inode_file_number;i++){
        if(strcmp(dir[i].name, path_split[items_split - 1]) == 0)
            break;
    }

    if(i != current_inode_file_number) {
        for(i=1;i<items_split;i++){
            printf("/%s", path_split[i]);
        }
        printf(" already exists\n");
        goto exit;
    }

    inode_create_dir(&s, &current_inode, current_inode_file_number, path_split[items_split - 1]);


    exit:

    if(dir)
        free(dir);

    for(i=0;i<MAX_FILE_AMOUNT;i++)
        if(path_returned_split[i])
            free(path_returned_split[i]);

    return 0;
}



