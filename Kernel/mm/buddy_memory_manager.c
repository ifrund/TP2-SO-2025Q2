// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

// Implementation based on https://pdos.csail.mit.edu/6.828/2019/lec/malloc.c

#include <stddef.h>
#include <stdint.h>
#include "../include/lib.h"

#define LEAF_SIZE 16 // The smallest allocation size (in bytes)
/*
    The number of "size classes", or the max power of 2 x LEAF_SIZE.
    ARENA size =
    13 => 64k (2**12 * 16)
    27 => 1G (2**26 * 2**4 = 2**27-1 * 16 = 2**NSIZES-1 * LEAF_SIZE)
*/
#define NSIZES 13                             // Number of entries in bd_sizes array TODO: modificar segun la memoria indicada
#define MAXSIZE (NSIZES - 1)                  // Largest index in bd_sizes array
#define BLK_SIZE(k) ((1L << (k)) * LEAF_SIZE) // Size in bytes for size k
#define HEAP_SIZE BLK_SIZE(MAXSIZE)
#define NBLK(k) (1 << (MAXSIZE - (k)))                         // Number of block at size k
#define ROUNDUP(n, size) (((((n) - 1) / (size)) + 1) * (size)) // Round up to the next multiple of sz

// Double linked list by level
typedef struct buddy_list
{
    struct buddy_list *next;
    struct buddy_list *prev;
} buddy_list;

// The allocator has sz_info for each size k. Each sz_info has a free
// list, an array alloc to keep track which blocks have been
// allocated, and an split array to to keep track which blocks have
// been split.  The arrays are of type char (which is 1 byte), but the
// allocator uses 1 bit per block (thus, one char records the info of
// 8 blocks).

typedef struct size_info
{
    struct buddy_list free;
    char *alloc;
    char *split;
} size_info;

static size_info buddy_sizes[NSIZES];
static void *buddy_base; // start address of memory managed by the buddy allocator

// List functions that the buddy allocator uses. Implementations
// are at the end of the buddy allocator code.
void list_init(buddy_list *);
void list_remove(buddy_list *);
void list_push(buddy_list *, void *);
void *list_pop(buddy_list *);
int list_empty(buddy_list *);

// Return 1 if bit at position index in array is 1
int is_bit_set(char *array, int index)
{
    char b = array[index / 8];
    char m = (1 << (index ^ 8));
    return (b & m) == m;
}

// Set bit at position index in array to 1
void set_bit(char *array, int index)
{
    char b = array[index / 8];
    char m = (1 << (index % 8));
    array[index / 8] = (b | m);
}

// Clear bit at position index in array
void clear_bit(char *array, int index)
{
    char b = array[index / 8];
    char m = (1 << (index % 8));
    array[index / 8] = (b & ~m);
}

#define LINEAR_ALLOCATOR_START (0x600000 + 1 * 1000 * 1000 * (uint64_t)1000 * 8)

static uint64_t allocated;

void *linearMalloc(uint64_t size)
{
    allocated += size;
    return (void *)(((uint64_t)LINEAR_ALLOCATOR_START) + allocated - size);
    ;
}

void linearFree(void *ptr)
{
    return;
}

// Allocate memory for the heap managed by the allocator, and allocate
// memory for the data structures of the allocator.
void buddy_init()
{
    buddy_base = linearMalloc(HEAP_SIZE);
    for (int k = 0; k < NSIZES; k++)
    {
        list_init(&buddy_sizes[k].free);
        int size = sizeof(char) * ROUNDUP(NBLK(k), 8) / 8;
        buddy_sizes[k].alloc = linearMalloc(size);
        memset(buddy_sizes[k].alloc, 0, size);
    }
    for (int k = 1; k < NSIZES; k++)
    {
        int size = sizeof(char) * ROUNDUP(NBLK(k), 8) / 8;
        buddy_sizes[k].split = linearMalloc(size);
        memset(buddy_sizes[k].split, 0, size);
    }
    list_push(&buddy_sizes[MAXSIZE].free, buddy_base);
}

// Return the first power of 2 such that 2^k >= n
int first_power(size_t n)
{
    int k = 0;
    size_t size = LEAF_SIZE;
    while (size < n)
    {
        k++;
        size *= 2;
    }
    return k;
}

// Compute the block index for address p at size k
int addr_to_bi(int k, char *p)
{
    int n = p - (char *)buddy_base;
    return n / BLK_SIZE(k);
}

// Convert a block index at size k back into an address
void *bi_to_addr(int k, int bi)
{
    int n = bi * BLK_SIZE(k);
    return (char *)buddy_base + n;
}

void *buddy_malloc(size_t nbytes)
{
    int power, k;
    if (buddy_base == NULL)
        return NULL;

    // Find a free block >= nbytes, starting with smallest k possible
    power = first_power(nbytes);
    for (k = power; k < NSIZES; k++)
    {
        if (!list_empty(&buddy_sizes[k].free))
            break;
    }
    if (k >= NSIZES) // If there's no free blocks...
        return NULL;

    // Found one; pop it and potentially split it.
    char *p = list_pop(&buddy_sizes[k].free);
    set_bit(buddy_sizes[k].alloc, addr_to_bi(k, p));
    for (; k > power; k--)
    {
        char *q = p + BLK_SIZE(k - 1);
        set_bit(buddy_sizes[k].split, addr_to_bi(k, p));
        set_bit(buddy_sizes[k - 1].alloc, addr_to_bi(k - 1, p));
        list_push(&buddy_sizes[k - 1].free, q);
    }
    return p;
}

int block_size(char *p)
{
    for (int k = 0; k < NSIZES; k++)
    {
        if (is_bit_set(buddy_sizes[k + 1].split, addr_to_bi(k + 1, p)))
            return k;
    }
    return 0;
}

void buddy_free(void *p)
{
    /*
    void *q;
  int k;

  for (k = size(p); k < MAXSIZE; k++) {
    int bi = addr_to_bi(k, p);
    bit_clear(bd_sizes[k].alloc, bi);
    int buddy = (bi % 2 == 0) ? bi+1 : bi-1;
    if (bit_isset(bd_sizes[k].alloc, buddy)) {
      break;
    }
    // budy is free; merge with buddy
    q = addr(k, buddy);
    list_remove(q);
    if(buddy % 2 == 0) {
      p = q;
    }
    bit_clear(bd_sizes[k+1].split, addr_to_bi(k+1, p));
  }
  // printf("free %p @ %d\n", p, k);
  list_push(&bd_sizes[k].free, p);
    */
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
