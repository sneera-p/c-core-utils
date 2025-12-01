#ifndef __QUEUE_H
#define __QUEUE_H

#include <stdbool.h>
#include <assert.h>
#include "static-assert.h"
#include "swap.h"
#include "memory-copy.h"


/**
 * DEFINE_QUEUE macro
 * ------------
 * Defines a generic queue type with an inline buffer of size `init_size`.
 *
 * Parameters:
 *   type      - Type of elements stored in the queue
 *   len_type  - Integer type used for length/size
 *   init_size - Number of elements to store inline before heap allocation
 * 
 * Output:
 *   Declaration of queue for type
 * 
 * Notes:
 *    Use only in a header (.h) file
 *    Use in combination with GENERATE_QUEUE(...), Ensure macro arguments match
 */
#define DEFINE_QUEUE(type, len_type, init_size) \
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
   len_type head; \
   len_type len; \
   len_type size; \
} type##_queue_s; \
\
static inline void type##_queue_init(type##_queue_s *const restrict queue) \
{ \
   queue->values = queue->inline_buffer; \
   queue->head = 0; \
   queue->len = 0; \
   queue->size = init_size; \
} \
static inline bool type##_queue_empty(const type##_queue_s *const restrict queue) \
{ \
   assert(queue); \
   return queue->len == 0; \
} \
static inline bool type##_queue_full(const type##_queue_s *const restrict queue) \
{ \
   assert(queue); \
   return queue->len == queue->size; \
} \
bool type##_queue_resize(type##_queue_s *const restrict); \
void type##_queue_clear(type##_queue_s *const restrict); \
void type##_queue_delete(type##_queue_s *const restrict); \
bool type##_queue_enque(type##_queue_s *const restrict, const type); \
bool type##_queue_deque(type##_queue_s *const restrict); \
type type##_queue_peek(const type##_queue_s *const restrict); \
void type##_queue_reverse(type##_queue_s *const restrict);


/**
 * GENERATE_QUEUE macro
 * ----------------
 * Implements the queue functions for a type.
 * 
 * Parameters:
 *   type           - Element type
 *   len_type       - Unsigned integer type for length & size
 *   init_size      - Inline buffer size (initial size)
 *   growth_factor  - Multiplier for resizing
 *   validate_value - Function to validate a value (asserted in debug)
 * 
 * Output:
 *   Implementation of queue for type
 * 
 * Notes:
 *    Use only in a source (.c) file
 *    Use in combination with DEFINE_QUEUE(...), Ensure macro arguments match
 */
#define GENERATE_QUEUE(type, len_type, init_size, growth_factor, validate_value_fn, alloc_fn, realloc_fn, free_fn) \
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
bool type##_queue_resize(type##_queue_s *const restrict queue) \
{ \
   assert(queue); \
   void *tmp; \
\
   if (queue->size > (len_type)(-1) / growth_factor) /* Prevent overflow */ \
      return false; \
   len_type new_size = queue->size * growth_factor; \
\
   const bool in_heap = (queue->values != queue->inline_buffer); /* underlying array is allocated in heap */ \
   const bool not_wrapped = (queue->head + queue->len <= queue->size); /* Elements not wrapped around the underlying array */ \
\
   if (not_wrapped && in_heap) \
   { \
      tmp = realloc_fn(queue->values, sizeof(type) * new_size); \
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
         MEMORY_COPY(tmp, queue->values, sizeof(type) * queue->len); \
      } \
      else \
      { \
         len_type first_chunk = queue->size - queue->head; \
         MEMORY_COPY(tmp, &queue->values[queue->head], sizeof(type) * first_chunk); \
         MEMORY_COPY((type*)tmp + first_chunk, queue->values, sizeof(type) * queue->head); \
      } \
\
      if (in_heap) \
         free_fn(queue->values); \
   } \
