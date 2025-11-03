include Makefile.inc

KERNEL=kernel.bin
SOURCES = $(filter-out memory_manager.c buddy_memory_manager.c, $(wildcard *.c))
SOURCES_ASM=$(wildcard asm/*.asm)
#TODO falta un SOURCES_TEST
OBJECTS=$(SOURCES:.c=.o)
OBJECTS_ASM=$(SOURCES_ASM:.asm=.o)
#TODO falta un OBJECTS_TEST

LOADERSRC=loader.asm
LOADEROBJECT=$(LOADERSRC:.asm=.o)
STATICLIBS=

all: $(LOADEROBJECT) $(OBJECTS) memory_manager.o $(STATICLIBS) $(OBJECTS_ASM)
	$(LD) $(LDFLAGS) -T kernel.ld -o $(KERNEL) $(LOADEROBJECT) $(OBJECTS) memory_manager.o $(OBJECTS_ASM) $(STATICLIBS)
	$(LD) $(LDFLAGS) -T kernel.ld --oformat=elf64-x86-64 -o kernel.elf  $(LOADEROBJECT) $(OBJECTS) memory_manager.o $(OBJECTS_ASM) $(STATICLIBS)
	
buddy: $(LOADEROBJECT) $(OBJECTS) buddy_memory_manager.o $(STATICLIBS) $(OBJECTS_ASM)
	$(LD) $(LDFLAGS) -T kernel.ld -o $(KERNEL) $(LOADEROBJECT) $(OBJECTS) buddy_memory_manager.o $(OBJECTS_ASM) $(STATICLIBS)
	$(LD) $(LDFLAGS) -T kernel.ld --oformat=elf64-x86-64 -o kernel.elf  $(LOADEROBJECT) $(OBJECTS) buddy_memory_manager.o $(OBJECTS_ASM) $(STATICLIBS)

%.o: %.c
	$(GCC) $(GCCFLAGS) -I./include -c $< -o $@
#TODO -I../Tests

%.o : %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

$(LOADEROBJECT):
	$(ASM) $(ASMFLAGS) $(LOADERSRC) -o $(LOADEROBJECT)

clean:
	rm -rf asm/*.o *.o *.bin
#TODO agregar al rm los test

.PHONY: all buddy clean
.NOTPARALLEL: all buddy