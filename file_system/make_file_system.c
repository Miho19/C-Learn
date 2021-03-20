#include "test_file_system.h"

#define VERSION 3


void file_system_make(void);
void superblock_get(superblock *s);
int ionde_get_list(inode *list, int off, int n);
int file_commit_write(user_file *file, inode *l);
int display_text(const char *data, int length);
int inode_file_data_commit(superblock *s, inode *l);
int superblock_update(superblock *s);


/*** 
 * 0-28 -> superblock
 * 28->512 | inode list | (512-28 / 16) = number of inodes -> 30
 * 512 -> buffer_size = data blocks
 * first inode and entry is . directory
*/

int BUFFER_SIZE = 0;

int fd_counter = 0;


void init(int size){
    superblock s;
    
    BUFFER_SIZE = size;

    f = NULL;
    memset(&s, 0, sizeof(superblock));
    
    f = fopen("output.txt", "r+b");

    if(f == NULL) {
       printf("creating output.txt\n");
       f = fopen("output.txt", "w+b");
       file_system_make();
       superblock_get(&s);

       if(s.type != VERSION) {
           printf("Creating version %d but found version %d of superblock \n",VERSION, s.type);
           exit(1);
       }
   }

   path = malloc(sizeof(path[0]) * BLOCK_SIZE);
   memset(path, 0, sizeof(path[0]) * BLOCK_SIZE);
 
   path[0] = '/';
   path[1] = '\0';


   rq = malloc(sizeof(request));
   memset(rq, 0, sizeof(*rq));

}

void destroy(void) {
    

    if(rq->file_command)
        free(rq->file_command->string);
    if(rq->file_data)
        free(rq->file_data->string);
    if(rq->file_name)
        free(rq->file_name->string);

    free(rq->file_command);
    free(rq->file_data);
    free(rq->file_name);
    free(rq);

    fclose(f);
    free(path);

}


/**
 *  0 superblock 
 *  1 inode list
 *  2 bit map
 *  3 root inode data block
 *  4 start of data blocks for files / dir 
 * 
 * */


void file_system_make(void) {

    superblock s;

    inode root_inode;
    direntry dir;

    unsigned char temp[512];

    unsigned char bitblock;

    int i;
    bitblock = 0;
    memset(temp, 0, sizeof(temp));
    memset(&s, 0, sizeof(s));
    memset(&root_inode, 0, sizeof(root_inode));
    memset(&dir, 0, sizeof(dir));

    s.type = VERSION;
    s.size = BUFFER_SIZE;
    s.MAX_INODE_NUMBER = (BLOCK_SIZE / sizeof(inode));
    s.index_inode = 1;
    s.index_bitmap = 2;
    s.index_data = 3;


    superblock_update(&s);

    

    root_inode.data_index = 0;
    root_inode.file_size = 0;
    root_inode.inode_number = 0;
    root_inode.type = text_file;
    root_inode.used = 0;
    
    fseek(f, BLOCK_SIZE * 1, 0);

    for(i=0;i<s.MAX_INODE_NUMBER;i++){
        root_inode.inode_number = i;
        fwrite(&root_inode, sizeof(inode), 1, f);
    }

    fseek(f, BLOCK_SIZE * 1, 0);
    root_inode.inode_number = 0;
    root_inode.file_size = sizeof(direntry) * 2;
    root_inode.type = directory;
    root_inode.used = 1;
    root_inode.data_index = 3;

    fwrite(&root_inode, sizeof(inode), 1, f);

    fseek(f, BLOCK_SIZE * 2, 0);
    fwrite(temp, sizeof(unsigned char), BLOCK_SIZE, f);

    fseek(f, BLOCK_SIZE * 2, 0);

    for(i=0;i<4;i++)
        bitblock |= 1 << i;
    
        

    fwrite(&bitblock, sizeof(unsigned char), 1, f);


    dir.inode_number = 0;
    strcpy(dir.name, ".");
    fseek(f, BLOCK_SIZE * 3, 0);
    fwrite(&dir, sizeof(direntry), 1, f);
    strcpy(dir.name, "..");
    fwrite(&dir, sizeof(direntry), 1, f);

    
}

void superblock_get(superblock *s) {
    memset(s, 0, sizeof(*s));
    fseek(f, 0, 0);
    fread(s, sizeof(superblock),1, f);
}


int inode_update(inode *i) {
    superblock *s;
    s = 0;
    s = malloc(sizeof(*s));
    memset(s, 0, sizeof(*s));
    superblock_get(s);

    fseek(f, (s->index_inode * BLOCK_SIZE) + (i->inode_number * sizeof(inode) ), 0);

    free(s);

    return fwrite(i, sizeof(inode), 1, f);
}

int superblock_update(superblock *s) {
    fseek(f, 0, 0);
    return fwrite(s, sizeof(superblock), 1, f);
}


