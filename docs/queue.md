# Queue Library (Generic, Type-Safe, Header-Only Interface)

A lightweight, efficient, type-safe generic queue (circular buffer) implementation for C, supporting inline storage, dynamic growth, modular validation, and compile-time type safety.

The design prioritizes:

- **Performance** — inline buffer, circular indexing, contiguous memory
- **Safety** — macro-based type checking, strong contract assertions
- **Flexibility** — custom allocators, customizable validation
- **Low overhead** — header-only interface, no void* containers


## Features

- Inline circular buffer for fast small queues
- Automatic dynamic resizing (power-of-two growth)
- Type-safe API via generated functions and wrapper macros
- Optional element validation in debug builds
- Custom user-supplied alloc/realloc/free
- Reversal, clearing, and deletion operations
- Strong design-by-contract behavior with explicit assertions



# Design Choices & Rationale

## 1. Circular Buffer With Inline Storage

Each queue begins with a fixed-size circular inline buffer (init_size).

Advantages:

- No heap allocation for small queues
- Cache-friendly contiguous memory
- Natural wraparound behavior using & (size - 1)
- First resize copies data into a flat, contiguous buffer


## 2. enqueue() and dequeue() Return Only Status

- enque() returns false on allocation failure
- deque() returns false on empty queue

Returning the removed value is intentionally avoided (performance + safety)

To dequeue while retrieving the value:

```c
T value = queue_peek(T, &q);
queue_deque(T, &q);
```


## 3. peek() Treats Empty Queue as Programmer Error

This library uses design-by-contract semantics for peek().

```c
assert(!queue_empty(queue));
```

Meaning:

- Calling `peek()` on an empty queue is a bug, not a recoverable error
- In debug builds → aborts with assertion failure
- In release builds (`NDEBUG`) → precondition disappears, and misuse results in undefined behavior (as intended)

To perform a safe runtime check, you must manually guard with:

```c
if (!queue_empty(T, &q)) {
    T v = queue_peek(T, &q);
}
```


## 4. Power-of-Two Sizes and Growth

Both *init_size* and *growth_factor* must be powers of two.

Benefits:

- Efficient modulo via masking: (i & (size - 1))
- Predictable geometric growth
- Lower fragmentation
- Simple wraparound logic


## 5. Full Compile-Time Type Safety

The public API uses macros like:

```c
queue_enque(int, &s, 42);
```

internally validated by:

```c
typecheck_queue_ptr(queue, type, expr)
```

This ensures:

- `queue(int)` is only used with the int specialization
- Accidental mixing of types produces compile-time errors


Guarantees:

- Only queue(type) works with type operations
- Passing mismatched queue types gives compile-time error
- No void*, no casts, no surprises


## 6. Validation Function for Enqueued Values

Debug builds assert:

```c
assert(validate_value_fn(value));
```

Useful for:

- Range checking
- Sanitizing user data
- Guaranteeing invariants



# API Overview

All operations are type-specific once generated.

1. Constructor / Destructor

- `type_queue_init(queue*)` — Initialize queue using inline circular buffer
- `type_queue_delete(queue*)` — Free heap storage if used
- `type_queue_clear(queue*)` — Reset queue to empty state

2. Capacity Functions

- `type_queue_empty(queue*) → bool` — len == 0
- `type_queue_full(queue*) → bool` — len == size
- `type_queue_resize(queue*) → bool` — Grow using growth_factor

3. Mutation Functions

- `type_queue_enque(queue*, value)` → bool — Append a value at tail (may resize)
- `type_queue_deque(queue*) → bool` — Remove head element
- `type_queue_reverse(queue*)` — Reverse in-place

4. View Functions

- `type_queue_peek(queue*) → type` — Returns head element; asserts non-empty



# Macros for User-Facing API

To call the generated functions with type safety:

```c
queue_init(type, qptr)
queue_delete(type, qptr)
queue_clear(type, qptr)
queue_resize(type, qptr)

queue_enque(type, qptr, value)
queue_deque(type, qptr)

queue_empty(type, qptr)
queue_full(type, qptr)

queue_peek(type, qptr)
queue_reverse(type, qptr)
```

Queue type shorthand:

```c
queue(type)   // expands to type##_queue_s
```



# Usage Example #1

```c
#include <stdlib.h>
#include "stack.h"

#define INT_QUEUE_BUFFER 8
#define INT_QUEUE_GROWTH 4

static bool validate_int(int x)
{ 
    return true; 
}

DEFINE_STACK(int, unsigned int, INT_QUEUE_BUFFER)
GENERATE_STACK(int. unsigned int, INT_QUEUE_BUFFER, INT_QUEUE_GROWTH, validate_int, malloc, realloc, free)

int main()
{
    queue(int) s;
    queue_init(int, &s);

    queue_enque(int, &s, 3);
    queue_enque(int. &s, -1);
    queue_enque(int, &s, 64);
    queue_enque(int, &s, 10);

    int x = queue_peek(int, &s); // 3

    queue_reverse(int, &s);
    int y = queue_peek(int, &s); // 10

    queue_deque(int, &s);
    int z = queue_peek(int, &s); // 64;

    stack_delete(int, &s); // Recommended to avoid memory leaks    
    return 0;
}
```



# Usage Example #2


1. In your header file (.h)

```c
#include <stdint.h>
#include "queue.h"

#define INT_QUEUE_BUFFER 16
#define INT_QUEUE_GROW   4

DEFINE_QUEUE(int, uint32_t, INT_QUEUE_BUFFER);
```

2. In your implementation file (.c)

```c
#include <stdlib.h>
#include "your_queue_header.h"

static bool validate_int(int x) {
    return true; /* accept any integer */
}

GENERATE_QUEUE(
    int,                // type
    uint32_t,           // len_type
    INT_QUEUE_BUFFER,   // inline buffer size (power of 2)
    INT_QUEUE_GROW,     // growth factor (power of 2)
    validate_int,       // value validation function
    malloc,             // allocate
    realloc,            // reallocate
    free                // free
);
```

3. Using the queue

```c
#include <stdio.h> /* for printf(...) */
#include "your_queue_header.h"

int main() {
    queue(int) q;
    queue_init(int, &q);

    queue_enque(int, &q, 10);
    queue_enque(int, &q, 20);

    if (!queue_empty(int, &q)) {
        int head = queue_peek(int, &q);
        printf("Head = %d\n", head);
    }

    queue_deque(int, &q);
    queue_deque(int, &q);

    queue_delete(int, &q);
}
```



# Error Handling Model

- `enqueue()`: may fail (returns false) if allocation fails
- `dequeue()`: Returns false if queue is empty
- `peek()`: asserts and aborts in debug builds if the queue is empty
- `resize()`: returns false on allocation failure

Runtime safety is explicit; misuse is treated as a programming error.



# Notes & Best Practices

- Always guard `queue_peek()` with `queue_empty()` in production code when you might call `queue_peek()` on an empty queue
- After `queue_delete()`, the queue returns to its inline buffer
- For large element types, store pointers instead of by-value objects
- Avoid mixing queue specializations — type safety is strict
- Inline buffer + circular wrap gives excellent cache performance