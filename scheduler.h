#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "jobs.h"

void scheduler_fcfs(const Job jobs[], int count);
void scheduler_round_robin(const Job jobs[], int count, int quantum);
void scheduler_priority(const Job jobs[], int count);
void scheduler_compare(const Job jobs[], int count, int quantum);

#endif
