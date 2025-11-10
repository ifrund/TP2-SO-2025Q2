// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

// Implementation based on https://pdos.csail.mit.edu/6.828/2019/lec/malloc.c

#include <stddef.h>
#include "../include/memory_manager.h"
#include "../include/lib_buddy.h"

// Return the first power of 2 such that 2^k >= n
uint16_t first_power(uint64_t n, uint64_t min_size)
{
    uint16_t k = 0;
    uint64_t size = min_size;
    while (size < n)
    {
        k++;
        size *= 2;
    }
    return k;
}

// Compute the block index for address p at size k
uint16_t addr_to_bi(uint16_t k, char *p, void* base_addr)
{
    uint16_t n = p - (char *)base_addr;
    return n / BLK_SIZE(k);
}

// Convert a block index at size k back into an address
void *bi_to_addr(uint16_t k, uint16_t bi, void* base_addr)
{
    uint32_t n = bi * BLK_SIZE(k);
    return (char *)base_addr + n;
}

// Implementation of lists: double-linked and circular. Double-linked
// makes remove fast. Circular simplifies code, because don't have to
// check for empty list in insert and remove.

void list_init(buddy_list *list)
{
    list->next = list;
    list->prev = list;
}

void list_remove(buddy_list *e)
{
    e->prev->next = e->next;
    e->next->prev = e->prev;
}

void list_push(buddy_list *list, void *p)
{
    buddy_list *e = (buddy_list *)p;
    e->next = list->next;
    e->prev = list;
    list->next->prev = p;
    list->next = e;
}

void *list_pop(buddy_list *list)
{
    if (list->next == list)
        return NULL;
    buddy_list *p = list->next;
    list_remove(p);
    return (void *)p;
}

int list_empty(buddy_list *list)
{
    return list->next == list;
}