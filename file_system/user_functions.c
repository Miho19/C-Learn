#include "test_file_system.h"

extern mem_inode *mem_inode_list[MAX_FILE_AMOUNT];
extern void superblock_get(superblock *s);
extern int file_create(user_file *file, inode *l);
extern int inode_update(inode *l);
extern int inode_create_dir(superblock *s, inode *parent, int parent_num_files, const char *name);

/** Command only */

int ls() {
    inode root_inode;
    direntry *dir;
    int number_of_files;
    int i;

    fseek(f, sizeof(superblock), 0);
    fread(&root_inode, sizeof(inode), 1, f);

    number_of_files = 0;
    number_of_files = root_inode.file_size / sizeof(direntry);

    dir = malloc(root_inode.file_size);
    fseek(f, (root_inode.data_index * BLOCK_SIZE), 0);
    fread(dir, sizeof(direntry), number_of_files, f);

    
    printf("inode | name\n");
    for(i=0;i<number_of_files;i++) {
        printf("%d| \"%s\"\n", dir[i].inode_number, dir[i].name);
    }

    free(dir);
    return number_of_files;
}


int superblock_print(){
    superblock s;
    int i;

    superblock_get(&s);

    printf("\n\nsuperblock stats\n\n");

    printf("version %d\n", s.type);
    printf("size %d bytes\n", s.size);
    printf("number of data blocks %d\n", s.number_of_blocks);
    printf("max number of inodes %d\n", s.number_inodes_max);
    printf("starting index of inode %d\n", s.index_inodes);
    printf("starting index of data %d\n", s.index_data);

    printf("block #|status\n");

    for(i=0;i<s.number_of_blocks;i++){
        printf("block %d|", i);
        if((s.data_block_free_list >> i) & 1) {
            printf(" Used\n");
        } else {
            printf(" Free\n");
        } 
    }

    return 1;    
}


int open_files_print_all() {
    int i;

    for(i=0;i<MAX_FILE_AMOUNT;i++){
        if(mem_inode_list[i]) {
            printf("fd: %d inode: %d | name: %s | data length: %d | offset %d |\n",mem_inode_list[i]->fd, mem_inode_list[i]->file->inode_number, mem_inode_list[i]->file->name, mem_inode_list[i]->file->data_length,mem_inode_list[i]->file->off);
        }
    }

    return 1;
}


/** File requests */

