// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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
#include "include/pipes.h"

#define USERSPACE_ADDRESS (void*)0x400000
#define DATASPACE_ADDRESS (void*)0x500000

extern uint8_t text;
extern uint8_t rodata;
extern uint8_t data;
extern uint8_t bss;
extern uint8_t endOfKernelBinary;
extern uint8_t endOfKernel;
static const uint64_t PageSize = 0x1000;

typedef int (*EntryPoint)();

void clearBSS(void *bssAddress, uint64_t bssSize)
{
	memset(bssAddress, 0, bssSize);
}

void *getStackBase()
{
	return (void *)((uint64_t)&endOfKernel + PageSize * 8 // The size of the stack itself, 32KiB
					- sizeof(uint64_t)					  // Begin at the top of the stack
	);
}

void *initializeKernelBinary()
{
	void *moduleAddresses[] = {
		USERSPACE_ADDRESS,
		DATASPACE_ADDRESS};

	loadModules(&endOfKernelBinary, moduleAddresses);

	clearBSS(&bss, &endOfKernel - &bss);

	ncClear();
	return getStackBase();
}

// proceso basura cuando no hay ninguno ready, llama constantemente a halt, osea al sch, osea a q pase al proximo pcs
// tmb lo usamos como init
static void idle()
{
	char *arg_null[1] = {NULL};
	SHELL_PID = create_process(USERSPACE_ADDRESS, "shell", 0, arg_null, NULL);

	while (1)
	{
		_hlt();
	}
}

int main()
{
	load_idt();
	flush_buffer();
	create_mm();
	pipe_init();
	char *arg_null[1] = {NULL};
	IDLE_PID = create_process(&idle, "idle", 0, arg_null, NULL);
	_sti(); // las desactivamos porq sino el sch nunca se activa y no toma el proceso idle

	while (1)
	{
		_hlt();
	}

	return 0;
}
