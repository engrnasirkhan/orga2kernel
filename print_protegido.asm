global print 

%define param(x) ebp+(x*4)

print:
	pusha
	mov ebp, esp
	add ebp, 0x20
	;; ebp queda apuntando al eip de retorno	
	push es
	push ds	

	;; corro todas las lineas
	;; de la pantalla hacia arriba
	mov ax, 0x18
	mov es, ax
	mov ds, ax

	xor edi, edi
	mov esi, 80<<1 		;80 columnas de word

	mov ecx, 80*24
	rep movsw		;subi todas las lineas para arriba
	
	;; ahora limpio la ultima linea
	xor eax, eax
	mov ecx, 80
	rep stosw 
	
	;; apunto es al segmento de datos
	;; junto con es:edi apunto al string
	mov ax, 0x10
	mov es, ax
	
 	mov edi, [param(1)]		;puntero al string
	
	;; calculo el largo del string que al final tiene un 0
	cld 			;limpio el flag de direccion
	xor ecx, ecx
	not ecx
	
	xor al, al
	repnz scasb

	mov ebx, ecx
	xor ecx, ecx
	not ecx
	sub ecx, ebx
	;; en ecx tengo la cantidad de caracteres
	;; ds:esi puntero al string
	;; es:edi puntero a la pantalla
	mov ax, 0x10
	mov ds, ax
 	mov esi, [param(1)]
	
	mov ax, 0x18
	mov es, ax
	mov edi, 80*24*2 	;me paro en la ultima linea

	mov ah, 0x3F
	
.ciclo:	
	lodsb
	stosw
	loop .ciclo
	
	pop ds
	pop es
	popa
	ret