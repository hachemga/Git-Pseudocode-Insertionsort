#include "../lib/RR.h"

static queue_object* RR_queue;
//You can add more global variables
static int pi;
static int pi_iterable;

process* RR_tick (process* running_process){
    // TODO
    if (running_process==NULL || running_process->time_left==0){
        running_process=queue_poll(RR_queue);
        pi_iterable=pi;
    }
    if (running_process!=NULL && pi_iterable==0){
        queue_add(running_process,RR_queue);
        pi_iterable=pi;
        running_process=queue_poll(RR_queue);
    }
    if (running_process!=NULL && pi_iterable!=0){
        running_process->time_left--;
        pi_iterable--;
    }

    return running_process;
}

int RR_startup(int quantum){
    // TODO
    pi=quantum;
    pi_iterable=quantum;
    RR_queue=new_queue();
    if (RR_queue==NULL){
        return 1;
    }
    return 0;
}


process* RR_new_arrival(process* arriving_process, process* running_process){
    // TODO
    if(arriving_process!=NULL){
        queue_add(arriving_process, RR_queue);
    }
    return running_process;
}


void RR_finish(){
    // TODO
    free_queue(RR_queue);
}
