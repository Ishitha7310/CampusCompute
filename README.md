# CampusCompute: Intelligent Job Scheduling and Resource Management

## Project Overview
CampusCompute is a C-based Linux application demonstrating CPU scheduling, process management, inter-process communication, thread synchronization, deadlock avoidance, and resource limits.

## Requirements
- Ubuntu/Linux
- GCC compiler
- GNU Make
- POSIX threads and shared memory support

## Build
Navigate to the project directory and run:

    make clean
    make

## Run
Execute:

    ./campuscompute

## Features
1. Job and process management
2. FCFS scheduling
3. Round Robin scheduling
4. Non-preemptive Priority scheduling
5. Producer-consumer synchronization using POSIX threads
6. Shared memory and named semaphore communication
7. Process creation and monitoring using fork() and waitpid()
8. Resource limits using setrlimit()
9. Banker’s algorithm for deadlock avoidance

## Cleanup
Run:

    make clean

The IPC module also removes its shared memory and semaphore objects after execution.

## Source Files
- main.c: Main menu and module integration
- jobs.c / jobs.h: Job management
- scheduler.c / scheduler.h: CPU scheduling algorithms
- banker.c / banker.h: Banker’s algorithm
- logger.c / logger.h: Producer-consumer logging
- ipc.c / ipc.h: Shared memory and semaphore communication
- worker.c / worker.h: Process management
- limits.c / limits.h: Resource limit demonstrations
- Makefile: Build configuration
