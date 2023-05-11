#ifndef SCHEDULER_H
#define SCHEDULER_H

#include"process.h"

typedef enum scheduling_strategy{
    FCFS, LCFS,HRRN, MLF,PRIOP,RR,SRTNP
}scheduling_strategy;

unsigned int overlapping_time(int p_id);
unsigned int end_time(int p_id);


char* scheduler(process processes[], int processcount, scheduling_strategy strategy,int RR_quantum);

#endif