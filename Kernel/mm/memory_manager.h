#ifndef MEMORY_MANAGER.H
#define MEMORY_MANAGER.H

#include <stddef.h>
//#include <stdint.h>

#define BASE_ADDR 0 //??

typedef struct mem_block_cdt* mem_block_adt;

mem_block_adt create_memory_manager();

void alloc_memory_manager();

void free_memory_manager();

int* status_count();

#endif