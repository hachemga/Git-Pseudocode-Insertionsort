#include "../lib/MLF.h"
#include <stdio.h>

static queue_object** MLF_queues;
//You can add more global variables here
int pi;
int level;

process* MLF_tick (process* running_process){
    // TODO
     if (running_process==NULL || running_process->time_left==0){
         level = 0;
         while(1){
        if (MLF_queues[level]->next!=NULL){
            running_process=queue_poll(MLF_queues[level]);
            pi=1;
            for(int j= 0; j < level;j++){
                pi=pi*3;
            }
            break;
        }
        else{
            level++;
        }
    }
}
if (running_process!=NULL && pi==0){
        queue_add(running_process,MLF_queues[level+1]);
        level=0;
        process* p = NULL;
        while (p==NULL){
        if (MLF_queues[level]->next!=NULL){
            p=queue_poll(MLF_queues[level]);
            pi=1;
            for(int j= 0; j < level;j++){
                pi=pi*3;
            }
        }
        else{
            level++;
        }
    }
    running_process=p;
}
        
if (running_process!=NULL && pi!=0){
        running_process->time_left--;
        pi--;
    }
return running_process;
}

/**
 * Do everything you have to at startup in this function. It is called once.
 * @result 0 if everything was fine. Any other number if there was an error.
 */
int MLF_startup(){
    // TODO
     MLF_queues= malloc(sizeof(queue_object*)*4); 
    if (MLF_queues==NULL){
        return 1;
    }
    for (int i = 0 ; i < 4 ; i++){
        MLF_queues[i]=new_queue();
        if (MLF_queues[i]==NULL){
            return 1;
        }
    }
    return 0;
}

process* MLF_new_arrival(process* arriving_process, process* running_process){
    // TODO
    if(arriving_process!=NULL){
        queue_add(arriving_process, MLF_queues[0]);
    }
    return running_process;
}

/**
 * is called once after the all processes were handled. In case you want to cleanup something
 */
void MLF_finish(){
    // TODO
    for(int i = 0 ; i < 4 ; i++){
        free_queue(MLF_queues[i]);
    }
    free(MLF_queues);
}
