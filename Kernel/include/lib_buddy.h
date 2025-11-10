#ifndef LIB_BUDDY_H
#define LIB_BUDDY_H

#include <stdint.h>

#define LEAF_SIZE 16 // The smallest allocation size (in bytes)
/*
    The number of "size classes", or the max power of 2 x LEAF_SIZE.
    ARENA size =
    13 => 64k (2**12 * 16)
    27 => 1G (2**26 * 2**4 = 2**27-1 * 16 = 2**NSIZES-1 * LEAF_SIZE)
*/
#define NSIZES 26                             // Number of entries in bd_sizes array
#define MAXSIZE (NSIZES - 1)                  // Largest index in bd_sizes array
#define BLK_SIZE(k) ((1L << (k)) * LEAF_SIZE) // Size in bytes for size k
#define HEAP_SIZE BLK_SIZE(MAXSIZE)
#define NBLK(k) (1 << (MAXSIZE - (k)))                         // Number of block at size k
#define ROUNDUP(n, size) (((((n) - 1) / (size)) + 1) * (size)) // Round up to the next multiple of sz

#define LINEAR_ALLOCATOR_START (void *)(0x0000000000500000)

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

uint16_t first_power(uint64_t n, uint64_t min_size);
uint16_t addr_to_bi(uint16_t k, char *p, void* base_addr);
void *bi_to_addr(uint16_t k, uint16_t bi, void* base_addr);

// List functions that the buddy allocator uses. Implementations
// are at the end of the buddy allocator code.
void list_init(buddy_list *list);
void list_remove(buddy_list *e);
void list_push(buddy_list *list, void *p);
void *list_pop(buddy_list *list);
int list_empty(buddy_list *list);

#endif