#include <stdio.h>
#include <stdlib.h>
#include "limits.h"

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: ./limits_test memory|cpu\n");
        return 1;
    }

    if (argv[1][0] == 'm')
        run_memory_limit();
    else if (argv[1][0] == 'c')
        run_cpu_limit();
    else
        printf("Invalid option.\n");

    return 0;
}
