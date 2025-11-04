
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

.PHONY: bootloader image collections kernel kernel-buddy toolchain userland all buddy clean
.NOTPARALLEL: all buddy
