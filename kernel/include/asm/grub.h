#ifndef __GRUB__H__
#define __GRUB__H__

#include <asm/types.h>

struct MultibootHeader {
	uint32_t magic, flags, checksum;
}

struct MultibootInformation {
	uint32_t flags;
	uint32_t mem_lower, mem_upper; // Si flags[0]
	uint32_t boot_device; // Si flags[1]
	char *cmdline; // Si flags[2]
	uint32_t mods_count, mods_addr; // Si flags[3]
	uint32_t syms[6]; // Si flags[4] o flags[5]
	uint32_t mmap_length, mmap_addr; // Si flags[6]
	uint32_t drives_length, drives_addr; // Si flags[7]
	uint32_t config_table; // Si flags[8]
	uint32_t boot_loader_name; // Si flags[9]
	ptr_t apm_table; // Si flags[10]
	// Si flags[11]:
	uint32_t vbe_control_info, vbe_mode_info;
	uint16_t vbe_mode, vbe_interface_seg, vbe_interface_off,
				vbe_interface_len;
};

#endif
