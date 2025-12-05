#ifndef __DEQUE_H
#define __DEQUE_H

#include <stdbool.h>
#include <assert.h>
#include "static-assert.h"
#include "swap.h"
#include "memory-copy.h"


/**
 * DEFINE_DEQUE macro
 * ------------
 * Defines a generic deque type with an inline buffer of size `init_size`.
 *
 * Parameters:
 *   type      - Type of elements stored in the deque
 *   len_type  - Integer type used for length/size
 *   init_size - Number of elements to store inline before heap allocation
 * 
 * Output:
 *   Declaration of deque for type
 * 
 * Notes:
 *    Use only in a header (.h) file
 *    Use in combination with GENERATE_DEQUE(...), Ensure macro arguments match
 */
#define DEFINE_DEQUE(type, len_type, init_size) \
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
   len_type front; \
   len_type len; \
   len_type size; \
} type##_deque_s; \
\
static inline void type##_deque_init(type##_deque_s *const restrict deque) \
{ \
   deque->values = deque->inline_buffer; \
   deque->front = 0; \
   deque->len = 0; \
   deque->size = init_size; \
} \
\
static inline type type##_deque_peek_front(const type##_deque_s *const restrict deque) \
{ \
   assert(deque); \
   assert(!deque_empty(type, deque)); \
   return deque->values[deque->front]; \
} \
\
static inline type type##_deque_peek_back(const type##_deque_s *const restrict deque) \
{ \
   assert(deque); \
   assert(!deque_empty(type, deque)); \
   /* return deque->values[(deque->front + deque->len - 1) % deque->size]; */ \
   return deque->values[(deque->front + deque->len - 1) & (deque->size - 1)]; \
} \
\
bool type##_deque_resize(type##_deque_s *const restrict); \
void type##_deque_delete(type##_deque_s *const restrict); \
bool type##_deque_insert_front(type##_deque_s *const restrict, const type); \
bool type##_deque_insert_back(type##_deque_s *const restrict, const type); \
bool type##_deque_remove_front(type##_deque_s *const restrict); \
bool type##_deque_remove_back(type##_deque_s *const restrict);


/**
 * deque(type) macro
 * -----------------
 * Declares a deque variable of the given type.
 * 
 * Parameters:
 *   type - The element type stored in the deque.
 * 
 * Usage (as variable):
 *   deque(int) dq = deque_create(int);
 * 
 * Usage (as parameter):
 *   void process_stack(deque(int) *const dq) { ... }
 * 
 * Notes:
 *   - This macro expands to the underlying deque struct type (type##_deque_s).
 */
#define deque(type) \
   type##_deque_s


/**
 * typecheck_dequek_ptr macro
 * ---------------------------
 * Compile-time validation that 'var' is a pointer to a deque of 'type'.
 *
 * Usage:
 *   typecheck_dequek_ptr(var, type, expr);
 *
 * Ensures that 'var' is either a pointer to 'type##_sdeque_s' or
 * 'const type##_sdeque_s'. Useful for generic deque macros to produce
 * clear compile-time errors if a non-deque pointer is passed.
 *
 * Behavior:
 *   - C11+: uses typecheck_ptr with _Generic for compile-time checking.
 *   - C99 fallback: simply evaluates 'expr' (no type enforcement).
 */
