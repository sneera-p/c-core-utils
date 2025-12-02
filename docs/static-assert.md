# Static Assertions and Type Checking Macros

This header file provides a set of macros for compile-time checks and assertions in C. These macros ensure that certain conditions hold true at compile time, improving type safety and debugging efficiency. Below are the provided macros and their usage.

## Table of Contents
1. [`static_assert`](#static_assert)
2. [`assert_type`](#assert_type)
3. [`assert_istype`](#assert_istype)
4. [`typecheck_ptr`](#typecheck_ptr)

---

### `static_assert`

The `static_assert` macro generates a compile-time error if the expression `expr` evaluates to false.

#### Usage:

```c
static_assert(expr, msg);
```

- `expr`: The expression that will be checked at compile time. If it evaluates to false, a compile-time error occurs.
- `msg`: A string that will be displayed in the error message.

#### Example:

```c
#define MAX_SIZE 100

static_assert(MAX_SIZE > 0, "MAX_SIZE must be greater than 0");

int main() {
    return 0;
}
```

In this example, the macro checks that `MAX_SIZE` is greater than 0 at compile time. If `MAX_SIZE` were less than or equal to 0, the compilation would fail with the message "`MAX_SIZE` must be greater than 0".


### `assert_type`

The `assert_type` macro ensures that an expression expr is of the type type. It checks the type compatibility at compile time.

#### Usage:

```c
assert_type(expr, type);
```

- `expr`: The expression whose type you want to check.
- `type`: The expected type.

#### Example:

```c
int a = 5;
assert_type(a, int);  // This will pass since 'a' is of type 'int'

float b = 3.14f;
assert_type(b, int);  // This will fail at compile time since 'b' is of type 'float', not 'int'
```

In the above example, the macro will produce a compile-time error if a is not of type int, or if b is not of type int.


### `assert_istype`

The `assert_istype` macro ensures that a given expression T is a valid type. This is useful for generic macros that require a type argument.

#### Usage:

```c
assert_istype(T);
```

#### Example:

```c
assert_istype(int);      // This will pass as 'int' is a valid type.
assert_istype(a);        // This will fail as 'a' is not a type but a variable.
```

In this case, the macro checks that int is a valid type. If a variable or function name is passed instead of a type, it will generate a compile-time error.


### `typecheck_ptr`

The `typecheck_ptr` macro checks that a given variable `var` is a pointer to a specific type `type` (or `const type`). This ensures that the variable is used correctly in pointer-related operations.

#### Usage:

```c
typecheck_ptr(var, type, expr);
```

- `var`: The variable that should be a pointer.
- `type`: The type that `var` should point to.
- `expr`: The expression to be evaluated, which will be checked if `var` is a pointer to `type`.

#### Example:

```c
int x = 10;
int *ptr = &x;

typecheck_ptr(ptr, int, *ptr = 20);  // This will pass because ptr is of type 'int*'

float y = 5.5;
typecheck_ptr(ptr, float, *ptr = 3.14);  // This will fail because 'ptr' is of type 'int*', not 'float*'
```

Here, `typecheck_ptr` ensures that `ptr` is of type `int*` before the expression `*ptr = 20` is executed. If `ptr` were of the wrong type, a compile-time error would occur.


## Notes on Portability

The macros work across multiple C standards and compilers, including:

- C23+: Uses `static_assert` and `_Static_assert`.
- C11–C22: Uses `_Static_assert` and a fallback method for type checks.
- Pre-C11 (GCC, Clang, Intel): Uses a `typedef` hack for compile-time type checking.
- Other compilers: A fallback no-op is used, with warnings when certain checks can't be enforced.

For compilers that do not support these features, the macros will be either a no-op or generate a warning message without enforcing the checks. Ensure your compiler supports these features
