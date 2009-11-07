#include "multiboot.h"
#include "screen/screen.h"

void kmain( multiboot_info_t* mbd, unsigned int magic)
{
    if (magic != 0x2BADB002)
    {
     
    }

    //use BIOS screen functions
    set_screen_mode(BIOS);
    //when paging is enabled, this pointer could change, or not.
    set_screen_pointer((ubyte*) 0xb8000);
    
    //clear screen
    kclrscreen();
    
    
    kprint("Initializing kernel...\n");
    
    //if flags' bit 0 is set, then mem_lower & mem_upper available
    if(mbd->flags & 0x0001){ 
        kprint("%u\n", mbd->flags);
        kprint("Lower memory: %u\n", mbd->mem_lower);
        kprint("Upper memory: %u\n", mbd->mem_upper);
    }
    
/*
    char * boot_loader_name = (char *)mbd + 64;
    unsigned char * videoram = (unsigned char *) 0xB8000;
    unsigned char * aux;
    videoram[0] = 65;
    videoram[1] = 0x07;
    aux = (unsigned char*)mbd; //para que no chille el compilador(soy un negro)
    videoram[2] = *aux;
    char* mensaje = "Este es un mensaje ahora \0";
    int i = 0; 
    while( mensaje[i] != 0)
    {
      videoram[i*2] = mensaje[i];
      videoram[i*2+1] = 0x07;
      i = i+1;
    }
    print(mensaje); 
*/
    while (1);
}


