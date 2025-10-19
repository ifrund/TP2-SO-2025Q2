#include "proc.h"

//tabla de procesos
PCB* processTable[MAX_PROC]= {NULL}; 

inline int isValid(int pid) {
    return (pid >= 0 && pid < MAX_PROC && processTable[pid] != NULL);
}

int create_process(void * rip, char *name, int argc, char *argv[]){
    
    int myPid = -1;

    for (int i = 0; i < MAX_PROC; i++){
        if (processTable[i] == NULL){
            myPid = i;
            break;
        }
       if(processTable[i]->state == ZOMBIE){
            myPid = i;
            //cleanup de pcs muerto
            cleanup_process(myPid);            
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

    memcpy(pcb->name, name, PROCESS_NAME_MAX_LENGTH - 1);
    pcb->name[PROCESS_NAME_MAX_LENGTH - 1] = '\0';
    
    pcb->PID = myPid;
    pcb->ParentPID = get_pid();
    pcb->isForeground = true;
    pcb->state = READY;

    //Stack
    void* stackBase = alloc(MAX_STACK_SIZE);
    if (stackBase == NULL) {
        free(pcb);
        processTable[myPid] = NULL;
        ncPrint("Error en el alloc a stackBase\n");
        return -1; 
    }

    pcb->stackBase = stackBase;
    pcb->rsp = stackBase + MAX_STACK_SIZE;
    pcb->rsp = _create_stack(pcb->rsp, rip, argc, argv);
    //TODO: validar create_stack
    
    //Argumentos
    pcb->argc = argc;
    pcb->argv = argv;

    //TODO: Prioridad

    //file descriptors
    memset(pcb->fileDescriptors, 0, sizeof(pcb->fileDescriptors));

    //Espera externa:
    pcb->isWaitingForExtern = false;
    pcb->externWaitingPID = -1;
    
    //Informacion de los hijos
    pcb->childrenAmount = 0;
    for(int i = 0; i < MAX_PROC; i++) {
        pcb->childProc[i] = -1;
    } 

    //Registrar como hijo del padre
    int parentPID = pcb->ParentPID;
    if (parentPID >= 0 && parentPID < MAX_PROC && processTable[parentPID] != NULL) {
        PCB *parent = processTable[parentPID];
        for (int i = 0; i < MAX_PROC; i++) {
            if (parent->childProc[i] == -1) {
                parent->childProc[i] = myPid;
                parent->childrenAmount++;
                break;
            }
        }
    }

    //TODO: agregar al sch

    return pcb->PID;
}

int block_process(uint64_t pid){
    if (!isValid(pid))
        return -1;

    if(processTable[pid]->state == READY || processTable[pid]->state == RUNNING)
        processTable[pid]->state = BLOCKED;
    
    return 0;
}

int unblock_process(uint64_t pid){
    if (!isValid(pid))
        return -1;
    if(processTable[pid]->state != BLOCKED )
        return -1;

    processTable[pid]->state = READY;
    return 0;
}

int kill_process(uint64_t pid){
    if (!isValid(pid))
        return -1;

    PCB* proc = processTable[pid];    

    //Aca iria: wait for all children. Hasta entonces no podes morir en paz. Quedaria "ciclando" en si mismo, hasta que childrenAmount=0?
    //TODO: Checkear que esto no de deadlocks
    wait_for_all_children();

    processTable[pid]->state = ZOMBIE;

    uint64_t parentPID = proc->ParentPID;
    if (parentPID < MAX_PROC && processTable[parentPID] != NULL) {
        PCB* parent = processTable[parentPID];

        //se elimina el hijo
        for (int i = 0; i < MAX_PROC; i++) {
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

    //Liberacion de recursos
    //TODO: La liberacion de recursos esta bien aca o habria que hacerla en el wait?
    cleanup_process(pid);
   

    return 0;
}

// void get_proc_list(char ** procNames, uint64_t * pids, uint64_t * parentPids, char ** status, uint64_t * rsps){
//     for (int i = 0; i < MAX_PROC; i++) {
//         if (processTable[i] == NULL)
//             continue;

//         memcpy(procNames[i], processTable[i]->name, PROCESS_NAME_MAX_LENGTH - 1);
//         procNames[i][PROCESS_NAME_MAX_LENGTH - 1] = '\0';
        
//         pids[i] = processTable[i]->PID;
//         parentPids[i] = processTable[i]->ParentPID;

//         rsps[i] = (uint64_t) processTable[i]->rsp;

//         if(processTable[i]->state == RUNNING){
//             memcpy(status[i], "RUNNING", PROCESS_NAME_MAX_LENGTH - 1);
//         } else if (processTable[i]->state == BLOCKED){
//             memcpy(status[i], "BLOCKED", PROCESS_NAME_MAX_LENGTH - 1);
//         } else if (processTable[i]->state == READY) {
//             memcpy(status[i], "READY", PROCESS_NAME_MAX_LENGTH - 1);
//         } else {
//             memcpy(status[i], "ZOMBIE", PROCESS_NAME_MAX_LENGTH - 1);
//         }
//         status[i][PROCESS_NAME_MAX_LENGTH - 1] = '\0';
//     }
// }

ProcessInfo* get_proc_list() {
    ProcessInfo* list = alloc(sizeof(ProcessInfo) * MAX_PROC);
    if (list == NULL)
        return NULL;

    for (int i = 0; i < MAX_PROC; i++) {
        PCB* p = processTable[i];
        ProcessInfo* info = &list[i];

        if (p == NULL) {
            // Proceso vacío
            info->pid = -1;
            strncpy(info->state, "INVALID", 15);
            continue;
        }

        // General
        strncpy(info->name, p->name, PROCESS_NAME_MAX_LENGTH - 1);
        info->name[PROCESS_NAME_MAX_LENGTH - 1] = '\0';

        info->pid = p->PID;
        info->parentPid = p->ParentPID;
        info->isForeground = p->isForeground;

        switch (p->state) {
            case RUNNING: strncpy(info->state, "RUNNING", 15); break;
            case BLOCKED: strncpy(info->state, "BLOCKED", 15); break;
            case READY:   strncpy(info->state, "READY", 15);   break;
            case ZOMBIE:  strncpy(info->state, "ZOMBIE", 15);  break;
            default:      strncpy(info->state, "UNKNOWN", 15); break;
        }

        info->state[15] = '\0';
        info->rsp = (uint64_t)p->rsp;

        // Args
        info->argc = p->argc;
        for (int j = 0; j < p->argc && j < MAX_ARGUMENTS; j++) {
            if (p->argv[j])
                strncpy(info->argv[j], p->argv[j], MAX_ARG_LENGTH - 1);
            else
                info->argv[j][0] = '\0';

            info->argv[j][MAX_ARG_LENGTH - 1] = '\0';
        }

        info->externWaitingPID = p->externWaitingPID;
        info->isWaitingForExtern = p->isWaitingForExtern;

        info->childrenAmount = p->childrenAmount;
        for (int j = 0; j < MAX_PROC; j++)
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
    for (int i = 0; i < MAX_PROC; i++) {
        if (processTable[i] != NULL && processTable[i]->state == RUNNING) {
            pid = i;
            break;
        }
    }

    return pid;
}

int wait(uint64_t pid){
    if (!isValid(pid))
        return -1;
    
    int currentPID = get_pid();
    if (!isValid(currentPID))
        return -1;

    PCB* current = processTable[currentPID];
    PCB* target = processTable[pid];

    //si el target termino primero, lo terminamos
    if (target->state == ZOMBIE) {
        return 0;
    }

    //Si el proceso que deberia esperar termino primero, hacemos el yield
    current->isWaitingForExtern = true;
    current->externWaitingPID = pid;
    block_process(currentPID);

    // yield();

    return 0;
}

int wait_for_all_children(){
    int currentPID = get_pid();
    if (!isValid(currentPID))
        return -1;

    PCB* current = processTable[currentPID];

    // Recolectamos y liberamos hijos zombie
    for (int i = 0; i < MAX_PROC; i++) {
        int childPID = current->childProc[i];
        if (childPID != -1 && processTable[childPID] != NULL) {
            PCB* child = processTable[childPID];

            if (child->state == ZOMBIE) {
                cleanup_process(childPID);
                current->childProc[i] = -1;
                current->childrenAmount--;
            }
        }
    }

    // Si no tiene hijos ya esta
    if (current->childrenAmount == 0)
        return 0;

    //Si tiene hijos, bloquear hasta que estos terminens
    block_process(currentPID);

    //yield();

    return 0;
}

//TODO: Consulta: espera activa en el wait_for_all_children

void cleanup_process(uint64_t pid) {
    if (!isValid(pid))
        return;
    
    PCB* pcb = processTable[pid];

    pcb->state = ZOMBIE;

    if (pcb->stackBase)
        free(pcb->stackBase);

    free(pcb);
    processTable[pid] = NULL;
}
