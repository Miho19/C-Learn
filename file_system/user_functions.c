#include "test_file_system.h"

extern mem_inode *mem_inode_list[MAX_FILE_AMOUNT];
extern void superblock_get(superblock *s);
extern int file_create(user_file *file, inode *l);
extern int inode_update(inode *l);
extern int inode_create_dir(superblock *s, inode *parent, int parent_num_files, const char *name);
extern int split_path(char **dest, const char *source, int length);
extern int root_inode_get(superblock *s, inode *root, direntry **file_list);
extern int inode_fetch(superblock *s, inode *l, int inode_number);
extern int print_list_inodes(direntry *dir, int length);

/** Command only */

int ls() {

    superblock s;
    inode root_ionde;
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

    superblock_get(&s);
    current_inode_number_of_files = root_inode_get(&s, &root_ionde, &dir);

    if(strcmp(path, "/") == 0) {
        print_list_inodes(dir + 2, current_inode_number_of_files - 2);
        goto exit;
    }

    items_split = split_path(path_split, path, strlen(path));



    /** ignore root / */
    path_split_index = 1;

    current_inode = root_ionde;

    while(path_split_index < items_split) {
        

        for(i=0;i<current_inode_number_of_files;i++){
            if(strcmp(path_split[path_split_index], dir[i].name) == 0) {
                break;
            }
        }

        inode_fetch(&s, &current_inode, dir[i].inode_number);
        current_inode_number_of_files = current_inode.file_size / sizeof(direntry);
        fseek(f, current_inode.data_index * BLOCK_SIZE, 0);
        fread(dir, sizeof(direntry), current_inode_number_of_files, f);
        path_split_index++;

    }

    memset(dir, 0 ,sizeof(direntry) * MAX_FILE_AMOUNT);

    current_inode_number_of_files = current_inode.file_size / sizeof(direntry);
    fseek(f, current_inode.data_index * BLOCK_SIZE, 0);
    fread(dir, sizeof(direntry), current_inode_number_of_files, f);

    
    print_list_inodes(dir, current_inode_number_of_files);


    exit:
    free(dir);
    for(i=0;i<items_split;i++)
        free(path_split[i]);

    return 0;
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


    direntry *dir;
    inode root_inode;
    superblock s;

    inode current_inode;
    inode parent_inode;

    int current_inode_number_of_files;
    int name_length;
    

    int split_index;
    int items_dirname_split;
    char *dirname_split[MAX_FILE_AMOUNT];

    char *new_path_split[MAX_FILE_AMOUNT];
    int new_path_split_index;

    char temp_path[512];
    int temp_path_index;

    
    i = 0;
    split_index = 0;
    new_path_split_index = 0;
    current_inode_number_of_files = 0;
    name_length = 0;
  
    
    superblock_get(&s);
    current_inode_number_of_files = root_inode_get(&s,&root_inode, &dir);
    items_dirname_split = split_path(dirname_split, dir_name->string, dir_name->length);


    if(strcmp(dirname_split[0], "/") == 0 && items_dirname_split == 1) {
        strcpy(path, "/");
        goto exit;
    }

    /** Given full path e.g cd /new_directory/usr/temp */
    
    new_path_split[new_path_split_index] = malloc(sizeof(char) * 2);
    strcpy(new_path_split[new_path_split_index++], "/");
    split_index++;

    current_inode = root_inode;
    parent_inode = current_inode;

    if(strcmp(dirname_split[0], "/") == 0) {
        

        while(split_index < items_dirname_split) {

            

            for(i=0;i<current_inode_number_of_files;i++){
                if(strcmp(dir[i].name, dirname_split[split_index]) == 0)
                    break;
            }

            if(i == current_inode_number_of_files) {
                printf("%s is not a directory\n", dirname_split[split_index]);
                goto exit;
            }

            parent_inode = current_inode;

            inode_fetch(&s, &current_inode, dir[i].inode_number);

            if(current_inode.type != directory) {
                printf("%s is not a directory\n", dirname_split[split_index]);
                goto exit;
            }

            if(strcmp(".", dirname_split[split_index]) == 0 ) {
                split_index++;
                continue;
            }

            if(strcmp("..", dirname_split[split_index]) == 0 ) {
                current_inode = parent_inode;
                
                split_index++;
                new_path_split_index--;
                continue;
            }

            new_path_split[new_path_split_index] = malloc(strlen(dirname_split[split_index] + 1));
            strcpy(new_path_split[new_path_split_index], dirname_split[split_index]);

            current_inode_number_of_files = current_inode.file_size / sizeof(direntry);
            

            fseek(f, current_inode.data_index * BLOCK_SIZE, 0);
            fread(dir, sizeof(direntry), current_inode_number_of_files, f);
            
            
            split_index++;
            new_path_split_index++;


        }
    
       
    }





    temp_path[0] = '/';

    for(i=1, temp_path_index = 1 ;i<new_path_split_index;i++){
        name_length = strlen(new_path_split[i]);

        for(j=0;j<name_length;j++, temp_path_index++){
            temp_path[temp_path_index] = new_path_split[i][j];
        }

        temp_path[temp_path_index++] = '/';
    }

    temp_path[temp_path_index++] = '\0';

    strcpy(path, temp_path);


    exit:
    free(dir);
    for(i=0;i<items_dirname_split;i++)
        free(dirname_split[i]);
    
    for(i=0;i<MAX_FILE_AMOUNT;i++){
        if(new_path_split[i]){
            free(new_path_split[i]);
        }
    }
    

  

    return 0;
}

