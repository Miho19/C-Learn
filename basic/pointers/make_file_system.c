#include "test_file_system.h"

#define VERSION 2.0


void file_system_make(void);
void superblock_get(superblock *s);
int ionde_get_list(inode *list, int off, int n);
int file_commit_write(user_file *file, inode *l);


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

   path = malloc(sizeof(path[0]) * 50);
   current_dir = malloc(sizeof(*current_dir));

   memset(path, 0, sizeof(path[0]) * 50);
   current_dir->inode_number = 0;
   strcpy(current_dir->name, "/");

   
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
    
    fclose(f);
    free(path);
    free(current_dir);
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

    dir.inode_number = l.inode_number;
    strcpy(dir.name, "/");

    l.file_size = sizeof(dir);

    fseek(f, sizeof(superblock), 0);
    fwrite(&l,sizeof(inode), 1, f);

    
    fseek(f, (l.data_index * BLOCK_SIZE), 0);
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
    l->type = 0;


    end:
        free(list);

    return l->inode_number;
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

void file_print_superblock(){
    superblock s;
    int i;

    superblock_get(&s);

    printf("type %d\n", s.type);
    printf("size %d\n", s.size);
    printf("number of data blocks %d\n", s.number_of_blocks);
    printf("max number of inodes %d\n", s.number_inodes_max);
    printf("starting index of inode %d\n", s.index_inodes);
    printf("starting index of data %d\n", s.index_data);

    printf("block |status\n");

    for(i=0;i<s.number_of_blocks;i++){
        printf("block %d|%d\n",i,(s.data_block_free_list >> i) & 1); 
    }    
}

/** search through root "/" direntry for filename */

int file_open(char *name){

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
            if(strcmp(mem_inode_list[i]->file->name, name) == 0) {
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
        if(strcmp(dir[dir_index].name, name) == 0) {
            break;
        }
    }

    mem_inode_list[index_free_mem_list] = malloc(sizeof(mem_inode));   
    
    mem_inode_list[index_free_mem_list]->user_inode = malloc(sizeof(inode));
    mem_inode_list[index_free_mem_list]->fd = ++fd_counter;

    mem_inode_list[index_free_mem_list]->file = malloc(sizeof(user_file));
    mem_inode_list[index_free_mem_list]->file->name = malloc(strlen(name));
    
    strcpy(mem_inode_list[index_free_mem_list]->file->name, name);
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


void file_print_all_files() {
    int i;

    for(i=0;i<MAX_FILE_AMOUNT;i++){
        if(mem_inode_list[i]) {
            printf("fd: %d inode: %d | name: %s | data length: %d | offset %d |\n",mem_inode_list[i]->fd, mem_inode_list[i]->file->inode_number, mem_inode_list[i]->file->name, mem_inode_list[i]->file->data_length,mem_inode_list[i]->file->off);
        }
    }
}

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

int file_read(int fd) {
    int mem_inode_index;

    for(mem_inode_index=0;mem_inode_index<MAX_FILE_AMOUNT;mem_inode_index++) {
        if(mem_inode_list[mem_inode_index]->fd == fd){
            break;
        }
    }

    if(mem_inode_index == MAX_FILE_AMOUNT) {
        printf("FD not valided\n");
        return -1;
    }

    printf("%s\n", mem_inode_list[mem_inode_index]->file->data);

    return strlen(mem_inode_list[mem_inode_index]->file->data);
}



int file_write(int fd, const void *data, int mode){
    int mem_inode_index;

    for(mem_inode_index=0;mem_inode_index<MAX_FILE_AMOUNT;mem_inode_index++) {
        if(mem_inode_list[mem_inode_index]->fd == fd){
            break;
        }
    }

    if(mem_inode_index == MAX_FILE_AMOUNT) {
        printf("FD not valided\n");
        return -1;
    }

    if(mem_inode_list[mem_inode_index]->file->data == 0) {
        mem_inode_list[mem_inode_index]->file->data = malloc(BLOCK_SIZE);
        memset(mem_inode_list[mem_inode_index]->file->data, 0, BLOCK_SIZE);
        strcpy(mem_inode_list[mem_inode_index]->file->data, data);
        mem_inode_list[mem_inode_index]->file->data_length += strlen(data);
        mem_inode_list[mem_inode_index]->file->off += strlen(data);
        return strlen(data);
    }

    if(mode == FILE_APPEND) {
        mem_inode_list[mem_inode_index]->file->data = realloc(mem_inode_list[mem_inode_index]->file->data, BLOCK_SIZE);
        strcpy(mem_inode_list[mem_inode_index]->file->data + mem_inode_list[mem_inode_index]->file->data_length, data);
        mem_inode_list[mem_inode_index]->file->data_length += strlen(data);
        mem_inode_list[mem_inode_index]->file->off = mem_inode_list[mem_inode_index]->file->data_length;
        return strlen(data);
    } else {
        mem_inode_list[mem_inode_index]->file->data = realloc(mem_inode_list[mem_inode_index]->file->data, BLOCK_SIZE);
        memset(mem_inode_list[mem_inode_index]->file->data, 0, BLOCK_SIZE);
        strcpy(mem_inode_list[mem_inode_index]->file->data, data);
        mem_inode_list[mem_inode_index]->file->data_length = strlen(data);
        mem_inode_list[mem_inode_index]->file->off = strlen(data);
        return strlen(data);
    }
    

    
    return 1;
}

int file_commit_write(user_file *file, inode *l) {
    
    if(file->data_length > BLOCK_SIZE)
        file->data_length = BLOCK_SIZE;

    l->file_size = file->data_length;
    inode_update(l);

    fseek(f, (l->data_index * BLOCK_SIZE), 0);
    fwrite(file->data, sizeof(char), file->data_length, f);

    return 1;
}
