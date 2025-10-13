#include "include/scheduler.h"

static PCB *process_table[MAX_PCS] = {0};
static int process_count = 0;
static int current_index = -1;

//TODO agregarle tiempos a los pcs y prioritys

void *scheduling(void *rsp) {
    if (current_index >= 0 && process_table[current_index] != NULL) {
        PCB *curr = process_table[current_index];
        curr->rsp = (uint64_t)rsp;

        if (curr->state == RUNNING)
            curr->state = READY;
    }

    if (process_count == 0)
        return rsp; // No hay pcs

    // Buscamos READY
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

    // No encontramos, seguimos en el actual
    return rsp;
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
