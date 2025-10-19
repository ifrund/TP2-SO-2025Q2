#include <stdint.h>
#include "../include/shell.h"   //write_out lib, for testing
#include "../include/userlibasm.h"

uint32_t GetUint();
uint32_t GetUniform(uint32_t max);
uint8_t memcheck(void *start, uint8_t value, uint32_t size);
int64_t satoi(char *str);
void bussy_wait(uint64_t n);
void endless_loop();
void endless_loop_print(uint64_t wait);

uint64_t test_mm(uint64_t argc, char *argv[]);
void test_prio();
int64_t test_processes(uint64_t argc, char *argv[]);
uint64_t test_sync(uint64_t argc, char *argv[]);

