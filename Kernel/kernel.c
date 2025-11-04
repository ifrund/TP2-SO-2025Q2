#include "include/lib.h"
#include <stdint.h>
#include <sound.h>
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

//proceso basura cuando no hay ninguno ready, llama constantemente a halt, osea al sch, osea a q pase al proximo pcs
//tmb lo usamos como init
static void idle(){
	char * argNull[1]={NULL};
	SHELL_PID = create_process(shell, "shell", 0, argNull); 

	while(1){
		_hlt();
	}
}

int main()
{	
    load_idt();
    flush_buffer();
	create_mm();
	//TODO init de pipes (?
	char * argNull[1]={NULL};
	IDLE_PID = create_process(&idle, "idle", 0, argNull);  
	_sti(); //las desactivamos porq sino el sch nunca se activa y no toma el proceso idle

	while(1){
		_hlt();
	}
    
	return 0;
}
