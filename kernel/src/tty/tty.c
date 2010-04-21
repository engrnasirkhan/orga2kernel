#include <asm/types.h>
#include <screen/screen.h>
#include <drivers/keyboard.h>
#include <tty/tty.h>
#include <mem/vmm.h>
#include <kernel/panic.h>
#include <drivers/keyboard.h>

struct tty_tty_node *  tty_tty_list = NULL;
struct tty_tty_node *  tty_tty_actual = NULL;
extern char * scan2ascii;
uint8_t * prompt = "~$ ";

//82 = insert
//83 = del
//71 = home
//79 = end
//73 = re Pag
//81 = av pag
//59~68 = F1 ~ F10
/** recibe la pulsacion de una tecla */
void tty_put_key(key tecla){
        char * letra = "X\0";
        char * tab = "    \0";
        bool isChar = true;
        letra[0] = scan2ascii[tecla];
        if(letra[0] == '\n'){
            tty_tty_actual->buff[tty_tty_actual->buff_pos] = '\n';
            tty_tty_actual->buff[tty_tty_actual->buff_pos+1] = '\0';
            kprint(tty_tty_actual->buff);
            kprint_tty_clear();
            if(tty_tty_actual->expects_string) {
                tty_tty_actual->expects_string = false;
                tty_tty_actual->buffer_out(tty_tty_actual->buff);
            }
            tty_tty_actual->buff_pos = 0;
            tty_tty_actual->buff[0] = '\0';
            kprint_tty(prompt);
        } else if(letra[0] == '\b') {
            if(tty_tty_actual->buff_pos != 0) {
                kprint_tty_backspace();
                tty_tty_actual->buff_pos--;
            }
        } else if(letra[0] == '\t') {
            size_t queda = kprint_tty(tab);
            tty_tty_actual->buff[tty_tty_actual->buff_pos] = ' ';
            tty_tty_actual->buff[tty_tty_actual->buff_pos+1] = ' ';
            tty_tty_actual->buff[tty_tty_actual->buff_pos+2] = ' ';
            tty_tty_actual->buff[tty_tty_actual->buff_pos+3] = ' ';
            tty_tty_actual->buff_pos += 4;
            if(queda <= 0){
                tty_tty_actual->buff_pos = 0;
                kprint_tty_clear();
            }
        } else if(letra[0] != ' ' || tecla == 57){
            size_t queda = kprint_tty(letra);
            tty_tty_actual->buff[tty_tty_actual->buff_pos] = letra[0];
            tty_tty_actual->buff_pos++;
            
            if(queda <= 0){
                tty_tty_actual->buff_pos = 0;
                kprint_tty_clear();
            }
        } else {
            isChar = false;
            kprint("Usted apreto la tecla numero: %d\n", tecla);
            kprint("tecla desconocida, se agradece si usted la agrega\n");
        }
        
        if(tty_tty_actual->buff_pos == MAX_TTY){
            tty_tty_actual->buff_pos = 0;
        }
        tty_tty_actual->buff[tty_tty_actual->buff_pos] = '\0';
        if(isChar && tty_tty_actual->expects_char) {
            tty_tty_actual->buffer_out(letra);
            tty_tty_actual->expects_char = false;
        }
}

/** inicializa las tty */
int tty_init(tty_t* tty, tty_buffer_out_f out_f) {
    tty_tty_add(tty, out_f);
    tty_tty_change(tty);
    if(tty_tty_actual == NULL){
        return true;
    }
    kprint_tty(prompt);
    return false;
}

/** encuentra el struct correspondiente a esta tty */
struct tty_tty_node * tty_tty_find(tty_t* tty){
    struct tty_tty_node * node = tty_tty_list;
    while(node != NULL){
        if(node->tty == *tty){
            return node;
        }
        node = (struct tty_tty_node *)node->next;
    }
    return NULL;
}

/** cambia de tty */
int tty_tty_change(tty_t* tty_number){

    //encuentro la proxima
    struct tty_tty_node * proximo = tty_tty_find(tty_number);
    if(proximo == NULL){
        return true;
    }
    //desarmo la actual
    kprint_tty_clear();
    
    //armo la proxima.
    tty_tty_actual = proximo;
    kprint_tty(prompt);
    kprint_tty(tty_tty_actual->buff);
    
    return false;
}

/** agrega una tty , recibe como entrada tty_buffer_out*/
int tty_tty_add(tty_t* tty, tty_buffer_out_f out_f){
    struct tty_tty_node * node = kmalloc(sizeof(struct tty_tty_node));
    if(node == NULL){
        //si fallo malloc
        kprint("fallo kmalloc\n");
        return true;
    }
    if(tty_tty_list == NULL){
        //si es el primer elemento
        *tty = 1;
    } else {
        // le doy elsiguiente.
        *tty = tty_tty_list->tty + 1;
    }
    node->tty = *tty;
    node->buff[0] = '\0';
    node->buff_pos = 0;
    node->expects_char = false;
    node->expects_string = false;
    node->enabled = true;
    node->buffer_out = out_f;
    node->next = (void*)tty_tty_list;
    tty_tty_list = node;
    return false;
}


bool tty_get_key(tty_t* tty){
    if(tty_tty_actual->expects_char){
        return true;
    }
    tty_tty_actual->expects_char = true;
    return false;
}

bool tty_get_string(tty_t* tty){
    if(tty_tty_actual->expects_string){
        return true;
    }
    tty_tty_actual->expects_string = true;
    return false;
}
