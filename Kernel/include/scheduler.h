#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <stddef.h>
// #include "include/proc.h"
#include "include/lib.h"

#define MAX_PCS 32
#define PROCESS_NAME_MAX_LENGTH 32

//Estados del proceso
typedef enum {
    READY,
    RUNNING,
    BLOCKED,
    ZOMBIE
} ProcessState;

//PCB definition
typedef struct {
    char name[PROCESS_NAME_MAX_LENGTH];
    uint64_t PID;
        
    //Stack
    uint64_t rsp;
    //Base pointer
    uint64_t rbp;

    char isForeground;
    uint64_t ParentPID;
    ProcessState state;

    int argc;
    char* argv;
} PCB;
//TODO eliminar, ya esta en proc.h

void * scheduling(void *rsp);
void add_pcs(PCB *pcb);
void delete_pcs(PCB *pcb);
void yield();

#endif