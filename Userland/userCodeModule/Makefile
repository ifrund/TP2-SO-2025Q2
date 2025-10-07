include ../Makefile.inc

MODULE=0000-userCodeModule.bin
MODULE_ELF=$(MODULE:.bin=.elf)

SOURCES=$(wildcard [^_]*.c)
SOURCES_ASM=$(wildcard asm/[^_]*.asm)
SOURCES_TEST = $(wildcard tests/[^_]*.c)

OBJECTS=$(SOURCES:.c=.o)
OBJECTS_ASM=$(SOURCES_ASM:.asm=.o)
OBJECTS_TEST=$(SOURCES_TEST:.c=.o)

all: $(MODULE)

$(MODULE): $(OBJECTS) $(OBJECTS_ASM) $(OBJECTS_TEST)
	$(GCC) $(GCCFLAGS) -T userCodeModule.ld _loader.c -o ../$(MODULE) $(OBJECTS) $(OBJECTS_ASM) $(OBJECTS_TEST)
	$(GCC) $(GCCFLAGS) -no-pie -T userCodeModule.ld _loader.c -Wl,--oformat=elf64-x86-64 -o ../$(MODULE_ELF) $(OBJECTS) $(OBJECTS_ASM) $(OBJECTS_TEST)

%.o : %.c
	$(GCC) $(GCCFLAGS) -I./include -c $< -o $@

%.o : %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

clean:
	rm -rf *.o
	rm -rf asm/*.o
	rm -rf tests/*.o

.PHONY: all clean print
