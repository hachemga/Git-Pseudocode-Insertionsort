#ifndef PROCESS_H
#define PROCESS_H

typedef struct _process{
    unsigned int start_time;
    unsigned int time_left;
    unsigned int priority;
    char id;
}process;

//process* running_process;
//process** arriving_processes;

#endif //PROCESS_H