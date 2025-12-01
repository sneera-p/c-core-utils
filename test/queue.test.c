#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stdlib.h>
#include <cmocka.h>
#include "queue.h"
#include "queue.fixture.h"

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))
#define FLOAT_EPS (1e-12)

struct test_state
{
   queue(float) float_queue;
   queue(car_s) car_queue;
};
typedef struct test_state test_state_s;

static int setup(void **state)
{
   test_state_s *tmp = (test_state_s*)malloc(sizeof(test_state_s));
   if (!tmp)
      return -1;

   queue_init(float, &tmp->float_queue);
   queue_init(car_s, &tmp->car_queue);
   *state = tmp;
   return 0;
}

static int teardown(void **state)
{
   test_state_s *tmp = (test_state_s*)(*state);
   queue_delete(float, &tmp->float_queue);
   queue_delete(car_s, &tmp->car_queue);
   free(tmp);
   *state = NULL;
   return 0;
}


/* Double queue */

// make sure no. elements > FLOAT_QUEUE_INIT_SIZE
const float mock_floats[] = 
{
   1.0, 2.5, 3.14, 4.0, 5.25, 6.75, 7.0, 8.125,
   9.5, 10.0, 11.11, 12.0, 13.3, 14.0, 15.5, 16.75,
   17.0, 18.25, 19.5, 20.0, 21.75, 22.0, 23.125, 24.0,
   25.5, 26.0, 27.75, 28.0
};

static void test_float_queue_init_delete(void **state)
{
   queue(float) queue;
   queue_init(float, &queue);
   assert_int_equal(queue.len, 0);
   assert_int_equal(queue.size, FLOAT_QUEUE_INIT_SIZE);
   assert_ptr_equal(queue.values, queue.inline_buffer);

   queue_delete(float, &queue);
   assert_int_equal(queue.len, 0);
   assert_int_equal(queue.size, FLOAT_QUEUE_INIT_SIZE);
   assert_ptr_equal(queue.values, queue.inline_buffer);
}

static void test_float_queue_resize(void **state)
{
   queue(float) queue;

   // not wrapped
   queue_init(float, &queue);

   size_t sum1 = FLOAT_QUEUE_INIT_SIZE;
   for (size_t i = 0; i < 3; i++)
   {
      const bool result = queue_resize(float, &queue);
      sum1 *= FLOAT_QUEUE_GROWTH_FACTOR;
      assert_int_equal(queue.size, sum1);
      assert_true(result);
   }

   queue_delete(float, &queue);
   assert_int_equal(queue.len, 0);
   assert_int_equal(queue.size, FLOAT_QUEUE_INIT_SIZE);
   assert_ptr_equal(queue.values, queue.inline_buffer);

   // wrapped
   queue_init(float, &queue);

   queue.head = 2;
   queue.len = queue.size;
   size_t sum2 = FLOAT_QUEUE_INIT_SIZE;
   for (size_t i = 0; i < 3; i++)
   {
      const bool result = queue_resize(float, &queue);
      sum2 *= FLOAT_QUEUE_GROWTH_FACTOR;
      assert_int_equal(queue.size, sum2);
      assert_true(result);
   }

   queue_delete(float, &queue);
   assert_int_equal(queue.len, 0);
   assert_int_equal(queue.size, FLOAT_QUEUE_INIT_SIZE);
   assert_ptr_equal(queue.values, queue.inline_buffer);
}

static void test_float_queue_peek(void **state)
{
   queue(float) *queue = &((test_state_s*)(*state))->float_queue;
   
   for (size_t i = 0; i < queue->size; i++)
   {
      queue->values[i] = mock_floats[i];
      queue->len++;

      float result1 = queue_peek(float, queue);
      assert_float_equal(result1, mock_floats[0], FLOAT_EPS);
   }
}

static void test_float_queue_enque_deque(void **state)
{
   queue(float) *queue = &((test_state_s*)(*state))->float_queue;

   for (size_t i = 0; i < ARRAY_LEN(mock_floats); i++)
   {
      queue_enque(float, queue, mock_floats[i]);
      float result = queue->values[(queue->head + queue->len - 1) % queue->size];
      assert_float_equal(result, mock_floats[i], FLOAT_EPS);
   }

   for (size_t i = 0; i < ARRAY_LEN(mock_floats); i++)
   {
      float result = queue_peek(float, queue);
      assert_float_equal(result, mock_floats[i], FLOAT_EPS);
      queue_deque(float, queue);
   }
}

static void test_float_queue_empty(void **state)
{
   queue(float) *queue = &((test_state_s*)(*state))->float_queue;

   const bool result1 = queue_empty(float, queue);
   assert_true(result1);

   queue_enque(float, queue, mock_floats[0]);

   const bool result2 = queue_empty(float, queue);
   assert_false(result2);
}

