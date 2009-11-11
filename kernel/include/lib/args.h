#ifndef __STDARG__H__
#define __STDARG__H__

typedef int *va_list;
#define va_start(x,y) ((x) = ((int *)&(y)) + (((sizeof(y) + 3) & ~3) >> 2)  )
#define va_arg(x,y) ( x += (((sizeof(y) + 3) & ~3)>>2), *((y *) (x - (((sizeof(y) + 3) & ~3)>>2))) )
#define va_end(x) ((x) = (int *)0)

#endif
