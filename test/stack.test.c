#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stdlib.h>
#include <cmocka.h>
#include "stack.h"
#include "stack.fixture.h"

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

struct test_state
{
   stack(int) int_stack;
   stack(cordinate_s) cordinate_stack;
};
typedef struct test_state test_state_s;

static int setup(void **state)
{
   test_state_s *tmp = (test_state_s*)malloc(sizeof(test_state_s));
   if (!tmp)
      return -1;

   stack_init(int, &tmp->int_stack);
   stack_init(cordinate_s, &tmp->cordinate_stack);
   *state = tmp;
   return 0;
}

static int teardown(void **state)
{
   test_state_s *tmp = (test_state_s*)(*state);
   stack_delete(int, &tmp->int_stack);
   stack_delete(cordinate_s, &tmp->cordinate_stack);
   free(tmp);
   *state = NULL;
   return 0;
}


/* Integer stack */

// make sure no. elements > INT_STACK_INIT_SIZE
const int mock_ints[] = { 1, 2, 3, 4, 5, 7, 8, 9, 10, 11, 13, 14, 15, 16, 17, 19, 20, 21, 22, 23 };

static void test_int_stack_init_delete(void **state)
{
   stack(int) stack;
   stack_init(int, &stack);
   assert_int_equal(stack.len, 0);
   assert_int_equal(stack.size, INT_STACK_INIT_SIZE);
   assert_ptr_equal(stack.values, stack.inline_buffer);

   stack_delete(int, &stack);
   assert_int_equal(stack.len, 0);
   assert_int_equal(stack.size, INT_STACK_INIT_SIZE);
   assert_ptr_equal(stack.values, stack.inline_buffer);
}

static void test_int_stack_resize(void **state)
{
   stack(int) stack;
   stack_init(int, &stack);

   size_t sum = INT_STACK_INIT_SIZE;
   for (size_t i = 0; i < 3; i++)
   {
      const bool result = stack_resize(int, &stack);
      sum *= INT_STACK_GROWTH_FACTOR;
      assert_int_equal(stack.size, sum);
      assert_true(result);
   }

   stack_delete(int, &stack);
   assert_int_equal(stack.len, 0);
   assert_int_equal(stack.size, INT_STACK_INIT_SIZE);
   assert_ptr_equal(stack.values, stack.inline_buffer);
}

static void test_int_stack_peek(void **state)
{
   stack(int) *stack = &((test_state_s*)(*state))->int_stack;
   
   for (size_t i = 0; i < 4; i++)
   {
      stack->values[i] = mock_ints[i];
      stack->len++;

      int result = stack_peek(int, stack);
      assert_int_equal(result, mock_ints[i]);
   }
}

static void test_int_stack_push_pop(void **state)
{
   stack(int) *stack = &((test_state_s*)(*state))->int_stack;
   
   for (size_t i = 0; i < ARRAY_LEN(mock_ints); i++)
   {
      stack_push(int, stack, mock_ints[i]);

      int result = stack_peek(int, stack);
      assert_int_equal(result, mock_ints[i]);
   }

   for (size_t i = ARRAY_LEN(mock_ints) - 1; i > 0; i--)
   {
      stack_pop(int, stack);

      int result = stack_peek(int, stack);
      assert_int_equal(result, mock_ints[i - 1]);
   }
}

static void test_int_stack_empty(void **state)
{
   stack(int) *stack = &((test_state_s*)(*state))->int_stack;

   const bool result1 = stack_empty(int, stack);
   assert_true(result1);

   stack_push(int, stack, mock_ints[0]);

   const bool result2 = stack_empty(int, stack);
   assert_false(result2);
}

static void test_int_stack_full(void **state)
{
   stack(int) *stack = &((test_state_s*)(*state))->int_stack;

   const bool result1 = stack_full(int, stack);
   assert_false(result1);

   for (size_t i = 0; i < stack->size; i++)
      stack_push(int, stack, mock_ints[i]);
   const bool result2 = stack_full(int, stack);
   assert_true(result2);

   stack_resize(int, stack);
   const bool result3 = stack_full(int, stack);
   assert_false(result3);

   const size_t extra_slots = stack->size - stack->len;
   for (size_t i = 0; i < extra_slots; i++)
      stack_push(int, stack, mock_ints[i]);
   const bool result4 = stack_full(int, stack);
   assert_true(result4);
}

static void test_int_stack_clear(void **state)
{
   stack(int) *stack = &((test_state_s*)(*state))->int_stack;

   for (size_t i = 0; i < ARRAY_LEN(mock_ints); i++)
      stack_push(int, stack, mock_ints[i]);

   stack_clear(int, stack);
   const bool result = stack_empty(int, stack);
   assert_true(result);
}

