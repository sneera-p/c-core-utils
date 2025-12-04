#ifndef __DEQUE_FIXTURE_H
#define __DEQUE_FIXTURE_H

#include <stddef.h>
#include <stdint.h>
#include "ccoutils.h"

/* Double deque */
#define DOUBLE_DEQUE_INIT_SIZE 4
#define DOUBLE_DEQUE_GROWTH_FACTOR 2
DEFINE_DEQUE(double, size_t, DOUBLE_DEQUE_INIT_SIZE)

/* Date deque */
typedef struct
{
   uint16_t year;
	uint8_t month;
	uint8_t day;
} date_s;
#define DATE_DEQUE_INIT_SIZE 4
#define DATE_DEQUE_GROWTH_FACTOR 2
DEFINE_DEQUE(date_s, size_t, DATE_DEQUE_INIT_SIZE)

#endif /* __DEQUE_FIXTURE_H */
