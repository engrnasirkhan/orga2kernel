#include "multiboot.h"
#include "screen/screen.h"

void kmain(multiboot_info_t* mbd, unsigned int magic)
{
    //use BIOS screen functions
    set_screen_mode(BIOS);
    //when paging is enabled, this pointer could change, or not.
    set_screen_pointer((ubyte*) 0xb8000);
    //clear screen
    kclrscreen();
    
    kprint("Initializing kernel...\n");

    if (magic != 0x2BADB002)
    {
        kprint("Error: Multiboot magic number is not 0x2BADB002 !!!\n");
        //halt !
        while(1);
    }
    
    //We can rely on multiboot structures :)

    //if flags' bit 0 is set, then mem_lower & mem_upper available
    if(mbd->flags & 0x0001)
    { 
        kprint("Lower memory: %u Kb\n", mbd->mem_lower);
        kprint("Upper memory: %u Kb\n", mbd->mem_upper);
    }
    
    //if flags' bit 6 is set, then memory map is available
    if(mbd->flags & 0x0020)
    {
       memory_map_t *mmap;
 
       kprint("Memory map:\n");
       for(mmap = (memory_map_t *) mbd->mmap_addr;
            (unsigned long) mmap < mbd->mmap_addr + mbd->mmap_length;
            mmap = (memory_map_t *) ((unsigned long) mmap
                                     + mmap->size + sizeof (mmap->size)))
         kprint("  Size = 0x%x, Base_Address = 0x%x,"
                 " Length = 0x%x, Type = 0x%x\n",
                 mmap->size,
                 mmap->base_addr_low,
                 mmap->length_low,
                 mmap->type);                   
    }
    
    while (1);
}


