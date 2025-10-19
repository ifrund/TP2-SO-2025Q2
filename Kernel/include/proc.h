#ifndef _proc_
#define _proc_

#include <stdint.h>
#include <stdbool.h>
#include "lib.h"
#include "memory_manager.h"
#include "naiveConsole.h"

//Constants
#define MAX_FD 128
#define MAX_PROC 128
#define MAX_STACK_SIZE 4096 //4KB
#define PROCESS_NAME_MAX_LENGTH 32
//TODO: Definir MAX_PRIORITY
#define MAX_ARGUMENTS 16
#define MAX_ARG_LENGTH 64 


//Puntero a funcion
typedef int (*ProcessEntryPoint)(int argc, char *argv[]);

//Estados del proceso
typedef enum {
    READY,
    RUNNING,
    BLOCKED,
    ZOMBIE,
    INVALID
} ProcessState;

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

    //TODO: Prioridad

    //Por si espera otro proceso
    uint64_t externWaitingPID;
    bool isWaitingForExtern;

    //Informacion de los hijos:
    int childrenAmount;
    uint64_t childProc[MAX_PROC];

} PCB;

//Para el get_proc_list
typedef struct {
    char name[PROCESS_NAME_MAX_LENGTH];
    uint64_t pid;
    uint64_t parentPid;
    bool isForeground;
    char state[16];            // "READY", etc.
    uint64_t rsp;
    
    int argc;
    char argv[MAX_ARGUMENTS][MAX_ARG_LENGTH];  

    uint64_t externWaitingPID;
    bool isWaitingForExtern;

    int childrenAmount;
    uint64_t children[MAX_PROC];

    // Podrías incluir file descriptors si querés: los ids nada más.
    uint64_t fileDescriptors[MAX_FD];
    int fileDescriptorCount;  // Número de FDs válidos
} ProcessInfo;


//Funciones:
int create_process(void * rip, char *name, int argc, char *argv[]);
int block_process(uint64_t pid);
int unblock_process(uint64_t pid);
int kill_process(uint64_t pid);
void get_proc_list(char ** procNames, uint64_t * pids, uint64_t * parentPids, char ** status, uint64_t * rsps);
int get_pid();
int wait(uint64_t pid);
int wait_for_all_children();

#endif