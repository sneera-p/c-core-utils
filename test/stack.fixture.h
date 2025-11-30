#ifndef __STACK_FIXTURE_H
#define __STACK_FIXTURE_H

#include <stddef.h>
#include <stdint.h>
#include "stack.h"

/* Int stack */
#define INT_STACK_INIT_SIZE 4
#define INT_STACK_GROWTH_FACTOR 2
DEFINE_STACK(int, size_t, INT_STACK_INIT_SIZE)

/* Cordinate stack */
typedef struct
{
   double x;
   double y;
   double z;
} cordinate_s;
#define CORDINATE_STACK_INIT_SIZE 4
#define CORDINATE_STACK_GROWTH_FACTOR 2
DEFINE_STACK(cordinate_s, size_t, CORDINATE_STACK_INIT_SIZE)

#endif /* __STACK_FIXTURE_H */
