#include "test_file_system.h"


int __read(char *buf, int index, int n){
    int i;
    for(i=0;i<n && index < BUFFER_SIZE;i++, index++) {
        buf[i] = buffer_space[index];
    }    
    return i;
}

int __write(char *buf, int index, int n) {
    int i;
    for(i=0;i<n && index < BUFFER_SIZE;i++,index++) {
        buffer_space[index] = buf[i];
    }
        
    return i;
}


int buffer_write(file *f, char *buf, int num) {
    int result;
    int i;
    int amount_write;
    int amount_to_write;
    int buf_index;
    int file_position;
    char temporay[BLOCK_SIZE];
    int file_pos_finder;
    file file_temp;

    result = 0;
    amount_write = 0;
    amount_to_write = 0;
    buf_index = 0;
    file_pos_finder = 0;
    file_position = f->off;
    file_temp.off = f->off;

   
    if(num > BUFFER_SIZE) {
        amount_to_write = BUFFER_SIZE;
    } else {
        amount_to_write = num;
    }

    if(amount_to_write + file_position > BUFFER_SIZE) {
        amount_to_write = BUFFER_SIZE - file_position;
    }

    if(amount_to_write <= 0)
        return 0;

    while(amount_write < amount_to_write) {

        for(i=0;i<BLOCK_SIZE;i++)
            temporay[i] = 0;
        

        
        /** Not at Start of Block*/
        if(((amount_to_write & MASK) != 0 || (file_position & MASK) != 0)) {
            file_pos_finder = file_position;
            
            

            while((file_pos_finder & MASK) != 0 && file_pos_finder > 0) {  /** Loop until at start of block*/
                file_pos_finder--;
            }

            

            file_temp.off = file_pos_finder; /** At Start of block according to file position index*/
            result = buffer_read(&file_temp, temporay, BLOCK_SIZE); /** Read in block*/

            
            

            for(i=(file_position % BLOCK_SIZE);buf_index < amount_to_write && i < BLOCK_SIZE;i++, buf_index++){
                temporay[i] = buf[buf_index];
            }

            /* *printf("%s amount_to_write %d file_position %d file_pos_ %d\n", buf, amount_to_write, file_position, file_pos_finder); */

            result = __write(temporay, file_pos_finder, BLOCK_SIZE);

            if(result != BLOCK_SIZE) {
                printf("Error: WRITE| MASK READ FAILED TO RETURN 512\n");
            }

            amount_write += i;
            file_position += i;


    

        } else {
            
            for(i=0;buf_index<amount_to_write && i < BLOCK_SIZE;i++,buf_index++){
                temporay[i] = buf[buf_index];
            }

            result = __write(temporay, file_position, BLOCK_SIZE);

            if(result != BLOCK_SIZE) {
                printf("Error: WRITE| STANDARD WRITE DID NOT RETURN 512\n");
            }

            amount_write += i;
            file_position += i;
        
        }
    
    }

    f->off = file_position;
    return buf_index;
}



int buffer_read(file *f, char *buf, int num) {
    
    int result;
    int i;
    int amount_read;
    int amount_to_read;
    int buf_index;
    int file_position;
    int file_position_to_start;
    char tempoary[BLOCK_SIZE];

    result = 0;
    i = 0;
    buf_index = 0;
    amount_read = 0;
    file_position = f->off;
    file_position_to_start = 0;

    if(num > BUFFER_SIZE) {
        amount_to_read = BUFFER_SIZE;
    } else {
        amount_to_read = num;
    }

    if(file_position + amount_to_read > BUFFER_SIZE) {
        amount_to_read = BUFFER_SIZE - file_position;
    }

    if(amount_to_read < 0)
        return 0;

    while(amount_read < amount_to_read) {


        if((amount_to_read & MASK) != 0 || (file_position & MASK) != 0) {
            file_position_to_start = file_position;

            while((file_position_to_start & MASK) != 0 && file_position_to_start > 0) {
                file_position_to_start--;
            }

           

            result = __read(tempoary, file_position_to_start,BLOCK_SIZE);

            if(result != BLOCK_SIZE) {
                printf("Error: Read| MASK READ FAILED TO RETURN 512\n");
            }

            for(i=(file_position % BLOCK_SIZE);i<BLOCK_SIZE && buf_index < amount_to_read;buf_index++, i++){
                buf[buf_index] = tempoary[i];
            }

            file_position += i;
            amount_read += i;

        } else {
            result = __read(tempoary, file_position,BLOCK_SIZE);
            if(result != BLOCK_SIZE) {
                printf("Error: Read| STANDARD BLOCK READ DID NOT RETURN 512\n");
            }
            for(i=0;buf_index<amount_to_read && i < BLOCK_SIZE;buf_index++, i++){
                buf[buf_index] = tempoary[i];
            }

            file_position += i;
            amount_read += i;
            
        }

    }
    
    f->off = file_position;
    
    return buf_index;
}

void buffer_reset(void) {
    int i;
     for(i=0;i<BUFFER_SIZE;i++) {
         buffer_space[i] = '0';
         if( (i & MASK) == 1) {
             buffer_space[i-1] = 'X';
         }
     }
}

void buffer_to_file(void) {
   int i;
   for(i=0;i<BUFFER_SIZE;i++){
       fprintf(f, "|%d|%c\n", i, buffer_space[i]);
   }
}

int file_set_position(file *f, int n) {
    int test;

    test = n;
    if(test > BUFFER_SIZE){
        test = BUFFER_SIZE;
        printf("SIZE > f->off %d \n", f->off);
    } 
        
    if(test < 0) {
        printf("SIZE < 0 f->off %d \n", f->off);
        test = 0;
    }
        

    f->off = test;

    
    return f->off;
}

void init(void){

    buffer_space = malloc(sizeof(buffer_space[0]) * BUFFER_SIZE);
    user_file = malloc(sizeof(*user_file));

    user_file->off = 0;

    buffer_reset();

    f = fopen("output.txt", "w");
    if(f == NULL) {
       printf("file open error\n");
       exit(1);
   }

}

void destroy(void) {
    fclose(f);
    free(user_file);
    free(buffer_space);
    
}
