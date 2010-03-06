#include <screen/screen.h>
#include <lib/utils.h>
#include <stdarg.h>

#define SCREEN_BIOS_ROWS    25
#define SCREEN_BIOS_COLS    80

//Current screen mode
static enum screen_mode current_screen_mode;
//Current screen pointer
static uint8_t* current_screen_pointer;

/*BIOS screen functions*/
void write_char_bios(const uint8_t c);
void clear_screen_bios();

static uint32_t bios_current_row = 0;
static uint32_t bios_current_col = 0;
/***********************/

int kprint(const uint8_t *format, ...)
{
    va_list ap;
	va_start(ap, format);

    uint8_t c;
    uint32_t char_count = 0;
    
    while((c = *format++) != '\0')
    {
        if(c!='%')
        {
            kputc(c);
            char_count++;
        }
        else
        {
            c = *format++;

            if(c=='d' || c=='i' || c=='u' || c=='x' || c=='o')
            {
                int32_t d = va_arg( ap, int32_t );
                //for 32-bit signed ints, 12 char's are enough (including \0) 
                uint8_t buff[12];
                
                uint32_t base;
                if(c=='d' || c=='i' || c=='u')
                {
                    base = 10;
                }
                else if(c=='o')
                {
                    base = 8;
                }
                else
                {
                    base = 16;
                }
                
                if(d<0 && (c=='d' || c=='i' || c=='o'))
                {
                    d = -d;
                    kputc('-');
                }
                
                uitoa(d, buff, base);
                int32_t i=0;
                while(buff[i] != '\0')
                {
                    kputc(buff[i++]);
                    char_count++;
                }
            }
            else if(c=='s')
            {
                uint8_t* s = va_arg( ap, uint8_t * );
                uint8_t p;
                while((p=*s++) != '\0')
                {
                    kputc(p);
                    char_count++;
                }
            }
        }
    }
	 va_end( ap );
    
    return char_count;
}



void kputc(const uint8_t c)
{
    if(current_screen_mode == BIOS)
    {
        write_char_bios(c);
    }
    else
    {
        //future screen modes should be called from here
    }
}


void write_char_bios(const uint8_t c)
{  
    if(c!='\n')
    {
        //write char
        *(current_screen_pointer + (bios_current_col + bios_current_row * SCREEN_BIOS_COLS) * 2)      = c & 0xFF;
        *(current_screen_pointer + (bios_current_col + bios_current_row * SCREEN_BIOS_COLS) * 2 + 1)  = 0x7;
        //update col   
        bios_current_col = (bios_current_col + 1) % SCREEN_BIOS_COLS;
    }
    else
    {
        //do not write \n and update col
        bios_current_col = 0;
    }
    
    if(bios_current_col==0)
    {
        bios_current_row++;
    }
    
    if(bios_current_row > SCREEN_BIOS_ROWS)
    {
        //move up the whole screen
        int32_t i,j;
        for(i=0; i<SCREEN_BIOS_ROWS-1; i++)
        {
            for(j=0; j<SCREEN_BIOS_COLS; j++)
            {
               *(current_screen_pointer + (j + i * SCREEN_BIOS_COLS) * 2) = *(current_screen_pointer + (j + (i+1) * SCREEN_BIOS_COLS) * 2);
               *(current_screen_pointer + (j + i * SCREEN_BIOS_COLS) * 2 + 1) = *(current_screen_pointer + (j + (i+1) * SCREEN_BIOS_COLS) * 2 + 1); 
            }
        }
        
        bios_current_row = SCREEN_BIOS_ROWS-1;
    }
}

void kclrscreen()
{
    if(current_screen_mode == BIOS)
    {
        clear_screen_bios();
    }
    else
    {
        //nothing...
    }
}


void clear_screen_bios()
{
    int32_t i,j;
    for(i=0; i<SCREEN_BIOS_ROWS; i++)
    {
        for(j=0; j<SCREEN_BIOS_COLS; j++)
        {
           *(current_screen_pointer + (j + i * SCREEN_BIOS_COLS) * 2) = 0;
           *(current_screen_pointer + (j + i * SCREEN_BIOS_COLS) * 2 + 1) = 0; 
        }
    }
    
    bios_current_row = 0;
    bios_current_col = 0;
}

//sets screen mode
void set_screen_mode(enum screen_mode mode)
{
    current_screen_mode = mode;
}

void set_screen_pointer(uint8_t* screen_ptr)
{
    current_screen_pointer = screen_ptr;
}


