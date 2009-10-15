#include "gdt.h"

gdt_entry gdt[255] = {
  (gdt_entry){ (unsigned int) 0x00000000, (unsigned int) 0x00000000 }, // 0x0
  (gdt_entry){ // 0x8
    (unsigned short) 0xFFFF, 
    (unsigned short) 0x0000,
    (unsigned char) 0x00,
    // type segmento de codigo no conforming redeable y accedido
    (unsigned char) 0xb, 
    // system
    (unsigned char) 1, 
    // dpl 
    (unsigned char) 0, 
    // P
    (unsigned char) 1, 
    // segment limit
    (unsigned char) 0xF,
    // avl
    (unsigned char) 0,  
    //L = 0
    (unsigned char) 0,  
    //D
    (unsigned char) 1,  
    //G
    (unsigned char) 1, 
    (unsigned char) 0x00 
  },

  (gdt_entry){ // 0x10
    (unsigned short) 0xFFFF, 
    (unsigned short) 0x0000,
    //base
    (unsigned char) 0x00, 
    //type
    (unsigned char) 0x3,
    //system
    (unsigned char) 1,
    //dpl
    (unsigned char) 0,
    //present
    (unsigned char) 1,
    //limit
    (unsigned char) 0xF,
    //avl
    (unsigned char) 0,
    //0
    (unsigned char) 0,
    //direction
    (unsigned char) 1,
    //granularity
    (unsigned char) 1, 
    //base
    (unsigned char) 0x00 
  },

  (gdt_entry){ // 0x18
    (unsigned short) 0x000F, 
    (unsigned short) 0x8000,
    //base
    (unsigned char) 0x0B,
    //type
    (unsigned char) 0x3,
    //system
    (unsigned char) 1,
    //dpl
    (unsigned char) 0,
    //present
    (unsigned char) 1,
    //limit
    (unsigned char) 0x0,
    //avl
    (unsigned char) 0, 
    //0
    (unsigned char) 0,
    //direction
    (unsigned char) 1,
    //granularity
    (unsigned char) 0,
    //base
    (unsigned char) 0x00 
  },

  (gdt_entry){ // 0x20
    (unsigned short) 0xFFFF, 
    (unsigned short) 0x8000,
    (unsigned char) 0x0B, 
    (unsigned char) 0x2, 
    (unsigned char) 1, 
    (unsigned char) 0, 
    (unsigned char) 1, 
    (unsigned char) 0xF,
    (unsigned char) 0,  
    (unsigned char) 0,  
    (unsigned char) 1,  
    (unsigned char) 0, 
    (unsigned char) 0x00 
  },
  (gdt_entry){ // 0x28
    (unsigned short) 0x12ec, 
    (unsigned short) 0x0008,
    (unsigned char) 0x00,//param count 
    (unsigned char) 0xc, //call gate type
    (unsigned char) 0,  //segmento system
    (unsigned char) 3, //dpl
    (unsigned char) 1, // present  
    (unsigned char) 0x0, //offset 16:31
    (unsigned char) 0,  //offset 16:31
    (unsigned char) 0,  //offset 16:31
    (unsigned char) 0,  //offset 16:31
    (unsigned char) 0, //offset 16:31
    (unsigned char) 0x00 //offset 16:31
  },
  (gdt_entry){ // 0x30 limite f9e
    (unsigned short) 0x0f9e,// 80X25 bytes 
    (unsigned short) 0x8000, //primer parte de la base 
    (unsigned char) 0x0b, // segunda parte de la base
    (unsigned char) 0x3, //typo de segmento de datos accedido con lectura y escritura
    (unsigned char) 1,  //segmento system
    (unsigned char) 0, //dpl
    (unsigned char) 1, // present
    (unsigned char) 0x0, //ultima parte del limite
    (unsigned char) 0,  //libre para el programador 
    (unsigned char) 0,  // usado solo en 64 bits
    (unsigned char) 1,  //default operatio size
    (unsigned char) 0, //granularidad
    (unsigned char) 0x00 //ultima parte de la base
  }

};

gdt_descriptor GDT_DESC = {sizeof(gdt)-1, (unsigned int)&gdt};
