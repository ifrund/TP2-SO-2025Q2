#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#define NULL ((void *)0)

void create_mm();

void * alloc(const unsigned long int size);

void free(void * address);

void status_count(int *status_out);

#endif