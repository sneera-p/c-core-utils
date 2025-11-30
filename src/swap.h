#ifndef __SWAP_H
#define __SWAP_H

#include "static-assert.h"

/**
 * SWAP macro
 * ----------------
 * Swaps 2 variables of any type
 * 
 * Inputs:
 *   two variable a, b
 * 
 * Return:
 *   a, b swapped,
 */
#define SWAP(type, a, b) do { \
   assert_istype(type);   \
   assert_type(a, type);  \
   assert_type(b, type);  \
   type _tmp = (a);       \
   (a) = (b);             \
   (b) = _tmp;            \
} while (0)

#endif /* __SWAP_H */