int file_open(sequence *file_name){

    inode root_inode;
    direntry *dir;
    direntry new_entry;

    int amount_of_files;
    int dir_index;
    int i;
    int index_free_mem_list;
    int free_index_found;

    free_index_found = 0;
    index_free_mem_list = -1;

    for(i=0;i<MAX_FILE_AMOUNT;i++) {

        if(mem_inode_list[i] == 0) {

            if(!free_index_found) {
                index_free_mem_list = i;
                free_index_found = 1;
            }

        } else {
            if(strcmp(mem_inode_list[i]->file->name, file_name->string) == 0) {
                return mem_inode_list[i]->fd;
            }
        }
        
        if(!free_index_found)
            index_free_mem_list++;
        
    }
    

    if(index_free_mem_list == -1) {
        printf("Cannot open another file\n");
        return -1;
    }


 /** Add file to memory list */ 
    fseek(f, sizeof(superblock), 0);                                                            /** Get root inode and its direntry array */
    fread(&root_inode, sizeof(inode), 1, f);

    amount_of_files = 0;
    amount_of_files = root_inode.file_size/sizeof(direntry);
    dir = malloc(root_inode.file_size);

    fseek(f, root_inode.data_index * BLOCK_SIZE, 0);
    fread(dir, sizeof(direntry), amount_of_files, f);

    for(dir_index=0;dir_index<amount_of_files;dir_index++){                                                           /** Search through direntry array */
        if(strcmp(dir[dir_index].name, file_name->string) == 0) {
            break;
        }
    }

    mem_inode_list[index_free_mem_list] = malloc(sizeof(mem_inode));   
    
    mem_inode_list[index_free_mem_list]->user_inode = malloc(sizeof(inode));
    mem_inode_list[index_free_mem_list]->fd = ++fd_counter;

    mem_inode_list[index_free_mem_list]->file = malloc(sizeof(user_file));
    mem_inode_list[index_free_mem_list]->file->name = malloc(file_name->length);
    
    strcpy(mem_inode_list[index_free_mem_list]->file->name, file_name->string);
    mem_inode_list[index_free_mem_list]->file->off = 0;
    mem_inode_list[index_free_mem_list]->file->data = 0;
    
    

    if(dir_index == amount_of_files) { 

        file_create(mem_inode_list[index_free_mem_list]->file, mem_inode_list[index_free_mem_list]->user_inode);                  /** create new file on disk*/
        
        new_entry.inode_number = mem_inode_list[index_free_mem_list]->user_inode->inode_number;                       /** create new entry into root */
        strcpy(new_entry.name, mem_inode_list[index_free_mem_list]->file->name);

        fseek(f, (root_inode.data_index * BLOCK_SIZE) + root_inode.file_size, 0);
        fwrite(&new_entry, sizeof(new_entry), 1, f);

        root_inode.file_size += sizeof(new_entry);
        inode_update(&root_inode);

    } else {

        fseek(f, sizeof(superblock) + (sizeof(inode) * dir[dir_index].inode_number), 0 );
        fread(mem_inode_list[index_free_mem_list]->user_inode, sizeof(inode), 1, f);

        mem_inode_list[index_free_mem_list]->file->inode_number = mem_inode_list[index_free_mem_list]->user_inode->inode_number;
        mem_inode_list[index_free_mem_list]->file->data_length = mem_inode_list[index_free_mem_list]->user_inode->file_size;

        if(mem_inode_list[index_free_mem_list]->user_inode->file_size > 0) {
            fseek(f, mem_inode_list[index_free_mem_list]->user_inode->data_index * BLOCK_SIZE, 0);
            mem_inode_list[index_free_mem_list]->file->data = malloc(mem_inode_list[index_free_mem_list]->user_inode->file_size);
            fread(mem_inode_list[index_free_mem_list]->file->data, mem_inode_list[index_free_mem_list]->user_inode->file_size, 1, f);
        }
    }

    free(dir);
        
    return mem_inode_list[index_free_mem_list]->fd;
}

int display_text(const char *data, int length) {
    int i;

    for(i=0;i<length;i++){
        if(data[i] == '\\' && data[i+1] == 'n') {
            printf("\n");
        } else {
            printf("%c", data[i]);
        }
    }

    printf("\n");
    return i;
}



int cat(sequence *file_name) {
    int mem_inode_index;
    inode root_inode;
    direntry *dir;
    int number_of_files;
    int i;
    inode file_inode;
    char *data;

    for(mem_inode_index=0;mem_inode_index<MAX_FILE_AMOUNT;mem_inode_index++) {
        if( mem_inode_list[mem_inode_index] && strcmp(mem_inode_list[mem_inode_index]->file->name, file_name->string) == 0){
            display_text(mem_inode_list[mem_inode_index]->file->data, strlen(mem_inode_list[mem_inode_index]->file->data));
            return mem_inode_list[mem_inode_index]->file->data_length;
        }
    }

    fseek(f, sizeof(superblock), 0);
    fread(&root_inode, sizeof(inode), 1, f);

    number_of_files = 0;
    number_of_files = root_inode.file_size / (sizeof(direntry));

    dir = malloc(root_inode.file_size);
    fseek(f, root_inode.data_index * BLOCK_SIZE, 0);
    fread(dir, sizeof(direntry), number_of_files, f);

    for(i=0;i<number_of_files;i++){
        if( strcmp(dir[i].name, file_name->string) == 0){
            break;
        }
    }

    if(i == number_of_files){
        printf("File is not part of the file system\n");
        return -1;
    }

    fseek(f, sizeof(superblock) + (sizeof(inode) * dir[i].inode_number), 0);
    fread(&file_inode, sizeof(inode), 1, f);

    if(file_inode.file_size <= 0) {
        printf("%s does not contain data\n", file_name->string);
        return 0;
    }


    data = malloc(file_inode.file_size);
    fseek(f, file_inode.data_index * BLOCK_SIZE, 0);
    fread(data, file_inode.file_size, 1, f);

    display_text(data, strlen(data));

    free(data);
    
    return -1;
}

