# Funciones #
  * ` void initPaging() `
  * ` int getFreeFrame( uint32_t *frame, int from ) `
  * ` int putFrame( uint32_t frame ) `
  * ` int getFreePage( va *address ) `
  * ` int mapPage( int frame, va address ) `
  * ` int unMapPage( va address ) `
  * ` va pa2va( pa address ) `
  * ` pa va2pa( va address ) `

# Mapa de la Memoria Física #
El kernel se carga en la dirección física 1MB. Su heap crece hacia arriba.

Cada proceso va a tener 8KB (2 páginas) dedicados a su pila en modo kernel.

Inicialmente debemos tener una pila provisoria mientras el kernel se inicia, para ello podemos mapear una página en algún lugar de la memoria pero no quitarla de la lista de marcos de página libres

Los procesos de usuario ocupan la parte alta de la memoria y crecen hacia abajo.

De momento, no es del todo importante esta convención.

# Mapa de la Memoria Virtual #
Podemos tener cada proceso dividido de manera tal que el kernel ocupe el último giga del espacio de direccionamiento y el proceso de usuario los primeros 3GB (igual que Linux). O alguna otra división (ej: 2GB/2GB).

El proceso de usuario no debe comenzar en las primeras páginas. Debemos dejar alguna zona reservada al inicio para capturar las desreferencias a punteros nulos.

El heap del usuario crece hacia arriba, y la pila crece, desde los 3GB hacia abajo.

En el espacio de direccionamiento de 1GB (o 2GB), el kernel posee mapeada toda la memoria física (siempre y cuando esa sea la cantidad de memoria que hay).

**NOTA:** El directorio de páginas de cada proceso debería tener en su último giga un puntero a la tabla de páginas del kernel (de modo que cada proceso comparta exactamente la misma imagen del kernel).

# Algoritmos #
Para esta primera iteración podemos tener algo bien sencillo.

Podríamos tener un mapa de bits para llevar la cuenta de los marcos de página libres. De esa forma, cada bit representa 1 página. Si tenemos un sistema con 2GB de RAM, ¿cuántos marcos de página tenemos? 2 GB / 4 KB/página = 524.288 marcos de página. Es decir, 524.288 bits, si lo dividimos por 8: 524.288 bits / 8 Bytes/bit = 65536 Bytes = 64K. Con 64K podemos manejar facilmente 2GB.

La interfaz para manejar la memoria libre podría ser algo así:
```
struct FreeBitmap {
   uint32_t pages, // Cantidad total de marcos de página (bits en el mapa).
            fpointer, // Un puntero al primer bit libre (o al anterior, para acelerar las cosas).
            lpointer, // Un puntero al último bit libre (o al posterior, para acelerar las cosas).
            free; // Cantidad de marcos de página libres (para estadística)
   unsigned char *map; // El mapa
};

struct FreeBitmap bitmap;

int getFreeFrame( uint32_t *frame, int from ) {
   if ( !frame) return -1;
   if ( from == PRINCIPIO )
      return obtenerDelPrincipio( frame );
   return obtenerDelFinal( frame );
}

#define bit(x,y) ( (y)[(x)>>3] & (1<<((x)&7)) )
#define setbit(x,y) ( (y)[(x)>>3] |= (1<<((x)&7)) )
#defien clearbit(x,y) ( (y)[(x)>>3] &= ~(1<<((x)&7)) )

int obtenerDelPrincipio( uint32_t *frame ) {
   while ( bit( bitmap.fpointer, bitmap.map ) && bitmap.fpointer < bitmap.pages )
      bitmap.fpointer++;
   if ( bitmap.fpointer == bitmap.pages ) {
      bitmap.fpointer = 0;
      while ( bit( bitmap.fpointer, bitmap.map ) && bitmap.fpointer < bitmap.pages )
         bitmap.fpointer++;
      if ( bitmap.fpointer == bitmap.pages ) return -1;
   }
   *frame = bitmap.fpointer;
   setbit( bitmap.fpointer, bitmap.map );
   bitmap.free--;
   return 0;
}

// ...
```

  * Obtener un marco de página: O(n) Hay que buscarlo :-(
  * Liberar un marco de página: O(1) Es simplemente poner el bit en 0

Seguro que utilizando alguna otra estructura se pueden mejorar los tiempos, pero no creo que necesitemos (al menos en primera instancia) unos muy buenos algoritmos, este parece sencillo y consume poca memoria.