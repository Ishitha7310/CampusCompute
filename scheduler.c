#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scheduler.h"

typedef struct {
    int index;
    int start;
    int finish;
} Segment;

typedef struct {
    double average_turnaround;
    double average_waiting;
} SchedulingMetrics;

static int comes_before(const Job *a, const Job *b) {
    if (a->arrival_time != b->arrival_time)
        return a->arrival_time < b->arrival_time;
    return strcmp(a->id, b->id) < 0;
}

static void calculate_metrics(const Job jobs[], int count,
                              const int completion[],
                              SchedulingMetrics *metrics) {
    double total_wait = 0.0;
    double total_tat = 0.0;

    for (int i = 0; i < count; i++) {
        int turnaround = completion[i] - jobs[i].arrival_time;
        int waiting = turnaround - jobs[i].burst_time;
        if (waiting < 0) waiting = 0;
        total_wait += waiting;
        total_tat += turnaround;
    }

    metrics->average_turnaround = count > 0 ? total_tat / count : 0.0;
    metrics->average_waiting = count > 0 ? total_wait / count : 0.0;
}

static void print_metrics(const Job jobs[], int count,
                          const int completion[], const char *algorithm,
                          const SchedulingMetrics *metrics) {
    printf("\n%s results\n", algorithm);
    printf("%-12s %-12s %-14s %-14s\n",
           "Job ID", "Completion", "Turnaround", "Waiting");
    puts("--------------------------------------------------------");

    for (int i = 0; i < count; i++) {
        int turnaround = completion[i] - jobs[i].arrival_time;
        int waiting = turnaround - jobs[i].burst_time;
        if (waiting < 0) waiting = 0;
        printf("%-12s %-12d %-14d %-14d\n",
               jobs[i].id, completion[i], turnaround, waiting);
    }
    printf("Average turnaround time: %.2f\n", metrics->average_turnaround);
    printf("Average waiting time:    %.2f\n", metrics->average_waiting);
}

static void print_segments(const Job jobs[], const Segment seg[], int n) {
    puts("\nExecution timeline (Gantt-style):");
    for (int i = 0; i < n; i++)
        printf("| %s %d-%d ", jobs[seg[i].index].id,
               seg[i].start, seg[i].finish);
    puts("|");
}

static void calculate_fcfs(const Job jobs[], int count,
                           int completion[], Segment seg[]) {
    int order[MAX_JOBS];
    for (int i = 0; i < count; i++) order[i] = i;

    for (int i = 0; i < count; i++) {
        for (int j = i + 1; j < count; j++) {
            if (comes_before(&jobs[order[j]], &jobs[order[i]])) {
                int temp = order[i];
                order[i] = order[j];
                order[j] = temp;
            }
        }
    }

    int time = 0;
    for (int k = 0; k < count; k++) {
        int index = order[k];
        int start = time > jobs[index].arrival_time
                        ? time : jobs[index].arrival_time;
        seg[k] = (Segment){index, start, start + jobs[index].burst_time};
        time = seg[k].finish;
        completion[index] = time;
    }
}

static void calculate_priority(const Job jobs[], int count,
                               int completion[], Segment seg[],
                               int *segment_count) {
    int done[MAX_JOBS] = {0};
    int time = 0;
    int finished = 0;
    *segment_count = 0;

    while (finished < count) {
        int best = -1;
        for (int i = 0; i < count; i++) {
            if (done[i] || jobs[i].arrival_time > time) continue;
            if (best < 0 || jobs[i].priority < jobs[best].priority ||
                (jobs[i].priority == jobs[best].priority &&
                 comes_before(&jobs[i], &jobs[best]))) {
                best = i;
            }
        }

        if (best < 0) {
            int next = -1;
            for (int i = 0; i < count; i++) {
                if (!done[i] && (next < 0 ||
                    jobs[i].arrival_time < jobs[next].arrival_time)) {
                    next = i;
                }
            }
            time = jobs[next].arrival_time;
            continue;
        }

        seg[*segment_count] = (Segment){best, time,
                                        time + jobs[best].burst_time};
        (*segment_count)++;
        time += jobs[best].burst_time;
        completion[best] = time;
        done[best] = 1;
        finished++;
    }
}

static void enqueue_arrived_jobs(const Job jobs[], int count, int time,
                                 int added[], int queue[], int *tail) {
    for (;;) {
        int next = -1;
        for (int i = 0; i < count; i++) {
            if (added[i] || jobs[i].arrival_time > time) continue;
            if (next < 0 || jobs[i].arrival_time < jobs[next].arrival_time)
                next = i;
        }
        if (next < 0) break;
        queue[(*tail)++] = next;
        added[next] = 1;
    }
}

