#include "include/scheduler.h"

#define IDLE_PID 0
int process_count = 0;
int current_index = -1;

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
                if(strcmp(curr->name, "idle") == 0){ //Si sos el idle te volvemos a bloquear, sino queda ready
                    curr->state = BLOCKED;
                }   
                else{
                    curr->state = READY;
                }           
            }
            else{
                if(curr->isYielding){
                    //aunque le queda tiempo, lo sacamos porq viene de un yield
                    curr->isYielding=0;
                    curr->time_used=0;
                    curr->state = READY; //como estaba running y metio yield, lo dejamos ready
                }
                else{
                    //seguimos en el mismo porq no consumio su tiempo ni esta en yield
                    return rsp;
                }
            }
            
        }
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
    //asiq vamos con el idle, q sabemos q siempre es el de pid 1
    
    processTable[IDLE_PID]->state = RUNNING;
    current_index = IDLE_PID; 
    return (void *)processTable[IDLE_PID]->rsp;
}

void yield(){
    //forzamos un tick y al proceso q esta forzando el tick aka cediendo el CPU
    //y activamos el flag, para que el sch sepa que lo tiene que sacar de ready
    processTable[get_pid()]->isYielding = 1;
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

Priorities int_to_priority(int n) {
    return (Priorities)n;
}

int be_nice(int pid, int new_prio){
    
    if(!is_pid_valid(pid)){
        return -1;
    }

    PCB *curr = NULL;

    for (int i = 0; i <MAX_PCS; i++) {
        if (processTable[i] && processTable[i]->PID == pid && processTable[i]->state != ZOMBIE) {
            curr = processTable[i];
            break;
        }
    }  

    if(strcmp(curr->name, "idle") == 0){ //el idle no lo podes cambiar xd
        return -2;
    }

    if(new_prio < 0 || new_prio > 4){
        return -3;
    }

    Priorities new = int_to_priority(new_prio);

    curr->my_prio = new;
    //actualizamso su max_time
    curr->my_max_time = get_max_time_for_priority(curr->my_prio);

    return 0;
}
