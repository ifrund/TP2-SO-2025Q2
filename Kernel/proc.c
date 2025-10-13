#include "proc.h"

//Puntero a funcion
typedef int (*ProcessEntryPoint)(int argc, char *argv[]);

void createProcess(ProcessEntryPoint entryPoint, const char *name, int argc, char *argv[]){
    int i = 0;

    for (int i = 0; i < MAX_PROC; i++){
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
    
    if(pid > MAX_PROC || pid < 0 || processTable[pid].PID == 0)
        return -1;

    processTable[pid].state = BLOCKED;
    return 0;
}

int unblockProcess(uint64_t pid){
    if(pid > MAX_PROC || pid < 0 || processTable[pid].PID == 0)
        return -1;

    processTable[pid].state = READY;
    return 0;
}

int killProcess(uint64_t pid){
    //TODO: Kill process
    if(pid > MAX_PROC || pid < 0 || processTable[pid].PID == 0)
        return -1;

    processTable[pid].PID = 0;  
    processTable[pid].state = ZOMBIE;
    
    //TODO: liberar recursos

    //desbloquear al padre si esta esperando      
    unblockProcess(processTable[pid].ParentPID);

    return 0;
}