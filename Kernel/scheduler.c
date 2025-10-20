#include "include/scheduler.h"

int process_count = 0;
int current_index = -1;
int idle_pid = -1;
int idle_running = 0;  //esta en 1 mientras q idle este en READY o RUNNING
int yielding =0;

//proceso basura cuando no hay ninguno ready, llama constantemente a halt, osea al scheduler, osea a q cambie al proximo
static void idle() {
    while(1) {
        _hlt();
    }
}

int find_index_by_pid(int pid) {
    for (int i = 0; i < process_count; i++) {
        if (processTable[i] != NULL && processTable[i]->PID == pid)
            return i;
    }
    return -1;  
}

//TODO, pensar difernetens situaciones
//el primer pcs entra al sch
//hay 3 ya corriendo
//hay 3 de diferentes prio
//el pcs pasa a estar ZOMBIE o BLOCKED
//casos con idle :(

void *scheduling(void *rsp) {

    if (process_count == 0)
        return rsp; // No hay pcs

    if (current_index >= 0 && processTable[current_index] != NULL) {
        PCB *curr = processTable[current_index];
        curr->rsp = rsp;

        if (curr->state == RUNNING){
            curr->time_used++;

            //si consumio su tiempo, resetiamos su tiempo y lo dejamos en ready 
            //esto quiere decir q si le aplicas nice a un pcs recien se va a ver el efecto una vez q consuma su tiempo 
            //y pase por get_max_time_for_priority
            if(curr->time_used >= curr->my_max_time){
                curr->time_used = 0;                
                curr->state = READY;
            }
            else{
                if(yielding){
                    //aunque le queda tiempo, lo sacamos porq viene de un yield
                    yielding=0;
                    curr->state = READY;
                }

                //seguimos en el mismo porq no consumio su tiempo
                return rsp;
            }
            
        }
    }

     //si el idle está y aparece un proceso READY
    if (idle_running) {
        for (int i = 0; i < process_count; i++) {
            PCB *p = processTable[i];
            if (p != NULL && p->state == READY && strcmp(p->name, "idle") != 0) {
                kill_process(idle_pid);
                idle_running = 0;
                break;
            }
        }
    }

    //solo hay un proceso
    if (process_count == 1 && processTable[0]->state == READY) {
        processTable[0]->state = RUNNING;
        return rsp;
    }

    //buscamos READY
    int next_index = current_index;
    int checked = 0;

    do {
        next_index = (next_index + 1) % process_count;
        checked++;
        PCB *candidate = processTable[next_index];

        if (candidate != NULL && candidate->state == READY) {
            current_index = next_index;
            candidate->state = RUNNING;
            return (void *)candidate->rsp;
        }
    } while (checked < process_count);

    //como no hay ningun proceso en ready, tenemos q dejar algo corriendo en el sch
    //asiq vamos con el idle
    
    char *argvAux[] = {0};

    //si no existe idle, lo creamos
    PCB *idle_pcb = NULL;

    for (int i = 0; i < process_count; i++) {
        PCB *p = processTable[i];
        if (p != NULL && strcmp(p->name, "idle") == 0) {
            idle_pcb = p;
            idle_pid = p->PID;
            break;
        }
    }

    if(idle_pcb == NULL){
        idle_pid = create_process(&idle, "idle", 0, argvAux);
        for (int i = 0; i < process_count; i++) {
            PCB *p1 = processTable[i];
            if (p1 != NULL && strcmp(p1->name, "idle") == 0) {
                idle_pcb = p1;
                break;
            }
        }
    }

    //lo ponemos a correr al idle
    if (idle_pcb != NULL) { //TODO en teoria este if siempre entra
        current_index = find_index_by_pid(idle_pid);
        idle_pcb->state = RUNNING;
        idle_running = 1;
        return (void *)idle_pcb->rsp;
    }

    return NULL; //KABOOM
}

void yield(){
    //forzamos un tick y al proceso q esta forzando el tick aka cediendo el CPU
    //y activamos el flag, para que el sch sepa que lo tiene que sacar de ready
    yielding = 1;
    _yield();
}

static int get_max_time_for_priority(Priorities p) {
    switch (p) {
        case LEVEL_0: return QUANTUM * 5; // 25
        case LEVEL_1: return QUANTUM * 4; // 20
        case LEVEL_2: return QUANTUM * 3; // 15
        case LEVEL_3: return QUANTUM * 2; // 10
        case LEVEL_4: return QUANTUM * 1; // 5
        case LEVEL_IDLE: return 1;
        default: return QUANTUM; //5
    }
} 

int be_nice(int pid){
    
    PCB *curr = NULL;

    for (int i = 0; i <MAX_PCS; i++) {
        if (processTable[i] && processTable[i]->PID == pid && processTable[i]->state != KILLED) {
            curr = processTable[i];
            break;
        }
    }

    if (curr == NULL) {
        return -1; //Este pid no esta en el sch
    }    

    if (curr->my_prio > LEVEL_0){
        curr->my_prio--; //Mejoramos su prio 
    }
    else{
        return -2; //Ya esta en el max de prio
    }

    curr->my_max_time = get_max_time_for_priority(curr->my_prio);

    return 0;
}
