#include "include/memory_manager.h"

#define BLOCK_SIZE 0x1000                       // 4KB, igual que una página
#define MEMORY_START (void*)0x0000000000500000  // Dirección del primer bloque. Todo el espacio desde ~0x40 0000 esta disponible
#define MEMORY_END (void*)0x0000000040000000    // No es una dirección válida. Pure64 es 64 bits, podría usarse más memoria.

#define TOTAL_BLOCK_COUNT ((uint32_t)(MEMORY_END - MEMORY_START) / BLOCK_SIZE)

#define _MEMORY_START 0x0000000000500000ULL
#define _MEMORY_END   0x0000000040000000ULL
#define _BLOCK_SIZE   0x1000

#define _TOTAL_BLOCK_COUNT ((_MEMORY_END - _MEMORY_START) / _BLOCK_SIZE)


#define FREE 0
#define USED 1

typedef struct block_info
{
    uint8_t status;
    uint32_t contiguous_blocks; // Cantidad de bloques contiguos, si se pide más memoria que BLOCK_SIZE. Luego deben liberarse todos juntos.
} block_info;

block_info block_array[_TOTAL_BLOCK_COUNT];  // TODO: tira warning porque void* es system-dependent.
static bool is_initialized = false;

uint32_t first_free_index = 0;
uint32_t free_blocks;

void reset_first_free_index();

void create_mm(){

    if (is_initialized)
        return;

    is_initialized = true;

    for(uint32_t i = 0; i < TOTAL_BLOCK_COUNT; i++) {
        block_array[i].status = FREE;
        block_array[i].contiguous_blocks = 0;
    }

    free_blocks = TOTAL_BLOCK_COUNT;
}

/**
 * @param size amount of bytes to allocate
 */
void* alloc(const uint64_t size){

    if(size <= 0)
        return NULL;
    
    uint32_t blocks_to_alloc = (uint32_t)((size + BLOCK_SIZE - 1) / BLOCK_SIZE);  // La información de los bloques se guarda en el espacio de kernel. Si se decidiera implementar headers que estén en los mismos bloques, deberia restarse su tamaño a BLOCK_SIZE (es decir, BLOCK_SIZE - sizeof(header))

    if (!is_initialized || blocks_to_alloc > free_blocks)
        return NULL;
    
    bool found = false;
    for (uint32_t found_index = first_free_index; found_index <= TOTAL_BLOCK_COUNT - blocks_to_alloc; found_index++) {

        if(block_array[found_index].status == FREE) {
            found = true;
            for (uint32_t contiguous_index = 1; contiguous_index < blocks_to_alloc && found; contiguous_index++) {
                if(block_array[found_index + contiguous_index].status != FREE) {
                    found = false;
                    found_index += contiguous_index;
                }
            }
        }

        if(found) {
            block_array[found_index].status = USED;
            block_array[found_index].contiguous_blocks = blocks_to_alloc;   // Solo lo guardo en el primero pues solo voy a usar su address. No voy a permitir liberar un bloque que no sea la cabeza de su "lista"

            for(uint32_t j = 1; j < blocks_to_alloc; j++) {
                block_array[found_index + j].status = USED;
            }

            if (block_array[first_free_index].status != FREE)
            {
                reset_first_free_index();
            }

            free_blocks -= blocks_to_alloc;

            return MEMORY_START + (found_index * BLOCK_SIZE);
        }
    }

    return NULL;
}

void free (void * address){
    if (address == NULL)
        return;
    
    uint32_t index = (uint32_t)(address - MEMORY_START) / BLOCK_SIZE;
    uint32_t blocks_to_free = block_array[index].contiguous_blocks;

    if (blocks_to_free == 0)
        return;

    for(uint32_t i = index; i < blocks_to_free + index; i++) {
        block_array[i].status = FREE;
        block_array[i].contiguous_blocks = 0;
    }
    
    reset_first_free_index();
    
    free_blocks += blocks_to_free;
}

void status_count(uint32_t *status_out){
    uint32_t block_count = TOTAL_BLOCK_COUNT;

    status_out[0] = block_count;
    status_out[1] = block_count - free_blocks;
    status_out[2] = free_blocks;
}

void reset_first_free_index() {
    bool found = false;
    for (uint32_t i = 0; i < TOTAL_BLOCK_COUNT && !found; i++)
        {
            if (block_array[i].status == FREE)
            {
                first_free_index = i;
                found = true;
            }
        }
}