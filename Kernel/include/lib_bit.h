#ifndef LIB_BIT_H
#define LIB_BIT_H

#include <stdint.h>
#include <stdbool.h>

bool is_bit_set(char *array, uint32_t index);
void set_bit(char *array, uint32_t index);
void clear_bit(char *array, uint32_t index);

#endif