int inode_create(superblock *s, inode *l) {
    inode *list;
    int result;
    int i;

    list = 0;
    result = 0;
    i = 0;


    list = malloc(sizeof(*list) * s->MAX_INODE_NUMBER);
    
    
    fseek(f, (s->index_inode * BLOCK_SIZE), 0);

    result = fread(list, sizeof(inode), s->MAX_INODE_NUMBER, f);                   /** Get entire list of inodes */
    if(result != s->MAX_INODE_NUMBER) {
        printf("Inode list incomplete\n");
    }

    for(i=0;i<result;i++){                                                          /** Find the first free one */
        if(list[i].used == 0)
            break;
    }

    if(i == result){
        printf("No free inodes remaining\n");
        l->inode_number = -1;
        goto end;
    }

    l->data_index = 0;                                                             /** init the inode */
    l->file_size = 0;
    l->inode_number = list[i].inode_number;
    l->used = 1;
    l->type = text_file;


    end:
        free(list);

    return l->inode_number;
}

/** 
 *  dir entry list
 *  .  me   
 *  .. parent
 *  example.txt
 *  other_dir
*/

int inode_create_dir(superblock *s, inode *parent, int parent_num_files, const char *name) {
    
    direntry temp_dir;
    inode new_inode;
    

    memset(&new_inode, 0, sizeof(new_inode));
    memset(&temp_dir, 0, sizeof(temp_dir));

    inode_create(s, &new_inode);

    new_inode.type = directory;
    new_inode.file_size = sizeof(direntry) * 2;

    inode_file_data_commit(s, &new_inode);

    temp_dir.inode_number = new_inode.inode_number;
    strcpy(temp_dir.name, ".");
    fseek(f, new_inode.data_index * BLOCK_SIZE, 0);
    fwrite(&temp_dir, sizeof(direntry), 1, f);

    temp_dir.inode_number = parent->inode_number;
    strcpy(temp_dir.name, "..");
    fwrite(&temp_dir, sizeof(direntry), 1, f);

    /** add to parent dir list */
    temp_dir.inode_number = new_inode.inode_number;
    strcpy(temp_dir.name, name);

    fseek(f, (parent->data_index * BLOCK_SIZE) + (sizeof(direntry) * parent_num_files), 0 );
    fwrite(&temp_dir, sizeof(direntry), 1, f);

    parent->file_size += sizeof(temp_dir);

    inode_update(parent);

    

    return 0;
}

int inode_create_text_file(superblock *s, inode *parent, int parent_num_files, const char *name){
    direntry temp_dir;
    inode *new_inode;

    new_inode = 0;
    new_inode = malloc(sizeof(*new_inode));
    memset(new_inode, 0, sizeof(*new_inode));
    memset(&temp_dir, 0, sizeof(temp_dir));

    inode_create(s, new_inode);
    inode_file_data_commit(s, new_inode);

    temp_dir.inode_number = new_inode->inode_number;
    strcpy(temp_dir.name, name);

    fseek(f, (parent->data_index * BLOCK_SIZE) + (sizeof(direntry) * parent_num_files), 0);
    fwrite(&temp_dir, sizeof(direntry), 1, f);

    parent->file_size += sizeof(temp_dir);
    inode_update(parent);

    free(new_inode);


    return 0;

}




/** 
 *  First version
 *  File size is 512 bytes by default
 * 
*/

/** 
 * set bit  number |= 1ULL << n; n is bit want to set
 * clear bit number &= ~(1UL << n);
 * bit = (number >> n) & 1U;
 * 
 * https://stackoverflow.com/questions/47981/how-do-you-set-clear-and-toggle-a-single-bit
*/



int inode_file_data_commit(superblock *s, inode *l) {
    int block;
    int i;
    unsigned char block_of_bits;

    char temp[512];

    block_of_bits = 0;

    for(i=0;i<(int)(BLOCK_SIZE/sizeof(unsigned char));i++) {
        fseek(f, (s->index_bitmap * BLOCK_SIZE) + (sizeof(unsigned char) * i),0);
        fread(&block_of_bits, sizeof(unsigned char),1, f);

        for(block = 0;block < (int)(sizeof(unsigned char) * BITS_IN_A_BYTE);block++) {
            if(!(block_of_bits >> block) & 1){
                goto exit_loop;
            }
        }
    }

    exit_loop:

    if( i == (BLOCK_SIZE / sizeof(unsigned char))) {
        printf("No free data blocks %d\n",block);
        return -1;
    }

    memset(temp, 0, sizeof(temp));
    
    block = block + (i * sizeof(unsigned char));

    block_of_bits |= 1 << block;

    fseek(f, (s->index_bitmap * BLOCK_SIZE) + i,0 );
    fwrite(&block_of_bits, sizeof(unsigned char), 1, f);

    l->data_index = block;

    inode_update(l);

    return l->data_index;
}


int split_path(char **dest, const char *source, int length){

    int items_split;
    int source_index;
    int temp_index;
   

    char temp[512];

    source_index = 0;
    items_split = 0;
    temp_index = 0;
    memset(temp, 0, sizeof(temp));

    if(source[source_index] == '/'){
        dest[items_split] = malloc(sizeof(char) * 2);
        strcpy(dest[items_split], "/");
        items_split++;
        source_index++;
    }

    while(source_index < length) {

        if(source_index == length || source[source_index] == '/' || source[source_index] == '\0'){
            temp[temp_index] = '\0';
            if(strlen(temp) == 0)
                break;
            dest[items_split] = malloc(temp_index + 1);
            strcpy(dest[items_split], temp);
            memset(temp, 0 , sizeof(temp));
            temp_index = 0;
            items_split++;
            source_index++;
            continue;
        }

        temp[temp_index] = source[source_index];
        source_index++;
        temp_index++;
    }

    return items_split;

}


