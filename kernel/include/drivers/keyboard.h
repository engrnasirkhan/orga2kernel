#ifndef __KEYBOARD__H__
#define __KEYBOARD__H__

/* KEYBOARD */
typedef uint8_t key;
typedef uint8_t key_control;
typedef void (*key_handler_t)(key);
struct key_handler_node {
        uint8_t tecla;
        key_handler_t handler;
        void* next;
};

typedef struct key_handler_node key_handler_node_t;

int irq_keyboard( struct registers * );
int key_register(key_handler_t , key );
int key_init();
#endif // __KEYBOARD__H__
