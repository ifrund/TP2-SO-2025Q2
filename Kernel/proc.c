#include "proc.h"
#include "mm/memory_manager.h"
#include "lib.c"

//Puntero a funcion
typedef int (*ProcessEntryPoint)(int argc, char *argv[]);

int createProcess(ProcessEntryPoint entryPoint, const char *name, int argc, char *argv[]){
    int i;

    for (i = 0; i < MAX_PROC; i++){
        if (processTable[i].PID == 0)
            break;
    }

    if (i >= MAX_PROC)
        return -1;
    
    //Reservamos el espacio
    void* stackBase = alloc(MAX_STACK_SIZE);
    if (stackBase == NULL) {
        return -1; // Error de memoria
    }
    //Asignamos el stack frame al final de la memoria
    StackFrame* frame = (StackFrame*)((uint8_t*)stackBase + MAX_STACK_SIZE - sizeof(StackFrame));
    memset(frame, 0, sizeof(StackFrame));

    //Inicializamos stack frame:
    frame->rip = (uint64_t)entryPoint;
    frame->rdi = (uint64_t)argc;
    frame->rsi = (uint64_t)argv;
    //TODO: verificar FLAGS
    frame->rflags = 0x202; // habilita interrupciones (IF = 1)

    //Inicializamos PCB:
    PCB * pcb = &processTable[i];

    //Informacion
    strncpy(pcb->name, name, PROCESS_NAME_MAX_LENGTH - 1);
    pcb->name[PROCESS_NAME_MAX_LENGTH - 1] = '\0';

    pcb->PID = i;
    //TODO: PARENT PID
    pcb->ParentPID = 0;
    pcb->isForeground = true;
    pcb->state = READY;

    //Datos
    pcb->rsp = (uint64_t)frame;
    pcb->rbp = pcb->rsp;
    pcb->stackFrame = frame;
    pcb->stackBase = stackBase;

    //Argumentos que recibe
    pcb->argc = argc;
    pcb->argv = argv;

    //TODO: Prioridad

    //Informacion de los hijos
    pcb->childrenAmount = 0;
    pcb->isWaitingForChildren = false;

    memset(pcb->fileDescriptors, 0, sizeof(pcb->fileDescriptors));

    return 0;
}

int blockProcess(uint64_t pid){
    if(pid > MAX_PROC || processTable[pid].PID == 0)
        return -1;

    processTable[pid].state = BLOCKED;
    return 0;
}

int unblockProcess(uint64_t pid){
    if(pid > MAX_PROC || processTable[pid].PID == 0)
        return -1;

    processTable[pid].state = READY;
    return 0;
}

int killProcess(uint64_t pid){
    if(pid > MAX_PROC || processTable[pid].PID == 0)
        return -1;

    //Liberacion de recursos
    if (processTable[pid].stackBase != NULL) {
        free(processTable[pid].stackBase);
        processTable[pid].stackBase = NULL;
    }

    processTable[pid].PID = 0;  
    processTable[pid].state = ZOMBIE;
    
    //desbloquear al padre si esta esperando      
    uint64_t parentPID = processTable[pid].ParentPID;
    if (parentPID < MAX_PROC && processTable[parentPID].PID != 0) {
        if (processTable[parentPID].isWaitingForChildren) {
            unblockProcess(parentPID);
            processTable[parentPID].isWaitingForChildren = false;
        }
    }

    return 0;
}