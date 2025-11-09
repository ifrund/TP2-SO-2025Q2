
all: bootloader kernel toolchain userland image

buddy: bootloader kernel-buddy toolchain userland image

bootloader:
	cd Bootloader; make all

kernel:
	cd Kernel; make all

kernel-buddy:
	cd Kernel; make buddy

toolchain:
	cd Toolchain; make all

userland:
	cd Userland; make all

image:
	cd Image; make all

clean:
	cd Bootloader; make clean
	cd Image; make clean
	cd Kernel; make clean
	cd Toolchain; make clean
	cd Userland; make clean
	
pvs:
	rm -rf salida.log strace_out
	rm -rf informe_completo.html
	make clean
	pvs-studio-analyzer trace -- make all
	pvs-studio-analyzer analyze -o salida.log
	plog-converter -a 'GA:1,2,3;64:1,2,3;OP:1,2,3;CS:1,2;MISRA:1,2;AUTOSAR:1' -t fullhtml -o informe_completo.html salida.log

.PHONY: bootloader image collections kernel kernel-buddy toolchain userland all buddy clean pvs
.NOTPARALLEL: all buddy
