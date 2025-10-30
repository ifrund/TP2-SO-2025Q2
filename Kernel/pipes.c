#include "pipes.h"
#include "sem.h"
#include "memory_manager.h" 
//#include "lib.h"
#include <stddef.h>

#define MAX_PIPES            16 
#define PIPE_BUFFER_SIZE     1024    // Tamaño del buffer
#define MAX_PIPE_NAME_LENGTH 64

typedef struct {
    // Informacion del pipe
    int is_named;
    char name[MAX_PIPE_NAME_LENGTH];
   
    int ref_count;

    //Datos del pipe
    char buffer[PIPE_BUFFER_SIZE];
    int read_index;                 //Indice de lectura
    int write_index;                //Indice de escritura
    int data_size;
    
    //Semaforos del pipe
    char sem_mutex_name[MAX_NAME_LENGTH];
    char sem_full_name[MAX_NAME_LENGTH];
    char sem_empty_name[MAX_NAME_LENGTH];
    
    //Para almacenamiento interno
    int is_in_use;
} Pipe;

// Tabla Global de Pipes 
Pipe global_pipe_table[MAX_PIPES] = {NULL};

void pipe_init() {
    // Inicializa el semaforo para la tabla global
    sem_open_init("GLOBAL_PIPE_TABLE_LOCK", 1);
    
    // Marca todos los slots de pipes como libres
    for (int i = 0; i < MAX_PIPES; i++) {
        global_pipe_table[i].is_in_use = 0;
    }
}

int pipe_close(int pipe_id) {
    if (pipe_id < 0 || pipe_id >= MAX_PIPES) {
        return -1; // PipeID invalido
    }

    sem_wait("GLOBAL_PIPE_TABLE_LOCK");

    if (!global_pipe_table[pipe_id].is_in_use) {
        sem_post("GLOBAL_PIPE_TABLE_LOCK");
        return -1; // Pipe ya cerrado o invalido
    }

    Pipe* pipe = &global_pipe_table[pipe_id];

    // Disminuir el contador de referencias
    pipe->ref_count--;

    //Si no hay mas referencias, liberar recursos
    if (pipe->ref_count == 0) {
        // Liberar recursos asociados al pipe
        sem_close(pipe->sem_mutex_name);
        sem_close(pipe->sem_full_name);
        sem_close(pipe->sem_empty_name);
        pipe->is_in_use = 0; // Marca el slot como libre
    } 

    sem_post("GLOBAL_PIPE_TABLE_LOCK");

    return 0;
}