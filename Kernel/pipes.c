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

// --- Tabla Global de Pipes ---
Pipe global_pipe_table[MAX_PIPES];