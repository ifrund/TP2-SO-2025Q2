#ifndef LIB_MATH_H
#define LIB_MATH_H

#include <stdint.h>

uint32_t uintToBase(uint64_t value, char *buffer, uint32_t base);
int pow(int base, int exp);

#endif