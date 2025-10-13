#ifndef _proc_
#define _proc_

#include <stdint.h>

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
    uint64_t rax, rbx, rcx, rdx, rsi, rdi, rbp, rsp, r8, r9, r10, r11, r12, r13, r14, r15;
    uint64_t rip;
    uint64_t rflags;
    //TODO: estos se utilizan?
    uint16_t cs; 
    uint16_t ss; 
    uint16_t ds;
    uint16_t es;
} StackFrame;

//PCB definition
typedef struct {
    char name[PROCESS_NAME_MAX_LENGTH];
    uint64_t PID;
    
    //TODO: Prioridad
    
    //Stack
    uint64_t rsp;
    //Base pointer
    uint64_t rbp;

    char isForeground;
    uint64_t ParentPID;
    ProcessState state;
    uint64_t fileDescriptors[MAX_FD];

    StackFrame stackFrame;
    int argc;
    char* argv;
} PCB;

//Tabla de procesos
PCB processTable[MAX_PROC];

#endif