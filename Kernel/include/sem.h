#ifndef _sem_
#define _sem_

#include <stdint.h>

#define MAX_SEMAPHORES 10
#define MAX_NAME_LENGTH 32
#define PROCESSES_BLOCKED 10

typedef struct
{
    char name[MAX_NAME_LENGTH];
    int value;
    int initialized;
    uint8_t lock;
    int blocked_processes[PROCESSES_BLOCKED];
    int amount_bprocesses;
    int amount_sem_openings;
} sem_internal_t;

typedef void *sem_t;

int sem_open_init(const char *name, unsigned int value);
int sem_wait(const char *name);
int sem_post(const char *name);
int sem_close(const char *name);
void close_all_sems();

#endif