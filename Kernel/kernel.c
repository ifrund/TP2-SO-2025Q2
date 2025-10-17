#include "include/lib.h"
#include <stdint.h>
#include <sound.h>
#include <string.h>
#include <lib.h>
#include <moduleLoader.h>
#include <videoDriver.h>
#include <naiveConsole.h>
#include <idtLoader.h>
#include <keyboard.h>
#include <interrupts.h>
#include "include/memory_manager.h"
#include "include/proc.h"

extern uint8_t text;
extern uint8_t rodata;
extern uint8_t data;
extern uint8_t bss;
extern uint8_t endOfKernelBinary;
extern uint8_t endOfKernel;

static const uint64_t PageSize = 0x1000;

static void * const userspaceAddress = (void*)0x400000;
static void * const dataspaceAddress = (void*)0x500000;

typedef int (*EntryPoint)();


void clearBSS(void * bssAddress, uint64_t bssSize)
{
	memset(bssAddress, 0, bssSize);
}

void * getStackBase()
{
	return (void*)(
		(uint64_t)&endOfKernel
		+ PageSize * 8				//The size of the stack itself, 32KiB
		- sizeof(uint64_t)			//Begin at the top of the stack
	);
}

void * initializeKernelBinary()
{
	void * moduleAddresses[] = {
		userspaceAddress,
		dataspaceAddress
	};

	loadModules(&endOfKernelBinary, moduleAddresses);

	clearBSS(&bss, &endOfKernel - &bss);

    ncClear();
	return getStackBase();
}

static void* const shell = (void *) 0x400000;

int main()
{	
    load_idt();
    flush_buffer();
	create_mm();
	//TODO init de pipes (?
	char * argShell[1]={NULL};
	create_process(shell, "shell", 0, argShell);
	_setUser();

//    Esto no hace falta porque el salto se hace en set user
//    ((EntryPoint) userspaceAddress)();
    
	return 0;

}
