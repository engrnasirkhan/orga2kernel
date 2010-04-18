/*
 * Defines functions to write on screen
 */
 
#ifndef __SCREEN__H__
#define __SCREEN__H__

#include <asm/types.h>

//los posibles modos de la pantalla. Por default se usa BIOS.
enum screen_mode {
  BIOS,
  VGA //vga?? que se yo, soñar no cuesta nada xD
};

//Returns number of char's put to screen
int kprint(const uint8_t* format, ...);

void kputc(const uint8_t c);

//clears screen
void kclrscreen();

//sets screen mode
void set_screen_mode(enum screen_mode mode);

void set_screen_pointer(uint8_t* screen_ptr);

//functions for tty area
size_t kprint_tty(uint8_t * );
void kprint_tty_clear();
void kprint_tty_backspace();
#endif
