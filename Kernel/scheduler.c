#include "include/scheduler.h"

int active_processes = 0; //procesos q no estan ZOMBIE
int current_index = -1;

void *scheduling(void *rsp) {

    if (active_processes == 0)
        return rsp; // No hay pcs

    if (current_index >= 0 && process_table[current_index] != NULL) {
        PCB *curr = process_table[current_index];
        curr->rsp = rsp;

        if (curr->state == RUNNING){
            curr->time_used++;
            curr->total_ticks++;

            //si consumio su tiempo, resetiamos su tiempo y lo dejamos en ready 
            //esto quiere decir q si le aplicas nice a un pcs recien se va a ver el efecto una vez q consuma su tiempo 
            //y pase por get_max_time_for_priority
            if(curr->time_used >= curr->my_max_time){
                curr->time_used = 0;
                curr->changes++;
                if(strcmp(curr->name, "idle") == 0){ //Si sos el idle te volvemos a bloquear, sino queda ready
                    curr->state = BLOCKED;
                }   
                else{
                    curr->state = READY;
                }           
            }
            else{
                if(curr->yielding){
                    //aunque le queda tiempo, lo sacamos porq viene de un yield
                    curr->yielding=0;
                    curr->time_used=0;
                    curr->yield_changes++;
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
        next_index = (next_index + 1) % MAX_PCS;
        checked++;
        PCB *candidate = process_table[next_index];

        if (candidate != NULL && candidate->state == READY) {
            current_index = next_index;
            candidate->state = RUNNING;
            return (void *)candidate->rsp;
        }
    } while (checked < MAX_PCS); 

    //como no hay ningun proceso en ready, tenemos q dejar algo corriendo en el sch
    //asiq vamos con el idle, q sabemos q siempre es el de pid 1
    process_table[IDLE_PID]->state = RUNNING;
    current_index = IDLE_PID; 
    return (void *)process_table[IDLE_PID]->rsp;
}

void yield(){
    //forzamos un tick y al proceso q esta forzando el tick aka cediendo el CPU
    //y activamos el flag, para que el sch sepa que lo tiene que sacar de ready
    process_table[get_pid()]->yielding = 1;
    _yield();
}

void last_wish(int pid){ //no se puede usar yield al final de kill, porq el get_pid devuelve -1 ya q el proceos esta ZOMBIE y no RUNNING
    process_table[pid]->yielding = 1;
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
        if (process_table[i] && process_table[i]->PID == pid && process_table[i]->state != ZOMBIE) {
            curr = process_table[i];
            break;
        }
    }  

    if(strcmp(curr->name, "idle") == 0){ //el idle no lo podes cambiar
        return -2;
    }

    if(new_prio < 0 || new_prio > 4){
        return -3;
    }

    Priorities new = int_to_priority(new_prio);

    curr->my_prio = new;
    //actualizamos su max_time
    curr->my_max_time = get_max_time_for_priority(curr->my_prio);

    return 0;
}
