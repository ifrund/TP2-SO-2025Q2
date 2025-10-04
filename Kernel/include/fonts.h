#ifndef FONTS_H
#define FONTS_H

#include <stdint.h>

//basada en ASCII(primeros 32 reservados)

#define charHeight 24
#define charWidth 16

extern uint16_t font[][charHeight];
extern uint16_t dibujitos[][charHeight];

#endif
