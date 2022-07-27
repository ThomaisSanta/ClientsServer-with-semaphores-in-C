#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 
#include "parent.h"
#include "child.h"

//command line arguments after executable's name: txt file, number of children 
//and number of requests
int main(int argc,char* argv[]) {
    if(argc>4) {
        perror("Error, too many arguments!\n") ;
        exit(EXIT_FAILURE) ;    
    }
    if (argc<4) {
        perror("Error, not enough arguments!\n");
        exit(EXIT_FAILURE) ;
    }
    int children = atoi(argv[2]), requests = atoi(argv[3]), lines=1;
    if(children*requests>MAX_REQUESTS) {
        perror("Sorry K * N must be <= 10000\n") ;
        exit(EXIT_FAILURE) ;
    }
    FILE *file = fopen(argv[1],"r") ;
    char new_line ;
    //if file cannot be opened exit from the program after printing a message
    if (file==NULL) {
        perror("Error occurred: could not open file\n");
		exit(EXIT_FAILURE);
    }
    //find the number of lines the txt file has 
    //and insert it in variable named lines
    while ((new_line=getc(file))!=EOF) {
        if(new_line=='\n') lines++ ; //'\n' means changing line in txt file
    }
    fclose(file);
    // request and response are structs in shared memory segment
    // and to see what is inside them check header file "shared_memory.h"
    request *req = NULL  ;  
    response *res = NULL ;  
    int shmid_req , shmid_res ;
    //create shared memory segment
    shmid_req = shmget(IPC_PRIVATE, sizeof(request), 0666 | IPC_CREAT) ;
    if (shmid_req == -1) {
		perror("shmget failed\n");
		exit(EXIT_FAILURE);
	}
	req = shmat(shmid_req,NULL,0) ;
    if (req == (request*)-1) {
		perror("shmat failed\n");
		exit(EXIT_FAILURE);
	}
    shmid_res = shmget(IPC_PRIVATE, children * sizeof(response), 0666 | IPC_CREAT) ;
    if (shmid_res == -1) {
		perror("shmget failed\n");
		exit(EXIT_FAILURE);
	}
	res = shmat(shmid_res,NULL,0) ;
    if (res == (response*)-1) {
		perror("shmat failed\n");
		exit(EXIT_FAILURE);
	}
    pid_t pid[children] ; //array containing process ids of children
    //initialize semaphores (add error handling)
    if(sem_init(&req->full,1,0)== -1) {
        perror("Failed to initialize semaphore!\n") ;
    }
    if(sem_init(&req->empty,1,children) == -1) {
        perror("Failed to initialize semaphore!\n") ;
    }
    if(sem_init(&req->mutex,1,1) == -1) {
        perror("Failed to initialize semaphore!\n") ;
    }
    if(sem_init(&req->lock,1,1) == -1) {
        perror("Failed to initialize semaphore!\n") ;
    }
    if(sem_init(&req->my_turn,1,1) == -1) {
        perror("Failed to initialize semaphore!\n") ;
    }
    for(int i=0;i<children;i++) {
        if(sem_init(&res[i].buff_sem,1,0) == -1) {
            perror("Failed to initialize semaphore!\n") ;
        } 
    }
    req->num_of_request = 0 ;
    res->num_of_response = 0 ;
    clock_t first=0, last=0 ;
    double total_time = 0.0;
    //create K children using fork()
    for(int i=0 ; i<children; i++) {
        pid[i] = fork() ;    //create a child and return its process id
        if(pid[i]<0) {
            perror("Fork failed, couldn't create children\n");
            exit(EXIT_FAILURE);
        }
        else if(pid[i]==0) {
            //feed for srand is getpid()+time(NULL) in order 
            // to generate different lines every time 
            srand(getpid()+time(NULL));
            //child proccess generates K*N requests
            for(int k=0; k<requests; k++)  {
                //child function is inside "child.c"
                total_time = child(req, res, i,lines,first,last) ; 
            }
            total_time = total_time / (double) requests ; //average time is asked
            sem_wait(&req->lock) ; // mutual exclusin for print
            printf("Time to execute for child  %d :  %f seconds\n",i+1,total_time) ;
            sem_post(&req->lock) ;
            exit(0) ;
        }  
    }
    //parent process running for all children for all the requests (K*N times)
    for (int i=0; i<children*requests;i++) {
        FILE* myfile=fopen(argv[1],"r") ;
        parent(req,res,myfile) ; //parent function is inside "parent.c"
        //wait if all the requests and all the responses have been done
        if(req->num_of_request==children*requests && res->num_of_response==children*requests) {
            int status;
            for (int j = 0; j < children; j++) {
                j = wait(&status);
            }
        }
    }
    //destroy semaphores
    sem_destroy(&req->empty) ;
    sem_destroy(&req->full) ;
    sem_destroy(&req->mutex) ;
    sem_destroy(&req->lock) ;
    for(int i=0;i<children;i++) {
        sem_destroy(&res[i].buff_sem) ;
    }
    sem_destroy(&req->my_turn) ;
    //detach shared memory segment
    if (shmdt(req) == -1) {
		perror("shmdt_request failed\n");
		exit(EXIT_FAILURE);
	}
	if (shmctl(shmid_req, IPC_RMID, 0) == -1) {
		perror("Request shmctl(IPC_RMID) failed\n");
		exit(EXIT_FAILURE);
	}
    if (shmdt(res) == -1) {
		perror("shmdt_response failed\n");
		exit(EXIT_FAILURE);
	}
	if (shmctl(shmid_res, IPC_RMID, 0) == -1) {
		perror("Response shmctl(IPC_RMID) failed\n");
		exit(EXIT_FAILURE);
	}
    exit(EXIT_SUCCESS);
}
