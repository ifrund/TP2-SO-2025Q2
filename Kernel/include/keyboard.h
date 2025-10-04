#include <stdint.h>

void key_handler();
int read_key(int fd);
void insert_key(int key);
void flush_buffer();
void checkShift();
void checkRegs(int key);
int altPressed();