int print_list_inodes(direntry *dir, int length){
    int i;

    for(i=0;i<length;i++){
        printf("%d | %s\n", dir[i].inode_number, dir[i].name);
    }

    return 0;
}



int root_inode_get(superblock *s, inode *root, direntry **file_list){
    
    int number_of_files;
    number_of_files = 0;

    fseek(f, (s->index_inode * BLOCK_SIZE), 0);
    fread(root, sizeof(inode), 1, f);

    number_of_files = root->file_size / sizeof(direntry);

    

    fseek(f, root->data_index * BLOCK_SIZE, 0);

    *file_list = 0;

    *file_list = malloc(sizeof(*(*file_list)) * MAX_FILE_AMOUNT);

    fread(*file_list, sizeof(direntry), number_of_files, f);

    return number_of_files;

}


int inode_fetch(superblock *s, inode *l, int inode_number) {
    fseek(f, (s->index_inode * BLOCK_SIZE) + (sizeof(inode) * inode_number), 0);
    fread(l, sizeof(inode), 1, f);

    if(l->inode_number != inode_number){
        printf("error obtaining correct inode.\n");
        exit(0);
    }

    

    return l->inode_number;
}



int path_get_inode(superblock *s, char **src, int src_items, char **dst, int *dst_items, inode *l){

    int i;
    int index;

    int result;

    inode root_inode;
    direntry *dir;

    inode current_inode;
    inode parent_inode;

    int src_index;
    int dst_index;
    


    int current_inode_number_of_files;
    result = 0;
    current_inode_number_of_files = 0;
    src_index = 0;
    *dst_items = 0;
    dst_index = 0;
    
    
    for(i=0;i<MAX_FILE_AMOUNT;i++)
        dst[i] = 0;
    


    current_inode_number_of_files = root_inode_get(s, &root_inode, &dir);

    current_inode = root_inode;
    parent_inode = root_inode;


    dst[dst_index] = malloc(sizeof(char) * 2);
    strcpy(dst[dst_index], "/");
    src_index++;
    dst_index++;


    
   


    while(src_index < src_items) {

        for(i=0;i<current_inode_number_of_files;i++){
            if(strcmp(src[src_index], dir[i].name) == 0){
                break;
            }
        }

        if(i == current_inode_number_of_files) {
            for(index=1;index<dst_index;index++){
                printf("/%s", dst[index]);
            }
            printf("/%s is not a directory: 1\n", src[src_index]);

            result = -404;
            goto exit;
        }

        if(strcmp(".", src[src_index]) == 0 ) {
                src_index++;
                continue;
            }

        if(strcmp("..", src[src_index]) == 0 ) {
            current_inode = parent_inode;
            current_inode_number_of_files = current_inode.file_size / sizeof(direntry);

            fseek(f, current_inode.data_index * BLOCK_SIZE, 0);
            fread(dir, sizeof(direntry), current_inode_number_of_files, f);
            free(dst[dst_index]);
            src_index++;
            dst_index--;
            continue;
        }

        parent_inode = current_inode;

        inode_fetch(s, &current_inode, dir[i].inode_number);

        if(current_inode.type != directory) {
            for(index=1;index<dst_index;index++){
                printf("/%s", dst[index]);
            }
            printf("/%s is not a directory: 2\n", src[src_index]);
            result = -404;
            goto exit;
        }

        

        dst[dst_index] = malloc(strlen(src[src_index]) +1);
        strcpy(dst[dst_index], src[src_index]);

        current_inode_number_of_files = current_inode.file_size / sizeof(direntry);

        fseek(f, current_inode.data_index * BLOCK_SIZE, 0);
        fread(dir, sizeof(direntry), current_inode_number_of_files, f);

        src_index++;
        dst_index++;

       
    }


    l->data_index = current_inode.data_index;
    l->file_size = current_inode.file_size;
    l->inode_number = current_inode.inode_number;
    l->type = current_inode.type;
    l->used = current_inode.used;


    exit:

    free(dir);

    if(result != 0)
        current_inode_number_of_files = result;
    
    *dst_items = dst_index;


    return current_inode_number_of_files;
}




int path_convert_to_full_path(sequence *user_path, char **path_split) {

    char temp[512];
    int items_split;
    

    items_split = 0;

    if(user_path->string[0] != '/') {
        memset(temp, 0, sizeof(temp));
        strcpy(temp, path);
        strcpy(temp + strlen(path), user_path->string);
        items_split = split_path(path_split, temp, strlen(temp) + 1);
    } else {
        items_split = split_path(path_split, user_path->string, user_path->length);
    }

    

  


    if(items_split == 1)
        items_split = 0;

    
    
    return items_split;
}