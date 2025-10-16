#include <stdint.h>

/*---Arquitectura de Computadoras---*/
void _print(int fd, char * string, int length);
int _read(int fd, char * buffer, int length);
int int_test();
void _sleep(int cant, int unidad);
void _getClock(int *hrs, int *min, int *seg);
void _draw(uint16_t * bitmap, uint32_t color, uint16_t height, uint64_t x, uint64_t y);
void _screenData(uint16_t * screenHeight, uint16_t * screenWidth, uint8_t * fontSize, uint8_t * drawSize);
void _changeSize(uint8_t newSize, uint8_t fd);
int _getRegs(uint64_t regs[]);
void _opError();
void _beep(uint32_t frequency, int duration);

/*---Sistemas Operativos---*/

//Memory Manager
void _create_mm();
void * _alloc(int size);
void _free(void *address);
void _status_count(int *status_count);
//Processes
int _create_process(int (*entryPoint)(int argc, char *argv[]), const char *name, int argc, char *argv[]);
int _kill_process(uint64_t pid);
int _block_process(uint64_t pid);
int _unblock_process(uint64_t pid);
void _get_proc_list(char ** procNames, uint64_t * pids, uint64_t * parentPids, char ** status, uint64_t * rsps);
