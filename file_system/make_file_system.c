#include "test_file_system.h"

#define VERSION 2.0


void file_system_make(void);
void superblock_get(superblock *s);
int ionde_get_list(inode *list, int off, int n);
int file_commit_write(user_file *file, inode *l);
int display_text(const char *data, int length);
int inode_file_data_commit(superblock *s, inode *l);


/*** 
 * 0-28 -> superblock
 * 28->512 | inode list | (512-28 / 16) = number of inodes -> 30
 * 512 -> buffer_size = data blocks
 * first inode and entry is . directory
*/

int BUFFER_SIZE = 4096;

int fd_counter = 0;

mem_inode *mem_inode_list[MAX_FILE_AMOUNT];



void init(int size){
    superblock s;
    int i;
    BUFFER_SIZE = size;
    
    f = fopen("output.txt", "r+b");

    if(f == NULL) {
       printf("creating output.txt\n");
       f = fopen("output.txt", "w+b");
       file_system_make();
       superblock_get(&s);

       if(s.type != VERSION) {
           printf("error creating superblock\n");
           exit(1);
       }
   }

   path = malloc(sizeof(path[0]) * BLOCK_SIZE);
   memset(path, 0, sizeof(path[0]) * BLOCK_SIZE);
 
   path[0] = '/';
   path[1] = '\0';


   rq = malloc(sizeof(request));
   rq->file_command = 0;
   rq->file_data = 0;
   rq->file_name = 0;
   rq->status = 0;


   

   

   
    for(i=0;i<MAX_FILE_AMOUNT;i++)
        mem_inode_list[i] = 0;
}

void destroy(void) {
    int i;

    for(i=0;i<MAX_FILE_AMOUNT;i++) {
        if(mem_inode_list[i]){

            file_commit_write(mem_inode_list[i]->file,mem_inode_list[i]->user_inode);

            if(mem_inode_list[i]->file->name)
                free(mem_inode_list[i]->file->name);

            if(mem_inode_list[i]->file->data)
                free(mem_inode_list[i]->file->data);

            if(mem_inode_list[i]->user_inode)
                free(mem_inode_list[i]->user_inode);

            free(mem_inode_list[i]->file);
            free(mem_inode_list[i]);
        }


    }

    if(rq->file_command)
        free(rq->file_command->string);
    if(rq->file_data)
        free(rq->file_data->string);
    if(rq->file_name)
        free(rq->file_name->string);
    free(rq);

    fclose(f);
    free(path);

}





void file_system_make(void) {

    int i;
    inode l;
    superblock *s;
    direntry dir;
    s = malloc(sizeof(*s));

    
   
    s->type = VERSION;
    s->size = BUFFER_SIZE;
    s->number_of_blocks = (BUFFER_SIZE / BLOCK_SIZE);
    s->index_inodes = sizeof(superblock);
    s->index_data = BLOCK_SIZE;
    s->number_inodes_max = (BLOCK_SIZE - sizeof(superblock)) / sizeof(inode);
    s->data_block_free_list = 0;

    s->data_block_free_list |= 1 << 0;
    s->data_block_free_list |= 1 << 1;

    fseek(f, 0, 0);
    fwrite(s, sizeof(superblock), 1, f);
    
 

    for(i=0;i<s->number_inodes_max;i++){
        l.data_index = 0;
        l.file_size = 0;
        l.inode_number = i;
        l.used = 0;
        l.type = text_file;
        fwrite(&l, sizeof(inode), 1, f);
    }

    l.data_index = 1;
    l.inode_number = 0;
    l.used = 1;
    l.type = directory;
    l.file_size = (sizeof(dir) * 2);

    fseek(f, sizeof(superblock), 0);
    fwrite(&l,sizeof(inode), 1, f);


    dir.inode_number = l.inode_number;
    strcpy(dir.name, ".");

    fseek(f, (l.data_index * BLOCK_SIZE), 0);
    fwrite(&dir, sizeof(dir), 1, f);


    dir.inode_number = l.inode_number;
    strcpy(dir.name, "..");
    fseek(f, (l.data_index * BLOCK_SIZE) + sizeof(direntry), 0);
    fwrite(&dir, sizeof(dir), 1, f);

    
    free(s);
}

