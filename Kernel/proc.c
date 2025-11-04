#include "proc.h"

PCB* processTable[MAX_PCS]= {NULL}; 
int IDLE_PID;
int SHELL_PID;

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
            PCB *zombie = processTable[i];

            if (zombie->argv != NULL) {
                for (int j = 0; zombie->argv[j] != NULL; j++) {
                    free(zombie->argv[j]);
                }
                free(zombie->argv);
            }

            free(zombie->stackBase);
            free(zombie);
            processTable[i] = NULL;
            myPid = i;
            break;
        }
    }
    //de esta manera el pid simepre es el indice en la tabla

    if (myPid == -1){
        return -1;
    }
    
    //Inicializamos PCB:
    PCB * pcb = alloc(sizeof(PCB));
    if (pcb == NULL)
        return -2;
    processTable[myPid] = pcb; 

    //Informacion general
    memset(pcb, 0, sizeof(PCB));

    //Reservamos espacio para el stack
    void* stackBase = alloc(MAX_STACK_SIZE);
    if (stackBase == NULL) {
        free(pcb);
        processTable[i] = NULL;
        return -2; 
    }    
    pcb->stackBase = stackBase;

    //Preparamos argv
    char **argv_copy = NULL;
    if (argc > 0 && argv != NULL) {
        argv_copy = alloc(sizeof(char*) * (argc + 1)); // +1 para NULL final
        if (argv_copy == NULL) {
            free(stackBase);
            free(processTable[i]);
            processTable[i] = NULL;
            return -2;
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
                free(pcb);
                processTable[i] = NULL;
                return -2;
            }
            memcpy(argv_copy[k], argv[k], len);
        }
        argv_copy[argc] = NULL;
    }
    processTable[myPid]->argv = argv_copy;

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
    for(int i = 0; i < MAX_PCS; i++) {
        pcb->childProc[i] = -1;
    } 

    //Registrar como hijo del padre
    int parentPID = pcb->ParentPID;
    if (parentPID >= 0 && parentPID < MAX_PCS && processTable[parentPID] != NULL) {
        PCB *parent = processTable[parentPID];
        for (int i = 0; i < MAX_PCS; i++) {
            if (parent->childProc[i] == -1) {
                parent->childProc[i] = myPid;
                parent->childrenAmount++;
                break;
            }
        }
    }
    pcb->blocksAmount = 0;
        
    memset(pcb->fileDescriptors, 0, sizeof(pcb->fileDescriptors));
    pcb->isYielding = 0;
    pcb->total_ticks = 0;
    pcb->changes = 0;
    pcb->yield_changes = 0;

    if(strcmp(name, "wait") == 0){
        active_processes++;
        yield(); //necesitamos q el pcs q crea un wait deje sus quehaceres y se frene
        return pcb->PID;
    }

    active_processes++;
    return pcb->PID;
}

int block_process(int pid){
    if(!is_pid_valid(pid))
        return -1;

    ProcessState state = processTable[pid]->state;
    if(state == READY || state == RUNNING){
        processTable[pid]->state = BLOCKED;
        processTable[pid]->blocksAmount++;
        yield();
        return 0;
    }
    if(state == BLOCKED){
        processTable[pid]->blocksAmount++;
        yield();
        return 0;
    }
    else{ //si entra aca es ZOMBIE o INVALID
        return -2;
    }
}

int unblock_process(uint64_t pid){
    if(!is_pid_valid(pid))
        return -1;
    
    if(processTable[pid]->state != BLOCKED )
        return -2;

    if(processTable[pid]->blocksAmount > 1){ //si hubo varios blocks, los descontamos
        processTable[pid]->blocksAmount--;
        return 0;
    }
    //si estamos aca, significa q solo hubo un block, asiq lo descontamos y liberamos el pcs
    processTable[pid]->blocksAmount--;
    processTable[pid]->state = READY;

    return 0;
}

int kill_process(uint64_t pid){
    if (!is_pid_valid(pid))
        return -1;

    PCB* proc = processTable[pid];    
    processTable[pid]->state = ZOMBIE;

    uint64_t parentPID = proc->ParentPID;

    if (parentPID < MAX_PCS && processTable[parentPID] != NULL) {
        PCB* parent = processTable[parentPID];

        //me elimino del padre
        for (int i = 0; i < MAX_PCS; i++) {
            if (parent->childProc[i] == pid) {
                parent->childProc[i] = -1;
                parent->childrenAmount--;
                break;
            }
        }
        unblock_process(parentPID);
    }

    //a todos mis hijos se los dejo a idle, no improta q este bloqueado
    PCB * idle = processTable[IDLE_PID];
    for(int i=0; i < proc->childrenAmount; i++){
        int childPid = proc->childProc[i];
        PCB* child = processTable[childPid];
        child->ParentPID = IDLE_PID;
        idle->childProc[idle->childrenAmount++] = childPid;
    }

    active_processes--;
    last_wish(pid);
    return 0;
}

