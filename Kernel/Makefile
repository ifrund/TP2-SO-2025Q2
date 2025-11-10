include Makefile.inc

KERNEL=kernel.bin
SOURCES = $(wildcard *.c)
SOURCES_LIB = $(wildcard libs/*.c)
SOURCES_MM = $(filter-out memory_management/memory_manager.c memory_management/buddy_memory_manager.c, $(wildcard memory_management/*.c))
SOURCES_ASM=$(wildcard asm/*.asm)
OBJECTS=$(SOURCES:.c=.o) $(SOURCES_LIB:.c=.o) $(filter-out memory_management/memory_manager.c memory_management/buddy_memory_manager.c, $(SOURCES_MM:.c=.o))
OBJECTS_ASM=$(SOURCES_ASM:.asm=.o)

LOADERSRC=loader.asm
LOADEROBJECT=$(LOADERSRC:.asm=.o)
STATICLIBS=

all: $(LOADEROBJECT) $(OBJECTS) memory_management/memory_manager.o $(STATICLIBS) $(OBJECTS_ASM)
	$(LD) $(LDFLAGS) -T kernel.ld -o $(KERNEL) $(LOADEROBJECT) $(OBJECTS) memory_management/memory_manager.o $(OBJECTS_ASM) $(STATICLIBS)
	$(LD) $(LDFLAGS) -T kernel.ld --oformat=elf64-x86-64 -o kernel.elf  $(LOADEROBJECT) $(OBJECTS) memory_management/memory_manager.o $(OBJECTS_ASM) $(STATICLIBS)
	
buddy: $(LOADEROBJECT) $(OBJECTS) memory_management/buddy_memory_manager.o $(STATICLIBS) $(OBJECTS_ASM)
	$(LD) $(LDFLAGS) -T kernel.ld -o $(KERNEL) $(LOADEROBJECT) $(OBJECTS) memory_management/buddy_memory_manager.o $(OBJECTS_ASM) $(STATICLIBS)
	$(LD) $(LDFLAGS) -T kernel.ld --oformat=elf64-x86-64 -o kernel.elf  $(LOADEROBJECT) $(OBJECTS) memory_management/buddy_memory_manager.o $(OBJECTS_ASM) $(STATICLIBS)

%.o: %.c
	$(GCC) $(GCCFLAGS) -I./include -c $< -o $@

%.o : %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

$(LOADEROBJECT):
	$(ASM) $(ASMFLAGS) $(LOADERSRC) -o $(LOADEROBJECT)

clean:
	rm -rf asm/*.o *.o libs/*.o memory_management/*.o *.bin

.PHONY: all buddy clean
.NOTPARALLEL: all buddy