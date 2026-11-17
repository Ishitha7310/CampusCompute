#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

void run_worker(int mode)
{
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        printf("Child process started. PID: %d\n", getpid());

        if (mode == 1) {
            printf("Worker completed successfully.\n");
            _exit(0);
        }
        else if (mode == 2) {
            printf("Worker failed.\n");
            _exit(3);
        }
        else {
            printf("Worker terminated by signal.\n");
            raise(SIGUSR1);
            _exit(1);
        }
    }

    int status;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status))
        printf("Child exited with code: %d\n", WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
        printf("Child killed by signal: %d\n", WTERMSIG(status));
}
