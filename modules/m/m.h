#ifndef M_H
#define M_H

#include <stddef.h>
#include <stdint.h>

// ================================================
//                    TYPES
//

typedef ptrdiff_t   m_isize;
typedef size_t      m_usize;

typedef uint8_t     m_u8;
typedef uint16_t    m_u16;
typedef uint32_t    m_u32;
typedef uint64_t    m_u64;

typedef int8_t      m_i8;
typedef int16_t     m_i16;
typedef int32_t     m_i32;
typedef int64_t     m_i64;

typedef float       m_f32;
typedef double      m_f64;


// ================================================
//                  ALLOCATOR
//

typedef void *(*m_alloc_proc)  (m_isize size, void *ctx);
typedef void  (*m_free_proc)   (void *ptr, m_isize size, void *ctx);
typedef void *(*m_realloc_proc)(void *ptr, m_isize old, m_isize new_, void *ctx);

struct m_allocator
{
    m_alloc_proc   alloc;
    m_free_proc    free;
    m_realloc_proc realloc;
    void          *ctx;
};
typedef struct m_allocator m_allocator;

#define M_ALLOC(a, size)             (a)->alloc(size, (a)->ctx)
#define M_FREE(a, ptr, size)         (a)->free(ptr, size, (a)->ctx)
#define M_REALLOC(a, ptr, old, new_) (a)->realloc(ptr, old, new_, (a)->ctx)

// ----------------------------------------------------------------
// default heap allocator.
// ----------------------------------------------------------------
// returns: pointer to the shared heap allocator instance.
const m_allocator *m_heap_allocator(void);


// ================================================
//                 UTILITY MACROS
//

#define M_CONCAT_(a, b) a##b
#define M_CONCAT(a, b)  M_CONCAT_(a, b)

#define M_STRINGIFY_(x) #x
#define M_STRINGIFY(x)  M_STRINGIFY_(x)

#define M_BLOCK(var, start, end) \
    for (int var = (start, 0); !var; var = (end, 1))

#define M_UNUSED(x) (void)(x)


// ================================================
//                 STATIC ASSERT
//

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#   define M_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#else
#   define M_STATIC_ASSERT(cond, msg) typedef char m_sa_##msg[(cond) ? 1 : -1]
#endif


// ================================================
//                  BUILD INS
//

#if defined(__GNUC__) || defined(__clang__)
#   define M_LIKELY(x)     __builtin_expect(!!(x), 1)
#   define M_UNLIKELY(x)   __builtin_expect(!!(x), 0)
#   define M_INLINE        __attribute__((always_inline)) inline
#   define M_NOINLINE      __attribute__((noinline))
#   define M_UNREACHABLE() __builtin_unreachable()
#   define M_NORETURN      __attribute__((noreturn))
#   define M_RESTRICT      __restrict__
#elif defined(_MSC_VER)
#   define M_LIKELY(x)     (x)
#   define M_UNLIKELY(x)   (x)
#   define M_INLINE        __forceinline
#   define M_NOINLINE      __declspec(noinline)
#   define M_UNREACHABLE() __assume(0)
#   define M_NORETURN      __declspec(noreturn)
#   define M_RESTRICT      __restrict
#else
#   define M_LIKELY(x)     (x)
#   define M_UNLIKELY(x)   (x)
#   define M_INLINE        inline
#   define M_NOINLINE
#   define M_UNREACHABLE()
#   define M_NORETURN
#   define M_RESTRICT
#endif


// ================================================
//                  C VERSION
//

#if defined(__STDC_VERSION__)
#    if __STDC_VERSION__ >= 202311L
#       define M_C23
#    endif
#    if __STDC_VERSION__ >= 201112L
#       define M_C11
#    endif
#    if __STDC_VERSION__ >= 199901L
#       define M_C99
#   endif
#endif


// ================================================
//               ARCH DETECTION
//

#if defined(__x86_64__) || defined(_M_X64)
#   define M_ARCH_X64
#elif defined(__i386__) || defined(_M_IX86)
#   define M_ARCH_X86
#elif defined(__aarch64__) || defined(_M_ARM64)
#   define M_ARCH_ARM64
#elif defined(__arm__) || defined(_M_ARM)
#   define M_ARCH_ARM32
#endif


// ================================================
//                 ENDIANNESS
//

#ifdef M_C23
#   include <stdbit.h>
#   define M_IS_LITTLE_ENDIAN (__STDC_ENDIAN_NATIVE__ == __STDC_ENDIAN_LITTLE__)
#   define M_IS_BIG_ENDIAN    (__STDC_ENDIAN_NATIVE__ == __STDC_ENDIAN_BIG__)
#else
static const int m__endian = 1;
#   define M_IS_LITTLE_ENDIAN (*(const char *)&m__endian)
#   define M_IS_BIG_ENDIAN    (!M_IS_LITTLE_ENDIAN)
#endif


// ================================================
//                  BYTE SWAP
//

// note:    always pass unsigned values.
//

#define M_BSWAP16(x) \
    ((m_u16)(((m_u16)(x) >> 8) | ((m_u16)(x) << 8)))

#define M_BSWAP32(x) \
    (((m_u32)(x) >> 24)                 | \
    (((m_u32)(x) & 0x00FF0000U) >>  8) | \
    (((m_u32)(x) & 0x0000FF00U) <<  8) | \
    ((m_u32)(x) << 24))

#define M_BSWAP64(x) \
    (((m_u64)(x) >> 56)                               | \
    (((m_u64)(x) & 0x00FF000000000000ULL) >> 40)     | \
    (((m_u64)(x) & 0x0000FF0000000000ULL) >> 24)     | \
    (((m_u64)(x) & 0x000000FF00000000ULL) >>  8)     | \
    (((m_u64)(x) & 0x00000000FF000000ULL) <<  8)     | \
    (((m_u64)(x) & 0x0000000000FF0000ULL) << 24)     | \
    (((m_u64)(x) & 0x000000000000FF00ULL) << 40)     | \
    ((m_u64)(x) << 56))

#define M_HTON16(x) (M_IS_LITTLE_ENDIAN ? M_BSWAP16(x) : (m_u16)(x))
#define M_HTON32(x) (M_IS_LITTLE_ENDIAN ? M_BSWAP32(x) : (m_u32)(x))
#define M_HTON64(x) (M_IS_LITTLE_ENDIAN ? M_BSWAP64(x) : (m_u64)(x))

#define M_NTOH16    M_HTON16
#define M_NTOH32    M_HTON32
#define M_NTOH64    M_HTON64

#endif //M_H