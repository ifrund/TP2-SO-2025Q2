include ../Makefile.inc

MODULE=0000-userCodeModule.bin
SOURCES=$(wildcard [^_]*.c)
SOURCES_ASM=$(wildcard asm/[^_]*.asm)
SOURCES_TEST = $(wildcard tests/[^_]*.c)
OBJECTS_ASM=$(SOURCES_ASM:.asm=.o)

all: $(MODULE)

$(MODULE): $(SOURCES) $(OBJECTS_ASM) $(SOURCES_TEST)
	$(GCC) $(GCCFLAGS) -T userCodeModule.ld _loader.c $(SOURCES) $(OBJECTS_ASM) $(SOURCES_TEST) -o ../$(MODULE)
	$(GCC) $(GCCFLAGS) -no-pie -T userCodeModule.ld _loader.c -Wl,--oformat=elf64-x86-64 $(OBJECTS_ASM) $(SOURCES) $(SOURCES_TEST) -o ../userCodeModuleLoader.elf
%.o : %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

clean:
	rm -rf *.o
	rm -rf asm/*.o

.PHONY: all clean print
