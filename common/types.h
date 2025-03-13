#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stdbool.h>

#define va_start(ap, param) __builtin_va_start(ap, param)
#define va_start(ap, param) __builtin_va_start(ap, param)
#define va_end(ap)          __builtin_va_end(ap)
#define va_arg(ap, type)    __builtin_va_arg(ap, type)
typedef __builtin_va_list va_list;

/* Define exact-width integer types */
/*  typedef unsigned char uint8_t; */
/*  typedef unsigned short uint16_t; */
/*  typedef unsigned int uint32_t; */
/*  typedef unsigned long long uint64_t; */
/*  */
/*  typedef signed char int8_t; */
/*  typedef signed short int16_t; */
/*  typedef signed int int32_t; */
/*  typedef signed long long int64_t; */

/* Define boolean type */
/*  typedef enum */
/*  { */
/*      false = 0, */
/*      true  = ~0 */
/*  } bool; */

typedef volatile uint32_t* RegIO;

/* Define NULL pointer */
#define NULL ((void*) 0)

#endif /* TYPES_H */
