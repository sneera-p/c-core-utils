#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stdlib.h>
#include <cmocka.h>
#include "deque.h"
#include "deque.fixture.h"

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))
#define DOUBLE_EPS (1e-12)

struct test_state
{
   deque(double) double_deque;
   deque(date_s) date_deque;
};
typedef struct test_state test_state_s;

static int setup(void **state)
{
   test_state_s *tmp = (test_state_s*)malloc(sizeof(test_state_s));
   if (!tmp)
      return -1;

   deque_init(double, &tmp->double_deque);
   deque_init(date_s, &tmp->date_deque);
   *state = tmp;
   return 0;
}

static int teardown(void **state)
{
   test_state_s *tmp = (test_state_s*)(*state);
   deque_delete(double, &tmp->double_deque);
   deque_delete(date_s, &tmp->date_deque);
   free(tmp);
   *state = NULL;
   return 0;
}


/* Double deque */

// make sure no. elements > DOUBLE_DEQUE_INIT_SIZE
const double mock_doubles[] = 
{
   1.0, 2.5, 3.14, 4.0, 5.25, 6.75, 7.0, 8.125,
   9.5, 10.0, 11.11, 12.0, 13.3, 14.0, 15.5, 16.75,
   17.0, 18.25, 19.5, 20.0, 21.75, 22.0, 23.125, 24.0,
   25.5, 26.0, 27.75, 28.0
};

static void test_double_deque_init_delete(void **state)
{
   deque(double) deque;
   deque_init(double, &deque);
   assert_int_equal(deque.len, 0);
   assert_int_equal(deque.size, DOUBLE_DEQUE_INIT_SIZE);
   assert_ptr_equal(deque.values, deque.inline_buffer);

   deque_delete(double, &deque);
   assert_int_equal(deque.len, 0);
   assert_int_equal(deque.size, DOUBLE_DEQUE_INIT_SIZE);
   assert_ptr_equal(deque.values, deque.inline_buffer);
}

static void test_double_deque_resize(void **state)
{
   deque(double) deque;

   // not wrapped
   deque_init(double, &deque);

   size_t sum1 = DOUBLE_DEQUE_INIT_SIZE;
   for (size_t i = 0; i < 3; i++)
   {
      const bool result = deque_resize(double, &deque);
      sum1 *= DOUBLE_DEQUE_GROWTH_FACTOR;
      assert_int_equal(deque.size, sum1);
      assert_true(result);
   }

   deque_delete(double, &deque);
   assert_int_equal(deque.len, 0);
   assert_int_equal(deque.size, DOUBLE_DEQUE_INIT_SIZE);
   assert_ptr_equal(deque.values, deque.inline_buffer);

   // wrapped
   deque_init(double, &deque);

   deque.front = 2;
   deque.len = deque.size;
   size_t sum2 = DOUBLE_DEQUE_INIT_SIZE;
   for (size_t i = 0; i < 3; i++)
   {
      const bool result = deque_resize(double, &deque);
      sum2 *= DOUBLE_DEQUE_GROWTH_FACTOR;
      assert_int_equal(deque.size, sum2);
      assert_true(result);
   }

   deque_delete(double, &deque);
   assert_int_equal(deque.len, 0);
   assert_int_equal(deque.size, DOUBLE_DEQUE_INIT_SIZE);
   assert_ptr_equal(deque.values, deque.inline_buffer);
}

static void test_double_deque_peek_front_back(void **state)
{
   deque(double) *deque = &((test_state_s*)(*state))->double_deque;
   
   for (size_t i = 0; i < deque->size; i++)
   {
      deque->values[i] = mock_doubles[i];
      deque->len++;

      double result1 = deque_peek_front(double, deque);
      assert_double_equal(result1, mock_doubles[0], DOUBLE_EPS);

      double result2 = deque_peek_back(double, deque);
      assert_double_equal(result2, mock_doubles[i], DOUBLE_EPS);
   }
}

static void test_double_deque_insert_remove_front(void **state)
{
   deque(double) *deque = &((test_state_s*)(*state))->double_deque;

   for (size_t i = 0; i < ARRAY_LEN(mock_doubles); i++)
   {
      deque_insert_front(double, deque, mock_doubles[i]);

      double result = deque_peek_front(double, deque);
      assert_double_equal(result, mock_doubles[i], DOUBLE_EPS);
   }

   for (size_t i = ARRAY_LEN(mock_doubles) - 1; i > 0; i--)
   {
      deque_remove_front(double, deque);

      double result = deque_peek_front(double, deque);
      assert_double_equal(result, mock_doubles[i - 1], DOUBLE_EPS);
   }
}