static int calculate_round_robin(const Job jobs[], int count, int quantum,
                                 int completion[], Segment **segments_out,
                                 int *segment_count_out) {
    long long total_burst = 0;
    for (int i = 0; i < count; i++) total_burst += jobs[i].burst_time;
    if (total_burst > 10000000) return 0;

    int capacity = (int)total_burst + count + 2;
    int *queue = malloc((size_t)capacity * sizeof(int));
    int *remaining = malloc((size_t)count * sizeof(int));
    int *added = calloc((size_t)count, sizeof(int));
    Segment *segments = malloc((size_t)capacity * sizeof(Segment));

    if (!queue || !remaining || !added || !segments) {
        free(queue);
        free(remaining);
        free(added);
        free(segments);
        return 0;
    }

    for (int i = 0; i < count; i++) {
        remaining[i] = jobs[i].burst_time;
        completion[i] = 0;
    }

    int head = 0, tail = 0, finished = 0, time = 0, segment_count = 0;

    while (finished < count) {
        enqueue_arrived_jobs(jobs, count, time, added, queue, &tail);

        if (head == tail) {
            int next_time = -1;
            for (int i = 0; i < count; i++) {
                if (!added[i] && (next_time < 0 ||
                    jobs[i].arrival_time < next_time)) {
                    next_time = jobs[i].arrival_time;
                }
            }
            if (next_time < 0) break;
            time = next_time;
            continue;
        }

        int index = queue[head++];
        int run = remaining[index] < quantum ? remaining[index] : quantum;
        int start = time;
        time += run;
        remaining[index] -= run;
        segments[segment_count++] = (Segment){index, start, time};

        enqueue_arrived_jobs(jobs, count, time, added, queue, &tail);

        if (remaining[index] > 0) {
            queue[tail++] = index;
        } else {
            completion[index] = time;
            finished++;
        }
    }

    free(queue);
    free(remaining);
    free(added);
    *segments_out = segments;
    *segment_count_out = segment_count;
    return 1;
}

void scheduler_fcfs(const Job jobs[], int count) {
    if (count <= 0) {
        puts("Register jobs first.");
        return;
    }

    int completion[MAX_JOBS] = {0};
    Segment segments[MAX_JOBS];
    SchedulingMetrics metrics;
    calculate_fcfs(jobs, count, completion, segments);
    calculate_metrics(jobs, count, completion, &metrics);
    print_segments(jobs, segments, count);
    print_metrics(jobs, count, completion, "First Come First Served (FCFS)",
                  &metrics);
}

void scheduler_priority(const Job jobs[], int count) {
    if (count <= 0) {
        puts("Register jobs first.");
        return;
    }

    int completion[MAX_JOBS] = {0};
    Segment segments[MAX_JOBS];
    int segment_count = 0;
    SchedulingMetrics metrics;
    calculate_priority(jobs, count, completion, segments, &segment_count);
    calculate_metrics(jobs, count, completion, &metrics);
    print_segments(jobs, segments, segment_count);
    print_metrics(jobs, count, completion, "Non-preemptive Priority", &metrics);
    puts("Priority convention: smaller number means higher priority.");
}

void scheduler_round_robin(const Job jobs[], int count, int quantum) {
    if (count <= 0) {
        puts("Register jobs first.");
        return;
    }
    if (quantum <= 0) {
        puts("Time quantum must be greater than zero.");
        return;
    }

    int completion[MAX_JOBS] = {0};
    Segment *segments = NULL;
    int segment_count = 0;
    SchedulingMetrics metrics;

    if (!calculate_round_robin(jobs, count, quantum, completion,
                               &segments, &segment_count)) {
        puts("Could not run Round Robin (allocation failure or workload too large).");
        return;
    }

    calculate_metrics(jobs, count, completion, &metrics);
    print_segments(jobs, segments, segment_count);
    print_metrics(jobs, count, completion, "Round Robin", &metrics);
    printf("Time quantum: %d\n", quantum);
    puts("Round Robin tie-breaking: earlier arrivals enter first; simultaneous arrivals follow registration order.");
    free(segments);
}

void scheduler_compare(const Job jobs[], int count, int quantum) {
    if (count <= 0) {
        puts("Register jobs first.");
        return;
    }
    if (quantum <= 0) {
        puts("Time quantum must be greater than zero.");
        return;
    }

    int fcfs_completion[MAX_JOBS] = {0};
    int priority_completion[MAX_JOBS] = {0};
    int rr_completion[MAX_JOBS] = {0};
    Segment fcfs_segments[MAX_JOBS];
    Segment priority_segments[MAX_JOBS];
    int priority_segment_count = 0;
    Segment *rr_segments = NULL;
    int rr_segment_count = 0;
    SchedulingMetrics fcfs_metrics, rr_metrics, priority_metrics;

    calculate_fcfs(jobs, count, fcfs_completion, fcfs_segments);
    calculate_metrics(jobs, count, fcfs_completion, &fcfs_metrics);

    if (!calculate_round_robin(jobs, count, quantum, rr_completion,
                               &rr_segments, &rr_segment_count)) {
        puts("Could not calculate comparison: Round Robin workload is too large or memory allocation failed.");
        return;
    }
    calculate_metrics(jobs, count, rr_completion, &rr_metrics);

    calculate_priority(jobs, count, priority_completion,
                       priority_segments, &priority_segment_count);
    calculate_metrics(jobs, count, priority_completion, &priority_metrics);
    free(rr_segments);

    puts("\n========== Scheduling Policy Comparison ==========");
    printf("Same registered job set | Round Robin quantum = %d\n", quantum);
    printf("%-24s %-22s %-20s\n",
           "Algorithm", "Avg. Turnaround", "Avg. Waiting");
    puts("-------------------------------------------------------------------");
    printf("%-24s %-22.2f %-20.2f\n",
           "FCFS", fcfs_metrics.average_turnaround,
           fcfs_metrics.average_waiting);
    printf("%-24s %-22.2f %-20.2f\n",
           "Round Robin", rr_metrics.average_turnaround,
           rr_metrics.average_waiting);
    printf("%-24s %-22.2f %-20.2f\n",
           "Non-preemptive Priority", priority_metrics.average_turnaround,
           priority_metrics.average_waiting);
    puts("-------------------------------------------------------------------");
    puts("Tie-breaking: FCFS uses earlier arrival, then job ID alphabetically.");
    puts("Priority uses lower priority number first; ties use earlier arrival, then job ID.");
    puts("Round Robin uses arrival order; simultaneous arrivals follow registration order.");
}
