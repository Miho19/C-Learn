#include "test_file_system.h"

#define VERSION 2.0


void file_system_make(void);
void superblock_get(superblock *s);
int ionde_get_list(inode *list, int off, int n);
int file_commit_write(user_file *file, inode *l);
int display_text(const char *data, int length);


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
   strcpy(path, "/");


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

int file_commit_write(user_file *file, inode *l) {
    int i;

    if(file->data_length > BLOCK_SIZE) {
        file->data_length = BLOCK_SIZE;
    }
    
    l->file_size = file->data_length;
    inode_update(l);

    fseek(f, (l->data_index * BLOCK_SIZE), 0);
    fwrite(file->data, sizeof(char), file->data_length, f);

    return 1;
}






