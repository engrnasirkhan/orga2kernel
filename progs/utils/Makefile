# Listado de programas.
BUILD ?= check_exe incrustar

.PHONY: all clean
all: $(BUILD)

# Fuentes y flags de check_exe
check_exe_CFLAGS = -std=gnu99 -Wall -pipe
check_exe: check_exe.c
	$(CC) $(CFLAGS) $(check_exe_CFLAGS) $< -o $@

# Fuentes y flags de incrustar
inscrutar_CFLAGS = -Wall -pipe
incrustar: incrustar.c
	$(CC) $(CFLAGS) $(inscrutar_CFLAGS) $< -o $@

clean:
	rm -f $(BUILD)
