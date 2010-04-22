.PHONY: all
all:
	make -C kernel clean
	cd progs && ./build.sh
	cd kernel && ./incrustar.sh
	make -C kernel
