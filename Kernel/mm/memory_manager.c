#include "memory_manager.h"

#define BLOCK_SIZE 0x1000                       // 4KB, igual que una página
#define MEMORY_START (void*)0x0000000000500000  // Dirección del primer bloque. Todo el espacio desde ~0x40 0000 esta disponible
#define MEMORY_END (void*)0x0000000001500000    // No es una dirección válida. Pure64 es 64 bits, podría usarse más memoria. TODO: me fallo con numeros grandes, a partir de 0x0200 0000. No es un tema de las direcciones puntualmente, sino de la cantidad de memoria.

#define TOTAL_BLOCK_COUNT ((unsigned int)(MEMORY_END - MEMORY_START) / BLOCK_SIZE)

#define FREE 0
#define USED 1

typedef int bool;

typedef struct block_info
{
    unsigned int status;
    unsigned int contiguous_blocks; // Cantidad de bloques contiguos, si se pide más memoria que BLOCK_SIZE. Luego deben liberarse todos juntos.
} block_info;

block_info block_array[TOTAL_BLOCK_COUNT];  // TODO: tira warning porque void* es system-dependent.
static bool is_initialized = 0;

unsigned int first_free_index = 0;
unsigned int free_blocks; // TODO: ver si es util

void reset_first_free_index();

void create_mm(){

    if (is_initialized)
        return;

    is_initialized = 1;

    for(int i = 0; i < TOTAL_BLOCK_COUNT; i++) {
        block_array[i].status = FREE;
        block_array[i].contiguous_blocks = 0;
    }

    free_blocks = TOTAL_BLOCK_COUNT;
}

/**
 * @param size amount of bytes to allocate
 */
void* alloc(const unsigned long int size){

    if(size <= 0)
        return NULL;
    
    int blocks_to_alloc = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;  // La información de los bloques se guarda en el espacio de kernel. Si se decidiera implementar headers que estén en los mismos bloques, deberia restarse su tamaño a BLOCK_SIZE (es decir, BLOCK_SIZE - sizeof(header))

    if (!is_initialized || blocks_to_alloc > free_blocks)
        return NULL;
    
    for (int found_index = first_free_index, found = 0; found_index <= TOTAL_BLOCK_COUNT - blocks_to_alloc; found_index++) {

        // TODO: no deberia tener que explicarlo, pero me parece un poco confuso este algoritmo. Idealmente debería ser más simple, para no precisar explicación
        // TODO: en OSDev se recomienda consultar, para una implementacion similar, if(addr_next_block - (addr_current_block - size_of_header) > size). No estamos usandolo.
        // En el primer ciclo, este if es true (porque se garantiza que first_free_index apunta a un bloque disponible)
        // Si el bloque actual esta disponible, comienzo desde el mismo.
        // Luego, verifico si los bloques posteriores a este están ocupados, según los bloques contiguos que necesite. Si alguno lo está corto la verificación y continúo desde el siguiente
        // Si la cantidad de bloques contiguos que necesito está disponible, aloco todo este espacio.
        if(block_array[found_index].status == FREE) {
            found = 1;
            for (int contiguous_index = 1; contiguous_index < blocks_to_alloc && found == 1; contiguous_index++) {
                if(block_array[found_index + contiguous_index].status != FREE) {
                    found = 0;
                    found_index += contiguous_index;
                }
            }
        }

        if(found) {
            block_array[found_index].status = USED;
            block_array[found_index].contiguous_blocks = blocks_to_alloc;   // Solo lo guardo en el primero pues solo voy a usar su address. No voy a permitir liberar un bloque que no sea la cabeza de su "lista"

            for(int j = 1; j < blocks_to_alloc; j++) {
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

    // TODO: esta funcion seria mas simple si los headers fueran parte de la pagina...
    unsigned int index = (unsigned int)(address - MEMORY_START) / BLOCK_SIZE;
    unsigned int blocks_to_free = block_array[index].contiguous_blocks;

    if (blocks_to_free == 0)
        return;

    for(int i = index; i < blocks_to_free; i++) {
        block_array[i].status = FREE;
        block_array[i].contiguous_blocks = 0;
    }
    
    free_blocks += blocks_to_free;
}

void status_count(int *status_out){
    int block_count = TOTAL_BLOCK_COUNT;

    status_out[0] = block_count;
    status_out[1] = block_count - free_blocks;
    status_out[2] = free_blocks;
}

void reset_first_free_index() {
    for (int i = first_free_index, found = 0; i < TOTAL_BLOCK_COUNT && found == 0; i++)
        {
            if (block_array[i].status == FREE)
            {
                first_free_index = i;
                found = 1;
            }
        }
}