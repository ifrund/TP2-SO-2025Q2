#include "memory_manager.h"

#define TOTAL_MEM_SIZE (16 * 1024)  //16KB de memoria iniciales. Pure64 deja mucho mas.
#define BLOCK_SIZE (4 * 1024)       //Mínimo 4KB, una página. No usamos páginas pero es un lindo numero
#define MAX_BLOCKS (TOTAL_MEM_SIZE/BLOCK_SIZE)
#define BASE_ADDRESS 0x0000000000100000

#define FREE 0
#define USED 1

//void * base_address;

typedef struct mem_block
{
    unsigned long int start;
    unsigned long int end;
    int status;
} mem_block;

mem_block memory_array[MAX_BLOCKS];
static int mem_initialize = 0;

//Crea la memoria a partir de una dirección inicial (base)

void create_mm(){
    if (mem_initialize)
        return;

    mem_initialize = 1;
    //base_address = base_addr;
    for(int i = 0; i <  MAX_BLOCKS; i++)
        memory_array[i].status = FREE;
}

void * alloc(const unsigned long int size){ 
    int blocks = size / BLOCK_SIZE;
    int available_space = 0;

    if (!mem_initialize)
        return NULL;

    for(int i = 0; i < MAX_BLOCKS; i++){
        if(memory_array[i].status == FREE){
            for(int j = 0; j < blocks; j++){
                if (memory_array[i+j].status == USED){
                    i += j;
                    break;
                }
                if (j == blocks - 1) 
                    available_space = 1;
            }
            if (available_space == 1){
                for(int k = 0; k < blocks; k++){
                    memory_array[i+k].status = USED;
                    memory_array[i+k].start = i;
                    memory_array[i+k].end = i + blocks - 1;
                }
            }
        }
        
        return (void *)((char *)BASE_ADDRESS + i * BLOCK_SIZE);

        //return (void *)((char *)base_address + i * BLOCK_SIZE);
    }
    return NULL;
}

void free (void * address){
    
    int index = ((char *)address - (char *)BASE_ADDRESS)/BLOCK_SIZE;
    
    //int index = ((char *)address - (char *)base_address)/BLOCK_SIZE;
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
