#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parent.h"

void parent(request* req, response* res, FILE* myfile) {
    sem_wait(&req->full) ;
    sem_wait(&req->mutex) ;
    //update share memory segment's data
    res->child_res = req->buffer[res->num_of_response].child_i; //number of child
    res->line_you_asked = req->buffer[res->num_of_response].number_of_line ; //requested line
    char every_line[101] ; 
    int num_of_line = 0 ;
    //search the txt file to find the line the child asked
    while((fgets(every_line,sizeof(every_line),myfile))!=NULL  && num_of_line!=res->line_you_asked) {
        num_of_line++ ;
        if(num_of_line==res->line_you_asked) {
            strcpy(res->line,every_line); //line found so copy it inside share memory segment
        }
    }
    sem_post(&res[res->child_res].buff_sem) ; //child now knows that the line it asked has been written in share memory segment
    res->num_of_response++ ; //increase number of responses
    sem_post(&req->my_turn) ; //now another request can be sent
    fclose(myfile);
    sem_post(&req->mutex) ;
    sem_post(&req->empty) ;    
}