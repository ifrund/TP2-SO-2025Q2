#include "proc.h"

PCB* processTable[MAX_PCS]= {NULL}; 

static int strlen(char * string){
    int i=0;
    while(string[i++]!=0);
    return i;
}
//tabla de procesos
int create_process(void * rip, char *name, int argc, char *argv[]){
    int i, myPid=-1;

    for (i = 0; i <MAX_PCS; i++){
        if (processTable[i] == NULL){
            myPid = i;
            break;
        }
       if(processTable[i]->state == ZOMBIE){ 
            free(processTable[i]->stackBase);
            free(processTable[i]);
            myPid = i;
            break;
        }
    }

    if (i >=MAX_PCS){
        return -1;
    }
    
    //Inicializamos PCB:
    PCB * pcb = alloc(sizeof(PCB));
    if (pcb == NULL) {
        return -1;
    }
    processTable[i] = pcb; 

    //Reservamos espacio para el stack
    void* stackBase = alloc(MAX_STACK_SIZE);
    if (stackBase == NULL) {
        free(processTable[i]);
        return -1; 
    }    
    pcb->stackBase = stackBase;

    //Preparamos argv
    char **argv_copy = NULL;
    if (argc > 0 && argv != NULL) {
        argv_copy = alloc(sizeof(char*) * (argc + 1)); // +1 para NULL final
        if (argv_copy == NULL) {
            free(stackBase);
            free(processTable[i]);
            return -1;
        }

        for (int k = 0; k < argc; k++) {
            int len = strlen(argv[k]) + 1;
            argv_copy[k] = alloc(len);
            if (argv_copy[k] == NULL) {
                //rollback en caso de error
                for (int m = 0; m < k; m++)
                    free(argv_copy[m]);
                free(argv_copy);
                free(stackBase);
                free(processTable[i]);
                return -1;
            }
            memcpy(argv_copy[k], argv[k], len);
        }
        argv_copy[argc] = NULL;
    }

    //Stack
    pcb->rsp = stackBase + MAX_STACK_SIZE;
    pcb->rsp = _create_stack(pcb->rsp, rip, argc, argv_copy);

    //Informacion
    memset(pcb->name, 0, PROCESS_NAME_MAX_LENGTH);
    memcpy(pcb->name, name, PROCESS_NAME_MAX_LENGTH - 1);
    pcb->name[PROCESS_NAME_MAX_LENGTH-1] = '\0';
    pcb->PID = myPid;
    pcb->ParentPID = get_pid();

    pcb->time_used=0;
    if(strcmp(name, "idle") == 0){
        pcb->my_max_time = IDLE_Q;
        pcb->my_prio = LEVEL_IDLE;
        //No lo dejamos ni en ready, eso se hara en el sch cuando se necesite
        pcb->state= BLOCKED;
    }
    else{
        pcb->my_max_time = QUANTUM;
        pcb->my_prio = LEVEL_4;
        pcb->state = READY;
    }
    
    //Informacion de los hijos
    pcb->childrenAmount = 0;
    pcb->isWaitingForChildren = false;
    for(int i = 0; i <MAX_PCS-1; i++) {
        pcb->childProc[i] = -1;
    } 

    memset(pcb->fileDescriptors, 0, sizeof(pcb->fileDescriptors));
    
    //TODO aca un yield ?? sino el padre va a consumir sus ticks en vez de dejar ser al hijo
    
    process_count++;
    return pcb->PID;
}

int block_process(uint64_t pid){
    if(!is_pid_valid(pid))
        return -1;

    if(processTable[pid]->state == READY || processTable[pid]->state == RUNNING)
        processTable[pid]->state = BLOCKED;
    
    return 0;
}

int unblock_process(uint64_t pid){
    if(!is_pid_valid(pid))
        return -1;
    if(processTable[pid]->state != BLOCKED )
        return -1;

    processTable[pid]->state = READY;
    return 0;
}

int kill_process(uint64_t pid){

    if(!is_pid_valid(pid))
        return -1;

    //Liberacion de recursos
    if(processTable[pid]->stackBase != NULL) {
        processTable[pid]->stackBase = NULL;
    }

    processTable[pid]->state = ZOMBIE;
    
    //desbloquear al padre si esta esperando      
    uint64_t parentPID = processTable[pid]->ParentPID;
    if (parentPID <MAX_PCS && processTable[parentPID] != NULL) {
        if (processTable[parentPID]->isWaitingForChildren) {
            unblock_process(parentPID);
            processTable[parentPID]->isWaitingForChildren = false;
        }
    }

    yield();

    return 0;
}

//TODO le faltan datos q tdv no existen
void get_proc_list(char ** procNames, uint64_t * pids, uint64_t * parentPids, char ** status, uint64_t * rsps){
    for(int i = 0; processTable[i] != NULL; i++){
        
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
    for (int i = 0; i <MAX_PCS; i++) {
        if (processTable[i] != NULL && processTable[i]->state == RUNNING) {
            pid = i;
            break;
        }
    }

    return pid;
}

int is_pid_valid(int pid){
    return (pid > MAX_PCS || processTable[pid] == NULL) ? -1 : 1;
}