#include <stdio.h>
#include <string.h>
#include "jobs.h"

int jobs_find(const Job jobs[], int count, const char *id) {
    for (int i = 0; i < count; i++) {
        if (strcmp(jobs[i].id, id) == 0) return i;
    }
    return -1;
}

int jobs_add(Job jobs[], int *count, const Job *new_job) {
    if (!jobs || !count || !new_job || *count < 0 || *count >= MAX_JOBS)
        return 0;
    if (new_job->id[0] == '\0' || new_job->arrival_time < 0 ||
        new_job->burst_time <= 0 || new_job->priority < 0)
        return 0;
    if (jobs_find(jobs, *count, new_job->id) >= 0) return 0;

    jobs[*count] = *new_job;
    jobs[*count].status = JOB_WAITING;
    (*count)++;
    return 1;
}

const char *jobs_status_name(JobStatus status) {
    switch (status) {
        case JOB_WAITING: return "WAITING";
        case JOB_RUNNING: return "RUNNING";
        case JOB_COMPLETED: return "COMPLETED";
        default: return "UNKNOWN";
    }
}

void jobs_display(const Job jobs[], int count) {
    if (count == 0) {
        puts("No jobs have been registered yet.");
        return;
    }

    printf("\n%-12s %-14s %-12s %-10s %-12s\n",
           "Job ID", "Arrival", "CPU Burst", "Priority", "Status");
    puts("---------------------------------------------------------------");
    for (int i = 0; i < count; i++) {
        printf("%-12s %-14d %-12d %-10d %-12s\n",
               jobs[i].id, jobs[i].arrival_time, jobs[i].burst_time,
               jobs[i].priority, jobs_status_name(jobs[i].status));
    }
}


int jobs_set_status(Job jobs[], int count, const char *id, JobStatus status) {
    if (!jobs || !id || status < JOB_WAITING || status > JOB_COMPLETED)
        return 0;
    int index = jobs_find(jobs, count, id);
    if (index < 0) return 0;
    jobs[index].status = status;
    return 1;
}
