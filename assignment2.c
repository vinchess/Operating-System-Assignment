/*
 *  Vincent Lim Jan Chian
 *  vince.lim@outlook.com
 *  0321775
 *  OS Assignment 2
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#define LINE_BUFFER_SIZE 1024       /* buffer size for malloc */
#define BUFFER_SIZE 16              /* buffer size for array of pointers */
#define BUFFER_START 0              /* buffer start flag */
#define BUFFER_END BUFFER_SIZE-1    /* buffer end flag */
#define READ 0                      /* Reading flag */
#define WRITE 1                     /* Writing flag */
#define READ_FILE 1                 /* Read file flag */
#define WRITE_FILE 2                /* Write file flag */
#define THREAD_COUNT 3              /* Thread count flag */
#define MIN_THREAD 1                /* Min Thread flag */
#define NUM_OF_ARGC 4               /* Number of argument flag */

typedef enum { false, true } bool;

pthread_mutex_t godLock = PTHREAD_MUTEX_INITIALIZER; /* glorious mutex */
bool read_loop = true;              /* read loop flag */
bool write_loop = true;             /* write loop flag */
bool end_of_file = false;           /* end of file flag */
int read_counter = 0;               /* read array counter */
int write_counter = 0;              /* write array counter */
FILE *fpRead, *fpWrite;             /* read and write file pointers */
char *buffer[BUFFER_SIZE] = {NULL}; /* For storing messages to read and write*/

bool file_exists(char *fileName);   /* checks if a specific file exists */
int reset_counter(int count);       /* resets counter if counter == BUFFER_SIZE */
void *read_function(void *data);    /* function to read from file */
void *write_function(void *data);   /* function to write to file */


int main(int argc,char *argv[]){

    int i;                      /* For use in for loops */
    
    printf("=== PROGRAM STARTED ===\n");
    
    /* Validate the numbers of arguments */
    if(argc != NUM_OF_ARGC){
        puts("Not enough arguments...");
        exit(EXIT_FAILURE);
    }
    
    /* Check if file exists */
    if(!file_exists(argv[READ_FILE])){
        puts("Source file error...");
        exit(EXIT_FAILURE);
    }

    int thread_count = atoi(argv[THREAD_COUNT]);
    
    /* Validate the amount of threads */
    if(thread_count < MIN_THREAD){
        puts("Invalid thread amount. Must be more than 0.");
        exit(EXIT_FAILURE);
    }
    
    pthread_t thread[2][thread_count];      /* Creates array for read and write thread */
    
    fpRead = fopen(argv[READ_FILE],"r");    /* Read-from pointer */
    fpWrite = fopen(argv[WRITE_FILE],"w");  /* Write-to pointer */
    
    for(i = 0 ; i < thread_count ; i++){
        /* Creates thread for read_function */
        if(pthread_create( &thread[READ][i], NULL, read_function, NULL) != 0){
            puts("Thread creation error....");
            exit(EXIT_FAILURE);
        }
        /* Creates thread for write_function */
        if(pthread_create( &thread[WRITE][i], NULL, write_function, NULL) != 0){
            puts("Thread creation error....");
            exit(EXIT_FAILURE);
        }
    }
    printf("%d threads created.\n",thread_count*2);
    
    /* wait for all threads to complete */
    for(i = 0 ; i < thread_count ; i++){
        pthread_join(thread[READ][i],NULL);
        pthread_join(thread[WRITE][i],NULL);
    }
    
    printf("Threads completed. Closing file pointers.\n");
    fclose(fpRead);     /* close read file pointer */
    fclose(fpWrite);    /* close write file pointer */
    
    printf("=== PROGRAM COMPLETED ===\n");
    
    return EXIT_SUCCESS;
}

bool file_exists(char *fileName){
    /* Access attempt of file */
    if(access(fileName, F_OK) != -1){
        return true;
    }
    return false;
}

int reset_counter(int count){
    /* check to see if counter need to reset */
    if(count == BUFFER_SIZE){
        return 0;
    }
    return count;
}

bool bufferFull(){
    /* check if buffer is full */
    if((read_counter + 1 == write_counter)){
        return true;
    }else if((write_counter == BUFFER_START && read_counter == BUFFER_END)){
        return true;
    }else{
        return false;
    }
}

void *read_function(void *data){
    /* loop read process until file fully read */
    while(read_loop){
        pthread_mutex_lock( &godLock );
        /* proceed to malloc is buffer not full */
        if(!bufferFull()){
            char *alloc = (char *)malloc(LINE_BUFFER_SIZE);
            /*  read from file, put to buffer, increment counter.
                if file reach the end, set end_of_file true and read_loop false */
            if(fgets(alloc,LINE_BUFFER_SIZE,fpRead)){
                buffer[read_counter] = alloc;
                read_counter++;
                read_counter = reset_counter(read_counter);
            }else{
                free(alloc);
                end_of_file = true;
                read_loop = false;
            }
        }
        pthread_mutex_unlock( &godLock );
    }
    return NULL;
}
void *write_function(void *data){
    /* loop write process until no more data to write */
    while(write_loop){
        pthread_mutex_lock( &godLock );
        /*  get data from buffer and write to file, increment counter.
            if buffer NULL and if end_of_file flag is true, write_loop false */
        if(buffer[write_counter] != NULL){
            fputs(buffer[write_counter],fpWrite);
            free(buffer[write_counter]);
            buffer[write_counter] = NULL;
            write_counter++;
            write_counter = reset_counter(write_counter);
        }else{
            if(end_of_file){
                write_loop = false;
            }
        }
        pthread_mutex_unlock( &godLock );
    }
    return NULL;
}

