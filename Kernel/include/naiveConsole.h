#ifndef NAIVE_CONSOLE_H
#define NAIVE_CONSOLE_H

#include <stdint.h>

// regular prints
void ncPrint(const char * string);
void ncPrintChar(char character);
void ncPrintCant(const char * string, int num);

// color prints
void ncPrintColor(const char * string, int color);
void ncPrintCharColor(char character, int color);
void ncPrintColorCant(const char * string, int num, int color);

// cleanup
void ncNewline();
void ncClear();

// numeric prints
void ncPrintDec(uint64_t value);
void ncPrintHex(uint64_t value);
void ncPrintBin(uint64_t value);
void ncPrintBase(uint64_t value, uint32_t base);

// spacing
uint8_t * getCurrentVideo();
void setCurrentVideo(uint8_t *location);

#endif
