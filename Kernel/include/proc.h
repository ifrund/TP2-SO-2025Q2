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
#define QUANTUM 2
#define IDLE_Q 1
#define MAX_ARGUMENTS 16
#define MAX_ARG_LENGTH 64
#define INFO_STR_LENGTH 16

extern int IDLE_PID; 
extern int SHELL_PID;

//Estados del proceso
typedef enum {
    READY,
    RUNNING,
    BLOCKED,
    ZOMBIE,
    INVALID
} process_state;

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
    uint64_t parent_pid;
    process_state state;

    //File Descriptors
    uint64_t file_descriptors[MAX_FD];

    //Datos
    void* rsp; 
    void* stack_base;
    char** argv; //solo existe para poder hacerle free

    Priorities my_prio;
    int time_used;
    int my_max_time;
    int total_ticks;
    int changes;
    int yield_changes;
    //Informacion de los hijos:
    int child_amount;
    int childs[MAX_PCS];
    int blocks_amount;

    int yielding; //1 true, 0 false
} PCB;

extern PCB* process_table[MAX_PCS]; 
  
//Para el get_proc_list
typedef struct {
    char name[PROCESS_NAME_MAX_LENGTH];
    uint64_t pid;
    uint64_t parent_pid;
    char state[INFO_STR_LENGTH];            // "READY", etc.
    uint64_t rsp;
    char my_prio[INFO_STR_LENGTH];
    int child_amount;

    int children[MAX_PCS];
    // Podrías incluir file descriptors si querés: los ids nada más.
    uint64_t file_descriptors[MAX_FD];
    int fds_count;  // Número de FDs válidos
} ProcessInfo;


//Funciones:
int create_process(void * rip, char *name, int argc, char *argv[], uint64_t *fds);
int block_process(int pid);
int unblock_process(uint64_t pid);
int kill_process(uint64_t pid);
ProcessInfo* get_proc_list();
int get_pid();
int is_pid_valid(int pid);
int wait(uint64_t target_pid, uint64_t my_pid);
int get_shell_pid();
int get_idle_pid();

#endif