static void test_float_queue_full(void **state)
{
   queue(float) *queue = &((test_state_s*)(*state))->float_queue;

   const bool result1 = queue_full(float, queue);
   assert_false(result1);

   for (size_t i = 0; i < queue->size; i++)
      queue_enque(float, queue, mock_floats[i]);
   const bool result2 = queue_full(float, queue);
   assert_true(result2);

   queue_resize(float, queue);
   const bool result3 = queue_full(float, queue);
   assert_false(result3);

   const size_t extra_slots = queue->size - queue->len;
   for (size_t i = 0; i < extra_slots; i++)
      queue_enque(float, queue, mock_floats[i]);
   const bool result4 = queue_full(float, queue);
   assert_true(result4);
}

static void test_float_queue_clear(void **state)
{
   queue(float) *queue = &((test_state_s*)(*state))->float_queue;

   for (size_t i = 0; i < ARRAY_LEN(mock_floats); i++)
      queue_enque(float, queue, mock_floats[i]);

   queue_clear(float, queue);
   const bool result = queue_empty(float, queue);
   assert_true(result);
}


/* Car queue */

// make sure no. elements > CAR_QUEUE_INIT_SIZE
const car_s mock_cars[28] = 
{
    { .engine = 6,  .chasis = 33,  .gearbox = 3.44 },
    { .engine = 4,  .chasis = 12,  .gearbox = 2.95 },
    { .engine = 8,  .chasis = 47,  .gearbox = 4.20 },
    { .engine = 5,  .chasis = 29,  .gearbox = 3.10 },
    { .engine = 6,  .chasis = 38,  .gearbox = 3.50 },
    { .engine = 4,  .chasis = 21,  .gearbox = 2.80 },
    { .engine = 6,  .chasis = 34,  .gearbox = 3.60 },
    { .engine = 8,  .chasis = 52,  .gearbox = 4.00 },
    { .engine = 5,  .chasis = 27,  .gearbox = 3.05 },
    { .engine = 6,  .chasis = 31,  .gearbox = 3.44 },
    { .engine = 4,  .chasis = 18,  .gearbox = 2.95 },
    { .engine = 8,  .chasis = 45,  .gearbox = 4.15 },
    { .engine = 5,  .chasis = 30,  .gearbox = 3.20 },
    { .engine = 6,  .chasis = 35,  .gearbox = 3.55 },
    { .engine = 4,  .chasis = 22,  .gearbox = 2.85 },
    { .engine = 6,  .chasis = 36,  .gearbox = 3.65 },
    { .engine = 8,  .chasis = 50,  .gearbox = 4.05 },
    { .engine = 5,  .chasis = 28,  .gearbox = 3.10 },
    { .engine = 6,  .chasis = 32,  .gearbox = 3.48 },
    { .engine = 4,  .chasis = 20,  .gearbox = 2.90 },
    { .engine = 8,  .chasis = 48,  .gearbox = 4.18 },
    { .engine = 5,  .chasis = 26,  .gearbox = 3.12 },
    { .engine = 6,  .chasis = 37,  .gearbox = 3.52 },
    { .engine = 4,  .chasis = 19,  .gearbox = 2.88 },
    { .engine = 6,  .chasis = 39,  .gearbox = 3.60 },
    { .engine = 8,  .chasis = 51,  .gearbox = 4.22 },
    { .engine = 5,  .chasis = 25,  .gearbox = 3.08 },
    { .engine = 6,  .chasis = 40,  .gearbox = 3.57 }
};

static void test_car_queue_init_delete(void **state)
{
   queue(car_s) queue;
   queue_init(car_s, &queue);
   assert_int_equal(queue.len, 0);
   assert_int_equal(queue.size, FLOAT_QUEUE_INIT_SIZE);
   assert_ptr_equal(queue.values, queue.inline_buffer);

   queue_delete(car_s, &queue);
   assert_int_equal(queue.len, 0);
   assert_int_equal(queue.size, FLOAT_QUEUE_INIT_SIZE);
   assert_ptr_equal(queue.values, queue.inline_buffer);
}

