extern print
	
global IDT_DESC

%define ORIGIN 0x100000
	
%define CG.OFFSET(x) \
	(0xffff & x) | ((0xffff_0000 & x)<<32)
%define CG.SELECTOR(x) ((0xffff & x)<<16)
%define CG.TYPE(x) ((x & 1111b) << 40) 
%define CG.PARAM(x) ((1_1111b & x) << 32 )
%define CG.P 1b<<15+32
%define CG.DPL(x) (x & 11b)<<13

%include "isr.asm"

times (0x8-(($-$$) & 111b)) db 0
	
BEGIN_IDT:
	;; isr 0
dq CG.P | CG.TYPE(1110b) | CG.OFFSET( isr0-$$+ORIGIN ) | CG.SELECTOR(0x8)
	;; isr 1
dq CG.P | CG.TYPE(1110b) | CG.OFFSET( isr1-$$+ORIGIN ) | CG.SELECTOR(0x8)
	;; isr 2
dq CG.P | CG.TYPE(1110b) | CG.OFFSET( isr2-$$+ORIGIN ) | CG.SELECTOR(0x8)
	;; isr 3
dq CG.P | CG.TYPE(1110b) | CG.OFFSET( isr3-$$+ORIGIN ) | CG.SELECTOR(0x8)
	;; isr 4
dq CG.P | CG.TYPE(1110b) | CG.OFFSET( isr4-$$+ORIGIN ) | CG.SELECTOR(0x8)
	;; isr 5
dq CG.P | CG.TYPE(1110b) | CG.OFFSET( isr5-$$+ORIGIN ) | CG.SELECTOR(0x8)
	;; isr 6
dq CG.P | CG.TYPE(1110b) | CG.OFFSET( isr6-$$+ORIGIN ) | CG.SELECTOR(0x8)
	;; isr 7
dq CG.P | CG.TYPE(1110b) | CG.OFFSET( isr7-$$+ORIGIN ) | CG.SELECTOR(0x8)
	;; isr 8
dq CG.P | CG.TYPE(1110b) | CG.OFFSET( isr8-$$+ORIGIN ) | CG.SELECTOR(0x8)
	;; isr 9
dq CG.P | CG.TYPE(1110b) | CG.OFFSET( isr9-$$+ORIGIN ) | CG.SELECTOR(0x8)
	;; isr 10
dq CG.P | CG.TYPE(1110b) | CG.OFFSET( isr10-$$+ORIGIN ) | CG.SELECTOR(0x8)
	;; isr 11
dq CG.P | CG.TYPE(1110b) | CG.OFFSET( isr11-$$+ORIGIN ) | CG.SELECTOR(0x8)
	;; isr 12
dq CG.P | CG.TYPE(1110b) | CG.OFFSET( isr12-$$+ORIGIN ) | CG.SELECTOR(0x8)
	;; isr 13
dq CG.P | CG.TYPE(1110b) | CG.OFFSET( isr13-$$+ORIGIN ) | CG.SELECTOR(0x8)
	;; isr 14
dq CG.P | CG.TYPE(1110b) | CG.OFFSET( isr14-$$+ORIGIN ) | CG.SELECTOR(0x8)
	;; isr 15
dq CG.P | CG.TYPE(1110b) | CG.OFFSET( isr15-$$+ORIGIN ) | CG.SELECTOR(0x8)
	;;
times 15 dq 0x0
dq CG.P | CG.TYPE(1110b) | CG.OFFSET( isr32-$$+ORIGIN ) | CG.SELECTOR(0x8)
dq CG.P | CG.TYPE(1110b) | CG.OFFSET( isr32-$$+ORIGIN ) | CG.SELECTOR(0x8)
dq CG.P | CG.TYPE(1110b) | CG.OFFSET( isr32-$$+ORIGIN ) | CG.SELECTOR(0x8)
END_IDT:
IDT_SIZE:
IDT_DESC:	dw END_IDT-BEGIN_IDT-1
		dd BEGIN_IDT