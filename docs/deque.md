# Deque Library (Generic, Type-Safe, Header-Only Interface)

A lightweight, efficient, type-safe generic deque (double-ended queue) implementation for C, supporting inline storage, dynamic growth, modular validation, and compile-time type safety.

## Features

- Inline circular buffer for fast small deques
- Automatic dynamic resizing (power-of-two growth)
- Type-safe API via generated functions and wrapper macros
- Optional element validation in debug builds
- Custom user-supplied allocators, reallocation, and free functions
- Deques on both ends (insert/remove from both the front and the back)
- Strong design-by-contract behavior with explicit assertions



# Design Choices & Rationale


## 1. Circular Buffer With Inline Storage

Each deque begins with a fixed-size circular inline buffer (init_size), which provides the following benefits:

- No heap allocation for small deques — improves performance.
- Cache-friendly contiguous memory.
- Wraparound behavior using the bitwise operation & (size - 1).


## 2. Operations Return Status

Functions

- `insert_front()`
- `insert_back()`
- `remove_front()`
- `remove_back()` 

return a status (true/false)

- Insert/Remove Failure: Allocation failure or deque being full/empty results in a return value of false.
- Peek Operations: peek_front() and peek_back() return values or assert if the deque is empty.


## 3. Full Compile-Time Type Safety

The library uses macros to ensure that only the correct deque type can be used, preventing any type mismatches. For example:

```c
deque(int) dq;
deque_init(int, &dq);
deque_insert_back(int, &dq, 10); // Valid
```

If a mismatch occurs, a compile-time error is generated.


## 4. Power-of-Two Sizes and Growth

- Both init_size and growth_factor must be powers of two for efficiency in memory allocation and modulo operation.
- Growth Factor: Resizing of the deque occurs geometrically by a factor of growth_factor.



# API Overview

All operations are type-specific once generated. The core operations available are:

1. Constructor / Destructor

- type_deque_init(deque*) — Initialize deque using inline circular buffer.
- type_deque_delete(deque*) — Frees heap memory, if used.
- type_deque_clear(deque*) — Resets deque to empty state.

2. Capacity Functions

- type_deque_empty(deque*) → bool — Returns true if the deque is empty.
- type_deque_full(deque*) → bool — Returns true if the deque is full.
- type_deque_resize(deque*) → bool — Grows the deque using growth_factor.

3. Mutation Functions

- type_deque_insert_front(deque*, value) — Inserts an element at the front of the deque.
- type_deque_insert_back(deque*, value) — Inserts an element at the back of the deque.
- type_deque_remove_front(deque*) — Removes the element at the front.
- type_deque_remove_back(deque*) — Removes the element at the back.

4. View Functions

- type_deque_peek_front(deque*) → type — Returns the front element.
- type_deque_peek_back(deque*) → type — Returns the back element.



# Macros for User-Facing API

To call the generated functions with type safety:

```c
deque_init(type, deque)      // Initialize the deque
deque_resize(type, deque)    // Resize the deque
deque_clear(type, deque)     // Clear the deque
deque_delete(type, deque)    // Delete the deque
deque_empty(type, deque)     // Check if deque is empty
deque_full(type, deque)      // Check if deque is full
deque_insert_front(type, deque, value) // Insert at the front
deque_insert_back(type, deque, value) // Insert at the back
deque_remove_front(type, deque)       // Remove from the front
deque_remove_back(type, deque)       // Remove from the back
deque_peek_front(type, deque)         // Peek at the front
deque_peek_back(type, deque)         // Peek at the back
```

Deque type shorthand:

```c
deque(type)  // Expands to type##_deque_s
```



# Usage Example

1. In your header file (.h)

```c
#include <stdint.h>
#include "deque.h"

#define INT_DEQUE_BUFFER 16
#define INT_DEQUE_GROW   4

DEFINE_DEQUE(int, uint32_t, INT_DEQUE_BUFFER);
```

2. In your implementation file (.c)

```c
#include <stdlib.h>
#include "your_deque_header.h"

static bool validate_int(int x) {
    return true; // Accept any integer
}

GENERATE_DEQUE(
    int,                // type
    uint32_t,           // len_type
    INT_DEQUE_BUFFER,   // inline buffer size (power of 2)
    INT_DEQUE_GROW,     // growth factor (power of 2)
    validate_int,       // value validation function
    malloc,             // allocate
    realloc,            // reallocate
    free                // free
);
```

3. Using the deque

```c
#include <stdio.h>  // for printf
#include "your_deque_header.h"

int main() {
    deque(int) dq;
    deque_init(int, &dq);

    deque_insert_back(int, &dq, 10);
    deque_insert_back(int, &dq, 20);

    if (!deque_empty(int, &dq)) {
        int front = deque_peek_front(int, &dq);
        printf("Head = %d\n", front);
    }

    deque_remove_front(int, &dq);
    deque_remove_back(int, &dq);

    deque_delete(int, &dq);
}
```



# Error Handling Model

- `insert_front()` / `insert_back()`: Return false if the deque is full or allocation fails.
- `remove_front()` / `remove_back()`: Return false if the deque is empty.
- `peek_front()` / `peek_back()`: Assert in debug builds if the deque is empty.
- `resize()`: Returns false if resizing fails due to allocation issues.



# Notes & Best Practices

- Always guard `deque_peek_front()` and `deque_peek_back()` with `deque_empty()` in production code when you might call these on an empty deque.
- After calling `deque_delete()`, the deque returns to its inline buffer.
- For large element types, store pointers instead of by-value objects.
- Avoid mixing deque specializations — type safety is strict, and mismatched types will result in compile-time errors.