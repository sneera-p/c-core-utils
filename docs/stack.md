# Stack Library (Generic, Type-Safe, Header-Only Interface)

A lightweight, efficient, type-safe generic stack implementation for C, supporting both inline storage and dynamic growth. This library uses macros to generate fully specialized stack types at compile time, with strong type checking and optional debug-time assertions.

The design prioritizes:
- Performance (inline buffer, contiguous memory, power-of-two resizing)
- Safety (type checking via macros, contract-style assertions)
- Flexibility (custom allocators, validation functions)
- Low overhead (no virtual dispatch, no generic void* containers)


## Features

- Inline buffer for small stacks (no heap allocation until growth is needed)
- Automatic dynamic resizing (power-of-two growth)
- Type-safe interface using generated functions & macros
- Optional element validation in debug builds
- Custom alloc/realloc/free injected by user
- Optional reversing, clearing, and deletion
- Strong contract assumptions with assertions



# Design Choices & Rationale

This implementation makes several intentional choices for performance and safety.
These behaviors should be understood when using the stack.


## 1. Inline Buffer First, Heap Allocation Later

Each stack starts with a fixed-size inline array (init_size).
This means:

- Small stacks avoid heap allocation entirely
- Cache locality is improved
- First resize allocates a flat buffer and copies existing elements
- Further resizes use realloc

This hybrid design offers significant performance improvements for small or short-lived stacks.


## 2. pop() Does Not Return the Popped Value

The signature:

```c
bool type##_stack_pop(type##_stack_s *stack);
```

pop() only removes the top element and returns false if the stack is empty.

Why?

- Returning a value from pop() can trigger unnecessary copying of large types
- Separating pop() and peek() avoids double reads
- Aligns with "check, then act" usage style common in C
- Ensures predictable API symmetry

To retrieve the top value before popping:

```c
T value = stack_peek(T, &stack);
stack_pop(T, &stack);
```


## 3. peek() Treats Empty Stack as a Programmer Error

This library uses design-by-contract semantics for peek().

```c
assert(!stack_empty(stack));
```

Meaning:

- Calling peek() on an empty stack is a bug, not a recoverable error
- In debug builds → aborts with assertion failure
- In release builds (NDEBUG) → precondition disappears, and misuse results in undefined behavior (as intended)

To perform a safe runtime check, you must manually guard with:

```c
if (!stack_empty(T, &stack)) {
    T v = stack_peek(T, &stack);
}
```


## 4. Resizing Uses Power-of-Two Growth

Your init_size and growth_factor are both required to be powers of two.

Benefits:

- Prevents overflow risks
- Enables bitwise optimizations
- Ensures predictable geometric growth
- Reduces memory fragmentation


## 5. Type Safety Guaranteed by Macros

The public API uses macros like:

```c
stack_push(int, &s, 42);
```

internally validated by:

```c
typecheck_stack_ptr(stack, type, expr)
```

This ensures:

- stack(int) is only used with the int specialization
- Accidental mixing of types produces compile-time errors


## 6. Validation Function for Values

During generation:

```c
assert_type(validate_value_fn((type){0}), bool);
```

validate_value_fn() is called in debug builds (assert(validate_value_fn(value));).

This allows:

- Range checking
- Sanitization
- Detecting invalid states early

In release builds, the checks disappear entirely.



# API Overview

All operations are type-specific once generated.

<!-- Constructor / Destructor
Function	Description
type_stack_init(stack*)	Initialize stack using inline buffer
type_stack_delete(stack*)	Free heap storage if used
type_stack_clear(stack*)	Reset length to 0
Capacity Functions
Function	Description
type_stack_empty(stack*) → bool	Returns whether length == 0
type_stack_full(stack*) → bool	Returns whether length == size
type_stack_resize(stack*) → bool	Grows stack using growth_factor
Mutation Functions
Function	Description
type_stack_push(stack*, value) → bool	Push value (may resize)
type_stack_pop(stack*) → bool	Remove top element, no return
type_stack_reverse(stack*)	Reverse in-place
Inspection Functions
Function	Description
type_stack_peek(stack*) → type	Returns top value; asserts non-empty -->

# Macros for User-Facing API

To call the generated functions with type safety:

```c
stack_init(type, stack_ptr)
stack_delete(type, stack_ptr)
stack_push(type, stack_ptr, value)
stack_pop(type, stack_ptr)
stack_peek(type, stack_ptr)
stack_empty(type, stack_ptr)
stack_full(type, stack_ptr)
stack_resize(type, stack_ptr)
stack_reverse(type, stack_ptr)
stack_clear(type, stack_ptr)
```

# Usage Example

1. In your header file (.h)

```c
#include <stdint.h>
#include "stack.h"

#define INT_STACK_BUFFER 16
#define INT_STACK_GROW 4
DEFINE_STACK(int, uint32_t, INT_STACK_BUFFER);
```

2. In your implementation file (.c)

```c
#include <stdlib.h>
#include "your_stack_header.h"

static bool validate_int(int x)
{ 
	return true; 
}

GENERATE_STACK(
    int,            		// type
    uint32_t,          	// len_type
    INT_STACK_BUFFER,   // inline buffer size (must be power of 2)
    INT_STACK_GROW,     // growth factor (must be power of 2)
    validate_int,   		// validation function
    malloc,         		// allocator
    realloc,        		// reallocator
    free            		// free
);
```

3. Using the stack

```c
#include <stdio.h> /* for printf(...) */
#include "your_stack_header.h"

int main() {
    stack(int) s;
    stack_init(int, &s);

    stack_push(int, &s, 10);
    stack_push(int, &s, 20);

    // Peek safely
    if (!stack_empty(int, &s)) {
        int top = stack_peek(int, &s);
        printf("Top = %d\n", top);
    }

    stack_pop(int, &s);
    stack_pop(int, &s);

    stack_delete(int, &s);
}
```



# Error Handling Model

- push(): may fail (returns false) if allocation fails
- pop(): returns false when called on empty stack
- peek(): asserts and aborts in debug builds if the stack is empty
- resize(): returns false on allocation failure

Runtime safety is explicit; misuse is treated as a programming error.



# Notes & Best Practices

- Always check stack_empty() before calling peek() in production code
- For large structures, prefer peek() + pop() instead of copying via pop
- After calling stack_delete(), the stack returns to its inline buffer
- If your type is large, consider using pointers to reduce copying
- You can embed the stack struct directly (no dynamic allocation needed)