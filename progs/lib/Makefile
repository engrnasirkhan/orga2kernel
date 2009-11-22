# Flags necesarios no dependientes de la arquitectura
INCLUDES = -Iinclude -Iinclude
C_FLAGS = -std=gnu99 -ffreestanding -nostdlib -nostdinc -nostartfiles -nodefaultlibs -static $(INCLUDES) # -fno-builtin
CXX_FLAGS = -fno-exceptions -fno-rtti -nostdinc++ -std=gnu++0x -ffreestanding -nostdlib -nostdinc -nostartfiles -nodefaultlibs -static $(INCLUDES) # -fno-builtin
LD_FLAGS = -nostdlib 
AS_FLAGS =
CAS_FLAGS = $(C_FLAGS)
CAS ?= $(CC)

# Flags dependientes de la arquitectura
ifeq ($(shell uname -m),x86_64)
	C_FLAGS += -m32
	CXX_FLAGS += -m32
	CAS_FLAGS += -m32
	AS_FLAGS += --32
	LD_FLAGS += --oformat elf32-i386 -m elf_i386 -nostdlib
endif

# Flags normales y modificables
CFLAGS ?= #-march=core2 -Os -fomit-frame-pointer -Wall -s
CXXFLAGS ?= $(CFLAGS)
ASFLAGS ?= 
CASFLAGS ?=
LDFLAGS ?= # -O1 -s -x -static

# Targets comunes
.obj/%.o: %.c
	$(CC) $(C_FLAGS) $(CFLAGS) -c $< -o $@

.obj/%.o: %.cpp
	$(CXX) $(CXX_FLAGS) $(CXXFLAGS) -c $< -o $@

.obj/%.o: %.s
	$(AS) $(AS_FLAGS) $(ASFLAGS) $< -o $@

.obj/%.o: %.S
	$(CAS) $(CAS_FLAGS) $(CASFLAGS) -c $< -o $@

include proyecto.mk

SOURCES = $(foreach i, $(DIRS), $(wildcard $(i)/*.c $(i)/*.cpp $(i)/*.S $(i)/*.s $(i)/*.asm))
OBJS = $(addprefix .obj/, $(addsuffix .o, $(basename $(SOURCES))) $(addsuffix .o, $(basename $(SRCS))))
OBJSDIRS = $(sort $(dir $(OBJS)))

.PHONY: all lib clean
all: $(TARGET)

lib: $(TARGET)
$(TARGET): $(OBJSDIRS) $(OBJS)
	$(AR) rcsv $(TARGET) $(OBJS)

$(OBJSDIRS):
	mkdir -p $@

clean:
	rm -rf $(OBJS)
