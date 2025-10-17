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

//Puntero a funcion
typedef int (*ProcessEntryPoint)(int argc, char *argv[]);

//Estados del proceso
typedef enum {
    READY,
    RUNNING,
    BLOCKED,
    ZOMBIE,
    KILLED //TODO es zombie o killed, pero no las dos, verdad?
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

    //Informacion de los hijos:
    int childrenAmount;
    uint64_t childProc[MAX_PROC];
    bool isWaitingForChildren;

} PCB;

//Tabla de procesos
//PCB processTable[MAX_PROC];
//uint64_t next_pid = 1;

//Funciones:
int create_process(void * rip, char *name, int argc, char *argv[]);
int block_process(uint64_t pid);
int unblock_process(uint64_t pid);
int kill_process(uint64_t pid);
void get_proc_list(char ** procNames, uint64_t * pids, uint64_t * parentPids, char ** status, uint64_t * rsps);
int get_pid();

#endif