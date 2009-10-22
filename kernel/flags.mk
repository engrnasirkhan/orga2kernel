# Flags necesarios no dependientes de la arquitectura
C_FLAGS = -std=gnu99 -ffreestanding -nostdlib -nostdinc -nostartfiles -nodefaultlibs -static -Iinclude # -fno-builtin
CXX_FLAGS = -fno-rtti -nostdinc++ -std=gnu++0x -ffreestanding -nostdlib -nostdinc -nostartfiles -nodefaultlibs -static -Iinclude # -fno-builtin
LD_FLAGS = -nostdlib
AS_FLAGS =

# Flags dependientes de la arquitectura
ifeq ($(shell uname -m),x86_64)
	C_FLAGS += -m32
	CXX_FLAGS += -m32
	AS_FLAGS += --32
	LD_FLAGS += --oformat elf32-i386 -m elf_i386 -nostdlib
endif

# Flags normales y modificables
CFLAGS ?= #-march=core2 -Os -fomit-frame-pointer -Wall -s
CXXFLAGS ?= $(CFLAGS)
ASFLAGS ?= 
LDFLAGS ?= # -O1 -s -x -static

# Targets comunes
%.o: %.c
	$(CC) $(C_FLAGS) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXX_FLAGS) $(CXXFLAGS) -c $< -o $@

%.o: %.S
	$(AS) $(AS_FLAGS) $(ASFLAGS) $< -o $@
