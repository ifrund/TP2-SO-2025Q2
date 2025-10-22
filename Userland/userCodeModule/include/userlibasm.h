#ifndef USERLIBASM_H
#define USERLIBASM_H

#include <stdint.h>
#include <stdbool.h>

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
#define MAX_FD 128
#define MAX_PCS 64
#define PROCESS_NAME_MAX_LENGTH 32
//Para el get_proc_list
typedef struct {
    char name[PROCESS_NAME_MAX_LENGTH];
    uint64_t pid;
    uint64_t parentPid;
    char state[16];            // "READY", etc.
    uint64_t rsp;
    char my_prio[16];

    //TODO probablmente borrarlas
    uint64_t externWaitingPID;
    bool isWaitingForExtern;
    int childrenAmount;
    uint64_t children[MAX_PCS];
    // Podrías incluir file descriptors si querés: los ids nada más.
    uint64_t fileDescriptors[MAX_FD];
    int fileDescriptorCount;  // Número de FDs válidos
} ProcessInfo;


int _create_process(void * rip, const char *name, int argc, char *argv[]);
int _kill_process(uint64_t pid);
int _block_process(uint64_t pid);
int _unblock_process(uint64_t pid);
ProcessInfo* _get_proc_list();
int _get_pid();
//Schedulers
void _yield();
int _be_nice(int pid, int newPrio);
int _wait(int pid);

#endif // USERLIBASM_H
