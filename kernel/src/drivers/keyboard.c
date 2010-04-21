
#include <asm/types.h>
#include <drivers/keyboard.h>
#include <asm/asm.h>
#include <tty/tty.h>
#include <screen/screen.h>
#include <mem/vmm.h>

struct key_handler_node * key_handler_list;

struct key_handler_node * key_is_registered(key tecla);

//en el futuro se pueden mas de 1 tecla(shift+...).
int key_register(key_handler_t funcion, key clave){
    key_handler_node_t *node = key_is_registered(clave);
    if(node != NULL){
        node->handler = funcion;
    } else {
        int tamanio = sizeof(key_handler_node_t);
        key_handler_node_t *nuevo = (key_handler_node_t*)kmalloc(tamanio);
        if(nuevo == NULL){
            return 1;
        }
        nuevo->tecla = clave;
        nuevo->handler = funcion;
        nuevo->next = (void*)key_handler_list;
        key_handler_list = nuevo;
    }
    //everything ok.
    return 0;
}


int key_init(){
    key_handler_list = NULL;
    return 0;
}

struct key_handler_node * key_is_registered(key tecla){
    struct key_handler_node * node = key_handler_list;
    while(node != NULL){
        if(node->tecla == tecla){
            return node;
        }
        node = (struct key_handler_node*)node->next;
    }
    return NULL;
}


int irq_keyboard( struct registers *r ) {
	key tecla = inb( 0x60 );
    key_control control = inb(0x64);
    if( !(tecla & 0x80) ){
        //key has been pressed
        struct key_handler_node * handler_node = key_is_registered(tecla);
        //si existe algun handler para esa tecla
        if(handler_node != NULL) {
            handler_node->handler(tecla);
        } else {
            tty_put_key(tecla);
        }
    }
	return 0;
}
