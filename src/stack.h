#ifndef __STACK_H
#define __STACK_H

#include <stdbool.h>
#include <assert.h>
#include "static-assert.h"
#include "swap.h"
#include "memory-copy.h"


/**
 * DEFINE_STACK macro
 * ------------
 * Defines a generic stack type with an inline buffer of size `init_size`.
 *
 * Parameters:
 *   type      - Type of elements stored in the stack
 *   len_type     - Integer type used for length/size
 *   init_size - Number of elements to store inline before heap allocation
 * 
 * Output:
 *   Declaration of stack for type
 * 
 * Notes:
 *    Use only in a header (.h) file
 *    Use in combination with GENERATE_STACK(...)
 */
#define DEFINE_STACK(type, len_type, init_size) \
   static_assert(init_size > 1, "Warning: init_size too small"); \
   static_assert(init_size < 256, "Warning: init_size too big"); \
   static_assert(init_size != 0 && (init_size & (init_size - 1)) == 0, "Warning: init_size must be a power of 2"); /* resize optimization */ \
   assert_istype(type); \
   assert_istype(len_type); \
\
typedef struct \
{ \
   type inline_buffer[init_size]; \
   type *values; \
   len_type len; \
   len_type size; \
} type##_stack_s; \
\
static inline void type##_stack_init(type##_stack_s *const restrict stack) \
{ \
   stack->values = stack->inline_buffer; \
   stack->len = 0; \
   stack->size = init_size; \
} \
static inline bool type##_stack_empty(const type##_stack_s *const restrict stack) \
{ \
   assert(stack); \
   return stack->len == 0; \
} \
static inline bool type##_stack_full(const type##_stack_s *const restrict stack) \
{ \
   assert(stack); \
   return stack->len == stack->size; \
} \
bool type##_stack_resize(type##_stack_s *const restrict); \
void type##_stack_clear(type##_stack_s *const restrict); \
void type##_stack_delete(type##_stack_s *const restrict); \
bool type##_stack_push(type##_stack_s *const restrict, const type); \
bool type##_stack_pop(type##_stack_s *const restrict); \
type type##_stack_peek(const type##_stack_s *const restrict); \
void type##_stack_reverse(type##_stack_s *const restrict);


/**
 * GENERATE_STACK macro
 * ----------------
 * Implements the stack functions for a type.
 * 
 * Parameters:
 *   type           - Element type
 *   len_type          - Unsigned integer type for length & size
 *   init_size      - Inline buffer size (initial size)
 *   growth_factor  - Multiplier for resizing
 *   validate_value - Function to validate a value (asserted in debug)
 * 
 * Output:
 *   Implementation of stack for type
 * 
 * Notes:
 *    Use only in a source (.c) file
 *    Use in combination with DEFINE_STACK(...)
 */
#define GENERATE_STACK(type, len_type, init_size, growth_factor, validate_value_fn, alloc_fn, realloc_fn, free_fn) \
   static_assert(init_size > 1, "Warning: init_size too small"); \
   static_assert(init_size < 256, "Warning: init_size too big"); \
   static_assert(init_size != 0 && (init_size & (init_size - 1)) == 0, "Warning: init_size must be a power of 2"); /* resize optimization */ \
   static_assert(growth_factor > 1, "Warning: growth_factor too small"); \
   static_assert(growth_factor < 32,  "Warning: growth_factor too big"); \
   static_assert(growth_factor != 0 && (growth_factor & (growth_factor - 1)) == 0, "Warning: growth_factor must be a power of 2"); /* resize optimization */ \
   assert_istype(type); \
   assert_istype(len_type); \
   assert_type(validate_value_fn((type){0}), bool); \
   assert_type(alloc_fn, void* (size_t)); \
   assert_type(realloc_fn, void* (void*, size_t)); \
   assert_type(free_fn, void (void*)); \
