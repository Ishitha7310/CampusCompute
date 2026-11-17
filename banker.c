#include <stdio.h>
#include <stdlib.h>
#include "banker.h"

#define MAX_PROCESSES 20
#define MAX_RESOURCES 10

static int read_nonnegative(const char *prompt, int *value) {
    char line[128], extra;
    long n;
    printf("%s", prompt);
    if (!fgets(line, sizeof(line), stdin)) return 0;
    if (sscanf(line, " %ld %c", &n, &extra) != 1 || n < 0 || n > 1000000)
        return 0;
    *value = (int)n;
    return 1;
}

void banker_run_interactive(void) {
    int n, m;
    if (!read_nonnegative("Number of processes (1-20): ", &n) ||
        n < 1 || n > MAX_PROCESSES) {
        puts("Invalid process count."); return;
    }
    if (!read_nonnegative("Number of resource types (1-10): ", &m) ||
        m < 1 || m > MAX_RESOURCES) {
        puts("Invalid resource count."); return;
    }

    int available[MAX_RESOURCES];
    int allocation[MAX_PROCESSES][MAX_RESOURCES];
    int maximum[MAX_PROCESSES][MAX_RESOURCES];
    int need[MAX_PROCESSES][MAX_RESOURCES];
    int work[MAX_RESOURCES], finish[MAX_PROCESSES] = {0};
    int safe_sequence[MAX_PROCESSES];

    puts("\nEnter Available vector:");
    for (int j = 0; j < m; j++) {
        char prompt[80]; snprintf(prompt, sizeof(prompt), "Available[R%d]: ", j);
        if (!read_nonnegative(prompt, &available[j])) {
            puts("Invalid available value."); return;
        }
    }

    puts("\nEnter Allocation matrix:");
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++) {
            char prompt[80]; snprintf(prompt, sizeof(prompt), "P%d Allocation[R%d]: ", i, j);
            if (!read_nonnegative(prompt, &allocation[i][j])) {
                puts("Invalid allocation value."); return;
            }
        }

    puts("\nEnter Maximum matrix:");
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++) {
            char prompt[80]; snprintf(prompt, sizeof(prompt), "P%d Maximum[R%d]: ", i, j);
            if (!read_nonnegative(prompt, &maximum[i][j])) {
                puts("Invalid maximum value."); return;
            }
            if (maximum[i][j] < allocation[i][j]) {
                printf("Invalid state: Maximum is less than Allocation for P%d, R%d.\n", i, j);
                return;
            }
            need[i][j] = maximum[i][j] - allocation[i][j];
        }

    puts("\nNeed matrix (Maximum - Allocation):");
    for (int i = 0; i < n; i++) {
        printf("P%d: ", i);
        for (int j = 0; j < m; j++) printf("%d ", need[i][j]);
        putchar('\n');
    }

    for (int j = 0; j < m; j++) work[j] = available[j];

    int completed = 0;
    while (completed < n) {
        int progress = 0;
        for (int i = 0; i < n; i++) {
            if (finish[i]) continue;
            int can_finish = 1;
            for (int j = 0; j < m; j++)
                if (need[i][j] > work[j]) { can_finish = 0; break; }

            if (can_finish) {
                for (int j = 0; j < m; j++) work[j] += allocation[i][j];
                finish[i] = 1;
                safe_sequence[completed++] = i;
                progress = 1;
            }
        }
        if (!progress) break;
    }

    if (completed == n) {
        printf("\nSAFE STATE\nSafe sequence: ");
        for (int i = 0; i < n; i++)
            printf("P%d%s", safe_sequence[i], i == n - 1 ? "\n" : " -> ");
    } else {
        puts("\nUNSAFE STATE");
        puts("No safe sequence containing every process was found.");
    }
}
