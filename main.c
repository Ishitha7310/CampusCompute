#include "worker.h"
#include "limits.h"
#include "ipc.h"
#include "logger.h"
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "jobs.h"
#include "scheduler.h"
#include "banker.h"

static int read_line(const char *prompt, char *buffer, size_t size) {
    printf("%s", prompt);
    if (!fgets(buffer, (int)size, stdin)) return 0;
    buffer[strcspn(buffer, "\n")] = '\0';
    return 1;
}

static int read_int(const char *prompt, int *value) {
    char line[128], extra;
    long n;
    if (!read_line(prompt, line, sizeof(line))) return 0;
    if (sscanf(line, " %ld %c", &n, &extra) != 1 ||
        n < -1 || n > 1000000) return 0;
    *value = (int)n;
    return 1;
}

static void add_job_menu(Job jobs[], int *count) {
    Job job = {0};
    if (!read_line("Job ID (e.g. J1): ", job.id, sizeof(job.id)) ||
        job.id[0] == '\0' || strchr(job.id, ' ')) {
        puts("Invalid job ID. Use a non-empty ID without spaces."); return;
    }
    if (!read_int("Arrival time (>= 0): ", &job.arrival_time) ||
        job.arrival_time < 0) {
        puts("Invalid arrival time."); return;
    }
    if (!read_int("CPU burst (> 0): ", &job.burst_time) ||
        job.burst_time <= 0) {
        puts("Invalid CPU burst."); return;
    }
    if (!read_int("Priority (>= 0; smaller number = higher priority): ", &job.priority) ||
        job.priority < 0) {
        puts("Invalid priority."); return;
    }

    if (jobs_add(jobs, count, &job))
        printf("Job %s registered successfully with WAITING status.\n", job.id);
    else
        puts("Could not add job. Check duplicate ID or the 100-job limit.");
}

static void update_status_menu(Job jobs[], int count) {
    char id[JOB_ID_LEN];
    int choice;
    if (count == 0) { puts("No jobs registered."); return; }
    if (!read_line("Enter job ID: ", id, sizeof(id))) return;
    if (jobs_find(jobs, count, id) < 0) {
        puts("Job ID not found."); return;
    }
    puts("1. WAITING\n2. RUNNING\n3. COMPLETED");
    if (!read_int("Choose status: ", &choice) || choice < 1 || choice > 3) {
        puts("Invalid status choice."); return;
    }
    jobs_set_status(jobs, count, id, (JobStatus)(choice - 1));
    puts("Job status updated.");
}

static void show_menu(void) {
    puts("\n========== CampusCompute: Intelligent Job Scheduling and Resource Management ==========");
    puts("1. Register a job (FR1)");
    puts("2. Display all jobs (FR1)");
    puts("3. Update a job status (FR1)");
    puts("4. Run FCFS scheduling (FR2)");
    puts("5. Run Round Robin scheduling (FR2)");
    puts("6. Run non-preemptive Priority scheduling (FR2)");
    puts("7. Run Banker's safety algorithm (FR5)");
    puts("8. Compare scheduling algorithms (FR2)");
   puts("9. Run Producer-Consumer Logger (FR3)");
puts("10. Run Shared Memory and Semaphore IPC (FR4)");
puts("11. Run Worker Processes (FR6)");
puts("13. Run Integrated Demonstration");
puts("12. Run Resource Limits (FR7)");
    puts("0. Exit");
}
static void run_limit_child(int mode)
{
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        if (mode == 1)
            run_memory_limit();
        else
            run_cpu_limit();

        _exit(0);
    }

    int status;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status))
        printf("Resource-limit child exited with code %d\n",
               WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
        printf("Resource-limit child terminated by signal %d\n",
               WTERMSIG(status));
}
static void run_integrated_demo(void)
{
    puts("\n===== CAMPUSCOMPUTE INTEGRATED DEMO =====");

    puts("\n[FR3] Logger");
    run_logger_demo();

    puts("\n[FR4] Shared Memory and Semaphore");
    run_ipc_demo();

    puts("\n[FR6] Worker Process");
    run_worker(1);

    puts("\n[FR7] Memory Resource Limit");
    run_limit_child(1);

    puts("\n===== INTEGRATED DEMONSTRATION FINISHED =====");
    puts("All four Person B module demonstrations completed.");
}

int main(void) {
    Job jobs[MAX_JOBS] = {0};
    int job_count = 0, choice;

    puts("CampusCompute - Person A modules");
    puts("Priority convention: smaller number means higher priority.");
    for (;;) {
        show_menu();
 if (!read_int("Enter choice: ", &choice)) {
    if (feof(stdin))
        return 0;

    puts("Invalid input. Enter a menu number.");
    continue;
}

        switch (choice) {
            case 1: add_job_menu(jobs, &job_count); break;
            case 2: jobs_display(jobs, job_count); break;
            case 3: update_status_menu(jobs, job_count); break;
            case 4: scheduler_fcfs(jobs, job_count); break;
            case 5: {
                int quantum;
                if (!read_int("Enter time quantum (> 0): ", &quantum) || quantum <= 0)
                    puts("Invalid time quantum.");
                else scheduler_round_robin(jobs, job_count, quantum);
                break;
            }
            case 6: scheduler_priority(jobs, job_count); break;
            case 7: banker_run_interactive(); break;
            case 8: {
                int quantum;
                if (!read_int("Enter Round Robin quantum for comparison (> 0): ", &quantum) || quantum <= 0)
                    puts("Invalid time quantum.");
                else scheduler_compare(jobs, job_count, quantum);
                break;
            }
case 9:
    run_logger_demo();
    break;

case 10:
    run_ipc_demo();
    break;

case 11: {
    int mode;
    puts("1. Normal worker");
    puts("2. Failed worker");
    puts("3. Crashed worker");

    if (read_int("Choose worker mode: ", &mode) &&
        mode >= 1 && mode <= 3)
        run_worker(mode);
    else
        puts("Invalid worker mode.");

    break;
}

case 12: {
    int mode;
    puts("1. Memory limit");
    puts("2. CPU time limit");

    if (read_int("Choose resource limit: ", &mode) &&
        (mode == 1 || mode == 2))
        run_limit_child(mode);
    else
        puts("Invalid resource limit choice.");

    break;
}
case 13:
    run_integrated_demo();
    break;
            case 0: puts("Exiting CampusCompute. All modules completed.");
            default: puts("Unknown menu option. Choose 0-8.");
        }
    }
}
