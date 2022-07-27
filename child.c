#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "child.h"

double total_time = 0.0 ;

double child(request* req,response* res,int child, int lines,clock_t first, clock_t last) {
    first = clock() ; //starting time of the request
    sem_wait(&req->my_turn) ;  // for synchronization between child and parent process
    sem_wait(&req->empty) ;
    sem_wait(&req->mutex) ;
    //update shared memory segment's data
    req->buffer[req->num_of_request].child_i = child ; 
    // generate a random number from 1 to lines (lines = 85 if pink_floyd_lyrics.txt is used) 
    req->buffer[req->num_of_request].number_of_line = rand() % lines + 1 ; 
    sem_wait(&req->lock) ; //mutual exclusion for printf command
    printf("Child  %d to parent:  %d\n",child+1, req->buffer[req->num_of_request].number_of_line) ;
    sem_post(&req->lock) ;
    req->num_of_request++ ; //increase request number
    sem_post(&req->mutex) ;
    sem_post(&req->full) ;
    sem_wait(&res[child].buff_sem); //wait for response
    last = clock() ;  //ending time of the request
    total_time = (double) (last - first) / CLOCKS_PER_SEC;
    sem_wait(&req->lock) ; 
    //print the line that the child asked
    printf("Parent to child  %d  line  %d:  %s\n",res->child_res+1,res->line_you_asked,res->line) ;
    sem_post(&req->lock) ;
    return total_time ; //return the time the request took
}