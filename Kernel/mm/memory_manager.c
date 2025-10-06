#include "memory_manager.h"

#define TOTAL_MEM_SIZE (1024 * 1024)  //(16 * 1024) 1M de memoria iniciales.
#define BLOCK_SIZE (4 * 1024)       //Mínimo 4KB, una página. No usamos páginas pero es un lindo numero
#define MAX_BLOCKS (TOTAL_MEM_SIZE/BLOCK_SIZE) //256
#define BASE_ADDRESS 0x0000000000100000  //TODO ver repo de la catedra para saber donde esta el kernel. buildin.md

#define FREE 0
#define USED 1

typedef struct mem_block
{
    unsigned long int start;
    unsigned long int end;
    int status;
} mem_block;

mem_block memory_array[MAX_BLOCKS];
static int mem_initialize = 0;

void create_mm(){
    if (mem_initialize)
        return;

    mem_initialize = 1;
    for(int i = 0; i <  MAX_BLOCKS; i++)
        memory_array[i].status = FREE;
}

void * alloc(const unsigned long int size){ 
    int blocks = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;

    if (!mem_initialize || blocks > MAX_BLOCKS)
        return NULL;

    for (int i = 0; i <= MAX_BLOCKS - blocks; i++) {
        int found = 1;

        for (int j = 0; j < blocks; j++) {
            if (memory_array[i + j].status == USED) {
                found = 0;
                i += j; 
                break;
            }
        }

        if (found) {
            for (int k = 0; k < blocks; k++) {
                memory_array[i + k].status = USED;
                memory_array[i + k].start = i;
                memory_array[i + k].end = i + blocks - 1;
            }

            return (void *)((char *)BASE_ADDRESS + i * BLOCK_SIZE);
        }
    }
    return NULL;
}


void free (void * address){
    if (address == NULL)
        return;
    
    int index = ((char *)address - (char *)BASE_ADDRESS)/BLOCK_SIZE;
        int start = memory_array[index].start;
    int end = memory_array[index].end;
    for(int i = start; i <= end; i++){
        memory_array[i].status = FREE;
    }
}

void status_count(int *status_out){
    int block_count = MAX_BLOCKS;
    int used_count = 0;

    for(int i = 0; i < MAX_BLOCKS; i++){
        if(memory_array[i].status == USED)
            used_count++;      
    }

    status_out[0] = block_count;
    status_out[1] = block_count - used_count;
    status_out[2] = used_count;
}
