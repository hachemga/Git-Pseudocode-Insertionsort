#include "../lib/SRTNP.h"

static queue_object* SRTNP_queue;
//You can add more global variables here

process* SRTNP_tick (process* running_process){
    // TODO
     if (running_process==NULL || running_process->time_left==0){
        running_process=queue_poll(SRTNP_queue);
    }
    if (running_process!=NULL){
        void* end = queue_peek(SRTNP_queue);
        if(end!=NULL){
        unsigned int  time = ((process*)end)->time_left; 
        if (time < running_process->time_left){
            process*p=queue_poll(SRTNP_queue);
            running_process=SRTNP_new_arrival(running_process,p);
        }
    }
    running_process->time_left--;
}
    return running_process;
}

int SRTNP_startup(){
    // TODO
    SRTNP_queue=new_queue();
    if (SRTNP_queue==NULL){
        return 1;
    }
    return 0;
}

process* SRTNP_new_arrival(process* arriving_process, process* running_process){
    // TODO
    if(arriving_process!=NULL){
        queue_add(arriving_process, SRTNP_queue);
        queue_object* o = SRTNP_queue->next;
        queue_object* p = SRTNP_queue;
        while(1){
            if (o->next!=NULL){
                void* x = o->object;
                unsigned int y = ((process*)x)->time_left;
                void* w = o->next->object;
                unsigned int v = ((process*)w)->time_left;
                if (v>y){
                    queue_object* q= o->next;
                    o->next=q->next;
                    p->next=q;
                    q->next=o;
                    p=q;
                }
                else{
                    break;
                }
            }
            else{
                break;
            }
        }
    }
    return running_process;
}

void SRTNP_finish(){
    // TODO
     free_queue(SRTNP_queue);
}