/** data request */

int file_write(sequence *file_name, sequence *file_data, int mode){
    int mem_inode_index;

    for(mem_inode_index=0;mem_inode_index<MAX_FILE_AMOUNT;mem_inode_index++) {
        if( mem_inode_list[mem_inode_index] && strcmp(mem_inode_list[mem_inode_index]->file->name, file_name->string) == 0){
            break;
        }
    }

    if(mem_inode_index == MAX_FILE_AMOUNT) {
        printf("%s is not an open file\n", file_name->string);
        return -1;
    }

    if(mem_inode_list[mem_inode_index]->user_inode->type == directory) {
        printf("can not overwrte dir\n");
        return -1;
    }

    if(mem_inode_list[mem_inode_index]->file->data == 0) {
        mem_inode_list[mem_inode_index]->file->data = malloc(BLOCK_SIZE);
        memset(mem_inode_list[mem_inode_index]->file->data, 0, BLOCK_SIZE);
        strcpy(mem_inode_list[mem_inode_index]->file->data, file_data->string);
        mem_inode_list[mem_inode_index]->file->data_length += file_data->length;
        mem_inode_list[mem_inode_index]->file->off += file_data->length;
        return file_data->length;
    }

    if(mode == FILE_APPEND) {
        mem_inode_list[mem_inode_index]->file->data = realloc(mem_inode_list[mem_inode_index]->file->data, BLOCK_SIZE);
        strcpy(mem_inode_list[mem_inode_index]->file->data + mem_inode_list[mem_inode_index]->file->data_length, file_data->string);
        mem_inode_list[mem_inode_index]->file->data_length += file_data->length;
        mem_inode_list[mem_inode_index]->file->data[mem_inode_list[mem_inode_index]->file->data_length + 1] = '\0';
        mem_inode_list[mem_inode_index]->file->off = mem_inode_list[mem_inode_index]->file->data_length;
        return file_data->length;
    } else {
        mem_inode_list[mem_inode_index]->file->data = realloc(mem_inode_list[mem_inode_index]->file->data, BLOCK_SIZE);
        memset(mem_inode_list[mem_inode_index]->file->data, 0, BLOCK_SIZE);
        strcpy(mem_inode_list[mem_inode_index]->file->data, file_data->string);
        printf("file_data = %s\n", mem_inode_list[mem_inode_index]->file->data);
        mem_inode_list[mem_inode_index]->file->data_length = file_data->length;
        mem_inode_list[mem_inode_index]->file->off = file_data->length;
        return file_data->length;
    }
    

    
    return 0;
}




