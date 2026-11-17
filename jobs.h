#ifndef JOBS_H
#define JOBS_H

#define MAX_JOBS 100
#define JOB_ID_LEN 32

typedef enum {
    JOB_WAITING = 0,
    JOB_RUNNING,
    JOB_COMPLETED
} JobStatus;

typedef struct {
    char id[JOB_ID_LEN];
    int arrival_time;
    int burst_time;
    int priority;
    JobStatus status;
} Job;

int jobs_add(Job jobs[], int *count, const Job *new_job);
void jobs_display(const Job jobs[], int count);
int jobs_find(const Job jobs[], int count, const char *id);
int jobs_set_status(Job jobs[], int count, const char *id, JobStatus status);
const char *jobs_status_name(JobStatus status);

#endif
