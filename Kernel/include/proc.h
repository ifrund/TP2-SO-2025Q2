#ifndef _proc_
#define _proc_

#include <stdint.h>
#include <stdbool.h>
#include "lib.h"
#include "memory_manager.h"
#include "naiveConsole.h"
#include "scheduler.h"

//Constants
#define MAX_FD 128
#define MAX_PCS 64
#define MAX_STACK_SIZE 4096 //4KB
#define PROCESS_NAME_MAX_LENGTH 32
#define QUANTUM 5
#define IDLE_Q 1

//Estados del proceso
typedef enum {
    READY,
    RUNNING,
    BLOCKED,
    ZOMBIE,
    KILLED //TODO es zombie o killed, pero no las dos, verdad?
} ProcessState;

typedef enum {
    LEVEL_0 = 0, //mayor prio
    LEVEL_1 = 1,
    LEVEL_2 = 2,
    LEVEL_3 = 3,
    LEVEL_4 = 4,  //menor prio
    LEVEL_IDLE = 5
} Priorities;

//PCB definition
typedef struct {
    //Informacion 
    char name[PROCESS_NAME_MAX_LENGTH];
    uint64_t PID;
    uint64_t ParentPID;
    bool isForeground;
    ProcessState state;

    //File Descriptors
    uint64_t fileDescriptors[MAX_FD];

    //Datos
    void* rsp; 
    void* stackBase;

    //Argumentos que recibe
    int argc;
    char** argv;

    Priorities my_prio;
    int time_used;
    int my_max_time;

    //Informacion de los hijos:
    int childrenAmount;
    uint64_t childProc[MAX_PCS];
    bool isWaitingForChildren;

} PCB;

extern PCB* processTable[MAX_PCS]; 

//Funciones:
int create_process(void * rip, char *name, int argc, char *argv[]);
int block_process(uint64_t pid);
int unblock_process(uint64_t pid);
int kill_process(uint64_t pid);
void get_proc_list(char ** procNames, uint64_t * pids, uint64_t * parentPids, char ** status, uint64_t * rsps);
int get_pid();
int is_pid_valid(int pid);

#endif