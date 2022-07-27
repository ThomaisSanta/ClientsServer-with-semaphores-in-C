#pragma once    //to be declared only once
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/wait.h>
#define LINESIZE 101 //100 is the maximum number of characters of every line + 1 for \n
#define MAX_REQUESTS 10000 //maximum number for total requests (K * N)

//two shared memory segments, one for requests and one for responses

typedef struct request_item {   
    int child_i ;           //number of child 
    int number_of_line ;    //number of line a child requests
} request_item;

//requests
typedef struct request {   
    request_item buffer[MAX_REQUESTS] ; // buffer contains number of child and line
    sem_t empty ;   
    sem_t full ;
    sem_t mutex ;
    sem_t lock ;    //mutual exclusion for printf command
    sem_t my_turn ;
    int num_of_request;
}request;

//responses
typedef struct response {
    char line[LINESIZE] ; //array where parent writes the line
    // for mutual exclusion, down buff_sem when child requests
    // and up buff_sem when parent writes inside line[LINESIZE]
    sem_t buff_sem ;
    int num_of_response ;
    int child_res ;
    int line_you_asked ;
} response ;