int mkdir(sequence *dir_name) {
    int i;

    superblock s;
    inode root_inode;

    inode current_inode;
    inode parent_inode;

    direntry *dir;
    int current_inode_number_of_files;

    char *dir_name_split[MAX_FILE_AMOUNT];
    int items_split_dir_name;
    int split_index_dir_name;


    current_inode_number_of_files = 0;
    items_split_dir_name = 0;
    split_index_dir_name = 0;

    superblock_get(&s);
    current_inode_number_of_files = root_inode_get(&s, &root_inode, &dir);
    items_split_dir_name = split_path(dir_name_split, dir_name->string, dir_name->length);

    /** 
     * Using full path e.g /new_directory/hello
     * 
    */

    if(strcmp(dir_name_split[0], "/") == 0 && items_split_dir_name == 1)
        goto exit;
    

    if(strcmp(dir_name_split[0], "/") == 0) {
        current_inode = root_inode;
        parent_inode = root_inode;

        split_index_dir_name = 1;

        while(split_index_dir_name < items_split_dir_name - 1){
            
            for(i=0;i<current_inode_number_of_files;i++){
                if(strcmp(dir[i].name, dir_name_split[split_index_dir_name]) == 0)
                    break;
            }

            if(i == current_inode_number_of_files) {
                printf("%s is not a directory\n", dir_name_split[split_index_dir_name]);
                goto exit;
            }

            if(strcmp(".", dir_name_split[split_index_dir_name]) == 0){
                split_index_dir_name++;
                continue;
            }

            if(strcmp(".", dir_name_split[split_index_dir_name]) == 0){
                current_inode = parent_inode;
                split_index_dir_name++;
                continue;
            }

            parent_inode = current_inode;

            inode_fetch(&s, &current_inode, dir[i].inode_number);

            if(current_inode.type != directory){
                printf("%s is not a directory\n", dir[i].name);
                goto exit;
            }

            current_inode_number_of_files = current_inode.file_size / sizeof(direntry);

            fseek(f, current_inode.data_index * BLOCK_SIZE, 0);
            fread(dir, sizeof(direntry), current_inode_number_of_files, f);

            split_index_dir_name++;
        }
    }

    for(i=0;i<current_inode_number_of_files;i++){
        if(strcmp(dir[i].name, dir_name_split[items_split_dir_name - 1]) == 0)
            break;
    }

    if(i != current_inode_number_of_files) {
        printf("%s is already a directory\n", dir_name_split[items_split_dir_name - 1]);
        goto exit;
    }


    

    inode_create_dir(&s, &current_inode, current_inode_number_of_files, dir_name_split[items_split_dir_name - 1]);



    exit:
    free(dir);
    for(i=0;i<items_split_dir_name;i++)
        free(dir_name_split[i]);
        

    return 0;
}



