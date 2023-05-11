#include "../lib/HRRN.h"

static queue_object* HRRN_queue;
//You can add more global variables and structs here

typedef struct _queue_object_HRRN{
    process* process;
    unsigned int wartezeit;
    struct _queue_object_HRRN* next;
}queue_object_HRRN;

static queue_object_HRRN* HRRN_queue_Warte;


queue_object_HRRN* new_queue_HRRN(){
    queue_object_HRRN * q = malloc(sizeof(queue_object_HRRN));
    q->process = NULL;
    q->wartezeit = 0;
    q->next = NULL;
    return q;
}

int queue_add_HRRN(process* p, queue_object_HRRN* queue){
    // TODO
    if (p == NULL || queue == NULL){
        return 1;
    }
    else{
        queue_object_HRRN* q = malloc (sizeof(queue_object_HRRN));
        q->process = p;
        q->wartezeit = 0;
        if(queue->next == NULL){
            q->next = NULL;
            queue->next = q;
        }
        else {
            q->next = queue->next;
            queue->next=q;
        }
        return 0;
    }
}
void free_queue_HRRN(queue_object_HRRN* queue){
    // TODO
    if (queue==NULL){
        return;
    }
    queue_object_HRRN* p = queue;
    queue_object_HRRN* s = queue->next;
    while(s!=NULL){
        free(p);
        p=s;
        s=s->next;
    }
    free(p);
    }





process* HRRN_tick (process* running_process){
    // TODO
    if (running_process==NULL || running_process->time_left==0){
        queue_object_HRRN* p = HRRN_queue_Warte;
        queue_object_HRRN* s = HRRN_queue_Warte->next;
        queue_object_HRRN* max = HRRN_queue_Warte->next;
        queue_object_HRRN* maxp = HRRN_queue_Warte;
        while(s!=NULL){
            if (((max->wartezeit + max->process->time_left)/max->process->time_left)<((s->wartezeit + s->process->time_left)/ s->process->time_left)){
                max=s;
                maxp=p;
            }
            p=s;
            s=s->next;
        }
        running_process = max->process;
        maxp->next=(maxp->next)->next;
        free(max);
    }
    if (running_process!=NULL){
        running_process->time_left--;
        queue_object_HRRN*q = HRRN_queue_Warte->next;
        while(q!=NULL){
            q->wartezeit++;
            q=q->next;
        }
}

    return running_process;
}

int HRRN_startup(){
    // TODO
   HRRN_queue_Warte=new_queue_HRRN();
    if (HRRN_queue_Warte==NULL){
        return 1;
    }
    return 0;
}

process* HRRN_new_arrival(process* arriving_process, process* running_process){
    // TODO
    if(arriving_process!=NULL){
        queue_add_HRRN(arriving_process, HRRN_queue_Warte);
    }
    return running_process;
     
}

void HRRN_finish(){
    // TODO
    free_queue_HRRN(HRRN_queue_Warte);
}
