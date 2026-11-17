#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define QUEUE_SIZE 5
#define PRODUCERS 2
#define MESSAGES 5

char queue[QUEUE_SIZE][100];
int front = 0, rear = 0, count = 0;
int finished = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;

void *producer(void *arg)
{
    int id = *(int *)arg;

    for (int i = 1; i <= MESSAGES; i++) {
        char message[100];

        snprintf(message, sizeof(message),
                 "Producer %d generated event %d", id, i);

        pthread_mutex_lock(&mutex);

        while (count == QUEUE_SIZE)
            pthread_cond_wait(&not_full, &mutex);

        strcpy(queue[rear], message);
        rear = (rear + 1) % QUEUE_SIZE;
        count++;

        printf("[Producer %d] Added: %s\n", id, message);

        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&mutex);

        sleep(1);
    }

    pthread_mutex_lock(&mutex);
    finished++;
    pthread_cond_signal(&not_empty);
    pthread_mutex_unlock(&mutex);

    return NULL;
}

void *consumer(void *arg)
{
    (void)arg;

    FILE *file = fopen("campus.log", "w");

    if (file == NULL) {
        perror("campus.log");
        return NULL;
    }

    while (1) {
        pthread_mutex_lock(&mutex);

        while (count == 0 && finished < PRODUCERS)
            pthread_cond_wait(&not_empty, &mutex);

        if (count == 0 && finished == PRODUCERS) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        char message[100];
        strcpy(message, queue[front]);

        front = (front + 1) % QUEUE_SIZE;
        count--;

        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&mutex);

        fprintf(file, "%s\n", message);
        printf("[Consumer] Logged: %s\n", message);
    }

    fclose(file);
    printf("Consumer finished. Log saved to campus.log\n");

    return NULL;
}

void run_logger_demo(void)
{
    pthread_t producers[PRODUCERS];
    pthread_t consumer_thread;
    int ids[PRODUCERS] = {1, 2};

    printf("Starting Producer-Consumer Logger...\n");
    printf("Queue capacity: %d\n", QUEUE_SIZE);

    if (pthread_create(&consumer_thread, NULL, consumer, NULL) != 0) {
        perror("consumer thread");
        return;
    }

    int created = 0;

    for (int i = 0; i < PRODUCERS; i++) {
        if (pthread_create(&producers[i], NULL,
                           producer, &ids[i]) != 0) {
            perror("producer thread");
            break;
        }
        created++;
    }

    pthread_mutex_lock(&mutex);
    while (finished < created)
        pthread_cond_wait(&not_empty, &mutex);
    pthread_mutex_unlock(&mutex);

    for (int i = 0; i < created; i++)
        pthread_join(producers[i], NULL);

    pthread_join(consumer_thread, NULL);

    printf("All threads completed successfully.\n");
}

