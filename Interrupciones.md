# Funciones #
Listado de funciones:
  * `bool set_irq_handler( int irq, void (*func)(int irq, regs_t *regs) )`
  * `bool set_exception_handler( int exception, void (*func)(int irq, regs_t *regs) )`
  * `void cli()`
  * `void sti()`
  * `void remap_irqs( int a_partir )`
  * `void init_interrupts()`
  * `void ack_irq( int irq )`
  * `void mask_irq( int irq )`
  * `void unmask_irq( int irq )`
  * `void set_trap_gate( int vector, uint16_t cs, uint32_t eip, int privilege )`
  * `void set_task_gate( int vector, uint16_t tss, int privilege )`
  * `void set_int_gate( int vector, uint16_t cs, uint32_t eip, int privilege )`

## set\_irq\_handler ##
Esta función establece como handler de la IRQ, la función pasada como parámetro.

#### Argumentos ####
  * **irq**: La IRQ que se desea manejar.
  * **func**: El handler (si es NULL se quita el handler actual).

#### Devuelve ####
  * **true**: Si está todo OK.
  * **false**: Si hubo un error.

#### Razones para fallar ####
  * **irq** fuera de rango (entre 0 y 15)
  * ¿**irq** ya mapeada?

## set\_exception\_handler ##
Esta función establece como handler de la excepción, la función pasada como parámetro.

#### Argumentos ####
  * **exception**: La excepción que se desea manejar.
  * **func**: El handler (si es NULL se quita el handler actual).

#### Devuelve ####
  * **true**: Si está todo OK.
  * **false**: Si hubo un error.

#### Razones para fallar ####
  * **exception** fuera de rango (entre 0 y 31)
  * ¿**exception** ya mapeada?

## cli ##
Deshabilita todas las interrupciones, se traduce en una llamda directa a **cli**. Podría ser implementada como una macro. Ej:

`#define cli() __asm__ __volatile__ ( "cli" )`

## sti ##
Habilita todas las interrupciones, se traduce en una llamda directa a **sti**. Podría ser implementada como una macro. Ej:

`#define sti() __asm__ __volatile__ ( "sti" )`

## remap\_irqs ##
Remapea las IRQs a partir del vector que le indiquemos como parámetro. Esto es necesario para evitar que las excepciones del microprocesador y las interrupciones se superpongan.

#### Argumentos ####
  * **a\_partir**: El vector de interrupción al partir del cual mapear las interruciones.

#### Devuelve ####
Nada.

#### Razones para fallar ####
Ninguna.

## init\_interrupts ##
Inicializa el sistema de interrupciones y excepciones, entre sus tareas se encuentran:
  * Establecer la IDT.
  * Establecer handlers para **todas** las excepciones.
  * Establecer un handler común para **todas** las IRQs.
  * Remapear las IRQs.
  * Enmascarar **todas** las IRQs.
  * Habilitar las interrupciones.

#### Argumentos ####
Ninguno.

#### Devuelve ####
Nada.

#### Razones para fallar ####
Ninguna.

## ack\_irq ##
Le indica al controlador de interrupciones que ya se manejo la interrupción pasada como parámetro.

Por lo menos al PIC no es necesario indicarle cuál fue la interrupción que se manejó, pero necesitamos saber cuál fue para determinar si avisarle al PIC maestro o al maestro y al esclavo. Otros controladores de interrupciones podrían tener un sistema más fino donde í sea necesario indicarle la IRQ adecuada.

#### Argumentos ####
  * **irq**: La IRQ que se acaba de manejar.

#### Devuelve ####
Nada.

#### Razones para fallar ####
Ninguna.

## mask\_irq ##
Enmascara una interrupción, interactuando con el PIC.

#### Argumentos ####
  * **irq**: La IRQ que se desea enmascarar.

#### Devuelve ####
Nada.

#### Razones para fallar ####
Ninguna.

## unmask\_irq ##
Desenmascara una interrupción, interactuando con el PIC.

#### Argumentos ####
  * **irq**: La IRQ que se desea desenmascarar.

#### Devuelve ####
Nada.

#### Razones para fallar ####
Ninguna.


## set\_trap\_gate ##
Establece una puerta de "trampa".

#### Argumentos ####
  * **vector**: El vector de la IDT donde colocarlo.
  * **cs**: El segmento de código a utilizar.
  * **eip**: La dirección del handler.
  * **privilege**: El privilegio que se le quiere asignar.

#### Devuelve ####
Nada.

#### Razones para fallar ####
Ninguna.

## set\_task\_gate ##
Establece una puerta de tarea.

#### Argumentos ####
  * **vector**: El vector de la IDT donde colocarlo.
  * **tss**: El descriptor de TSS a utilizar.
  * **privilege**: El privilegio que se le quiere asignar.

#### Devuelve ####
Nada.

#### Razones para fallar ####
Ninguna.


## set\_int\_gate ##
Establece una puerta de interrupción.

#### Argumentos ####
  * **vector**: El vector de la IDT donde colocarlo.
  * **cs**: El segmento de código a utilizar.
  * **eip**: La dirección del handler.
  * **privilege**: El privilegio que se le quiere asignar.

#### Devuelve ####
Nada.

#### Razones para fallar ####
Ninguna.

# Notas sobre set\_irq\_handler #
Esta función recibe dos argumentos, uno es el IRQ y otro es un puntero a una estructura reg\_t. De momento, no podemos saber que argumentos debe recibir, pero he aquí la justificación de cada uno:

### Parámetro irq ###
Uno podriá querer, con un mismo handler, manejar varias interrupciones. ¿Por qué? Veamos el listado de interrupciones del PIC:

| **IRQ** | **Description** |
|:--------|:----------------|
| 0       | Programmable Interrupt Timer Interrupt |
| 1       | Keyboard Interrupt |
| 2       | Cascade (used internally by the two PICs. never raised) |
| 3       | COM2 (if enabled) |
| 4       | COM1 (if enabled) |
| 6       | Floppy Disk     |
| 7       | Unreliable "spurious" interrupt (usually) |
| 8       | CMOS real-time clock (if enabled) |
| 14      | Primary ATA Hard Disk |
| 15      | Secondary ATA Hard Disk |

Por lo que tanto el controlador de disco rígido, como el del puerto serie, podrían querer establecer un solo handler para dos interrupciones. Luego, necesitan una forma de identificarlo.

### Parámetro reg\_t ###
En los experimentos que hice, por lo general pusheaba en la pila un conjunto de registros, que luego accedía con un puntero. Quizás una IRQ no lo necesita, pero para un manejador de excepción puede ser interesante tener acceso al ESP e EIP de la tarea actual.