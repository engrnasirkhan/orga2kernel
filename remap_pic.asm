%define PIC1 0x20
%define PIC2 0xA0
ICW1:	db 0x11
ICW4:	db 0x01
ICW_AUX:	db 	0x0
;; 	init_pics()
;; 	  init the PICs and remap them
;; init_pics(0x20, 0x28)	
;; (int pic1, int pic2) 
init_pics:	
	pusha

	mov ebp, esp
	add ebp, 8*4 		;ebp queda apuntando a eip
	
	push ds
	mov ax, 0x10
	mov ds, ax
	
;; 	send ICW1
;; 	outb(PIC1, ICW1)	;
;; 	outb(PIC2, ICW1)	;
	lea esi, [ICW1]
	mov dx, PIC1
	outsb			;dx <- [ds:esi]
	
	lea esi, [ICW1]
	mov dx, PIC2
	outsb


;; 	send ICW2 
;; 	outb(PIC1 + 1, pic1)	;	/* remap */
;; 	outb(PIC2 + 1, pic2)	;	/*  pics */
	mov eax, ebp
	mov ebx, ebp
	add eax, 4		; pic1 param address 
	add ebx, 8		; pic2 param address 

	mov esi,eax		; ds:esi -> pic1
	mov dx, PIC1
	inc dx
	outsb

	mov esi, ebx		; ds:esi -> pic1
	mov dx, PIC2
	inc dx
	outsb

;; 	send ICW3
;; 	outb(PIC1 + 1, 4)	;	/* IRQ2 -> connection to slave */
;; 	outb(PIC2 + 1, 2)	;
	mov edi, 4
	lea esi, [ICW_AUX]
	mov [esi], edi
	mov dx, PIC1
	inc dx
	outsb

	mov edi, 2
	lea esi, [ICW_AUX]
	mov [esi], edi
	mov dx, PIC2
	inc dx
	outsb

;; 	send ICW4 
;; 	outb(PIC1 + 1, ICW4)	;
;; 	outb(PIC2 + 1, ICW4)	;
	lea esi, [ICW4]
	mov dx, PIC1
	inc dx
	outsb			;dx <- [ds:esi]
	
	lea esi, [ICW4]
	mov dx, PIC2
	inc dx
	outsb			;dx <- [ds:esi]

;; 	disable all IRQs
;; 	outb(PIC1 + 1, 0xFF)
	mov edi, 0xff
	lea esi, [ICW_AUX]
	mov [esi], edi
	mov dx, PIC1
	inc dx
	outsb
	
	pop ds
	popa
	ret
	
	