int cd(sequence *dir_name){

    int i;
    int j;
    int x;

    int y;


    int path_length;
    int root_dir_file_number;
    direntry *dir;
    inode root_inode;
    superblock s;
    char temp_path[512];
    inode current_inode;
    int current_inode_file_number;
    int name_length;

    inode test_inode;
    
    int items_path_split;
    char *path_split[MAX_FILE_AMOUNT];
    int items_dirname_split;
    char *dirname_split[MAX_FILE_AMOUNT];

    char temp[512];
    int temp_index;

    superblock_get(&s);

    fseek(f, sizeof(superblock), 0 );                        
    fread(&root_inode, sizeof(inode), 1, f);

    fseek(f, root_inode.data_index * BLOCK_SIZE, 0);
    root_dir_file_number = root_inode.file_size / sizeof(direntry);
    dir = malloc(sizeof(direntry) * MAX_FILE_AMOUNT);
    fread(dir, sizeof(direntry), root_dir_file_number, f);
    
    /** printf("%d %d %c\n", i, dir_name->string[i],dir_name->string[i]);*/
    path_length = 0;
    path_length = strlen(path);

    i = 0;
    items_dirname_split = 0;
    temp_index = 0;
    memset(temp, 0, sizeof(temp));
    current_inode_file_number = 0;
    name_length = 0;

    if(dir_name->string[0] == '/') {
        strcpy(temp, "/");
        dirname_split[items_dirname_split] = malloc(strlen(temp));
        strcpy(dirname_split[items_dirname_split], temp);
        items_dirname_split++;
        i++;
    }

  

    while(i < dir_name->length) {
        
        if( i == (dir_name->length) || dir_name->string[i] == '/' || dir_name->string[i] == '\0') {
           
           temp[temp_index] = '\0';
           dirname_split[items_dirname_split] = malloc(strlen(temp));
           
           strcpy(dirname_split[items_dirname_split], temp);
           
           memset(temp, 0, sizeof(temp));
           temp_index = 0;
           items_dirname_split++;
           i++;
           continue;
        }

        temp[temp_index] = dir_name->string[i]; 
        temp_index++;
        i++;
    }
/**
    for(i=0;i<items_dirname_split;i++){
        printf("%d:%s\n", strlen(dirname_split[i]), dirname_split[i]);
        
    }

  */

    if(strcmp(dirname_split[0], "/") == 0){

       
        if(strlen(dirname_split[1]) == 0){
            strcpy(path, "/");
            goto exit;
        }


        temp[0] = '/';
        temp_index = 1;
        i = 1;
        current_inode = root_inode;

        while(i < items_dirname_split && strlen(dirname_split[i]) > 0) {
            current_inode_file_number = current_inode.file_size / sizeof(direntry);
            memset(dir, 0, BLOCK_SIZE);
            fseek(f, current_inode.data_index * BLOCK_SIZE, 0);
            fread(dir, sizeof(direntry), current_inode_file_number, f);

            for(j=0;j<current_inode_file_number;j++){

                if(strcmp(dir[j].name, dirname_split[i]) == 0) {
                    fseek(f, s.index_inodes + (sizeof(inode) * dir[j].inode_number), 0);
                    fread(&test_inode, sizeof(inode), 1, f);
                    
                    if(test_inode.type != directory){
                        printf("\"%s\" is not a directory\n", dirname_split[i]);
                        goto exit;
                    }

                    name_length = strlen(dir[j].name);


                    for(x = 0;x<name_length;x++, temp_index++)
                        temp[temp_index] = dir[j].name[x];
                    temp[temp_index++] = '/';
                    break;
                }

            }

            if(j == current_inode_file_number) {
                printf("\"%s\" is not a directory\n", dirname_split[i]);
                goto exit;
            }


            i++;
        }

        strcpy(path, temp);
        goto exit;

    }


    
   

   
    exit(0);



    if(strcmp(path, "/") == 0 ){

        if(!(strcmp(dir_name->string, ".") == 0 || strcmp(dir_name->string, "..") == 0)){
            for(i=0;i<root_dir_file_number;i++){
                if(strcmp(dir[i].name, dir_name->string) == 0){

                    fseek(f, s.index_inodes + (sizeof(inode) * dir[i].inode_number), 0);
                    fread(&current_inode, sizeof(inode), 1, f);

                    if(current_inode.type != directory){
                        printf("\"/%s\" is not a directory\n", dir_name->string);
                        goto exit;
                    }

                    for(j = 0;j<path_length;j++){
                        temp_path[j] = path[j];
                    }

                
                    for(x = 0;x<dir_name->length;x++, j++){
                        temp_path[j] = dir_name->string[x];
                    }
                    break;
                }
            }

            if(i == root_dir_file_number){
                printf("\"/%s\" is not a directory\n", dir_name->string);
                goto exit;
            }

            strcpy(path, temp_path);
            goto exit;
        } else {
            strcpy(path, "/");
            goto exit;
        }

    }

    

    
    exit:
    free(dir);
    for(i=0;i<items_dirname_split;i++)
        free(dirname_split[i]);
    


    return 0;
}

