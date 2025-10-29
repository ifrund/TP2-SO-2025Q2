#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_

typedef struct Semaphore sem_t;

sem_t sem_init(sem_t *sem, int pshared, unsigned int value);

int sem_destroy(sem_t * sem);

int sem_wait(sem_t * sem);

int sem_post(sem_t * sem);

#endif // _SEMAPHORE_H_