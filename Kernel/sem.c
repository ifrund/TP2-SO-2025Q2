#include "include/sem.h"

static sem_internal_t* find_sem(const char* name) {
    for (int i = 0; i < MAX_SEMAPHORES; i++) {
        if (semaphores[i].initialized && strcmp(semaphores[i].name, name) == 0) {
            return &semaphores[i];
        }
    }
    return NULL;
}

int sem_open(const char* name, unsigned int value) {

    if(init==0){
        for (int i = 0; i < MAX_SEMAPHORES; i++) {
            semaphores[i].initialized=0;
        }
        init=1;
    }

    sem_internal_t* existing_sem = find_sem(name);
    if (existing_sem != NULL) {
        existing_sem->amount_sem_openings = existing_sem->amount_sem_openings + 1;
        return 1;  //semaforo ya existe
    }

    sem_internal_t* new_sem = NULL;
    for (int i = 0; i < MAX_SEMAPHORES; i++) {
        if (semaphores[i].initialized == 0) {
            new_sem = &semaphores[i];
        }
    }

    if (new_sem == NULL) {
        return 0;  //no hay slots libres
    }

    strncpy(new_sem->name, name, MAX_NAME_LENGTH - 1);
    new_sem->name[MAX_NAME_LENGTH-1] = '\0';
    new_sem->value = value;
    new_sem->initialized = 1;
    new_sem->amount_bprocesses = 0;
    new_sem->lock = 1;
    new_sem->amount_sem_openings = 1;
    return 2;
}

int sem_wait(const char* name) {
    sem_internal_t* sem = find_sem(name);
    if (sem == NULL) {
        return -1;
    }

    _wait(&(sem->lock));     
    int pid = get_pid();

    sem->value--;

    if(sem->value < 0) { //Toma menos uno como un numero grande positivo y no entra xd
        sem->blocked_processes[sem->amount_bprocesses++] = pid;
        block_process(pid);
    }

    _post(&(sem->lock));

    return 0;
}

int sem_post(const char* name) {
    sem_internal_t* sem = find_sem(name);
    if (sem == NULL) {
        return -1;
    }

    _wait(&(sem->lock));
    sem->value++;

    if(sem->value <= 0 && sem->amount_bprocesses > 0) {
            int pid = sem->blocked_processes[0];
            for(int i=1; i<sem->amount_bprocesses; i++) { 
                sem->blocked_processes[i-1] = sem->blocked_processes[i];
            }
            sem->amount_bprocesses--;
            unblock_process(pid);
    }

    _post(&(sem->lock));

    return 0;
}

int sem_close(const char* name) {
    sem_internal_t* sem = find_sem(name);
    if (sem == NULL) {
        return -1;
    }

    sem->amount_sem_openings = sem->amount_sem_openings - 1;
    if(sem->amount_sem_openings > 0){
        return -1;
    }

    sem->initialized = 0;
    memset(sem->name, 0, MAX_NAME_LENGTH);
    sem->value = 0;

    return 0;
}