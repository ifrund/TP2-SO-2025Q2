#ifndef _proc_
#define _proc_

#include <stdint.h>
#include <stdbool.h>

//Constants
#define MAX_FD 128
#define MAX_PROC 128
#define MAX_STACK_SIZE 4096 //4KB
#define PROCESS_NAME_MAX_LENGTH 32
//TODO: Definir MAX_PRIORITY

//Estados del proceso
typedef enum {
    READY,
    RUNNING,
    BLOCKED,
    ZOMBIE
} ProcessState;

//context
typedef struct {
    uint64_t rax, rbx, rcx, rdx, rsi, rdi, rbp, rsp;
    uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
    uint64_t rip;
    uint64_t rflags;
    //TODO: estos se utilizan?
    // uint16_t cs; 
    // uint16_t ss; 
    // uint16_t ds;
    // uint16_t es;
} StackFrame;

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
    uint64_t rsp; 
    uint64_t rbp;
    StackFrame * stackFrame;
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
PCB processTable[MAX_PROC];
//uint64_t next_pid = 1;

//Funciones:
int createProcess(ProcessEntryPoint entryPoint, const char *name, int argc, char *argv[]);
int blockProcess(uint64_t pid);
int unblockProcess(uint64_t pid);
int killProcess(uint64_t pid);
void getProcList(char ** procNames, uint64_t * pids, uint64_t * parentPids, char ** status, uint64_t * rsps, uint64_t * rbps);

#endif