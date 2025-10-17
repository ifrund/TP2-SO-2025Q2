#include "proc.h"

//tabla de procesos
PCB* processTable[MAX_PROC]= {NULL}; 

int create_process(void * rip, char *name, int argc, char *argv[]){
    int i, myPid=-1;

    for (i = 0; i < MAX_PROC; i++){
        if (processTable[i] == NULL){
            myPid = i;
            break;
        }
       if(processTable[i]->state == KILLED){
            myPid = i;
            //TODO, cleanup de pcs muerto
            break;
        }
    }

    if (i >= MAX_PROC){
        ncPrint("No hay espacio para mas procesos\n");
        return -1;
    }
    
    //Inicializamos PCB:
    PCB * pcb = alloc(sizeof(PCB));
    if (pcb == NULL) {
        ncPrint("Error en el alloc a el proceso\n");
        return -1; // Error de memoria
    }
    processTable[i] = pcb; 

    //Informacion
    memcpy(pcb->name, name, PROCESS_NAME_MAX_LENGTH - 1);
    pcb->name[PROCESS_NAME_MAX_LENGTH - 1] = '\0';
    pcb->PID = myPid;
    pcb->ParentPID = get_pid();
    pcb->isForeground = true;
    pcb->state = READY;

    //Reservamos el espacio
    void* stackBase = alloc(MAX_STACK_SIZE);
    if (stackBase == NULL) {
        free(processTable[i]);
        ncPrint("Error en el alloc a stackBase\n");
        return -1; // Error de memoria
    }
    pcb->stackBase = stackBase;
    pcb->rsp = stackBase + MAX_STACK_SIZE;
    pcb->rsp = _create_stack(pcb->rsp, rip, argc, argv);
    pcb->argc = argc;
    pcb->argv = argv;

    //TODO: Prioridad

    //Informacion de los hijos
    pcb->childrenAmount = 0;
    pcb->isWaitingForChildren = false;
    for(int i = 0; i < MAX_PROC-1; i++) {
        pcb->childProc[i] = -1;
    } 

    memset(pcb->fileDescriptors, 0, sizeof(pcb->fileDescriptors));

    //TODO agregar al sch
    return pcb->PID;
}

int block_process(uint64_t pid){
    if(pid > MAX_PROC || processTable[pid]->PID == 0)
        return -1;

    processTable[pid]->state = BLOCKED;
    return 0;
}

int unblock_process(uint64_t pid){
    if(pid > MAX_PROC || processTable[pid]->PID == 0)
        return -1;

    processTable[pid]->state = READY;
    return 0;
}

int kill_process(uint64_t pid){
    if(pid > MAX_PROC || processTable[pid]->PID == 0)
        return -1;

    //Liberacion de recursos
    if (processTable[pid]->stackBase != NULL) {
        free(processTable[pid]->stackBase);
        free(processTable[pid]);
        processTable[pid]->stackBase = NULL;
    }

    processTable[pid]->PID = 0;  
    processTable[pid]->state = ZOMBIE;
    
    //desbloquear al padre si esta esperando      
    uint64_t parentPID = processTable[pid]->ParentPID;
    if (parentPID < MAX_PROC && processTable[parentPID]->PID != 0) {
        if (processTable[parentPID]->isWaitingForChildren) {
            unblock_process(parentPID);
            processTable[parentPID]->isWaitingForChildren = false;
        }
    }

    return 0;
}

void get_proc_list(char ** procNames, uint64_t * pids, uint64_t * parentPids, char ** status, uint64_t * rsps){
    for(int i = 0; processTable[i]->PID != 0; i++){
        
        memcpy(procNames[i], processTable[i]->name, PROCESS_NAME_MAX_LENGTH - 1);
        procNames[i][PROCESS_NAME_MAX_LENGTH - 1] = '\0';
        
        pids[i] = processTable[i]->PID;
        parentPids[i] = processTable[i]->ParentPID;

        rsps[i] = (uint64_t) processTable[i]->rsp;

        if(processTable[i]->state == RUNNING){
            memcpy(status[i], "RUNNING", PROCESS_NAME_MAX_LENGTH - 1);
        } else if (processTable[i]->state == BLOCKED){
            memcpy(status[i], "BLOCKED", PROCESS_NAME_MAX_LENGTH - 1);
        } else if (processTable[i]->state == READY) {
            memcpy(status[i], "READY", PROCESS_NAME_MAX_LENGTH - 1);
        } else {
            memcpy(status[i], "ZOMBIE", PROCESS_NAME_MAX_LENGTH - 1);
        }
        status[i][PROCESS_NAME_MAX_LENGTH - 1] = '\0';
    }
}

int get_pid(){
    
    int pid = -1;
    for (int i = 0; i < MAX_PROC; i++) {
        if (processTable[i] != NULL && processTable[i]->state == RUNNING) {
            pid = i;
            break;
        }
    }

    return pid;
}