static void test_int_stack_reverse(void **state)
{
   stack(int) *stack = &((test_state_s*)(*state))->int_stack;

   // inline_buffer
   for (size_t i = 0; i < stack->size; i++)
      stack_push(int, stack, mock_ints[i]);

   stack_reverse(int, stack);
   for (size_t i = 0; i < stack->len; i++)
      assert_int_equal(stack->values[i], mock_ints[stack->len - i - 1]);

   // heap
   stack_clear(int, stack);
   for (size_t i = 0; i < ARRAY_LEN(mock_ints); i++)
      stack_push(int, stack, mock_ints[i]);

   stack_reverse(int, stack);
   for (size_t i = 0; i < stack->len; i++)
      assert_int_equal(stack->values[i], mock_ints[stack->len - i - 1]);
}


/* Cordinate stack */

// make sure no. elements > CORDINATE_STACK_INIT_SIZE
const cordinate_s mock_cords[] = 
{
   { 3.12, 33.2, 34.0 },
   { 12.5, 7.3, 19.1 },
   { 8.6, 21.9, 4.7 },
   { 14.2, 18.3, 23.6 },
   { 6.1, 12.8, 9.4 },
   { 22.7, 3.5, 16.9 },
   { 5.3, 27.1, 11.2 },
   { 19.8, 14.6, 2.4 },
   { 7.7, 6.3, 28.0 },
   { 11.9, 25.5, 13.3 },
   { 2.8, 10.2, 21.7 },
   { 16.4, 8.8, 5.6 },
   { 9.1, 20.0, 15.5 },
   { 13.5, 1.2, 17.8 },
   { 4.4, 23.7, 6.6 },
   { 18.2, 9.5, 12.1 },
   { 1.7, 16.8, 3.9 },
   { 24.3, 7.1, 10.0 },
   { 10.6, 13.4, 22.9 },
   { 15.8, 5.2, 8.3 },
   { 6.9, 18.6, 14.7 },
   { 21.0, 11.3, 1.5 },
   { 3.8, 4.1, 20.6 },
   { 17.5, 19.9, 9.2 },
   { 8.0, 12.7, 25.4 },
   { 2.3, 14.9, 7.8 },
   { 20.2, 6.6, 18.5 },
   { 12.1, 3.4, 16.0 }
};

static void test_cordinate_stack_init_delete(void **state)
{
   stack(cordinate_s) stack;
   stack_init(cordinate_s, &stack);
   assert_int_equal(stack.len, 0);
   assert_int_equal(stack.size, CORDINATE_STACK_INIT_SIZE);
   assert_ptr_equal(stack.values, stack.inline_buffer);

   stack_delete(cordinate_s, &stack);
   assert_int_equal(stack.len, 0);
   assert_int_equal(stack.size, CORDINATE_STACK_INIT_SIZE);
   assert_ptr_equal(stack.values, stack.inline_buffer);
}

static void test_cordinate_stack_resize(void **state)
{
   stack(cordinate_s) stack;
   stack_init(cordinate_s, &stack);

   size_t sum = CORDINATE_STACK_INIT_SIZE;
   for (size_t i = 0; i < 3; i++)
   {
      const bool result = stack_resize(cordinate_s, &stack);
      sum *= CORDINATE_STACK_GROWTH_FACTOR;
      assert_int_equal(stack.size, sum);
      assert_true(result);
   }

   stack_delete(cordinate_s, &stack);
   assert_int_equal(stack.len, 0);
   assert_int_equal(stack.size, CORDINATE_STACK_INIT_SIZE);
   assert_ptr_equal(stack.values, stack.inline_buffer);
}

static void test_cordinate_stack_peek(void **state)
{
   stack(cordinate_s) *stack = &((test_state_s*)(*state))->cordinate_stack;
   
   for (size_t i = 0; i < 4; i++)
   {
      stack->values[i] = mock_cords[i];
      stack->len++;

      cordinate_s result = stack_peek(cordinate_s, stack);
      assert_memory_equal(&result, &mock_cords[i], sizeof(cordinate_s));
   }
}

