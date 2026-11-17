#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>

void run_memory_limit()
{
    struct rlimit limit;

    limit.rlim_cur = 64 * 1024 * 1024;
    limit.rlim_max = 64 * 1024 * 1024;

    if (setrlimit(RLIMIT_AS, &limit) == -1) {
        perror("setrlimit");
        return;
    }

    printf("Memory limit set to 64 MB.\n");

    size_t size = 128 * 1024 * 1024;
    char *memory = malloc(size);

    if (memory == NULL) {
        printf("Memory allocation failed: limit exceeded.\n");
        return;
    }

    for (size_t i = 0; i < size; i += 4096)
        memory[i] = 1;

    printf("Memory allocation successful.\n");
    free(memory);
}

void run_cpu_limit()
{
    struct rlimit limit;

    limit.rlim_cur = 1;
    limit.rlim_max = 2;

    if (setrlimit(RLIMIT_CPU, &limit) == -1) {
        perror("setrlimit");
        return;
    }

    printf("CPU time limit set to 1 second.\n");
    fflush(stdout);

    volatile unsigned long long count = 0;

    while (1)
        count++;
}

