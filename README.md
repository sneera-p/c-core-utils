# Generic Containers for C – Stack, Queue, Deque + Utility Macros

Fast, type-safe, allocation-efficient containers in pure C using macros.

## Features
- Compile-time type safety (C11) with C99 fallback
- Inline buffer → no heap until you exceed initial size
- Power-of-2 sizing + exponential growth
- True circular buffers (Queue & Deque)
- Works on GCC, Clang, MSVC, ARMCC, IAR, etc.
- Optional validation + custom allocators
- Full CMocka test suite

---

### Utility Macros Example

#### packed-enum.h – Force 1-byte enums (saves memory!)

```c
#include "packed-enum.h"

// Old way (might be 4 bytes):
// enum color { RED = 1, GREEN, BLUE };

// New way – guaranteed 1 byte on all compilers:
PACKED_ENUM(color_e) {
    RED   = 1,
    GREEN = 2,
    BLUE  = 3
};

typedef PACKED_ENUM(state_e) {
    IDLE,
    RUNNING,
    ERROR
} state_t;

struct entity {
    PACKED_ENUM(color_e) color;
    state_t              state;
    // ... other fields
};
// → Only 2 bytes for both enums instead of 8!
```


#### static-assert.h – Compile-time checks

```c
#include "static-assert.h"

// Basic static assertion
static_assert(sizeof(int) >= 4, "This code requires 32-bit ints");

// Check that an expression has the correct type
void process_float(float f);
float x = 3.14f;
assert_type(x, float);        // OK
assert_type(x, double);       // Compile error!

// Ensure macro argument is a type, not a variable
#define MAKE_CONTAINER(T) \
    assert_istype(T); \
    DEFINE_STACK(T, size_t, 8)

MAKE_CONTAINER(int);          // OK
MAKE_CONTAINER(42);           // Compile error: "42 is not a type"

// Type-safe pointer check (C11+)
typecheck_ptr(&x, float, process_float(x));  // OK
// typecheck_ptr(&x, double, ...)            // Compile error
```


#### swap.h – Type-safe swap with compile-time checks

```c
#include "swap.h"

int a = 10, b = 20;
SWAP(int, a, b);        // a == 20, b == 10

float f1 = 1.5f, f2 = 2.8f;
SWAP(float, f1, f2);

// Compile-time safety:
// SWAP(int, a, f1);    // Error: types don't match
// SWAP(int, a, 42);    // Error: second argument not modifiable
```


#### memory-copy.h – Optimized memcpy with compiler hints

```c
#include "memory-copy.h"

typedef struct { float v[100]; } big_t;
big_t src = { .v = {1.0f} };
big_t dst;

MEMORY_COPY(&dst, &src, sizeof(big_t));  // often faster than plain memcpy()
```


### Stack Example (LIFO)

➡️ **[Full Stack Documentation](docs/stack.md)**  

```c
// my_stack.h
#pragma once
#include "stack.h"

#define INT_STACK_INIT_SIZE  8
#define INT_STACK_GROWTH     2
DEFINE_STACK(int, size_t, INT_STACK_INIT_SIZE)
```

```c
// my_stack.c
#include "my_stack.h"
#include <stdlib.h>

static bool validate_int(int v) { return v >= 0; }

GENERATE_STACK(int, size_t, INT_STACK_INIT_SIZE, INT_STACK_GROWTH, validate_int, malloc, realloc, free)
```

```c
// usage.c
#include "my_stack.h"
#include <stdio.h>

void demo_stack(void)
{
    stack(int) s;
    stack_init(int, &s);

    stack_push(int, &s, 100);
    stack_push(int, &s, 200);
    stack_push(int, &s, 300);

    printf("Stack contents (top to bottom):\n");
    while (!stack_empty(int, &s)) {
        printf("  %d\n", stack_peek(int, &s));
        stack_pop(int, &s);
    }

    stack_delete(int, &s);  // frees heap memory if any
}
```

### Queue Example (FIFO)

```c
// my_queue.h
#pragma once
#include "queue.h"

#define FLOAT_QUEUE_INIT_SIZE  16
#define FLOAT_QUEUE_GROWTH     2
DEFINE_QUEUE(float, size_t, FLOAT_QUEUE_INIT_SIZE)
```

```c
// my_queue.c
#include "my_queue.h"
#include <stdlib.h>

static bool validate_float(float f) { return f == f; }  // no NaN

GENERATE_QUEUE(float, size_t, FLOAT_QUEUE_INIT_SIZE, FLOAT_QUEUE_GROWTH, validate_float, malloc, realloc, free)
```

```c
// usage.c
#include "my_queue.h"
#include <stdio.h>

void demo_queue(void)
{
    queue(float) q;
    queue_init(float, &q);

    for (int i = 0; i < 50; ++i)
        queue_enque(float, &q, i * 0.5f);

    printf("Queue contents (front to back):\n");
    while (!queue_empty(float, &q)) {
        printf("  %.1f\n", queue_peek(float, &q));
        queue_deque(float, &q);
    }

    queue_delete(float, &q);
}
```

### Deque Example (Double-Ended Queue)

```c
// my_deque.h
#pragma once
#include "deque.h"

typedef struct { float x, y, z; } vec3_t;

#define VEC3_DEQUE_INIT_SIZE  4
#define VEC3_DEQUE_GROWTH     2
DEFINE_DEQUE(vec3_t, size_t, VEC3_DEQUE_INIT_SIZE)
```

```c
// my_deque.c
#include "my_deque.h"
#include <stdlib.h>

static bool validate_vec3(vec3_t v) { return v.x == v.x; }

GENERATE_DEQUE(vec3_t, size_t, VEC3_DEQUE_INIT_SIZE, VEC3_DEQUE_GROWTH, validate_vec3, malloc, realloc, free)
```

```c
// usage.c
#include "my_deque.h"
#include <stdio.h>

void demo_deque(void)
{
    deque(vec3_t) dq;
    deque_init(vec3_t, &dq);

    vec3_t a = {1, 1, 1};
    vec3_t b = {2, 2, 2};
    vec3_t c = {3, 3, 3};

    deque_insert_tail(vec3_t, &dq, a);
    deque_insert_head(vec3_t, &dq, b);
    deque_insert_tail(vec3_t, &dq, c);

    // Order: b → a → c

    printf("Head: (%.0f,%.0f,%.0f)\n", deque_peek_head(vec3_t, &dq).x, deque_peek_head(vec3_t, &dq).y, deque_peek_head(vec3_t, &dq).z);
    printf("Tail: (%.0f,%.0f,%.0f)\n", deque_peek_tail(vec3_t, &dq).x, deque_peek_tail(vec3_t, &dq).y, deque_peek_tail(vec3_t, &dq).z);

    deque_remove_head(vec3_t, &dq);
    deque_remove_tail(vec3_t, &dq);

    deque_delete(vec3_t, &dq);
}
```