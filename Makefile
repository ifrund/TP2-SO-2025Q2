
all:  bootloader kernel toolchain userland image

bootloader:
	cd Bootloader; make all

kernel:
	cd Kernel; make all

toolchain:
	cd Toolchain; make all

userland:
	cd Userland; make all

image: kernel bootloader toolchain userland
	cd Image; make all

#TODO los archivos de Tests

clean:
	cd Bootloader; make clean
	cd Image; make clean
	cd Kernel; make clean
	cd Toolchain; make clean
	cd Userland; make clean

.PHONY: bootloader image collections kernel toolchain userland all clean
