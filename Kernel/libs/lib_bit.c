// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

// Implementation based on https://pdos.csail.mit.edu/6.828/2019/lec/malloc.c

#include "../include/lib_bit.h"

/**
 * Retorna 1 si el bit de índice index en el arreglo array es 1
 * @param array arreglo de bits
 * @param index indice a chequear
 * @return 1 si array[index] es 1
 */
bool is_bit_set(char *array, uint32_t index)
{
    char b = array[index / 8];
    char m = (1 << (index % 8));
    return (b & m) == m;
}

/**
 * Setea el bit de índice index en el arreglo array a 1
 * @param array arreglo de bits
 * @param index índice a setear
 */
void set_bit(char *array, uint32_t index)
{
    char b = array[index / 8];
    char m = (1 << (index % 8));
    array[index / 8] = (b | m);
}

/**
 * Limpia el bit de índice index en el arreglo array a 0
 * @param array arreglo de bits
 * @param index índice a limpiar
 */
void clear_bit(char *array, uint32_t index)
{
    char b = array[index / 8];
    char m = (1 << (index % 8));
    array[index / 8] = (b & ~m);
}