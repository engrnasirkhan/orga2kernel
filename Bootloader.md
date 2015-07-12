

# GRUB #
Este bootloader cumple con la especificación [multiboot](http://www.gnu.org/software/grub/manual/multiboot/multiboot.html). Cualquier bootloader que lo haga es capaz de bootear un archivo que cumpla con las siguientes propiedades:

  * Esté en formato ELF
  * Defina 3 enteros de 32 bits (en los primeros 8KB del archivo y alineados a 4 Bytes) con los siguientes datos (y en el siguiente orden):
    * **0x1BADB002**: Un número "mágico" para identificar esta estructura de datos.
    * **flags**: Un conjunto de bits que le indican a GRUB qué cosas hacer (además de cargarnos en memoria)
    * **checksum**: Una suma de comprobación para verificar que realmente se trata de la estructura adecuada. ` checksum = -(flags + 0x1BADB002) `

Si no está en formato ELF, se lo puede indicar en los flags y luego indicarle a mano (agregando campos a la estructura) dónde cargar el código y demás.

## Estado de la Máquina ##
Al pasarle el control a nuestro programa, la máquina se encuentra en el siguiente estado:

  * **eax**: Tiene el valor "mágico" 0x2BADB002.
  * **ebx**: Contiene la dirección física de la estructura que nos pasa el GRUB.
  * **CS**: Segmento de código, RING 0, 0 --> 4GB
  * **DS, ES, FS, GS, SS**: Segmento de datos, RING 0, 0 --> 4GB
  * **Línea A20**: Habilitada
  * **CR0**: Paginación deshabilitado. Modo protegido habilitado.
  * **EFLAGS**: Interrupciones deshabilitadas. VM86 deshabilitado.
  * **ESP**: Indefinido.

## Ejemplo ##

#### main.c ####
```

// La estructura que lee GRUB
struct MultibootHeader {
	unsigned int magic, flags, checksum;
};

// La estructura mediante la cual GRUB nos pasa información.
struct MultibootInformation {
	unsigned int flags, mem_lower, mem_upper;
	unsigned int boot_device;
	char *cmdline;
};

// Número mágico y fags.
#define MAGIC 0x1BADB002
#define MEMORIA 1
#define CMDLINE 4
#define FLAGS (MEMORIA|CMDLINE)

// Definimos la estructura que lee el GRUB (debe estar en los primeros 8KB).
struct MultibootHeader mboot = {
	.magic = MAGIC, // Un número sin sentido
	.flags = FLAGS,
	.checksum = -(FLAGS + MAGIC)
};

// Escribe una cadena en pantalla, bien sencillo.
void puts ( const char *str ) {
	static unsigned short *video = 0xB8000;
	static int x = 0;
	static int y = 0;

	while ( str && *str ) {
		if ( *str == '\n' ) {
			y++; x = 0;
			str++;
			continue;
		}
		video [ y * 80 + x ] = *str | 0x7F00;
		str++;
		x++;
		if (x >= 80) { x = 0; y++; }
	}
}

// Escribe un entero en pantalla.
void put_uint( unsigned int nro ) {
	static char buffer[32];
	static char salida[32];
	char *ptr = buffer;
	char *out = salida;

	if ( nro == 0 ) *ptr++ = '0';

	while ( nro ) {
		*ptr = nro % 10 + '0';
		nro /= 10;
		ptr++;
	}

	--ptr;
	while ( ptr != buffer )
		*out++ = *ptr--;
	*out++ = *ptr;
	*out = 0;
	puts ( salida );
}

// La primer rutina que se ejecuta.
// 1. Establecemos una pila que "crezca" de 640KB hacia abajo.
// 2. Llamamos a main, pasándole como parámetro la estructura
//    que nos pasa GRUB en *ebx*.
void kmain () {
	__asm__ __volatile__ (
		"movl $(640 * 1024), %esp\n\t"
		"pushl %ebx\n\t"
		"call main\n\t"
		"hlt\n\t"
		);
}

void main(struct MultibootInformation *info) {
	puts ( "mem_lower: " );
	put_uint ( info->mem_lower );
	puts ( "\nmem upper: " );
	put_uint ( info->mem_upper );
	puts ( "\nCommand line: " );
	puts ( info->cmdline );

	while (1)
		__asm__ __volatile__ ( "hlt" );
}
```


#### Makefile ####
```
TARGET = kernel
CFLAGS = -m32 -Wall -ffreestanding -nostdlib -Wl,-e,kmain -Wl,-Ttext,0x100000
SOURCES = main.c

.PHONY: all clean

all: $(TARGET)
clean:
	rm -f $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)
```

Esto basicamente le dice al compilar y linkear:
  * **-m32**: Compilate como código de 32 bits.
  * **-ffreestanding**: Estás corriendo por tu cuenta, no hay ningún sistema operativo.
  * **-nostdlib**: No linkees ninguna librería que no te indique.
  * **-Wl,-e,kmain**: El punto de entrada es la función _kmain_
  * **-Wl,-Ttext,0x100000**: Cargate en la dirección física 1MB

#### Prueba bajo QEMU/BOCHS ####
```
$ dd if=/dev/zero of=disk.img bs=1024 count=1440
$ mke2fs disk.img (o el sistema de archivos que más les guste)
$ su 
$ modprobe loop; mount -o loop disk.img mnt/
$ mkdir -p mnt/boot/grub
$ cp -r /boot/grub/* boot/grub
$ editar mnt/boot/grub/menu.lst
# Colorcitos "lindos"
color cyan/blue white/blue

title           Mi Kernel
root            (fd0)
kernel          /kernel parametros del kernel :-)
$ cp kernel mnt/
$ umount mnt
$ grub
grub> device (fd0) disk.img
grub> root (fd0)
grub> setup (fd0)
grub> quit
$ exit
$ qemu -fda disk.img
```


## Ventajas y Desventajes ##
Ventajas:
  * Relativamente sencillo de aprender.
  * Puede bootear de practicamente cualquier medio.
  * Reconoce montones de sistemas de archivos: FAT, ext2, ext3, reiserfs, xfs, iso9660, etc.
  * Obtiene el mapa de memoria del BIOS y nos los pasa a nosotros.
  * Puede bootear archivos ELFs.
Desventajas:
  * Si bien sencillo, también requiere un poco de "setup" extra.
  * Muy pesado, demasiada funcionalidad para lo que podríamos necesitar.

# Lector de Sectores del Disco #
```
BITS 16
ORG 0x7C00
   
jmp empeza

empeza:
            mov ax, cs             ; Actualizo los registros de segmento
            mov ds, ax
            mov es, ax


    reset:                      ; Reseteo floppy
            mov ax, 0           ; AH= 0 para resetear
            mov dl, 0           ; DL= primer floppy
            int 13h             ;
            jc reset            ; Si hay error, intenta resetear infinitas veces


    read:
            mov ax, 0           ;
            mov es, ax          ;
            mov bx, 0x1200      ;  Esta es la direccion donde empieza a guardar los siguientes sectores

            mov ah, 2           ; Para leer del diskette lo guarda en el buffer apuntado por es:bx
            mov al, 5           ; Cantidad de sectores que cargo
            mov cl, 2           ; primer sector a copiar (empieza en 1)= 2
            mov ch, 0           ; Cylinder=0
            mov dh, 0           ; Head=0
            mov dl, 0           ; Drive=0
            int 13h             ;

            jc read             ; si hay error que intente de nuevo  
            jmp 0x120:0000      ; salta a la posicion de memoria para empezar a ejecutar el kernel



            times 510-($-$$) db 0x00    ; hacemos que el disco sea "booteable"
            dw 0xAA55
```

## Ventajas y Desventajas ##
Ventajas:
  * Sencillo de entender.
  * Sencillo de empezar a utilizar.
  * Extremadamente liviano.
  * La sensación de "lo hizimos nosotros" :P
  * Se pueden elegir los distintos drives cambiando el registro dl. La especificacion de como hacerlo esta en http://courses.ece.illinois.edu/ece390/books/artofasm/CH13/CH13-2.html#HEADING2-73

Desventajas:
  * No respeta ningún filesystem.
  * No provee información acerca del mapa de memoria.
  * No pasa argumentos al kernel.
  * No es flexible como GRUB.

# FAT32 Bootloader #
Tomado de http://forum.osdev.org/viewtopic.php?f=1&t=18519

```
;; Stage 1 bootloader for Firebird O.S
;;  FAT32 version
;;  Version 1.1

;; To do:
;;  Figure real first data sector

org 0x7C00
bits 16

BS_jmpBoot:
	jmp start
	nop

;; FAT fields, labeled here for convenience
BS_OEMName:
	dd 16843009
	dd 16843009
BPB_BytsPerSec:
	dw 257
BPB_SecPerClus:
	db 1
BPB_ResvdSecCnt:
	dw 257
BPB_NumFATs:
	db 1
BPB_RootEntCnt:
	dw 257
BPB_TotSec16:
	dw 257
BPB_Media:
	db 1
BPB_FATSz16:
	dw 257
BPB_SecPerTrk:
	dw 257
BPB_NumHeads:
	dw 257
BPB_HiddSec:
	dd 16843009
BPB_TotSec32:
	dd 16843009
BPB_FATSz32:
	dd 16843009
BPB_ExtFlags:
	;; Bits 0-3 = Active FAT, 7 = !FAT mirroring
	dw 257
BPB_FSVer:
	dw 257
BPB_RootClus:
	dd 16843009
BPB_FSInfo:
	dw 257
BPB_BkBootSec:
	dw 257
BPB_Reserved:
	dd 16843009
	dd 16843009
	dd 16843009
BS_DrvNum:
	db 1
BS_Reseved1:
	db 1
BS_BootSig:
	db 1
BS_VolID:
	dd 16843009
BS_VolLab:
	dd 16843009
	dd 16843009
	dw 257
	db 1
BS_FilSysType:
	dd 16843009
	dd 16843009

start:
	cli
	;; Set CS, DS, ES, & SS to a known value
	;; I used eax instead of just ax because I want to have the upper word cleared later
	xor eax, eax
	jmp 0:loadCS
loadCS:
	mov ds, ax
	mov es, ax
	mov ss, ax
	
	;; set up the stack ( a temporary 512-byte stack )
	mov sp, 0x8000
	mov bp, sp
	
	;; Save the boot drive
	;; BootDrive = dl
	push dx
	
	;; calculate the first data sector
	;; FirstDataSector = BPB_NumFATs * BPB_FATSz32 + BPB_ResvdSecCnt
	mov al, [BPB_NumFATs]
	mov ebx, [BPB_FATSz32]
	mul ebx
	xor ebx, ebx
	mov bx, [BPB_ResvdSecCnt]
	add eax, ebx
	;; eax = first data sector, for now let's just assume it's relative to MBR
	push eax
	
	;; now let's get the location of the first FAT
	;; FATsector = BPB_ResvdSecCnt
	xor eax, eax
	mov ax, [BPB_ResvdSecCnt]
	push eax
	
	;; BytsPerCluster = BPB_BytsPerSec * BPB_SecPerClus
	mov ax, [BPB_BytsPerSec]
	mov bx, [BPB_SecPerClus]
	mul bx
	push eax
	
	;; FATClusterMask = 0x0FFFFFFF
	mov eax, 0x0FFFFFFF
	push eax
	
	;; FATEoFMask = FATClusterMask & 0xFFFFFFF8
	and al, 0xF8
	push eax
	
	;; CurrentCluster = BPB_RootClus
	mov eax, [BPB_RootClus]
	push eax
	
	;; Stack is now as follows:
BootDrive 		equ BP- 2
FirstDataSector	equ BP- 6
FATsector			equ BP-10
BytsPerCluster		equ BP-14
FATClusterMask		equ BP-18
FATEoFMask		equ BP-22
CurrentCluster		equ BP-26
	
	;; Get the first root directory cluster
	mov eax, [CurrentCluster]
	mov di, bp
	call readCluster
	
	;; parse first cluster to see if it has what we want
nextDirCluster:
	;; Set bx to the end of the cluster
	mov bx, [BytsPerCluster]
	add bx, bp
	;; ax = 0x8000 - sizeof(FAT_DIR_entry)
	;; this simplifies the upcoming loop a bit
	mov ax, 0x7FE0

findloop:
	;; move to next entry
	add ax, 32
	;; check if we're at the end of the cluster
	cmp ax, bx
	;; if so, handle it
	jz notFound
	
	;; else let's check the entry
	mov si, ax
	mov di, fileName
	mov cx, 11
	;; compare names
	repe cmpsb
	;; if not the same, try next entry
	jnz findloop
	
	;; file found!
	;; +9 instead of +20 because SI is already 11 bytes into the entry
	mov ax, [si+9]
	;; push the low word
	push ax
	mov ax, [si+15]
	;; now push the high word
	push ax
	;; and pop the double word
	pop eax
	;; eax = cluster of file
	mov di, bp
	call readCluster
loadFileLoop:
	;; if we're already at the EoF, this won't read anything
	call readNextCluster
	;; it will, however, set the carry flag to indicate that we're at the EoF
	;; so if( !cf )
	;;  keep loading clusters
	jnc loadFileLoop
	;; else
	;;  jump to the second stage
	jmp bp

notFound:
	;; try reading the next cluster
	call readNextCluster
	jnc nextDirCluster
	;; if this was the last one
	;; show an error
	mov si, noFile
	call putStr
	;; and stop
	hlt
	
;; A few useful routines	
putStr:
	;; ah = useTeletype
	mov ah, 0x0E
	;; bh = use page 0, bl = useless in Bochs
	xor bx, bx
loopPut:
	;; lodsb == mov al, si \ inc si
	;; too bad it doesn't do "or al, al" also
	lodsb
	;; check if al == 0
	or al, al
	;; if so, we're done
	jz donePut
	;; otherwise, let's put it
	int 10h
	;; and go again for the next character
	jmp loopPut
donePut:
	ret

;;  readNextCluster :: ES:DI = dest
readNextCluster:
	;; Check if we're already at the EoF
	mov eax, [CurrentCluster]
	and eax, [FATClusterMask]
	cmp eax, [FATEoFMask]
	;; if so, bail
	jz eofLoad
	
	;; now, let's get the number of
	;; cluster pointers in a sector
	xor ebx, ebx
	mov bx, [BPB_BytsPerSec]
	shr bx, 2
	;; now that we have that, let's figure
	;; out the offset, in sectors, of the
	;; current cluster pointer
	xor edx, edx
	div eax, ebx
	;; now that we have those, let's save them
	push ebx
	push eax
	;; add in the location of the first FAT sector
	;; to get the location on disk
	add eax, [FATsector]
	;; convert to the BIOS's format
	call getCHS
	;; figure out where to put it
	mov bx, 0x7C00
	sub bx, [BPB_BytsPerSec]
	xor esi, esi
	;; save it for later
	mov si, bx
	mov al, 1
	call readDisk
	
	pop eax
	pop ebx
	;; eax = how many sectors into the FAT to look
	;; ebx = clusterpointers per sector
	
	;; let's figure out the memory offset
	;; for the cluster pointer
	mul ebx
	mov ebx, [CurrentCluster]
	and ebx, [FATClusterMask]
	sub ebx, eax
	sal ebx, 2
	;; offset the pointer
	add esi, ebx
	;; load the next cluster
	mov eax, [esi]
	;; and set it to the new current cluster
	mov [CurrentCluster], eax
readCluster:
	;; Get the first sector of the cluster
	;; Sector = (cluster - 2) * BPB_SecPerClus + FirstDataSector
	and eax, [FATClusterMask]
	dec eax
	dec eax
	xor ebx, ebx
	mov bl, [BPB_SecPerClus]
	mul ebx
	add eax, [FirstDataSector]
	;; eax = first sector of cluster
	
	;; convert to CHS to load directory
	call getCHS
	
	;; fetch the cluster and put it where the user wanted
	mov bx, di
	mov al, [BPB_SecPerClus]
	call readDisk

	add di, [BytsPerCluster]
	
	;; clear the EoF flag and return
	clc
	ret
	
eofLoad:
	;; set the EoF flag and return
	stc
	ret

getCHS:
	;; Borrowed from http://en.wikipedia.org/wiki/CHS_conversion
	xor dx, dx
	mov bx, [BPB_SecPerTrk]
	div ax, bx
	inc dx
	mov cl, dl
	xor dx, dx
	mov bx, [BPB_NumHeads]
	div ax, bx
	mov ch, al
	mov dh, dl
	sal ah, 6	
	or ah, cl
	mov cl, ah
	ret

readDisk:
	;; set the error count to 0
	xor ah, ah
	push ax
.tryLoop:
	mov ah, 0x02
	mov dl, [BootDrive]
	
	int 13h
	jc .readError
	pop ax
	ret

.readError:
	;; whoops! there was an error reading the drive!
	;; Let's check and see if we've already tried too many times
	
	;; grab the value
	pop ax
	inc ah
	push ax
	
	;; It just so happens that the number of times we want to loop
	;; also can be used as the bit mask for comparing. How useful!
	and ah, 0x04
	jnz .freeze
	jmp .tryLoop
	
.freeze:
	;; Too many failures, display an error and freeze
	mov si, readFail
	call putStr
	hlt
	
;; Some basic data

readFail:
	db "Couldn't read drive!", 0
noFile:
	db "Couldn't find /FBOOT.SYS!",0

fileName:
	db "FBOOT   SYS"

;; FAT layout:
; DIR_name:
;	 db "FILE    EXT"
; DIR_Attr:
;	 db 1	; ReadOnly = 0x01, Hidden = 0x02, System = 0x04, VolID = 0x08, Dir = 0x10, Arch = 0x20, LongName = 0x0F
; DIR_NTRes:
;	 db 1
; DIR_CrtTimeTenth:
;	 db 1
; DIR_CrtTime:
;	 dw 257
; DIR_CrtDate:
;	 dw 257
; DIR_LstAccDate:
;	 dw 257
; DIR_FstClusHI:
;	 dw 257
; WrtTime:
;	 dw 257
; WrtDate:
;	 dw 257
; FstClusLO:
;	 dw 257
; FileSize:
;	 dd 16843009

	times 510-($-$$) db 0
	db 0x55, 0xAA		;; Standard end of boot sector
```

## Ventajas y Desventajas ##
Ventajas:
  * Sencillo.
  * Pequeño.
  * Reconoce al menos un filesystem.

Desventajas:
  * Reconoce sólo un filesystem.
  * Requiere modificaciones al código fuente para cumplir nuestras espectativas.
  * No provee información acerca del mapa de memoria.
  * No pasa argumentos al kernel.
  * No es flexible como GRUB.