void superblock_get(superblock *s) {
    fseek(f, 0, 0);
    fread(s, sizeof(superblock),1, f);
}


int inode_update(inode *i) {
    superblock s;
    superblock_get(&s);
    fseek(f, (s.index_inodes + (i->inode_number * sizeof(*i) )), 0);
    return fwrite(i, sizeof(*i), 1, f);

}
int superblock_update(superblock *s) {
    fseek(f, 0, 0);
    return fwrite(s, sizeof(superblock), 1, f);
}


int inode_create(superblock *s, inode *l) {
    inode *list;
    int result;
    int i;

    list = malloc(sizeof(*list) * s->number_inodes_max);
    
    result = 0;
    fseek(f, s->index_inodes, 0);
    result = fread(list, sizeof(inode), s->number_inodes_max, f);                   /** Get entire list of inodes */
    if(result != s->number_inodes_max) {
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
    inode *list;
    direntry temp_dir;
    inode new_inode;
    int i;

    list = malloc(sizeof(inode) * s->number_inodes_max);
    fseek(f,s->index_inodes, 0);
    fread(list, sizeof(inode), s->number_inodes_max, f);

    for(i=0;i<s->number_inodes_max;i++){
       if(list[i].used == 0) 
            break;
    }

    if(i == s->number_inodes_max){
        printf("reached maxed size of file system\n");
        exit(0);
    }

    new_inode.data_index = 0;
    new_inode.file_size = sizeof(direntry) * 2;
    new_inode.inode_number = i;
    new_inode.type = directory;
    new_inode.used = 1;


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


    free(list);

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

    for(block=0;block<(int)(sizeof(unsigned char) * 8);block++){
        if(!(s->data_block_free_list >> block) & 1){
            break;
        }
    }

    if(block >= (int)(sizeof(unsigned char) * 8)){
        printf("No free data blocks %d\n",block);
        return -1;
    }

    s->data_block_free_list |= 1 << block;                                      /** commit changes to superblock */
    superblock_update(s);
    
    l->data_index = block;                                                      /** update block field */
    inode_update(l);

    return 1;
}


int file_create(user_file *file, inode *l) {
    superblock s;
    int result;

    result = 0;

    superblock_get(&s);

    result = inode_create(&s,l);

    if(result < 0) {
        printf("Error allocating inode\n");
        return result;
    }

    printf("creating text file: %s\n", file->name);
    result = inode_file_data_commit(&s, l);


    if(result < 0) {
        printf("error commiting to disk\n");
    }

    file->inode_number = l->inode_number;

    return result;
}

int file_commit_write(user_file *file, inode *l) {
   
    if(file->data_length > BLOCK_SIZE) {
        file->data_length = BLOCK_SIZE;
    }
    
    l->file_size = file->data_length;
    inode_update(l);

    fseek(f, (l->data_index * BLOCK_SIZE), 0);
    fwrite(file->data, sizeof(char), file->data_length, f);

    return 1;
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
    printf("inode | name\n");

    for(i=0;i<length;i++){
        printf("%d | %s\n", dir[i].inode_number, dir[i].name);
    }

    return 0;
}



int root_inode_get(superblock *s, inode *root, direntry **file_list){
    
    int number_of_files;

    number_of_files = 0;
    
    fseek(f, s->index_inodes, 0);
    fread(root, sizeof(inode), 1, f);

    number_of_files = root->file_size / sizeof(direntry);

    fseek(f, root->data_index * BLOCK_SIZE, 0);



    (*file_list) = malloc(sizeof(direntry) * MAX_FILE_AMOUNT);
    memset((*file_list), 0, MAX_FILE_AMOUNT * sizeof(direntry));

    fread((*file_list), sizeof(direntry), number_of_files, f);

    return number_of_files;

}


int inode_fetch(superblock *s, inode *l, int inode_number) {
    fseek(f, s->index_inodes + (sizeof(inode) * inode_number), 0);
    fread(l, sizeof(inode), 1, f);

    if(l->inode_number != inode_number){
        printf("???\n");
        exit(0);
    }

    

    return l->inode_number;
}








