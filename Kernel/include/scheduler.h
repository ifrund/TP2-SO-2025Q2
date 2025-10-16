#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <stddef.h>
// #include "include/proc.h"
#include "lib.h"
#include "interrupts.h"

#define QUANTUM 5
#define MAX_PCS 32
#define PROCESS_NAME_MAX_LENGTH 32

//Estados del proceso
typedef enum {
    READY,
    RUNNING,
    BLOCKED,
    ZOMBIE
} ProcessState;

typedef enum {
    LEVEL_0 = 0, //mayor prio
    LEVEL_1 = 1,
    LEVEL_2 = 2,
    LEVEL_3 = 3,
    LEVEL_4 = 4  //menor prio
} Priorities;

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

    Priorities my_prio;
    int time_used;
    int my_max_time;
    
    int argc;
    char* argv;
} PCB;
//TODO eliminar, ya esta en proc.h

void init_sch();
void * scheduling(void *rsp);
void add_pcs(PCB *pcb);
void delete_pcs(PCB *pcb);
void yield();
int be_nice(int pid);

#endif