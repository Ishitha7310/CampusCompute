#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>

#define SHM_NAME "/campuscompute_shm"
#define SEM_NAME "/campuscompute_sem"
#define SHM_SIZE 1024

typedef struct {
    char message[SHM_SIZE];
} SharedData;

void run_ipc_demo(void)
{
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);

    if (fd == -1) {
        perror("shm_open");
        return;
    }

    if (ftruncate(fd, sizeof(SharedData)) == -1) {
        perror("ftruncate");
        close(fd);
        shm_unlink(SHM_NAME);
        return;
    }

    SharedData *data = mmap(NULL, sizeof(SharedData),
                            PROT_READ | PROT_WRITE,
                            MAP_SHARED, fd, 0);

    if (data == MAP_FAILED) {
        perror("mmap");
        close(fd);
        shm_unlink(SHM_NAME);
        return;
    }

    close(fd);

    sem_unlink(SEM_NAME);

    sem_t *sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0666, 0);

    if (sem == SEM_FAILED) {
        perror("sem_open");
        munmap(data, sizeof(SharedData));
        shm_unlink(SHM_NAME);
        return;
    }

    printf("Shared memory created successfully.\n");
    printf("Semaphore created successfully.\n");

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        sem_close(sem);
        sem_unlink(SEM_NAME);
        munmap(data, sizeof(SharedData));
        shm_unlink(SHM_NAME);
        return;
    }

    if (pid == 0) {
        printf("Child waiting for semaphore...\n");
        fflush(stdout);

        if (sem_wait(sem) == -1) {
            perror("sem_wait");
            _exit(1);
        }

        printf("Child received: %s\n", data->message);

        munmap(data, sizeof(SharedData));
        sem_close(sem);
        _exit(0);
    }

    strcpy(data->message, "Hello from parent process!");

    printf("Parent wrote message into shared memory.\n");

    sem_post(sem);

    printf("Parent signalled semaphore.\n");

    int status;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status))
        printf("Child exit status: %d\n", WEXITSTATUS(status));

    sem_close(sem);
    sem_unlink(SEM_NAME);
    munmap(data, sizeof(SharedData));
    shm_unlink(SHM_NAME);

    printf("Shared memory and semaphore cleaned up.\n");
}

