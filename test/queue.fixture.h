#ifndef __QUEUE_FIXTURE_H
#define __QUEUE_FIXTURE_H

#include <stddef.h>
#include <stdint.h>
#include "queue.h"

/* Float queue */
#define FLOAT_QUEUE_INIT_SIZE 4
#define FLOAT_QUEUE_GROWTH_FACTOR 2
DEFINE_QUEUE(float, size_t, FLOAT_QUEUE_INIT_SIZE)

/* Car queue */
typedef struct
{
   int engine;
   size_t chasis;
   float gearbox;
} car_s;
#define CAR_QUEUE_INIT_SIZE 4
#define CAR_QUEUE_GROWTH_FACTOR 2
DEFINE_QUEUE(car_s, size_t, CAR_QUEUE_INIT_SIZE)

#endif /* __QUEUE_FIXTURE_H */
