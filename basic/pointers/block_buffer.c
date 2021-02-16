#include <stdio.h>
#include <stdlib.h>


#define BUFFER_SIZE 4096

#define MASK (512 - 1)


char *buffer_space;


typedef struct file {
    int off;
} file;

file *user_file;

int buffer_read(file *f, char *buf, int num);
int buffer_write(file *f, char *buf, int num);

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

    char temporay[512];

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

        for(i=0;i<512;i++)
            temporay[i] = 0;
        

        
        /** Not at Start of Block*/
        if(((amount_to_write & MASK) != 0 || (file_position & MASK) != 0)) {
            file_pos_finder = file_position;
            
            

            while((file_pos_finder & MASK) != 0 && file_pos_finder > 0) {  /** Loop until at start of block*/
                file_pos_finder--;
            }

            

            file_temp.off = file_pos_finder; /** At Start of block according to file position index*/
            result = buffer_read(&file_temp, temporay, 512); /** Read in block*/

            
            

            for(i=(file_position % 512);buf_index < amount_to_write && i < 512;i++, buf_index++){
                temporay[i] = buf[buf_index];
            }

            /* *printf("%s amount_to_write %d file_position %d file_pos_ %d\n", buf, amount_to_write, file_position, file_pos_finder); */

            result = __write(temporay, file_pos_finder, 512);

            amount_write += (i);
            file_position += (i);


    

        } else {
            
            for(i=0;buf_index<amount_to_write && i < 512;i++,buf_index++){
                temporay[i] = buf[buf_index];
            }

            result = __write(temporay, file_position, 512);

            amount_write += result;
            file_position += result;
        
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

    char tempoary[512];

    result = 0;
    i = 0;
    
    buf_index = 0;

    amount_read = 0;

    file_position = f->off;

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

        result = __read(tempoary, file_position,512);
        
        for(i=0;buf_index<amount_to_read && i < result;buf_index++, i++){
            buf[buf_index] = tempoary[i];
        }

        file_position += i;
        amount_read += result;

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

void buffer_print(void) {
   FILE *f;
   int i;

   f = fopen("output.txt", "w");
   if(f == NULL) {
       printf("file open error\n");
       exit(1);
   }

   for(i=0;i<BUFFER_SIZE;i++){
       fprintf(f, "|%d|%c\n", i, buffer_space[i]);
   }
   fclose(f);
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
    

}

void destroy(void) {
    free(user_file);
    free(buffer_space);
}

void run(void) {
   
    int i;
    char buf[2048];

   

    for(i=0;i<2048;i++)
        buf[i] = '6';
   


    file_set_position(user_file, 0);
    buffer_write(user_file, buf, 2000);


    file_set_position(user_file, 0);

    buffer_write(user_file, "hello world", 11);

    

    file_set_position(user_file, BUFFER_SIZE-15);
    
    buffer_write(user_file, "Good Bye World!", 15);
    
    

    file_set_position(user_file, 512-3);
    buffer_write(user_file, "111111111111", 7);



    file_set_position(user_file, 512);

    for(i=0;i<512;i++){
        buf[i] = '2';
    }
    buffer_write(user_file, buf, 512);

    buffer_print();
    
    return;

  

}



int main(void){
  
  init();

  run();

  destroy();

    return 0;
}
