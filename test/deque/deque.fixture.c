#include <stdlib.h>
#include "deque.fixture.h"


/* Double deque */

bool mock_valid(double x)
{
   return true;
}

GENERATE_DEQUE(double, size_t, DOUBLE_DEQUE_INIT_SIZE, DOUBLE_DEQUE_GROWTH_FACTOR, mock_valid, malloc, realloc, free)


/* Date deque */

bool date_valid(date_s x)
{
   return (x.year < 2100) && (x.month > 0) && (x.month <= 12) && (x.day > 0) && (x.day <= 31);
}

GENERATE_DEQUE(date_s, size_t, DATE_DEQUE_INIT_SIZE, DATE_DEQUE_GROWTH_FACTOR, date_valid, malloc, realloc, free)