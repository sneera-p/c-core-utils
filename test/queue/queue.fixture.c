#include <stdlib.h>
#include "queue.fixture.h"


/* Float queue */

bool mock_valid(float x)
{
   return true;
}

GENERATE_QUEUE(float, size_t, FLOAT_QUEUE_INIT_SIZE, FLOAT_QUEUE_GROWTH_FACTOR, mock_valid, malloc, realloc, free)


/* Car queue */

bool car_valid(car_s x)
{
   return true;
}

GENERATE_QUEUE(car_s, size_t, CAR_QUEUE_INIT_SIZE, CAR_QUEUE_GROWTH_FACTOR, car_valid, malloc, realloc, free)