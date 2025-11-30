#ifndef __MEMORY_COPY_H
#define __MEMORY_COPY_H

#include <stddef.h>

/**
 * MEMORY_COPY macro
 * ----------------
 * Portable memory copy macro with compiler hints.
 *
 * Usage:
 *   MEMORY_COPY(dest, src, size);
 *
 * Notes:
 *   Modern compilers inline memcpy automatically.
 *   Optimized intrinsics are compiler-specific and may not exist.
 */

// GCC, Clang
#if defined(__GNUC__) || defined(__clang__)
    #define MEMORY_COPY(dest, src, size) __builtin_memcpy(dest, src, size)

// MSVC, Pelles C
#elif defined(_MSC_VER) || defined(__POCC__)
    #include <intrin.h>
    #define MEMORY_COPY(dest, src, size) __movsb(dest, src, size)

// Other
#else
    #include <stdlib.h>
    #define MEMORY_COPY(dest, src, size) memcpy(dest, src, size)

#endif

#endif /* __MEMORY_COPY_H */