int mkdir(sequence *dir_name) {
    int i;
    int j;
    int items_split;
    int temp_index;
    int path_length;
    inode root_inode;
    inode current_inode;

    int current_dir_file_number;
    int root_dir_file_number;
    direntry *dir;
    char temp[512];
    
    superblock s;

    char *path_split[MAX_FILE_AMOUNT];

    
    path_length = 0;
    path_length = strlen(path);


    root_dir_file_number = 0;
    dir = 0;
    memset(temp, 0, sizeof(temp));
    memset(path_split, 0, sizeof(path_split));
    

    superblock_get(&s);

    fseek(f, sizeof(superblock), 0 );                         /** root inode at index 0*/
    fread(&root_inode, sizeof(inode), 1, f);

    fseek(f, root_inode.data_index * BLOCK_SIZE, 0);
    root_dir_file_number = root_inode.file_size / sizeof(direntry);
    dir = malloc(sizeof(direntry) * MAX_FILE_AMOUNT);
    fread(dir, sizeof(direntry), root_dir_file_number, f);

    
    if(path_length <= 1) {
        inode_create_dir(&s,&root_inode, root_dir_file_number, dir_name->string);
        return 0;
    }

    i = 1;
    items_split = 0;
    temp_index = 0;

    while(i < path_length + 1) {
        
        if( i == (path_length) || path[i] == '/' || path[i] == '\0') {
           temp[temp_index] = '\0';
           path_split[items_split] = malloc(strlen(temp));
           strcpy(path_split[items_split], temp);
           memset(temp, 0, sizeof(temp));
           temp_index = 0;
           items_split++;
           i++;
           continue;
        }

        temp[temp_index] = path[i]; 
        temp_index++;
        i++;
    }

    

    for(i=0;i<root_dir_file_number;i++){
        if(strcmp(path_split[0], dir[i].name) == 0){
            break;
        }
    }

    if(i == root_dir_file_number){
        printf("\"/%s\" is not a directory\n", path_split[0]);
       goto exit;
    }

    fseek(f, s.index_inodes + ((sizeof(inode) * dir[i].inode_number)), 0);
    fread(&current_inode, sizeof(inode), 1, f);

    if(current_inode.type != directory){
        printf("\"/%s\" is not a directory\n", path_split[0]);
        goto exit;
    }

    

    current_dir_file_number = 0;
    current_dir_file_number = current_inode.file_size / sizeof(direntry);
    j = 1;


    while(j < items_split) {
        
        fseek(f, (current_inode.data_index * BLOCK_SIZE), 0 );
        memset(dir, 0, (sizeof(direntry) * MAX_FILE_AMOUNT));
        fread(dir, sizeof(direntry), current_dir_file_number,f);

        for(i=0;i<current_dir_file_number;i++){
            if(strcmp(path_split[j], dir[i].name) == 0){
                fseek(f, s.index_inodes + (sizeof(inode) * dir[i].inode_number),0);
                fread(&current_inode, sizeof(inode), 1, f);
                break;
            }
        }

        if(i == current_dir_file_number){
            printf("\"");
            for(i=0;i<j+1;i++){
                printf("/%s", path_split[i]);
            }
            printf("/\" is not a directory\n");
            goto exit;
        }

        if(current_inode.type != directory){
            printf("\"");
            for(i=0;i<j;i++){
                printf("\"/%s", path_split[i]);
            }
            printf("/\" is not a directory\n");
            goto exit;
        }
        current_dir_file_number = current_inode.file_size / sizeof(direntry);
        j++;
    }
    printf("stoppping execution until cd is made\n");
    exit(0);
    inode_create_dir(&s,&current_inode, current_dir_file_number, dir_name->string);


    exit:
    for(i = 0;i<30;i++){
        if(path_split[i] != 0)
            free(path_split[i]);
    }


    return 0;
}

/** printf("%d | %d | %c \n", i, path[i], path[i]);*/