static void test_cordinate_stack_push_pop(void **state)
{
   stack(cordinate_s) *stack = &((test_state_s*)(*state))->cordinate_stack;

   for (size_t i = 0; i < ARRAY_LEN(mock_cords); i++)
   {
      stack_push(cordinate_s, stack, mock_cords[i]);

      cordinate_s result = stack_peek(cordinate_s, stack);
      assert_memory_equal(&result, &mock_cords[i], sizeof(cordinate_s));
   }

   for (size_t i = ARRAY_LEN(mock_cords) - 1; i > 0; i--)
   {
      stack_pop(cordinate_s, stack);

      cordinate_s result = stack_peek(cordinate_s, stack);
      assert_memory_equal(&result, &mock_cords[i - 1], sizeof(cordinate_s));
   }
}

static void test_cordinate_stack_empty(void **state)
{
   stack(cordinate_s) *stack = &((test_state_s*)(*state))->cordinate_stack;

   const bool result1 = stack_empty(cordinate_s, stack);
   assert_true(result1);

   stack_push(cordinate_s, stack, mock_cords[0]);

   const bool result2 = stack_empty(cordinate_s, stack);
   assert_false(result2);
}

static void test_cordinate_stack_full(void **state)
{
   stack(cordinate_s) *stack = &((test_state_s*)(*state))->cordinate_stack;

   const bool result1 = stack_full(cordinate_s, stack);
   assert_false(result1);

   for (size_t i = 0; i < stack->size; i++)
      stack_push(cordinate_s, stack, mock_cords[i]);
   const bool result2 = stack_full(cordinate_s, stack);
   assert_true(result2);

   stack_resize(cordinate_s, stack);
   const bool result3 = stack_full(cordinate_s, stack);
   assert_false(result3);

   const size_t extra_slots = stack->size - stack->len;
   for (size_t i = 0; i < extra_slots; i++)
      stack_push(cordinate_s, stack, mock_cords[i]);
   const bool result4 = stack_full(cordinate_s, stack);
   assert_true(result4);
}

static void test_cordinate_stack_clear(void **state)
{
   stack(cordinate_s) *stack = &((test_state_s*)(*state))->cordinate_stack;

   for (size_t i = 0; i < ARRAY_LEN(mock_cords); i++)
      stack_push(cordinate_s, stack, mock_cords[i]);

   stack_clear(cordinate_s, stack);
   const bool result = stack_empty(cordinate_s, stack);
   assert_true(result);
}

static void test_cordinate_stack_reverse(void **state)
{
   stack(cordinate_s) *stack = &((test_state_s*)(*state))->cordinate_stack;

   // inline_buffer
   for (size_t i = 0; i < stack->size; i++)
      stack_push(cordinate_s, stack, mock_cords[i]);

   stack_reverse(cordinate_s, stack);
   for (size_t i = 0; i < stack->len; i++)
      assert_memory_equal(&stack->values[i], &mock_cords[stack->len - i - 1], sizeof(cordinate_s));

   // heap
   stack_clear(cordinate_s, stack);
   for (size_t i = 0; i < ARRAY_LEN(mock_cords); i++)
      stack_push(cordinate_s, stack, mock_cords[i]);

   stack_reverse(cordinate_s, stack);
   for (size_t i = 0; i < stack->len; i++)
      assert_memory_equal(&stack->values[i], &mock_cords[stack->len - i - 1], sizeof(cordinate_s));
}


int main(void)
{  
   const struct CMUnitTest tests[] = 
   {
      cmocka_unit_test(test_int_stack_init_delete),
      cmocka_unit_test(test_int_stack_resize),
      cmocka_unit_test_setup_teardown(test_int_stack_peek, setup, teardown),
      cmocka_unit_test_setup_teardown(test_int_stack_push_pop, setup, teardown),
      cmocka_unit_test_setup_teardown(test_int_stack_empty, setup, teardown),
      cmocka_unit_test_setup_teardown(test_int_stack_full, setup, teardown),
      cmocka_unit_test_setup_teardown(test_int_stack_clear, setup, teardown),
      cmocka_unit_test_setup_teardown(test_int_stack_reverse, setup, teardown),
      cmocka_unit_test(test_cordinate_stack_init_delete),
      cmocka_unit_test(test_cordinate_stack_resize),
      cmocka_unit_test_setup_teardown(test_cordinate_stack_peek, setup, teardown),
      cmocka_unit_test_setup_teardown(test_cordinate_stack_push_pop, setup, teardown),
      cmocka_unit_test_setup_teardown(test_cordinate_stack_empty, setup, teardown),
      cmocka_unit_test_setup_teardown(test_cordinate_stack_full, setup, teardown),
      cmocka_unit_test_setup_teardown(test_cordinate_stack_clear, setup, teardown),
      cmocka_unit_test_setup_teardown(test_cordinate_stack_reverse, setup, teardown),
   };
   return cmocka_run_group_tests(tests, NULL, NULL);
}