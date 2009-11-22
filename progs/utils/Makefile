# Listado de programas.
BUILD ?= check_exe

.PHONY: all clean
all: $(BUILD)

# Fuentes y flags de check_exe
check_exe_CFLAGS = -std=gnu99 -Wall -pipe
check_exe: check_exe.c
	$(CC) $(CFLAGS) $(check_exe_CFLAGS) $< -o $@

clean:
	rm -f $(BUILD)
