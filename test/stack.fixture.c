#include <stdlib.h>
#include <stddef.h>
#include "stack.fixture.h"

/* Int stack */
bool mock_valid(int x)
{
   return (x % 6 != 0);
}

GENERATE_STACK(int, size_t, INT_STACK_INIT_SIZE, INT_STACK_GROWTH_FACTOR, mock_valid, malloc, realloc, free)

/* Cordinate stack */
bool cord_valid(cordinate_s x)
{
   return true;
}

GENERATE_STACK(cordinate_s, size_t, CORDINATE_STACK_INIT_SIZE, CORDINATE_STACK_GROWTH_FACTOR, cord_valid, malloc, realloc, free)
