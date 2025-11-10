// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "../include/memory_manager.h"

#define BLOCK_SIZE 0x1000                       // 4KB, igual que una página
#define MEMORY_START (void *)0x0000000000500000 // Dirección del primer bloque. Todo el espacio desde ~0x40 0000 esta disponible
#define MEMORY_END (void *)0x0000000040000000   // No es una dirección válida. Pure64 es 64 bits, podría usarse más memoria.

#define TOTAL_BLOCK_COUNT ((uint32_t)(MEMORY_END - MEMORY_START) / BLOCK_SIZE)

#define _MEMORY_START 0x0000000000500000ULL
#define _MEMORY_END 0x0000000040000000ULL

#define _TOTAL_MEMORY (_MEMORY_END - _MEMORY_START)
#define _TOTAL_BLOCK_COUNT (_TOTAL_MEMORY / BLOCK_SIZE)

#define FREE 0
#define USED 1

typedef struct block_info
{
    uint8_t status;
    uint32_t contiguous_blocks; // Cantidad de bloques contiguos, si se pide más memoria que BLOCK_SIZE. Luego deben liberarse todos juntos.
} block_info;

block_info block_array[_TOTAL_BLOCK_COUNT];
static bool is_initialized = false;

uint32_t first_free_index = 0;
uint32_t free_blocks;

/**
 * Reubica el índice del primer bloque libre tras una operación que podría haberlo cambiado
 */
void reset_first_free_index()
{
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

/**
 * Inicializa la memoria a administrar y las estructuras de datos del alocador
 */
void create_mm()
{

    if (is_initialized)
        return;

    is_initialized = true;

    for (uint32_t i = 0; i < TOTAL_BLOCK_COUNT; i++)
    {
        block_array[i].status = FREE;
        block_array[i].contiguous_blocks = 0;
    }

    free_blocks = TOTAL_BLOCK_COUNT;
}

/**
 * Aloca una cantidad de bytes en memoria
 * @param size cantidad de bytes a alocar
 */
void *alloc(const uint64_t size)
{

    if (size <= 0)
        return NULL;

    uint32_t blocks_to_alloc = (uint32_t)((size + BLOCK_SIZE - 1) / BLOCK_SIZE); // La informacion de los bloques se guarda en el espacio de kernel. Si se decidiera implementar headers que estén en los mismos bloques, deberia restarse su tamaño a BLOCK_SIZE (es decir, BLOCK_SIZE - sizeof(header))

    if (!is_initialized || blocks_to_alloc > free_blocks)
        return NULL;

    bool found = false;
    for (uint32_t found_index = first_free_index; found_index <= TOTAL_BLOCK_COUNT - blocks_to_alloc; found_index++)
    {

        if (block_array[found_index].status == FREE)
        {
            found = true;
            for (uint32_t contiguous_index = 1; contiguous_index < blocks_to_alloc && found; contiguous_index++)
            {
                if (block_array[found_index + contiguous_index].status != FREE)
                {
                    found = false;
                    found_index += contiguous_index;
                }
            }
        }

        if (found)
        {
            block_array[found_index].status = USED;
            block_array[found_index].contiguous_blocks = blocks_to_alloc; // Solo lo guardo en el primero pues solo voy a usar su address. No voy a permitir liberar un bloque que no sea la cabeza de su "lista"

            for (uint32_t j = 1; j < blocks_to_alloc; j++)
            {
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

/**
 * Libera la memoria alocada en la dirección p
 * @param address puntero de la memoria a liberar
 */
void free(void *address)
{
    if (address == NULL)
        return;

    uint32_t index = (uint32_t)(address - MEMORY_START) / BLOCK_SIZE;
    uint32_t blocks_to_free = block_array[index].contiguous_blocks;

    if (blocks_to_free == 0)
        return;

    for (uint32_t i = index; i < blocks_to_free + index; i++)
    {
        block_array[i].status = FREE;
        block_array[i].contiguous_blocks = 0;
    }

    reset_first_free_index();

    free_blocks += blocks_to_free;
}

/**
 * Almacena el estado de la memoria en el arreglo recibido
 * @param status_out arreglo de 6 enteros
 */
void status_count(uint32_t *status_out)
{
    status_out[0] = _TOTAL_MEMORY;
    status_out[1] = _TOTAL_MEMORY - (free_blocks * BLOCK_SIZE);
    status_out[2] = (free_blocks * BLOCK_SIZE);
    status_out[3] = BLOCK_SIZE;
    status_out[4] = TOTAL_BLOCK_COUNT;
    status_out[5] = TOTAL_BLOCK_COUNT - free_blocks;
}