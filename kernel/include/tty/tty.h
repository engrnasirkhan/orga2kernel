#ifndef __TTY__H__
#define __TTY__H__

#include <asm/types.h>
#define MAX_TTY 80
typedef void (*tty_buffer_out_f)(uint8_t *);
typedef uint32_t tty_t;
struct tty_tty_node {
    
    tty_t tty;
    uint8_t buff[MAX_TTY+2];
    uint32_t buff_pos;
    bool expects_char;
    bool expects_string;
    bool enabled;
    tty_buffer_out_f buffer_out;
    void * next;
};

/*escribe un caracter en el prompt */
void tty_put_key(key);
/*el proximo caracter que se presione se enviara a travez del tty_buffer_out_f asociado
 * solo si no se pidio */
bool tty_get_key(tty_t* tty);
/*la proxima linea que se ingrese se enviara a travez del tty_buffer_out_f asociado
 * solo si no se pidio */
bool tty_get_string(tty_t* tty);
/* inicializa el tty con 1 TTYs al comienzo */
int tty_init(tty_t* tty, tty_buffer_out_f out_f);
/* agrega un tty con un buffer asociado, tty es de salida, devuelve el tty asociado a esta vista. */
int tty_tty_add(tty_t* tty, tty_buffer_out_f out_f);
/* cambia al tty designado */
int tty_tty_change(tty_t* tty);





#endif // __TTY__H_
