#ifndef __TYPES__H__
#define __TYPES__H__

/* Tipos signados */
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long long int int64_t;

/* Tipos no signados */
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long int uint64_t;

typedef uint32_t reg_t;
typedef void *ptr_t;
typedef unsigned int size_t;
typedef signed int time_t;

typedef struct __attribute__ ((packed)) {
	uint16_t limit;
	uint32_t base;
} gdtr_t; // __attribute__ ((packed));
typedef gdtr_t idtr_t;


#endif
