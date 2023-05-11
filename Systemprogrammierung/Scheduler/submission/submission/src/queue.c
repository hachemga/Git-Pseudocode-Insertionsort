#include "../lib/queue.h"
#include <stdlib.h>
#include <stdio.h>

int queue_add(void* new_obejct, queue_object* queue){
    // TODO
    if (new_obejct == NULL || queue== NULL){
        return 1;
    }
    else{
        queue_object* q = malloc (sizeof(queue_object));
        q->object = new_obejct;
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

void* queue_poll(queue_object* queue){
    // TODO
     void* o = NULL ;
    if (queue == NULL || queue->next == NULL ){
        return o;
    }
    else{
        queue_object* s = queue->next;
        queue_object* p= queue;
        
        while(s->next!=NULL){
            p=s;
            s=s->next;
        }
        o = s->object;
        p->next = NULL;
        free(s);
        return o ;
    }
}

queue_object* new_queue(){
    // TODO
     queue_object * q = malloc(sizeof(queue_object));
    q->object = NULL;
    q->next = NULL;
    return q;
}


void free_queue(queue_object* queue){
    // TODO
     if (queue==NULL){
        return;
    }
    queue_object* p = queue;
    queue_object* s = queue->next;
    while(s!=NULL){
        free(p);
        p=s;
        s=s->next;
    }
    free(p);
}

void* queue_peek(queue_object* queue){
    // TODO
     void* o = NULL ;
    if (queue == NULL || queue->next == NULL ){
        return o;
    }
    else{
        queue_object* q = queue->next;
        while(q->next!=NULL){
            q=q->next;
        }
        o = q->object; 
        return o ;
}
}
