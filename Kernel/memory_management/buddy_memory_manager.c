// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

// Implementation based on https://pdos.csail.mit.edu/6.828/2019/lec/malloc.c

#include <stddef.h>
#include "../include/lib.h"
#include "../include/memory_manager.h"
#include "../include/lib_bit.h"
#include "../include/lib_buddy.h"

static size_info buddy_sizes[NSIZES];
static void *buddy_base; // start address of memory managed by the buddy allocator
static uint64_t allocated;

uint32_t block_size(char *p)
{
    for (uint16_t k = 0; k < NSIZES; k++)
    {
        if (is_bit_set(buddy_sizes[k + 1].split, addr_to_bi(k + 1, p, buddy_base)))
            return k;
    }
    return 0;
}

/**
 * Aloca un tamaño fijo de bytes
 * @param size bytes a alocar
 */
void *linearMalloc(uint64_t size)
{
    allocated += size;
    return (void *)(((uint64_t)LINEAR_ALLOCATOR_START) + allocated - size);
}

/**
 * Inicializa la memoria a administrar y las estructuras de datos del alocador
 */
void create_mm()
{
    buddy_base = linearMalloc(HEAP_SIZE);
    for (uint16_t k = 0; k < NSIZES; k++)
    {
        list_init(&buddy_sizes[k].free);
        uint32_t size = sizeof(char) * ROUNDUP(NBLK(k), 8) / 8;
        buddy_sizes[k].alloc = linearMalloc(size);
        memset(buddy_sizes[k].alloc, 0, size);
    }
    for (uint16_t k = 1; k < NSIZES; k++)
    {
        uint32_t size = sizeof(char) * ROUNDUP(NBLK(k), 8) / 8;
        buddy_sizes[k].split = linearMalloc(size);
        memset(buddy_sizes[k].split, 0, size);
    }
    list_push(&buddy_sizes[MAXSIZE].free, buddy_base);
}

/**
 * Aloca una cantidad de bytes en memoria
 * @param nbytes cantidad de bytes a alocar
 */
void *alloc(const uint64_t nbytes)
{
    uint16_t power, k;
    if (buddy_base == NULL)
        return NULL;

    // Find a free block >= nbytes, starting with smallest k possible
    power = first_power(nbytes, LEAF_SIZE);
    for (k = power; k < NSIZES; k++)
    {
        if (!list_empty(&buddy_sizes[k].free))
            break;
    }
    if (k >= NSIZES) // If there's no free blocks...
        return NULL;

    // Found one; pop it and potentially split it.
    char *p = list_pop(&buddy_sizes[k].free);
    set_bit(buddy_sizes[k].alloc, addr_to_bi(k, p, buddy_base));
    for (; k > power; k--)
    {
        char *q = p + BLK_SIZE(k - 1);
        set_bit(buddy_sizes[k].split, addr_to_bi(k, p, buddy_base));
        set_bit(buddy_sizes[k - 1].alloc, addr_to_bi(k - 1, p, buddy_base));
        list_push(&buddy_sizes[k - 1].free, q);
    }
    return p;
}

/**
 * Libera la memoria alocada en la dirección p
 * @param p puntero de la memoria a liberar
 */
void free(void *p)
{

    void *q;
    uint16_t k;

    for (k = block_size(p); k < MAXSIZE; k++)
    {
        uint16_t bi = addr_to_bi(k, p, buddy_base);
        clear_bit(buddy_sizes[k].alloc, bi);
        uint16_t buddy = (bi % 2 == 0) ? bi + 1 : bi - 1;
        if (is_bit_set(buddy_sizes[k].alloc, buddy))
        {
            break;
        }
        q = bi_to_addr(k, buddy, buddy_base);
        list_remove(q);
        if (buddy % 2 == 0)
        {
            p = q;
        }
        clear_bit(buddy_sizes[k + 1].split, addr_to_bi(k + 1, p, buddy_base));
    }
    list_push(&buddy_sizes[k].free, p);
}

/**
 * Almacena el estado de la memoria en el arreglo recibido
 * @param status_out arreglo de 6 enteros
 */
void status_count(uint32_t *status_out)
{
    uint8_t busy = 0;
    for (uint32_t i = 0; i < MAXSIZE * NSIZES; i++)
        if (is_bit_set(buddy_sizes[MAXSIZE].alloc, i))
            busy += LEAF_SIZE;
    status_out[0] = HEAP_SIZE;
    status_out[1] = busy * LEAF_SIZE;
    status_out[2] = HEAP_SIZE - (busy * LEAF_SIZE);
    status_out[3] = LEAF_SIZE;
    status_out[4] = HEAP_SIZE / LEAF_SIZE;
    status_out[5] = busy;
}