static void test_double_deque_insert_remove_back(void **state)
{
   deque(double) *deque = &((test_state_s*)(*state))->double_deque;

   for (size_t i = 0; i < ARRAY_LEN(mock_doubles); i++)
   {
      deque_insert_back(double, deque, mock_doubles[i]);

      double result = deque_peek_back(double, deque);
      assert_double_equal(result, mock_doubles[i], DOUBLE_EPS);
   }

   for (size_t i = ARRAY_LEN(mock_doubles) - 1; i > 0; i--)
   {
      deque_remove_back(double, deque);

      double result = deque_peek_back(double, deque);
      assert_double_equal(result, mock_doubles[i - 1], DOUBLE_EPS);
   }
}

static void test_double_deque_empty(void **state)
{
   deque(double) *deque = &((test_state_s*)(*state))->double_deque;

   const bool result1 = deque_empty(double, deque);
   assert_true(result1);

   deque_insert_back(double, deque, mock_doubles[0]);

   const bool result2 = deque_empty(double, deque);
   assert_false(result2);
}

static void test_double_deque_full(void **state)
{
   deque(double) *deque = &((test_state_s*)(*state))->double_deque;

   const bool result1 = deque_full(double, deque);
   assert_false(result1);

   for (size_t i = 0; i < deque->size; i++)
      deque_insert_front(double, deque, mock_doubles[i]);
   const bool result2 = deque_full(double, deque);
   assert_true(result2);

   deque_resize(double, deque);
   const bool result3 = deque_full(double, deque);
   assert_false(result3);

   const size_t extra_slots = deque->size - deque->len;
   for (size_t i = 0; i < extra_slots; i++)
      deque_insert_back(double, deque, mock_doubles[i]);
   const bool result4 = deque_full(double, deque);
   assert_true(result4);
}

static void test_double_deque_clear(void **state)
{
   deque(double) *deque = &((test_state_s*)(*state))->double_deque;

   for (size_t i = 0; i < ARRAY_LEN(mock_doubles); i++)
      deque_insert_back(double, deque, mock_doubles[i]);

   deque_clear(double, deque);
   const bool result = deque_empty(double, deque);
   assert_true(result);
}


/* Date deque */

// make sure no. elements > DOUBLE_DEQUE_INIT_SIZE
const date_s mock_dates[] = 
{
   {1200,  1,  1},
   {1201,  2,  2},
   {1202,  3,  3},
   {1203,  4,  4},
   {1204,  5,  5},
   {1205,  6,  6},
   {1206,  7,  7},
   {1207,  8,  8},
   {1208,  9,  9},
   {1209, 10, 10},
   {1210, 11, 11},
   {1211, 12, 12},
   {1300,  1, 13},
   {1301,  2, 14},
   {1302,  3, 15},
   {1303,  4, 16},
   {1304,  5, 17},
   {1305,  6, 18},
   {1306,  7, 19},
   {1307,  8, 20},
   {1308,  9, 21},
   {1309, 10, 22},
   {1310, 11, 23},
   {1311, 12, 24},
   {1400,  1, 25},
   {1401,  2, 26},
   {1402,  3, 27},
   {1403,  4, 28}
};

static void test_date_deque_init_delete(void **state)
{
   deque(date_s) deque;
   deque_init(date_s, &deque);
   assert_int_equal(deque.len, 0);
   assert_int_equal(deque.size, DOUBLE_DEQUE_INIT_SIZE);
   assert_ptr_equal(deque.values, deque.inline_buffer);

   deque_delete(date_s, &deque);
   assert_int_equal(deque.len, 0);
   assert_int_equal(deque.size, DOUBLE_DEQUE_INIT_SIZE);
   assert_ptr_equal(deque.values, deque.inline_buffer);
}

static void test_date_deque_resize(void **state)
{
   deque(date_s) deque;

   // not wrapped
   deque_init(date_s, &deque);

   size_t sum1 = DOUBLE_DEQUE_INIT_SIZE;
   for (size_t i = 0; i < 3; i++)
   {
      const bool result = deque_resize(date_s, &deque);
      sum1 *= DOUBLE_DEQUE_GROWTH_FACTOR;
      assert_int_equal(deque.size, sum1);
      assert_true(result);
   }

   deque_delete(date_s, &deque);
   assert_int_equal(deque.len, 0);
   assert_int_equal(deque.size, DOUBLE_DEQUE_INIT_SIZE);
   assert_ptr_equal(deque.values, deque.inline_buffer);

   // wrapped
   deque_init(date_s, &deque);

   deque.front = 2;
   deque.len = deque.size;
   size_t sum2 = DOUBLE_DEQUE_INIT_SIZE;
   for (size_t i = 0; i < 3; i++)
   {
      const bool result = deque_resize(date_s, &deque);
      sum2 *= DOUBLE_DEQUE_GROWTH_FACTOR;
      assert_int_equal(deque.size, sum2);
      assert_true(result);
   }

   deque_delete(date_s, &deque);
   assert_int_equal(deque.len, 0);
   assert_int_equal(deque.size, DOUBLE_DEQUE_INIT_SIZE);
   assert_ptr_equal(deque.values, deque.inline_buffer);
}

