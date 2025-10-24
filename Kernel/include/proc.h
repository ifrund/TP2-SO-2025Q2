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
#define MAX_ARGUMENTS 16
#define MAX_ARG_LENGTH 64 


//Estados del proceso
typedef enum {
    READY,
    RUNNING,
    BLOCKED,
    ZOMBIE,
    INVALID
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
    ProcessState state;

    //File Descriptors
    uint64_t fileDescriptors[MAX_FD];

    //Datos
    void* rsp; 
    void* stackBase;
    char** argv; //solo existe para poder hacerle free

    Priorities my_prio;
    int time_used;
    int my_max_time;

    //Por si espera otro proceso
    uint64_t externWaitingPID;
    bool isWaitingForExtern;

    //Informacion de los hijos:
    int childrenAmount;
    uint64_t childProc[MAX_PCS];

} PCB;

extern PCB* processTable[MAX_PCS]; 
  
//Para el get_proc_list
typedef struct {
    char name[PROCESS_NAME_MAX_LENGTH];
    uint64_t pid;
    uint64_t parentPid;
    char state[16];            // "READY", etc.
    uint64_t rsp;
    char my_prio[16];

    //TODO probablmente borrarlas
    uint64_t externWaitingPID;
    bool isWaitingForExtern;
    int childrenAmount;
    uint64_t children[MAX_PCS];
    // Podrías incluir file descriptors si querés: los ids nada más.
    uint64_t fileDescriptors[MAX_FD];
    int fileDescriptorCount;  // Número de FDs válidos
} ProcessInfo;


//Funciones:
int create_process(void * rip, char *name, int argc, char *argv[]);
int block_process(int pid);
int unblock_process(uint64_t pid);
int kill_process(uint64_t pid);
ProcessInfo* get_proc_list();
int get_pid();
int is_pid_valid(int pid);
int wait(uint64_t target_pid, uint64_t my_pid);

#endif