ProcessInfo* get_proc_list() {
    ProcessInfo* list = alloc(sizeof(ProcessInfo) * MAX_PCS);
    if (list == NULL)
        return NULL;

    for (int i = 0; i < MAX_PCS; i++) {
        PCB* p = processTable[i];
        ProcessInfo* info = &list[i];

        if (p == NULL) {
            // Proceso vacío
            info->pid = -1;
            strncpy(info->state, "INVALID", 15);
            continue;
        }

        // General
        memcpy(info->name, p->name, PROCESS_NAME_MAX_LENGTH - 1);
        info->name[PROCESS_NAME_MAX_LENGTH - 1] = '\0';

        info->pid = p->PID;
        info->parentPid = p->ParentPID;

        switch (p->state) {
            case RUNNING: memcpy(info->state, "RUNNING", 15); break;
            case BLOCKED: memcpy(info->state, "BLOCKED", 15); break;
            case READY:   memcpy(info->state, "READY", 15);   break;
            case ZOMBIE:  memcpy(info->state, "ZOMBIE", 15);  break;
            default:      memcpy(info->state, "UNKNOWN", 15); break;
        }

        info->state[15] = '\0';
        info->rsp = (uint64_t)p->rsp;

        switch (p->my_prio) {
            case LEVEL_0: memcpy(info->my_prio, "LEVEL_0", 15); break;
            case LEVEL_1: memcpy(info->my_prio, "LEVEL_1", 15); break;
            case LEVEL_2: memcpy(info->my_prio, "LEVEL_2", 15); break;
            case LEVEL_3: memcpy(info->my_prio, "LEVEL_3", 15); break;
            case LEVEL_4: memcpy(info->my_prio, "LEVEL_4", 15); break;
            case LEVEL_IDLE: memcpy(info->my_prio, "LEVEL_IDLE", 15); break;
            default: memcpy(info->my_prio, "UNKNOWN", 15); break;
        }

        info->my_prio[15] = '\0';


        info->childrenAmount = p->childrenAmount;
        for (int j = 0; j < MAX_PCS; j++)
            info->children[j] = p->childProc[j];

        // File descriptors
        int fdCount = 0;
        for (int j = 0; j < MAX_FD; j++) {
            info->fileDescriptors[j] = p->fileDescriptors[j];
            if (p->fileDescriptors[j] != 0)
                fdCount++;
        }
        info->fileDescriptorCount = fdCount;
    }

    return list;
}

int get_pid(){
    
    int pid = -1;
    for (int i = 0; i <MAX_PCS; i++) {
        if (processTable[i] != NULL && processTable[i]->state == RUNNING) {
            pid = processTable[i]->PID;
            break;
        }
    }

    return pid;
}

int is_pid_valid(int pid){
    return (pid > MAX_PCS || processTable[pid] == NULL) ? 0 : 1;
}

int real_wait(uint64_t target_pid, uint64_t my_pid);

int wait(uint64_t target_pid, uint64_t my_pid, char* target_name){

    if (!is_pid_valid(target_pid))
        return -1;
    
    if (!is_pid_valid(my_pid)) 
        return -2;

    if (strcmp(target_name, processTable[target_pid]->name) != 0)
        return -3;

    PCB* target = processTable[target_pid]; //a quien tenemos q esperar

    //si el target termino primero, lo terminamos
    if (target->state == ZOMBIE) { 
        return 0;
    }

    return real_wait(target_pid, my_pid);
}

int real_wait(uint64_t target_pid, uint64_t my_pid){

    //Si el proceso que deberia esperar termino primero, hacemos el yield
    block_process(my_pid);
    
    while (is_pid_valid(target_pid) && processTable[target_pid]->state != ZOMBIE) {
        yield();
        //TODO usar sems
        // #################################################################################
        //  ESPERA ACTIVA - BORRAR SI QUERES BORRAR A BARRACAS CENTRAL
        // ##################################################################################
    }

    return 0;
}

int get_shell_pid(){
    return SHELL_PID;
}

int get_idle_pid(){
    return IDLE_PID;
}