static void test_date_deque_peek_front_back(void **state)
{
   deque(date_s) *deque = &((test_state_s*)(*state))->date_deque;
   
   for (size_t i = 0; i < deque->size; i++)
   {
      deque->values[i] = mock_dates[i];
      deque->len++;

      date_s result1 = deque_peek_front(date_s, deque);
      assert_memory_equal(&result1, &mock_dates[0], sizeof(date_s));

      date_s result2 = deque_peek_back(date_s, deque);
      assert_memory_equal(&result2, &mock_dates[i], sizeof(date_s));
   }
}

static void test_date_deque_insert_remove_front(void **state)
{
   deque(date_s) *deque = &((test_state_s*)(*state))->date_deque;

   for (size_t i = 0; i < ARRAY_LEN(mock_dates); i++)
   {
      deque_insert_front(date_s, deque, mock_dates[i]);

      date_s result = deque_peek_front(date_s, deque);
      assert_memory_equal(&result, &mock_dates[i], sizeof(date_s));
   }

   for (size_t i = ARRAY_LEN(mock_dates) - 1; i > 0; i--)
   {
      deque_remove_front(date_s, deque);

      date_s result = deque_peek_front(date_s, deque);
      assert_memory_equal(&result, &mock_dates[i - 1], sizeof(date_s));
   }
}

static void test_date_deque_insert_remove_back(void **state)
{
   deque(date_s) *deque = &((test_state_s*)(*state))->date_deque;

   for (size_t i = 0; i < ARRAY_LEN(mock_dates); i++)
   {
      deque_insert_back(date_s, deque, mock_dates[i]);

      date_s result = deque_peek_back(date_s, deque);
      assert_memory_equal(&result, &mock_dates[i], sizeof(date_s));
   }

   for (size_t i = ARRAY_LEN(mock_dates) - 1; i > 0; i--)
   {
      deque_remove_back(date_s, deque);

      date_s result = deque_peek_back(date_s, deque);
      assert_memory_equal(&result, &mock_dates[i - 1], sizeof(date_s));
   }
}

static void test_date_deque_empty(void **state)
{
   deque(date_s) *deque = &((test_state_s*)(*state))->date_deque;

   const bool result1 = deque_empty(date_s, deque);
   assert_true(result1);

   deque_insert_back(date_s, deque, mock_dates[0]);

   const bool result2 = deque_empty(date_s, deque);
   assert_false(result2);
}

static void test_date_deque_full(void **state)
{
   deque(date_s) *deque = &((test_state_s*)(*state))->date_deque;

   const bool result1 = deque_full(date_s, deque);
   assert_false(result1);

   for (size_t i = 0; i < deque->size; i++)
      deque_insert_front(date_s, deque, mock_dates[i]);
   const bool result2 = deque_full(date_s, deque);
   assert_true(result2);

   deque_resize(date_s, deque);
   const bool result3 = deque_full(date_s, deque);
   assert_false(result3);

   const size_t extra_slots = deque->size - deque->len;
   for (size_t i = 0; i < extra_slots; i++)
      deque_insert_back(date_s, deque, mock_dates[i]);
   const bool result4 = deque_full(date_s, deque);
   assert_true(result4);
}

static void test_date_deque_clear(void **state)
{
   deque(date_s) *deque = &((test_state_s*)(*state))->date_deque;

   for (size_t i = 0; i < ARRAY_LEN(mock_dates); i++)
      deque_insert_back(date_s, deque, mock_dates[i]);

   deque_clear(date_s, deque);
   const bool result = deque_empty(date_s, deque);
   assert_true(result);
}

int main(void)
{  
   const struct CMUnitTest tests[] = 
   {
      cmocka_unit_test(test_double_deque_init_delete),
      cmocka_unit_test(test_double_deque_resize),
      cmocka_unit_test_setup_teardown(test_double_deque_peek_front_back, setup, teardown),
      cmocka_unit_test_setup_teardown(test_double_deque_insert_remove_front, setup, teardown),
      cmocka_unit_test_setup_teardown(test_double_deque_insert_remove_back, setup, teardown),
      cmocka_unit_test_setup_teardown(test_double_deque_empty, setup, teardown),
      cmocka_unit_test_setup_teardown(test_double_deque_full, setup, teardown),
      cmocka_unit_test_setup_teardown(test_double_deque_clear, setup, teardown),
      cmocka_unit_test(test_date_deque_init_delete),
      cmocka_unit_test(test_date_deque_resize),
      cmocka_unit_test_setup_teardown(test_date_deque_peek_front_back, setup, teardown),
      cmocka_unit_test_setup_teardown(test_date_deque_insert_remove_front, setup, teardown),
      cmocka_unit_test_setup_teardown(test_date_deque_insert_remove_back, setup, teardown),
      cmocka_unit_test_setup_teardown(test_date_deque_empty, setup, teardown),
      cmocka_unit_test_setup_teardown(test_date_deque_full, setup, teardown),
      cmocka_unit_test_setup_teardown(test_date_deque_clear, setup, teardown),
   };
   return cmocka_run_group_tests(tests, NULL, NULL);
}