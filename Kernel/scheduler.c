#include "include/scheduler.h"

static PCB *process_table[MAX_PCS] = {0};
static int process_count = 0;
static int current_index = -1;

void init_sch(){
    //mmm nose si hace falta, creo q no
    //create_pcs(idle);
}

static int get_max_time_for_priority(Priorities p) {
    switch (p) {
        case LEVEL_0: return QUANTUM * 5; // 25
        case LEVEL_1: return QUANTUM * 4; // 20
        case LEVEL_2: return QUANTUM * 3; // 15
        case LEVEL_3: return QUANTUM * 2; // 10
        case LEVEL_4: return QUANTUM * 1; // 5
        default: return QUANTUM; //5
    }
}

//TODO, pensar difernetens situaciones
//el primer pcs entra al sch
//hay 3 ya corriendo
//hay 3 de diferentes prio
//el pcs pasa a estar ZOMBIE o BLOCKED

void *scheduling(void *rsp) {

    if (current_index >= 0 && process_table[current_index] != NULL) {
        PCB *curr = process_table[current_index];
        curr->rsp = (uint64_t)rsp;

        if (curr->state == RUNNING){
            curr->time_used++;
            int max_time = curr->my_max_time;

            //si consumio su tiempo, resetiamos su tiempo, actualizamos su prio, actualizamos el tiempo y lo dejamos en ready 
            //esto quiere decir q si le aplicas nice a un pcs recien se va a ver el efecto una vez q salga del sch y vuelva
            if(curr->time_used >= max_time){
            
                curr->time_used = 0;
                //TODO nose si el sch tiene q subir la prio
                if (curr->my_prio > LEVEL_0)
                    curr->my_prio--;
                
                max_time = get_max_time_for_priority(curr->my_prio);
                curr->state = READY;
            }
            else{
                //seguimos en el mismo
                return rsp;
            }
            
        }
    }

    if (process_count == 0)
        return rsp; // No hay pcs

    //solo hay un proceso
    if (process_count == 1 && process_table[0]->state == READY) {
        process_table[0]->state = RUNNING;
        return rsp;
    }

    //buscamos READY
    int next_index = current_index;
    int checked = 0;

    do {
        next_index = (next_index + 1) % process_count;
        checked++;
        PCB *candidate = process_table[next_index];

        if (candidate != NULL && candidate->state == READY) {
            current_index = next_index;
            candidate->state = RUNNING;
            return (void *)candidate->rsp;
        }
    } while (checked <= process_count);

    //TODO si llegamos acá no hay nignun pcs en ready, creo q se deberia usar idle 
}

void add_pcs(PCB *pcb) {
    if (process_count >= MAX_PCS)
        return;

    pcb->state = READY;
    process_table[process_count++] = pcb;
}

// Eliminamos (marcamos) un proceso de la tabla
void delete_pcs(PCB *pcb) {
    for (int i = 0; i < process_count; i++) {
        if (process_table[i] == pcb) {
            pcb->state = ZOMBIE;
            process_table[i] = NULL;
            return;
        }
    }
}

void yield(){
    _yield();
}

int be_nice(int pid){
    
    PCB *curr = NULL;

    for (int i = 0; i < MAX_PCS; i++) {
        if (process_table[i] && process_table[i]->PID == pid) {
            curr = process_table[i];
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

    return 0;
}

//proceso basura cuando no hay ninguno ready, llama constantemente a halt, osea al scheduler, osea a q cambie al proximo
static void idle() {
    while(1) {
        _hlt();
    }
}