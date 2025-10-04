#ifndef LIB_H
#define LIB_H

#include <stdint.h>

void * memset(void * destination, int32_t character, uint64_t length);
void * memcpy(void * destination, const void * source, uint64_t length);

// asm functions
char *cpuVendor(char *result);
int rtcInfo(int value);
int _getKey();
long* _regsInterrupt();
void _saveRegs();
void _outb();
uint8_t _inb();


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

// base changing
uint32_t uintToBase(uint64_t value, char * buffer, uint32_t base);

int pow(int base, int exp);

#endif
