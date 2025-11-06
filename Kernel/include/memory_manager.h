#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

#define NULL ((void *)0)

void create_mm();

void * alloc(const uint64_t size);

void free(void * address);

void status_count(uint32_t *status_out);

#endif