\
   queue->size = new_size; \
   queue->values = (type*)tmp; \
   if (!not_wrapped) \
      queue->head = 0; \
   return true; \
} \
\
void type##_queue_clear(type##_queue_s *const restrict queue) \
{ \
   assert(queue); \
   queue->len = 0; \
   queue->head = 0; \
} \
\
void type##_queue_delete(type##_queue_s *const restrict queue) \
{ \
   assert(queue); \
   type##_queue_clear(queue); \
   if (queue->values != queue->inline_buffer) \
   { \
      free_fn(queue->values); \
      queue->values = queue->inline_buffer; \
      queue->size = init_size; \
   } \
} \
\
bool type##_queue_enque(type##_queue_s *const restrict queue, const type value) \
{ \
   assert(queue); \
   assert(validate_value_fn(value)); \
   if (type##_queue_full(queue) && !type##_queue_resize(queue)) \
      return false; \
   /* queue->values[(queue->head + queue->len) % queue->size] = value; */ \
   queue->values[(queue->head + queue->len) & (queue->size - 1)] = value; \
   queue->len++; \
   return true; \
} \
\
bool type##_queue_deque(type##_queue_s *const restrict queue) \
{ \
   assert(queue); \
   if (type##_queue_empty(queue)) \
      return false; \
   /* queue->head = (queue->head + 1) % queue->size; */ \
   queue->head = (queue->head + 1) & (queue->size - 1); \
   queue->len--; \
   return true; \
} \
\
type type##_queue_peek(const type##_queue_s *const restrict queue) \
{ \
   assert(queue); \
   assert(!type##_queue_empty(queue)); \
   return queue->values[queue->head]; \
} \
\
void type##_queue_reverse(type##_queue_s *const restrict queue) \
{ \
   assert(queue); \
   if (queue->len < 2) \
      return; \
\
   len_type head = queue->head; \
   len_type tail = queue->head + queue->len - 1; \
   len_type mask = queue->size - 1; \
   for (len_type i = 0; i < queue->len / 2; i++) \
      /* SWAP(type, queue->values[(head + i) % queue->size], queue->values[(tail - i) % queue->size]); */ \
      SWAP(type, queue->values[(head + i) & mask], queue->values[(tail - i) & mask]); \
}


/**
 * queue(type) macro
 * -----------------
 * Declares a queue variable of the given type.
 * 
 * Parameters:
 *   type - The element type stored in the queue.
 * 
 * Usage (as variable):
 *   queue(int) dq = queue_create(int);
 * 
 * Usage (as parameter):
 *   void process_queue(queue(int) *const dq) { ... }
 * 
 * Notes:
 *   - This macro expands to the underlying queue struct type (type##_queue_s).
 */
#define queue(type) \
   type##_queue_s


/**
 * Deque function macros
 * --------------------
 * Provides type-generic macros for queue operations.
 * 
 * Each macro wraps the corresponding type-specific function and ensures
 * the correct type is used.
 * 
 * Usage:
 *   queue(int) q = queue_create(int);    // Create a new int queue
 *   queue_insert_tail(int, &q, 42);      // Insert a value
 *   int top = queue_peek_head(int, &q);  // Peek at the top value
 *   queue_remove_head(int, &q);          // Pop the top value
 *   queue_clear(int, &q);                // Reset the queue
 *   queue_delete(int, &q);               // Free any heap memory
 */
#define queue_init(type, queue) \
   type##_queue_init((queue))

#define queue_resize(type, queue) \
   type##_queue_resize((queue))

#define queue_clear(type, queue) \
   type##_queue_clear((queue))

#define queue_delete(type, queue) \
   type##_queue_delete((queue))

#define queue_empty(type, queue) \
   type##_queue_empty((queue))

#define queue_full(type, queue) \
   type##_queue_full((queue))

#define queue_enque(type, queue, value) \
   type##_queue_enque((queue), (value))

#define queue_deque(type, queue) \
   type##_queue_deque((queue))

#define queue_peek(type, queue) \
   type##_queue_peek((queue))

#define queue_reverse(type, queue) \
   type##_queue_reverse((queue))


#endif /* __QUEUE_H */