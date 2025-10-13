#include "memory_manager.h"

#define BLOCK_SIZE 0x1000
#define MEMORY_START (void*)0x0000000000500000
#define MEMORY_END (void*)0x0000000001500000

#define TOTAL_MEM_SIZE (unsigned int)(MEMORY_END - MEMORY_START)
#define MAX_BLOCKS (TOTAL_MEM_SIZE / BLOCK_SIZE)
#define BASE_ADDRESS (unsigned int)MEMORY_START

#define FREE 0
#define USED 1

typedef struct mem_block
{
    unsigned long int start;
    unsigned long int end;
    int status;
} mem_block;

mem_block memory_array[MAX_BLOCKS];
static int is_initialized = 0;

void create_mm(){
    if (is_initialized)
        return;

    is_initialized = 1;

    for(int i = 0; i <  MAX_BLOCKS; i++)
        memory_array[i].status = FREE;
}

/**
 * @param size amount of bytes to allocate
 */
void* alloc(const unsigned long int size){

    if (!is_initialized || size <= 0)
        return NULL;

    int blocks = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;

    if (blocks > MAX_BLOCKS)
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


void free (void* address){
    if (address == NULL)
        return;
    
    int index = ((char*)address - (char*)BASE_ADDRESS)/BLOCK_SIZE;
    int start = memory_array[index].start;
    int end = memory_array[index].end;
    for(int i = start; i <= end; i++){
        memory_array[i].status = FREE;
    }
}

void status_count(int* status_out){
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
