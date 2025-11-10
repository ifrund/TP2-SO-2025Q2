#ifndef LIB_ASM_H
#define LIB_ASM_H

#include <stdint.h>

// asm functions
char *cpuVendor(char *result);
int rtcInfo(int value);
int _getKey();
long *_regsInterrupt();
void _saveRegs();
void _outb(uint8_t port, uint8_t freq);
uint8_t _inb(uint8_t port);

// asm time functions
int _getSeconds();
int _getMinutes();
int _getHours();
int _getDay();
int _getMonth();
int _getYear();

int _getDayWeek();
int _getDayMonth();
int _getDateTimeFormat();

// asm SO functions
void _yield();
void *_create_stack(void *stack_top, void *rip, int argc, char **argv);
void _wait(uint8_t *lock);
void _post(uint8_t *lock);

#endif
