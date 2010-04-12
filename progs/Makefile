# Listado de programas.
BUILD ?= ej1 ej2

.PHONY: all clean $(BUILD)
all: $(BUILD)

$(BUILD):
	$(MAKE) -C $@

clean:
	$(foreach i, $(BUILD), $(MAKE) -C $(i) clean)
