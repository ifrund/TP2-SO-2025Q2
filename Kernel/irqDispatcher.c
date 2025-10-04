#include <time.h>
#include <keyboard.h>
#include <stdint.h>
#include <syscall_handler.h>
#include <naiveConsole.h>
#include <registers.h>

static void int_20();
static void int_21();
static void int_48();
static void int_80();

void irqDispatcher(uint64_t irq) {
	switch (irq) {
		case 0:
			int_20();
			break;
        case 1:
            int_21();
            break;

        case 0x48:
            int_48();
            break;

        case 0x80:
            int_80();
            break;
	}
	return;
}

void int_20() {
	timer_handler();
}

void int_21() {
    key_handler();
}

void int_48(){
}

void int_80() {
    syscall_handler();
}
