#include "../lib/LCFS.h"

static queue_object* LCFS_queue;
//You can add more global variables here

int LCFS_startup(){
    // TODO
    LCFS_queue=new_queue();
    if (LCFS_queue==NULL){
        return 1;
    }
    return 0;
}

process* LCFS_tick (process* running_process){
    // TODO
    if (running_process==NULL || running_process->time_left==0){
        queue_object* o = LCFS_queue->next;
        running_process=o->object;
        LCFS_queue->next= (LCFS_queue->next)->next;
        free(o);
    }
    if (running_process!=NULL){
        running_process->time_left--;
    }

    return running_process;
}


process* LCFS_new_arrival(process* arriving_process, process* running_process){
    // TODO
     if(arriving_process!=NULL){
        queue_add(arriving_process, LCFS_queue);
    }
    return running_process;
}

void LCFS_finish(){
    // TODO
    free_queue(LCFS_queue);
}
