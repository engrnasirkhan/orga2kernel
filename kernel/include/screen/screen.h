/*
 * Defines functions to write on screen
 */
 
#ifndef __SCREEN_H__
#define __SCREEN_H__

#include <asm/types.h>

//los posibles modos de la pantalla. Por default se usa BIOS.
enum screen_mode {
  BIOS,
  VGA //vga?? que se yo, soñar no cuesta nada xD
};

//Returns number of char's put to screen
int kprint(const char* format, ...);

void kputc(const char c);

//clears screen
void kclrscreen();

//sets screen mode
void set_screen_mode(enum screen_mode mode);

void set_screen_pointer(uint8_t* screen_ptr);

#endif