#define typecheck_deque_ptr(var, type, expr) \
   typecheck_ptr(var, type##_deque_s, expr)


/**
 * Deque Expression Macros
 * -----------------------
 * Provide direct access to deque properties or simple computed expressions
 * without calling a function. These macros are type-checked at compile-time
 * (C11+) and perform a runtime NULL check (via assert) for safety.
 *
 * Each macro accepts:
 *   - type  : The element type stored in the deque.
 *   - deque : Pointer to the deque instance.
 *
 * Notes:
 *   - These macros expand to expressions, not function calls.
 *   - They can be safely used in conditions, assignments, and other expressions.
 *   - Runtime checks are included for non-NULL deque pointers.
 *   - Type safety is enforced at compile time in C11 and later.
 *
 * Example:
 *   deque(int) s;
 *   deque_init(int, &s);
 *   if (!deque_full(int, &s))
 *       deque_insert_back(int, &s, 42);
 *   int len = deque_len(int, &s);
 *   deque_clear(int, &s);
 */
#define deque_len(type, deque) \
   typecheck_deque_ptr(deque, type, \
      deque->len \
   )

#define deque_size(type, deque) \
   typecheck_deque_ptr(deque, type, \
      deque->size \
   )

#define deque_full(type, deque) \
   typecheck_deque_ptr(deque, type, \
      deque->len == deque->size \
   )

#define deque_empty(type, deque) \
   typecheck_deque_ptr(deque, type, \
      deque->len == 0 \
   )

#define deque_clear(type, deque) \
   typecheck_deque_ptr(deque, type, \
      deque->len = 0 \
   )


/**
 * GENERATE_DEQUE macro
 * ----------------
 * Implements the deque functions for a type.
 * 
 * Parameters:
 *   type           - Element type
 *   len_type       - Unsigned integer type for length & size
 *   init_size      - Inline buffer size (initial size)
 *   growth_factor  - Multiplier for resizing
 *   validate_value - Function to validate a value (asserted in debug)
 * 
 * Output:
 *   Implementation of deque for type
 * 
 * Notes:
 *    Use only in a source (.c) file
 *    Use in combination with DEFINE_DEQUE(...), Ensure macro arguments match
 */
#define GENERATE_DEQUE(type, len_type, init_size, growth_factor, validate_value_fn, alloc_fn, realloc_fn, free_fn) \
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
bool type##_deque_resize(type##_deque_s *const restrict deque) \
{ \
   assert(deque); \
   void *tmp; \
\
   if (deque->size > (len_type)(-1) / growth_factor) /* Prevent overflow */ \
      return false; \
   len_type new_size = deque->size * growth_factor; \
\
   const bool in_heap = (deque->values != deque->inline_buffer); /* underlying array is allocated in heap */ \
   const bool not_wrapped = (deque->front + deque->len <= deque->size); /* Elements not wrapped around the underlying array */ \
\
   if (not_wrapped && in_heap) \
   { \
      tmp = realloc_fn(deque->values, sizeof(type) * new_size); \
      if (!tmp)  \
         return false; \
   } \
   else \
   { \
      tmp = alloc_fn(sizeof(type) * new_size); \
      if (!tmp) \
         return false; \
\
      if (not_wrapped) \
      { \
         MEMORY_COPY(tmp, deque->values, sizeof(type) * deque->len); \
      } \
      else \
      { \
         len_type first_chunk = deque->size - deque->front; \
         MEMORY_COPY(tmp, &deque->values[deque->front], sizeof(type) * first_chunk); \
         MEMORY_COPY((type*)tmp + first_chunk, deque->values, sizeof(type) * deque->front); \
      } \
\
      if (in_heap) \
         free_fn(deque->values); \
   } \
\
   deque->size = new_size; \
   deque->values = (type*)tmp; \
   if (!not_wrapped) \
      deque->front = 0; \
   return true; \
} \
\
void type##_deque_delete(type##_deque_s *const restrict deque) \
{ \
   assert(deque); \
   deque_clear(type, deque); \
   deque->front = 0; \
   if (deque->values != deque->inline_buffer) \
   { \
      free_fn(deque->values); \
      deque->values = deque->inline_buffer; \
      deque->size = init_size; \
   } \
} \
\
bool type##_deque_insert_front(type##_deque_s *const restrict deque, const type value) \
{ \
   assert(deque); \
   assert(validate_value_fn(value)); \
   if (deque_full(type, deque) && !type##_deque_resize(deque)) \
      return false; \
   deque->len++; \
   /* deque->front = (deque->front + deque->size - 1) % deque->size; */ \
   deque->front = (deque->front + deque->size - 1) & (deque->size - 1); \
   deque->values[deque->front] = value; \
   return true; \
} \
\
bool type##_deque_insert_back(type##_deque_s *const restrict deque, const type value) \
{ \
   assert(deque); \
   assert(validate_value_fn(value)); \
   if (deque_full(type, deque) && !type##_deque_resize(deque)) \
      return false; \
   /* deque->values[(deque->front + deque->len) % deque->size] = value; */ \
   deque->values[(deque->front + deque->len) & (deque->size - 1)] = value; \
   deque->len++; \
   return true; \
} \
\
bool type##_deque_remove_front(type##_deque_s *const restrict deque) \
{ \
   assert(deque); \
   if (deque_empty(type, deque)) \
      return false; \
   /* deque->front = (deque->front + 1) % deque->size; */ \
   deque->front = (deque->front + 1) & (deque->size - 1); \
   deque->len--; \
   return true; \
} \
\
bool type##_deque_remove_back(type##_deque_s *const restrict deque) \
{ \
   assert(deque); \
   if (deque_empty(type, deque)) \
      return false; \
   deque->len--; \
   return true; \
}



/**
 * Deque function macros
 * --------------------
 * Provides type-generic macros for deque operations.
 * 
 * Each macro wraps the corresponding type-specific function and ensures
 * the correct type is used.
 * 
 * Usage:
 *   deque(int) dq = deque_create(int);    // Create a new int deque
 *   deque_insert_back(int, &dq, 42);      // Insert a value
 *   int top = deque_peek_front(int, &dq);  // Peek at the top value
 *   deque_remove_front(int, &dq);          // Pop the top value
 *   deque_clear(int, &dq);                // Reset the deque
 *   deque_delete(int, &dq);               // Free any heap memory
 */
#define deque_init(type, deque) \
   typecheck_deque_ptr(deque, type, \
      type##_deque_init((deque)) \
   )

#define deque_peek_front(type, deque) \
   typecheck_deque_ptr(deque, type, \
      type##_deque_peek_front((deque)) \
   )

#define deque_peek_back(type, deque) \
   typecheck_deque_ptr(deque, type, \
      type##_deque_peek_back((deque)) \
   )

#define deque_resize(type, deque) \
   typecheck_deque_ptr(deque, type, \
      type##_deque_resize((deque)) \
   )

#define deque_delete(type, deque) \
   typecheck_deque_ptr(deque, type, \
      type##_deque_delete((deque)) \
   )

#define deque_insert_front(type, deque, value) \
   typecheck_deque_ptr(deque, type, \
      type##_deque_insert_front((deque), (value)) \
   )

#define deque_insert_back(type, deque, value) \
   typecheck_deque_ptr(deque, type, \
      type##_deque_insert_back((deque), (value)) \
   )

#define deque_remove_front(type, deque) \
   typecheck_deque_ptr(deque, type, \
      type##_deque_remove_front((deque)) \
   )

#define deque_remove_back(type, deque) \
   typecheck_deque_ptr(deque, type, \
      type##_deque_remove_back((deque)) \
   )


#endif /* __DEQUE_H */