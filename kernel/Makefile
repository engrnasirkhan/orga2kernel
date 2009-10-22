include flags.mk

# DIRS le dice a que directorios debe descender
# OBJS le indica los archivos objetos a linkear,
#      los cuales deben estar en el orden adecuado.
DIRS = src/boot
OBJS = src/boot/boot.o

.PHONY: all clean
all: kernel.elf
kernel.elf: $(OBJS)
#	$(foreach dir,$(DIRS), $(MAKE) -C $(dir))
	$(LD) $(LD_FLAGS) $(LDFLAGS) -T linker.ld $(OBJS) -o kernel.elf
	mcopy -oi disquette.img kernel.elf ::/BOOT

clean:
	$(foreach dir,$(DIRS), $(MAKE) -C $(dir) clean)
