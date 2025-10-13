#include "proc.h"
#include "mm/memory_manager.h"

//Puntero a funcion
typedef int (*ProcessEntryPoint)(int argc, char *argv[]);

void createProcess(ProcessEntryPoint entryPoint, const char *name, int argc, char *argv[]){
    int i;

    for (i = 0; i < MAX_PROC; i++){
        if (processTable[i].PID == 0)
            break;
    }

    if (i >= MAX_PROC)
        return;
    
    //TODO: Inicializamos todo:
    processTable[i].PID = i;
    processTable[i].state = READY;
    strncpy(processTable[i].name, name, PROCESS_NAME_MAX_LENGTH);

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