static void test_car_queue_resize(void **state)
{
   queue(car_s) queue;

   // not wrapped
   queue_init(car_s, &queue);

   size_t sum1 = FLOAT_QUEUE_INIT_SIZE;
   for (size_t i = 0; i < 3; i++)
   {
      const bool result = queue_resize(car_s, &queue);
      sum1 *= FLOAT_QUEUE_GROWTH_FACTOR;
      assert_int_equal(queue.size, sum1);
      assert_true(result);
   }

   queue_delete(car_s, &queue);
   assert_int_equal(queue.len, 0);
   assert_int_equal(queue.size, FLOAT_QUEUE_INIT_SIZE);
   assert_ptr_equal(queue.values, queue.inline_buffer);

   // wrapped
   queue_init(car_s, &queue);

   queue.head = 2;
   queue.len = queue.size;
   size_t sum2 = FLOAT_QUEUE_INIT_SIZE;
   for (size_t i = 0; i < 3; i++)
   {
      const bool result = queue_resize(car_s, &queue);
      sum2 *= FLOAT_QUEUE_GROWTH_FACTOR;
      assert_int_equal(queue.size, sum2);
      assert_true(result);
   }

   queue_delete(car_s, &queue);
   assert_int_equal(queue.len, 0);
   assert_int_equal(queue.size, FLOAT_QUEUE_INIT_SIZE);
   assert_ptr_equal(queue.values, queue.inline_buffer);
}

static void test_car_queue_peek(void **state)
{
   queue(car_s) *queue = &((test_state_s*)(*state))->car_queue;
   
   for (size_t i = 0; i < queue->size; i++)
   {
      queue->values[i] = mock_cars[i];
      queue->len++;

      car_s result1 = queue_peek(car_s, queue);
      assert_memory_equal(&result1, &mock_cars[0], sizeof(car_s));
   }
}

static void test_car_queue_enque_deque(void **state)
{
   queue(car_s) *queue = &((test_state_s*)(*state))->car_queue;

   for (size_t i = 0; i < ARRAY_LEN(mock_cars); i++)
   {
      queue_enque(car_s, queue, mock_cars[i]);
      car_s result = queue->values[(queue->head + queue->len - 1) % queue->size];
      assert_memory_equal(&result, &mock_cars[i], sizeof(car_s));
   }

   for (size_t i = 0; i < ARRAY_LEN(mock_cars); i++)
   {
      car_s result = queue_peek(car_s, queue);
      assert_memory_equal(&result, &mock_cars[i], sizeof(car_s));
      queue_deque(car_s, queue);
   }
}

static void test_car_queue_empty(void **state)
{
   queue(car_s) *queue = &((test_state_s*)(*state))->car_queue;

   const bool result1 = queue_empty(car_s, queue);
   assert_true(result1);

   queue_enque(car_s, queue, mock_cars[0]);

   const bool result2 = queue_empty(car_s, queue);
   assert_false(result2);
}

static void test_car_queue_full(void **state)
{
   queue(car_s) *queue = &((test_state_s*)(*state))->car_queue;

   const bool result1 = queue_full(car_s, queue);
   assert_false(result1);

   for (size_t i = 0; i < queue->size; i++)
      queue_enque(car_s, queue, mock_cars[i]);
   const bool result2 = queue_full(car_s, queue);
   assert_true(result2);

   queue_resize(car_s, queue);
   const bool result3 = queue_full(car_s, queue);
   assert_false(result3);

   const size_t extra_slots = queue->size - queue->len;
   for (size_t i = 0; i < extra_slots; i++)
      queue_enque(car_s, queue, mock_cars[i]);
   const bool result4 = queue_full(car_s, queue);
   assert_true(result4);
}

static void test_car_queue_clear(void **state)
{
   queue(car_s) *queue = &((test_state_s*)(*state))->car_queue;

   for (size_t i = 0; i < ARRAY_LEN(mock_cars); i++)
      queue_enque(car_s, queue, mock_cars[i]);

   queue_clear(car_s, queue);
   const bool result = queue_empty(car_s, queue);
   assert_true(result);
}

int main(void)
{  
   const struct CMUnitTest tests[] = 
   {
      cmocka_unit_test(test_float_queue_init_delete),
      cmocka_unit_test(test_float_queue_resize),
      cmocka_unit_test_setup_teardown(test_float_queue_peek, setup, teardown),
      cmocka_unit_test_setup_teardown(test_float_queue_enque_deque, setup, teardown),
      cmocka_unit_test_setup_teardown(test_float_queue_empty, setup, teardown),
      cmocka_unit_test_setup_teardown(test_float_queue_full, setup, teardown),
      cmocka_unit_test_setup_teardown(test_float_queue_clear, setup, teardown),
      cmocka_unit_test(test_car_queue_init_delete),
      cmocka_unit_test(test_car_queue_resize),
      cmocka_unit_test_setup_teardown(test_car_queue_peek, setup, teardown),
      cmocka_unit_test_setup_teardown(test_car_queue_enque_deque, setup, teardown),
      cmocka_unit_test_setup_teardown(test_car_queue_empty, setup, teardown),
      cmocka_unit_test_setup_teardown(test_car_queue_full, setup, teardown),
      cmocka_unit_test_setup_teardown(test_car_queue_clear, setup, teardown),
   };
   return cmocka_run_group_tests(tests, NULL, NULL);
}