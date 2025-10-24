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
            free(processTable[i]->argv);
            myPid = i;
            break;
        }
    }

    if (myPid == -1){
        return -1;
    }
    
    //Inicializamos PCB:
    PCB * pcb = alloc(sizeof(PCB));
    if (pcb == NULL)
        return -1;
    processTable[myPid] = pcb; 

    //Informacion general
    memset(pcb, 0, sizeof(PCB));

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
        
    //Espera externa:
    pcb->isWaitingForExtern = false;
    pcb->externWaitingPID = -1;
    
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
        
    memset(pcb->fileDescriptors, 0, sizeof(pcb->fileDescriptors));

    //TODO este yield debe ser para todos??
    if(strcmp(name, "wait") == 0){
        yield(); //necesitamos q el pcs q crea un wait deje sus quehaceres y se frene
    }

    process_count++;
    return pcb->PID;
}

int get_process_index_by_pid(int pid) {
    for (int i = 0; i < process_count; i++) {
        if (processTable[i] != NULL && processTable[i]->PID == pid) {
            return i;
        }
    }
    return -1; // no encontrado
}

int block_process(int pid){
    if(!is_pid_valid(pid))
        return -1;

    int index = get_process_index_by_pid(pid);

    if(processTable[index]->state == READY || processTable[index]->state == RUNNING)
        processTable[index]->state = BLOCKED;
    else{
        return -2;
    }

    return 0;
}

int unblock_process(uint64_t pid){
    if(!is_pid_valid(pid))
        return -1;
    
    int index = get_process_index_by_pid(pid);

    if(processTable[index]->state != BLOCKED )
        return -2;

    processTable[index]->state = READY;
    return 0;
}

int kill_process(uint64_t pid){
    if (!is_pid_valid(pid))
        return -1;

    PCB* proc = processTable[pid];    
    int index = get_process_index_by_pid(pid);

    processTable[index]->state = ZOMBIE;

    uint64_t parentPID = proc->ParentPID;
    int index_parent = get_process_index_by_pid(parentPID);

    if (index_parent < MAX_PCS && processTable[index_parent] != NULL) {
        PCB* parent = processTable[index_parent];

        //me elimino del padre
        for (int i = 0; i < MAX_PCS; i++) {
            if (parent->childProc[i] == pid) {
                parent->childProc[i] = -1;
                parent->childrenAmount--;
                break;
            }
        }

        //Si el padre esta haciendo wait por exactamente este, desbloq
        if (parent->isWaitingForExtern && parent->externWaitingPID == pid) {
            parent->isWaitingForExtern = false;
            parent->externWaitingPID = -1;
            unblock_process(parentPID);
        }

        if (parent->childrenAmount == 0 && parent->state == BLOCKED) {
            unblock_process(parentPID);
        }
    }

    //TODO manejar huerfanos

    yield();

    return 0;
}

char *strncpy(char *dest, const char *src, unsigned int n) {
    unsigned int i;
    for (i = 0; i < n && src[i] != '\0'; i++)
        dest[i] = src[i];

    // Rellenar con '\0' si src es más corta que n
    for (; i < n; i++)
        dest[i] = '\0';

    return dest;
}

ProcessInfo* get_proc_list() {
    ProcessInfo* list = alloc(sizeof(ProcessInfo) * MAX_PCS); //TODO y si en vez de MAX_PCS pondemos process_count
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

        info->externWaitingPID = p->externWaitingPID;
        info->isWaitingForExtern = p->isWaitingForExtern;

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

void cleanup_process(uint64_t pid) {
    if (!is_pid_valid(pid))
        return;
    
    PCB* pcb = processTable[pid];

    pcb->state = ZOMBIE;

    if (pcb->stackBase)
        free(pcb->stackBase);

    free(pcb);
    processTable[pid] = NULL;
}

int wait(uint64_t target_pid, uint64_t my_pid){
    
    if (!is_pid_valid(target_pid))
        return -1;
    
    if (!is_pid_valid(my_pid)) //TODO cuando entra a aca?? este if no es "che yo existo??" xd
        return -2;

    int my_index = get_process_index_by_pid(my_pid);
    PCB* current = processTable[my_index];
    int t_index = get_process_index_by_pid(target_pid);
    PCB* target = processTable[t_index];
    int t_pid = target->PID;

    //si el target termino primero, lo terminamos
    if (target->state == ZOMBIE) {
        cleanup_process(t_pid);
        return 0;
    }

    //Si el proceso que deberia esperar termino primero, hacemos el yield
    current->isWaitingForExtern = true;
    current->externWaitingPID = t_pid;
    block_process(processTable[my_index]->PID);
    
    // #################################################################################
    //  ESPERA ACTIVA - BORRAR SI QUERES BORRAR A BARRACAS CENTRAL
    // ##################################################################################
    while (is_pid_valid(t_pid) && processTable[t_pid]->state != ZOMBIE) {
        yield();
        //TODO usar sems
    }
    // #################################################################################

    unblock_process(processTable[my_index]->PID);
    return 0;
}
