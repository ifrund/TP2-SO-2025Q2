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

    //TODO aca un yield ?? sino el padre va a consumir sus ticks en vez de dejar ser al hijo
    process_count++;
    return pcb->PID;
}

int block_process(uint64_t pid){
    if(!is_pid_valid(pid))
        return -1;

    if(processTable[pid]->state == READY || processTable[pid]->state == RUNNING)
        processTable[pid]->state = BLOCKED;
    else{
        return -2;
    }
    
    return 0;
}

int unblock_process(uint64_t pid){
    if(!is_pid_valid(pid))
        return -1;
    if(processTable[pid]->state != BLOCKED )
        return -2;

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
            pid = i;
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

int wait(uint64_t pid){
    if (!is_pid_valid(pid))
        return -1;
    
    int currentPID = get_pid();
    if (!is_pid_valid(currentPID))
        return -2;

    PCB* current = processTable[currentPID];
    PCB* target = processTable[pid];

    //si el target termino primero, lo terminamos
    if (target->state == ZOMBIE) {
        cleanup_process(pid);
        return 0;
    }

    //Si el proceso que deberia esperar termino primero, hacemos el yield
    current->isWaitingForExtern = true;
    current->externWaitingPID = pid;
    block_process(currentPID);

    // #################################################################################
    //  ESPERA ACTIVA - BORRAR SI QUERES BORRAR A BARRACAS CENTRAL
    // ##################################################################################
    while (is_pid_valid(pid) && processTable[pid]->state != ZOMBIE) {
        //TODO usar sems
    }
    // #################################################################################

    return 0;
}
