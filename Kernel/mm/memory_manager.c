#include "memory_manager.h"

//TOTAL_MEM_SIZE

//MAX_BLOCKS ??
//BLOCK_SIZE ??

#define TOTAL_MEM_SIZE (16 * 1024)  //16KB de memoria iniciales. Pure64 deja mucho mas.
#define BLOCK_SIZE (4 * 1024)       //Mínimo 4KB, una página. No usamos páginas pero es un lindo numero
#define MAX_BLOCKS (TOTAL_MEM_SIZE/BLOCK_SIZE)

#define FREE 0
#define USED 1

/**
 * Crea un bloque de memoria
 * Formato lista: conoce su tamaño y quién le sigue
 * Para fines de consulta, también conoce su estado
*/
typedef struct mem_block_cdt
{
    mem_block_adt next_addr;
    unsigned long int size; //TODO: para que...?
    int status;
} mem_block_cdt;

static mem_block_adt base_block;

/**
 * Crea la memoria a partir de una dirección inicial (base)
*/
mem_block_adt create_mm(void * const restrict base_addr){
    
    base_block = (mem_block_adt) base_addr;
    
    base_block->next_addr = NULL;
    base_block->size = BLOCK_SIZE;
    base_block->status = FREE;
    
    return base_block;
}

/**
 * Aloco un nuevo bloque
*/
void *alloc_mm(const unsigned long int size){
    //TODO: recibe un size. entonces el size es dinamico? o lo hacemos fijo para cada bloque y le asignamos una secuencia de bloques?

    //TODO: probablemente algunos casos mas
    if(size <= 0 || size >= TOTAL_MEM_SIZE || size + used_mem() >= TOTAL_MEM_SIZE){
        return NULL;
    }

    //TODO: Recorro la lista hasta encontrar uno FREE ó llegar al final de la lista
    //while();
    mem_block_adt new_block = NULL;

    new_block->next_addr += size;

	return (void *) new_block;

}

void free_mm(){

    //algoritmo eliminacion de nodo
    //mem_block_adt aux;

}

int *status_count(){
    int block_count = 0;
    int free_count = 0;
    int used_count = 0;
    mem_block_adt next_block = BASE_ADDR;    //inicio de la memoria
    while(next_block != NULL){
        block_count++;
        if(next_block->status == FREE)
            free_count++;
        else if(next_block->status == USED)
            used_count++;

        next_block = next_block->next_addr;
    }
    int status_count[3] = {block_count, free_count, used_count};
    return status_count;
}

/**
 * Cuenta la memoria asignada Y libre
*/
int used_mem(){
    size_t used_mem = 0;

    mem_block_adt current = base_block;

    while(current){
        if(current->status == USED){
            used_mem+=current->size;
        }
        current = current->next_addr;
    }

    return used_mem;
}