\
bool type##_stack_resize(type##_stack_s *const restrict stack) \
{ \
   assert(stack); \
   void *tmp; \
\
   if (stack->size > (len_type)(-1) / growth_factor) /* Prevent overflow */ \
      return false; \
   len_type new_size = stack->size * growth_factor; \
\
   if (stack->values == stack->inline_buffer) \
   { \
      tmp = alloc_fn(sizeof(type) * new_size); \
      if (!tmp)  \
         return false; \
      MEMORY_COPY(tmp, stack->values, stack->len * sizeof(type)); \
   } \
   else \
   { \
      tmp = realloc_fn(stack->values, sizeof(type) * new_size); \
      if (!tmp)  \
         return false; \
   } \
\
   stack->size = new_size; \
   stack->values = (type*)tmp; \
   return true; \
} \
\
void type##_stack_clear(type##_stack_s *const restrict stack) \
{ \
   assert(stack); \
   stack->len = 0; \
} \
\
void type##_stack_delete(type##_stack_s *const restrict stack) \
{ \
   assert(stack); \
   type##_stack_clear(stack); \
   if (stack->values != stack->inline_buffer) \
   { \
      free_fn(stack->values); \
      stack->values = stack->inline_buffer; \
      stack->size = init_size; \
   } \
} \
\
bool type##_stack_push(type##_stack_s *const restrict stack, const type value) \
{ \
   assert(stack); \
   assert(validate_value_fn(value)); \
   if (type##_stack_full(stack) && !type##_stack_resize(stack)) \
      return false; \
\
   stack->values[stack->len] = value; \
   stack->len++; \
   return true; \
} \
\
bool type##_stack_pop(type##_stack_s *const restrict stack) \
{ \
   assert(stack); \
   if (type##_stack_empty(stack)) \
      return false; \
\
   stack->len--; \
   return true; \
} \
\
type type##_stack_peek(const type##_stack_s *const restrict stack) \
{ \
   assert(stack); \
   assert(!type##_stack_empty(stack)); \
   return stack->values[stack->len - 1]; \
} \
\
void type##_stack_reverse(type##_stack_s *const restrict stack) \
{ \
   assert(stack); \
   if (stack->len < 2) \
      return; \
\
   for (len_type i = 0; i < stack->len / 2; i++) \
      SWAP(type, stack->values[i], stack->values[stack->len - 1 - i]); \
}


/**
 * stack(type) macro
 * -----------------
 * Declares a stack variable of the given type.
 * 
 * Parameters:
 *   type - The element type stored in the stack.
 * 
 * Usage (as variable):
 *   stack(int) s = stack_create(int);
 * 
 * Usage (as parameter):
 *   void process_stack(stack(int) *const s) { ... }
 * 
 * Notes:
 *   - This macro expands to the underlying stack struct type (type##_stack_s).
 */
#define stack(type) \
   type##_stack_s


/**
 * typecheck_stack_ptr macro
 * --------------------------
 * Compile-time validation that 'var' is a pointer to a stack of 'type'.
 *
 * Usage:
 *   typecheck_stack_ptr(var, type, expr);
 *
 * Ensures that 'var' is either a pointer to 'type##_stack_s' or
 * 'const type##_stack_s'. Useful for generic stack macros to produce
 * clear compile-time errors if a non-stack pointer is passed.
 *
 * Behavior:
 *   - C11+: uses typecheck_ptr with _Generic for compile-time checking.
 *   - C99 fallback: simply evaluates 'expr' (no type enforcement).
 */
#define typecheck_stack_ptr(var, type, expr) \
   typecheck_ptr(var, type##_stack_s, expr)


/**
 * Stack function macros
 * --------------------
 * Provides type-generic macros for stack operations.
 * 
 * Each macro wraps the corresponding type-specific function and ensures
 * the correct type is used.
 * 
 * Usage:
 *   stack(int) s;
 *   stack_init(int, &s);               // Create a new int stack
 *   stack_push(int, &s, 42);           // Push a value
 *   int top = stack_peek(int, &s);     // Peek at the top value
 *   stack_pop(int, &s);                // Pop the top value
 *   stack_clear(int, &s);              // Reset the stack
 *   stack_delete(int, &s);             // Free any heap memory
 */
#define stack_init(type, stack) \
   typecheck_stack_ptr(stack, type, \
      type##_stack_init((stack)) \
   )

#define stack_resize(type, stack) \
   typecheck_stack_ptr(stack, type, \
      type##_stack_resize((stack)) \
   )

#define stack_clear(type, stack) \
   typecheck_stack_ptr(stack, type, \
      type##_stack_clear((stack)) \
   )

#define stack_delete(type, stack) \
   typecheck_stack_ptr(stack, type, \
      type##_stack_delete((stack)) \
   )

#define stack_empty(type, stack) \
   typecheck_stack_ptr(stack, type, \
      type##_stack_empty((stack)) \
   )

#define stack_full(type, stack) \
   typecheck_stack_ptr(stack, type, \
      type##_stack_full((stack)) \
   )

#define stack_push(type, stack, value) \
   typecheck_stack_ptr(stack, type, \
      type##_stack_push((stack), (value)) \
   )

#define stack_pop(type, stack) \
   typecheck_stack_ptr(stack, type, \
      type##_stack_pop((stack)) \
   )

#define stack_peek(type, stack) \
   typecheck_stack_ptr(stack, type, \
      type##_stack_peek((stack)) \
   )

#define stack_reverse(type, stack) \
   typecheck_stack_ptr(stack, type, \
      type##_stack_reverse((stack)) \
   )


#endif /* __STACK_H */