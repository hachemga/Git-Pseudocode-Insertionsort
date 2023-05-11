#include "../lib/PRIOP.h"
#include <stdio.h>

static queue_object* PRIOP_queue;
//You can add more global variables here

process* PRIOP_tick (process* running_process){
    // TODO
    if (running_process==NULL || running_process->time_left==0){
        running_process=queue_poll(PRIOP_queue);
    }
    if (running_process!=NULL){
        void* end = queue_peek(PRIOP_queue);
        if(end!=NULL){
        unsigned int  pri = ((process*)end)->priority; 
        if (pri > running_process->priority){
            running_process=PRIOP_new_arrival(running_process,queue_poll(PRIOP_queue));
        }
        running_process->time_left--;
    }
}

    return running_process;
}

int PRIOP_startup(){
    // TODO
    PRIOP_queue=new_queue();
    if (PRIOP_queue==NULL){
        return 1;
    }
    return 0;
}


process* PRIOP_new_arrival(process* arriving_process, process* running_process){
    // TODO
    if(arriving_process!=NULL){
        queue_add(arriving_process, PRIOP_queue);
        queue_object* o = PRIOP_queue->next;
        queue_object* p = PRIOP_queue;
        while(1){
            if (o->next!=NULL){
                void* x = o->object;
                unsigned int y = ((process*)x)->priority;
                void* w = o->next->object;
                unsigned int v = ((process*)w)->priority;
                if (y>v){
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

void PRIOP_finish(){
    // TODO
    free_queue(PRIOP_queue);
}
