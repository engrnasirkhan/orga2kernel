#include <screen/screen.h>
#include <lib/utils.h>
#include <stdarg.h>

#define SCREEN_BIOS_ROWS    25
#define SCREEN_BIOS_COLS    80

#define SCREEN_STATUS_ROW SCREEN_BIOS_TTY_ROW + 1
#define SCREEN_BIOS_TTY_ROW (SCREEN_BIOS_ROWS - 2)
#define SCREEN_BIOS_TTY_INIT (SCREEN_BIOS_TTY_ROW * SCREEN_BIOS_COLS)
//Current screen mode
static enum screen_mode current_screen_mode;
//Current screen pointer
static uint8_t* current_screen_pointer;

static uint32_t bios_current_row = 0;
static uint32_t bios_current_col = 0;

static uint32_t bios_tty_current_col = 0;
static uint32_t bios_tty_caret_position = 0;
static uint8_t  bios_tty_hided_caracter = ' ';

//private
void kscrllscreen();

/*BIOS screen functions*/
void write_char_bios(const uint8_t c);
void clear_screen_bios();
void bios_scrllscreen();
void bios_tty_backspace();
void bios_tty_show_caret();
void bios_tty_hide_caret();

/* bios tty functions */
void bios_tty_clear();
size_t bios_tty_print(uint8_t*);

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
				else if(c=='c')
				{
					char c = va_arg( ap, int );
					kputc(c);
					char_count++;
				}
        }
    }
	 va_end( ap );
    
    return char_count;
}

void kprint_tty_clear(){
    
    if(current_screen_mode == BIOS)
    {
        bios_tty_clear();
    }
    else
    {
        //future screen modes should be called from here
    }
}
void bios_tty_clear() {
    bios_tty_hide_caret();
    bios_tty_current_col = 0;
    for(int x = 0; x < SCREEN_BIOS_COLS; x++){
        *(current_screen_pointer + (x + SCREEN_BIOS_TTY_INIT) * 2) = 0;
        *(current_screen_pointer + (x + SCREEN_BIOS_TTY_INIT) * 2 + 1) = 0;
    }
    bios_tty_show_caret();
}


size_t kprint_tty(uint8_t*msg){
    if(current_screen_mode == BIOS)
    {
        return bios_tty_print(msg);
        bios_current_row = SCREEN_BIOS_ROWS-2;
    }
    else
    {
        //future screen modes should be called from here
    }
}

size_t bios_tty_print(uint8_t * msg){
    bios_tty_hide_caret();
    int i = 0;
    while(msg[i] != '\0' && bios_tty_current_col < SCREEN_BIOS_COLS) {
        *(current_screen_pointer + (bios_tty_current_col + SCREEN_BIOS_TTY_INIT) * 2)      = msg[i] & 0xFF;
        *(current_screen_pointer + (bios_tty_current_col + SCREEN_BIOS_TTY_INIT) * 2 + 1)  = 0x7;
        bios_tty_current_col++;
        i++;
    }
    //caret
    bios_tty_show_caret();
    return SCREEN_BIOS_COLS - bios_tty_current_col;
}

void bios_tty_show_caret(){
    if(bios_tty_caret_position < SCREEN_BIOS_COLS){
        bios_tty_hided_caracter = *(current_screen_pointer + (bios_tty_current_col + SCREEN_BIOS_TTY_INIT) * 2) &0xFF;
        *(current_screen_pointer + (bios_tty_current_col + SCREEN_BIOS_TTY_INIT) * 2)      = 0x01;
        *(current_screen_pointer + (bios_tty_current_col + SCREEN_BIOS_TTY_INIT) * 2 + 1)  = 0x7;
    }
    return;
}

void bios_tty_hide_caret(){
    if(bios_tty_current_col+1 < SCREEN_BIOS_COLS){
        *(current_screen_pointer + (bios_tty_current_col + SCREEN_BIOS_TTY_INIT) * 2)      = bios_tty_hided_caracter & 0xFF;
        *(current_screen_pointer + (bios_tty_current_col + SCREEN_BIOS_TTY_INIT) * 2 + 1)  = 0x7;
    }
    return;
}


void kprint_tty_backspace() {
    if(current_screen_mode == BIOS)
    {
        bios_tty_backspace();
    }
    else
    {
        //future screen modes should be called from here
    }
}

void bios_tty_backspace(){
    bios_tty_hide_caret();
    if(bios_tty_current_col > 0){
        bios_tty_current_col--;
        *(current_screen_pointer + (bios_tty_current_col + SCREEN_BIOS_TTY_INIT) * 2)      = ' ';
        *(current_screen_pointer + (bios_tty_current_col + SCREEN_BIOS_TTY_INIT) * 2 + 1)  = 0x7;
    }
    bios_tty_show_caret();
    return;
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
    if(bios_current_row == SCREEN_BIOS_TTY_ROW)
    {
        //if in the last row.
        bios_current_row--;
        bios_scrllscreen();
    }
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
}
/* scroll up this screen */
void bios_scrllscreen(){
    //move up the whole screen
    int32_t i,j;
    for(i=0; i < SCREEN_BIOS_TTY_ROW - 1; i++)
    {
        for(j=0; j<SCREEN_BIOS_COLS; j++)
        {
           *(current_screen_pointer + (j + i * SCREEN_BIOS_COLS) * 2) = *(current_screen_pointer + (j + (i+1) * SCREEN_BIOS_COLS) * 2);
           *(current_screen_pointer + (j + i * SCREEN_BIOS_COLS) * 2 + 1) = *(current_screen_pointer + (j + (i+1) * SCREEN_BIOS_COLS) * 2 + 1); 
        }
    }
    
    //cleaning the last line , it's neater.
    for(j=0; j < SCREEN_BIOS_COLS; j++)
    {
        *(current_screen_pointer + (j + bios_current_row * SCREEN_BIOS_COLS) * 2) = 0;
        *(current_screen_pointer + (j + bios_current_row * SCREEN_BIOS_COLS) * 2 + 1) = 0;
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
    for(i=0; i<SCREEN_BIOS_TTY_ROW - 1; i++)
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


