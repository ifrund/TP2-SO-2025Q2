#include <videoDriver.h>
#include <keyboard.h>
#include <stdint.h>

long registros[18];

extern uint64_t regsBuf[];
extern uint8_t regs_saved;

int getRegs(uint64_t regs[]){
    
    if (!regs_saved){
        return 0;
    }

    for (int i=0; i<18; i++) {
        regs[i]=regsBuf[i];
    }

    return 1;
}

