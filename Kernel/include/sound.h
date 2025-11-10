#ifndef SOUND_H
#define SOUND_H

#include <stdint.h>
void beep(uint32_t frequency, int duration);
void beep_asm();

#endif