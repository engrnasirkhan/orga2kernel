#include "screen.h"
#include "../utils/utils.h"

#define SCREEN_BIOS_ROWS    25
#define SCREEN_BIOS_COLS    80

//Current screen mode
static enum screen_mode current_screen_mode;
//Current screen pointer
static ubyte* current_screen_pointer;

/*BIOS screen functions*/
void write_char_bios(const char c);
void clear_screen_bios();
/***********************/

int kprint(const char *format, ...)
{
    char **arguments = (char **) &format;
    
    //now 'arguments' should be pointing to the other args that follow 'format'
    arguments++;
    
    char c;
    int char_count = 0;
    
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
                int d = *((int*) arguments++);
                //for 32-bit signed ints, 12 char's are enough (including \0) 
                char buff[12];
                
                uint base;
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
                int i=0;
                while(buff[i] != '\0')
                {
                    kputc(buff[i++]);
                    char_count++;
                }
            }
            else if(c=='s')
            {
                char* s = *arguments++;
                char p;
                while((p=*s++) != '\0')
                {
                    kputc(p);
                    char_count++;
                }
            }
        }
    }
    
    return char_count;
}



void kputc(const char c)
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


void write_char_bios(const char c)
{
    static int current_row = 0;
    static int current_col = 0;
    
    if(c!='\n')
    {
        //write char
        *(current_screen_pointer + (current_col + current_row * SCREEN_BIOS_COLS) * 2)      = c & 0xFF;
        *(current_screen_pointer + (current_col + current_row * SCREEN_BIOS_COLS) * 2 + 1)  = 0x7;
        //update col   
        current_col = (current_col + 1) % SCREEN_BIOS_COLS;
    }
    else
    {
        //do not write \n and update col
        current_col = 0;
    }
    
    if(current_col==0)
    {
        current_row++;
    }
    
    if(current_row > SCREEN_BIOS_ROWS)
    {
        //move up the whole screen
        int i,j;
        for(i=0; i<SCREEN_BIOS_ROWS-1; i++)
        {
            for(j=0; j<SCREEN_BIOS_COLS; j++)
            {
               *(current_screen_pointer + (j + i * SCREEN_BIOS_COLS) * 2) = *(current_screen_pointer + (j + (i+1) * SCREEN_BIOS_COLS) * 2);
               *(current_screen_pointer + (j + i * SCREEN_BIOS_COLS) * 2 + 1) = *(current_screen_pointer + (j + (i+1) * SCREEN_BIOS_COLS) * 2 + 1); 
            }
        }
        
        current_row = SCREEN_BIOS_ROWS-1;
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
    int i,j;
    for(i=0; i<SCREEN_BIOS_ROWS; i++)
    {
        for(j=0; j<SCREEN_BIOS_COLS; j++)
        {
           *(current_screen_pointer + (j + i * SCREEN_BIOS_COLS) * 2) = 0;
           *(current_screen_pointer + (j + i * SCREEN_BIOS_COLS) * 2 + 1) = 0; 
        }
    }
}

//sets screen mode
void set_screen_mode(enum screen_mode mode)
{
    current_screen_mode = mode;
}

void set_screen_pointer(ubyte* screen_ptr)
{
    current_screen_pointer = screen_ptr;
}


