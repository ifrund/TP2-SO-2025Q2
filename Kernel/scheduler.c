#include "include/scheduler.h"

static PCB *pcs_in_sch[MAX_PCS] = {0};
static int process_count = 0;
static int current_index = -1;
static int idle_running = 0;  //esta en 1 mientras q idle este en READY o RUNNING

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

//TODO, pensar difernetens situaciones
//el primer pcs entra al sch
//hay 3 ya corriendo
//hay 3 de diferentes prio
//el pcs pasa a estar ZOMBIE o BLOCKED
//casos con idle :(

void *scheduling(void *rsp) {

    if (current_index >= 0 && pcs_in_sch[current_index] != NULL) {
        PCB *curr = pcs_in_sch[current_index];
        curr->rsp = (uint64_t)rsp;

        if (curr->state == RUNNING){
            curr->time_used++;
            int max_time = curr->my_max_time;

            //si consumio su tiempo, resetiamos su tiempo y lo dejamos en ready 
            //esto quiere decir q si le aplicas nice a un pcs recien se va a ver el efecto una vez q consuma su tiempo 
            //y pase por get_max_time_for_priority
            if(curr->time_used >= max_time){
            
                curr->time_used = 0;                
                max_time = get_max_time_for_priority(curr->my_prio);
                curr->state = READY;
            }
            else{
                //seguimos en el mismo porq no consumio su tiempo
                return rsp;
            }
            
        }
    }

    if (process_count == 0)
        return rsp; // No hay pcs

     //si el idle está y aparece un proceso READY
    if (idle_running) {
        for (int i = 0; i < process_count; i++) {
            PCB *p = pcs_in_sch[i];
            if (p != NULL && p->state == READY && strcmp(p->name, "idle") != 0) {
                //kill_process(idle_pid); //TODO q kill te saque de la tabla de pcs_in_sch 
                idle_running = 0;
                break;
            }
        }
    }

    //solo hay un proceso
    if (process_count == 1 && pcs_in_sch[0]->state == READY) {
        pcs_in_sch[0]->state = RUNNING;
        return rsp;
    }

    //buscamos READY
    int next_index = current_index;
    int checked = 0;

    do {
        next_index = (next_index + 1) % process_count;
        checked++;
        PCB *candidate = pcs_in_sch[next_index];

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
    int idle_index = -1;

    for (int i = 0; i < process_count; i++) {
        PCB *p = pcs_in_sch[i];
        if (p != NULL && strcmp(p->name, "idle") == 0) {
            idle_pcb = p;
            idle_index = i;
            break;
        }
    }

    if(idle_pcb == NULL){
        //idle_pid = create_process(&idle, "idle", 0, argvAux)
        //TODO iniciar bien el max_time y prio del idle
    }

    //lo ponemos a correr al idle
    if (idle_pcb != NULL) {
        current_index = idle_index;
        idle_pcb->state = RUNNING;
        idle_running = 1;
        return (void *)idle_pcb->rsp;
    }

    return NULL; //KABOOM
}

void add_pcs(PCB *pcb) {
    if (process_count >= MAX_PCS)
        return;

    pcb->state = READY;
    pcs_in_sch[process_count++] = pcb;
}

// Eliminamos (marcamos) un proceso de la tabla
void delete_pcs(PCB *pcb) {
    for (int i = 0; i < process_count; i++) {
        if (pcs_in_sch[i] == pcb) {
            pcb->state = ZOMBIE;
            pcs_in_sch[i] = NULL;
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
        if (pcs_in_sch[i] && pcs_in_sch[i]->PID == pid) {
            curr = pcs_in_sch[i];
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