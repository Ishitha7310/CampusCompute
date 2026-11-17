#include <stdio.h>
#include "worker.h"

int main()
{
    printf("\n--- NORMAL WORKER ---\n");
    run_worker(1);

    printf("\n--- FAILED WORKER ---\n");
    run_worker(2);

    printf("\n--- CRASHED WORKER ---\n");
    run_worker(3);

    return 0;
}
