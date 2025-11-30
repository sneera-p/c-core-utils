#ifndef __STATIC_ASSERT_H
#define __STATIC_ASSERT_H


/**
 * static_assert macro
 * ------------------
 * Compile-time assertion.
 *
 * Usage:
 *   static_assert(expr, "optional message");
 *
 * Purpose:
 *   Generates a compile-time error if expr is false.
 *   Works across multiple C standards and compilers:
 *     - C23+: static_assert
 *     - C11–C22: _static_assert
 *     - C99 / pre-C11: GCC/Clang typedef hack
 *     - Other compilers: fallback safe (no-op)
 */

// C23, external definition
#if defined(static_assert)
    // Already defined by compiler

// C11 / C17 / C22
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
    #define static_assert(expr, msg) \
        _Static_assert(expr, msg)

// Pre-C11 GCC, Clang, Intel
#elif defined(__GNUC__) || defined(__clang__) || defined(__INTEL_COMPILER) || defined(__INTEL_CLANG_COMPILER)
    #define static_assert(expr, msg) \
        typedef char __static_assert_##__FILE__##at##__LINE__[(expr) ? 1 : -1]

// Fallback
#else
    #warning "Warning: static_assert(expr, msg) cannot be enforced"
    #define static_assert(expr, msg) ((void)0)

#endif


/**
 * assert_type macro
 * ----------------
 * Compile-time type assertion.
 * 
 * Usage:
 *   assert_type(expr, type);
 *
 * Purpose:
 *   Generates a compile-time error if `expr` is not of type `type`.
 *   Works across GCC, Clang, MSVC, Intel, IBM XL, SunCC, Green Hills, ARMCC, IAR.
 */

// C11 and above
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)

    // GCC, Clang, Intel, IBM XL, SunCC, Green Hills
    #if defined(__GNUC__) || defined(__clang__) || defined(__INTEL_COMPILER) || defined(__INTEL_CLANG_COMPILER) || defined(__IBMC__) || defined(__xlC__) || defined(__SUNPRO_C) || defined(__ghs__)
        #define assert_type(expr, type) \
            _Static_assert(__builtin_types_compatible_p(__typeof__(expr), type), "Type mismatch")
    
    // Fallback
    #else
        #define assert_type(expr, type) \
            _Static_assert(_Generic((expr), type: 1, default: 0), "Type mismatch")
    
    #endif

// C99 and above untill C11
#else

    // GCC, Clang, Intel
    #if defined(__GNUC__) || defined(__clang__) || defined(__INTEL_COMPILER) || defined(__INTEL_CLANG_COMPILER)
        #define assert_type(expr, type) \
            typedef char __assert_type_##__FILE__##at##__LINE__[__builtin_types_compatible_p(__typeof__(expr), type) ? 1 : -1]
    
    // Fallback
    #else
        #warning "Warning: assert_type(expr, type) cannot be enforced"
        #define assert_type(expr, type) ((void)0)
    #endif

#endif /* (__STDC_VERSION__ >= 201112L) */


/**
 * assert_istype macro
 * --------------------
 * Compile-time validation that a macro argument is a type name.
 *
 * Usage:
 *   assert_istype(T);
 *
 * Purpose:
 *   Ensures that the expression passed to the macro is a valid type,
 *   not a variable, constant, or function name. This is useful for
 *   macros that expect a type parameter (e.g., stack(int), vector(T), etc.)
 *   and want to produce a clear compile-time error if misused.
 *
 * Behavior:
 *   - C11 and later:
 *       Uses _Static_assert and sizeof((T*)0) to check that T is a type.
 *       Produces a readable error message such as:
 *           "foo is not a type"
 *
 *   - C99 fallback:
 *       Uses a typedef trick with sizeof((T*)0) to trigger a compile-time
 *       error if T is not a type.
 *
 * Limitations:
 *   - In the C99 fallback, the error message cannot include the type name.
 *   - Only checks that T forms a valid pointer type (i.e., is a legitimate
 *     type name); it does not distinguish between different categories
 *     of types (structs, typedefs, builtins, etc.).
 *
 * Portability:
 *   Works on all conforming C compilers.
 *   The C11 version provides better diagnostics; the C99 version
 *   provides compatibility.
 */

#if (__STDC_VERSION__ >= 201112L)
    #define assert_istype(type) \
        _Static_assert(!!sizeof((type*)0), #type " is not a type")
#else  // C99 fallback
    #define assert_istype(type) \
        typedef char assert_fail_not_a_type##__FILE__##at##__LINE__[(sizeof((type*)0)) ? 1 : -1]
#endif


#endif /* __STATIC_ASSERT_H */