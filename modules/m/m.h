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

enum m_result
{
    M_OK = 0,
    M_OOM = -1,
};
typedef enum m_result m_result;


// ================================================
//                  ALLOCATOR
//

// ----------------------------------------------------------------
// allocate a block of memory.
//
// size: number of bytes to allocate.
//
// ctx:  allocator context.
// ----------------------------------------------------------------
// returns: pointer to allocated memory, or NULL on failure.
typedef void* (*m_alloc_proc)(m_usize size, void* ctx);

// ----------------------------------------------------------------
// free a previously allocated block.
//
// ptr:  pointer returned by alloc or realloc.
//       if NULL, this is a no-op.
//
// size: original size passed to alloc or realloc.
//
// ctx:  allocator context.
// ----------------------------------------------------------------
typedef void (*m_free_proc)(void* ptr, m_usize size, void* ctx);

// ----------------------------------------------------------------
// resize a previously allocated block.
//
// ptr:      pointer returned by alloc or realloc.
//           if NULL, behaves like alloc(new_size, ctx).
//
// old_size: original size of the block.
//           ignored if ptr is NULL.
//
// new_size: requested new size.
//           if 0, behaves like free(ptr, old_size, ctx).
//
// ctx:      allocator context.
// ----------------------------------------------------------------
// returns: pointer to resized memory, or NULL on failure.
//          original ptr is still valid if NULL is returned.
typedef void* (*m_realloc_proc)(void* ptr, m_usize old_size, m_usize new_size, void* ctx);

struct m_allocator
{
    m_alloc_proc    alloc;
    m_free_proc     free;
    m_realloc_proc  realloc;
    void            *ctx;
};
typedef struct m_allocator m_allocator;

#define M_ALLOC(a, size)             (a)->alloc(size, (a)->ctx)
#define M_FREE(a, ptr, size)         (a)->free(ptr, size, (a)->ctx)
#define M_REALLOC(a, ptr, old_size, new_size) (a)->realloc(ptr, old_size, new_size, (a)->ctx)

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
    for (int var = ((start), 0); !var; (void)(end), var = 1)

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
//              COMPILER DETECTION
//

#define M_COMPILER_GCC   0
#define M_COMPILER_CLANG 0
#define M_COMPILER_MSVC  0
#define M_COMPILER_OTHER 0

#if defined(__clang__)

#   undef  M_COMPILER_CLANG
#   define M_COMPILER_CLANG 1

#   if defined(_MSC_VER)
#       undef  M_COMPILER_MSVC
#       define M_COMPILER_MSVC 1
#   endif

#elif defined(__GNUC__) && !defined(__clang__)

#   undef  M_COMPILER_GCC
#   define M_COMPILER_GCC 1

#elif defined(_MSC_VER)

#   undef  M_COMPILER_MSVC
#   define M_COMPILER_MSVC 1

#else

#   undef  M_COMPILER_OTHER
#   define M_COMPILER_OTHER 1

#endif

#define M_COMPILER_CLANG_OR_GCC (M_COMPILER_CLANG || M_COMPILER_GCC)


// ================================================
//                  C VERSION
//

#if defined(__STDC_VERSION__)

#   if __STDC_VERSION__ >= 202311L
#       define M_C23 1
#       define M_C17 1
#       define M_C11 1
#       define M_C99 1

#   elif __STDC_VERSION__ >= 201710L
#       define M_C23 0
#       define M_C17 1
#       define M_C11 1
#       define M_C99 1

#   elif __STDC_VERSION__ >= 201112L
#       define M_C23 0
#       define M_C17 0
#       define M_C11 1
#       define M_C99 1

#   elif __STDC_VERSION__ >= 199901L
#       define M_C23 0
#       define M_C17 0
#       define M_C11 0
#       define M_C99 1

#   else
#       define M_C23 0
#       define M_C17 0
#       define M_C11 0
#       define M_C99 0
#   endif

#else

#   define M_C23 0
#   define M_C17 0
#   define M_C11 0
#   define M_C99 0

#endif

#define M_C99_OR_NEWER (M_C99 || M_C11 || M_C17 || M_C23)
#define M_C11_OR_NEWER (M_C11 || M_C17 || M_C23)
#define M_C17_OR_NEWER (M_C17 || M_C23)
#define M_C23_OR_NEWER (M_C23)


// =====================================================
//              COMPILER BUILTINS
//
//  M_LIKELY(x)
//  M_UNLIKELY(x)
//  M_INLINE
//  M_NOINLINE
//  M_UNREACHABLE()
//  M_NORETURN
//  M_RESTRICT

#if M_COMPILER_GCC || M_COMPILER_CLANG
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
#if M_C99_OR_NEWER
#   define M_RESTRICT restrict
#else
#   define M_RESTRICT
#endif
#endif


// ================================================
//                OS DETECTION
//

#if defined(_WIN32) || defined(_WIN64)
#   define M_OS_WINDOWS
#elif defined(__APPLE__) && defined(__MACH__)
#   define M_OS_MACOS
#elif defined(__linux__)
#   define M_OS_LINUX
#elif defined(__FreeBSD__)
#   define M_OS_FREEBSD
#elif defined(__OpenBSD__)
#   define M_OS_OPENBSD
#elif defined(__NetBSD__)
#   define M_OS_NETBSD
#elif defined(__unix__)
#   define M_OS_UNIX
#else
#   define M_OS_UNKNOWN
#endif

#if defined(M_OS_FREEBSD) || defined(M_OS_OPENBSD) || defined(M_OS_NETBSD)
#   define M_OS_BSD
#endif

#if defined(M_OS_LINUX) || defined(M_OS_MACOS) || defined(M_OS_BSD) || defined(M_OS_UNIX)
#   define M_OS_POSIX
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
//                BITNESS
//

#if UINTPTR_MAX == 0xffffffffffffffff
#   define M_64_BIT 1
#   define M_32_BIT 0
#elif UINTPTR_MAX == 0xffffffff
#   define M_64_BIT 0
#   define M_32_BIT 1
#else
#   error "Unsupported pointer size"
#endif


// =====================================================
//              ENDIANNESS DETECTION
//
// provides compile-time or runtime detection depending on
// compiler support.
//
// prefer compile-time macros when available.
//
// M_IS_LITTLE_ENDIAN() and M_IS_BIG_ENDIAN() is a runtime version. (can be compile time, depends)
// M_IS_LITTLE_ENDIAN_CT and M_IS_BIG_ENDIAN_CT are compile-time and require M_HAS_COMPILE_TIME_ENDIAN == 1

#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && defined(__ORDER_BIG_ENDIAN__)

#   define M_HAS_COMPILE_TIME_ENDIAN 1

#   define M_IS_LITTLE_ENDIAN() (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#   define M_IS_BIG_ENDIAN()    (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)

#   define M_IS_LITTLE_ENDIAN_CT (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)    // compile time
#   define M_IS_BIG_ENDIAN_CT    (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)       // compile time

#else

#   define M_HAS_COMPILE_TIME_ENDIAN 0

#   define M_IS_LITTLE_ENDIAN_CT 0
#   define M_IS_BIG_ENDIAN_CT    0

static M_INLINE int m__is_little_endian_runtime(void)
{
    const m_u16 x = 1;
    return (*(const m_u8*)&x) == 1;
}

static M_INLINE int m__is_big_endian_runtime(void)
{
    return !m__is_little_endian_runtime();
}

#   define M_IS_LITTLE_ENDIAN() m__is_little_endian_runtime()
#   define M_IS_BIG_ENDIAN()    m__is_big_endian_runtime()

#endif


// ================================================
//                  BYTE SWAP
//
// NOTE: always pass unsigned values

#if M_COMPILER_GCC || M_COMPILER_CLANG

#   define M_BSWAP16(x) __builtin_bswap16((m_u16)(x))
#   define M_BSWAP32(x) __builtin_bswap32((m_u32)(x))
#   define M_BSWAP64(x) __builtin_bswap64((m_u64)(x))

#elif M_COMPILER_MSVC

#   include <intrin.h>

#   define M_BSWAP16(x) _byteswap_ushort((unsigned short)(x))
#   define M_BSWAP32(x) _byteswap_ulong((unsigned long)(x))
#   define M_BSWAP64(x) _byteswap_uint64((unsigned __int64)(x))

#else

#   define M_BSWAP16(x) \
    ((m_u16)(((m_u16)(x) >> 8) | ((m_u16)(x) << 8)))

#   define M_BSWAP32(x) \
    (((m_u32)(x) >> 24) | \
    (((m_u32)(x) >> 8) & 0x0000FF00u) | \
    (((m_u32)(x) << 8) & 0x00FF0000u) | \
    ((m_u32)(x) << 24))

#   define M_BSWAP64(x) \
    (((m_u64)(x) >> 56) | \
    (((m_u64)(x) >> 40) & 0x000000000000FF00ull) | \
    (((m_u64)(x) >> 24) & 0x0000000000FF0000ull) | \
    (((m_u64)(x) >>  8) & 0x00000000FF000000ull) | \
    (((m_u64)(x) <<  8) & 0x000000FF00000000ull) | \
    (((m_u64)(x) << 24) & 0x0000FF0000000000ull) | \
    (((m_u64)(x) << 40) & 0x00FF000000000000ull) | \
    ((m_u64)(x) << 56))

#endif


// ================================================
//              ENDIAN CONVERSION
//

#if M_HAS_COMPILE_TIME_ENDIAN

#   if M_IS_LITTLE_ENDIAN_CT

#       define M_HTON16(x) M_BSWAP16(x)
#       define M_HTON32(x) M_BSWAP32(x)
#       define M_HTON64(x) M_BSWAP64(x)

#       define M_NTOH16(x) M_BSWAP16(x)
#       define M_NTOH32(x) M_BSWAP32(x)
#       define M_NTOH64(x) M_BSWAP64(x)

#   else

#       define M_HTON16(x) ((m_u16)(x))
#       define M_HTON32(x) ((m_u32)(x))
#       define M_HTON64(x) ((m_u64)(x))

#       define M_NTOH16(x) ((m_u16)(x))
#       define M_NTOH32(x) ((m_u32)(x))
#       define M_NTOH64(x) ((m_u64)(x))

#   endif

#else

#define M_HTON16(x) (M_IS_LITTLE_ENDIAN() ? M_BSWAP16(x) : (m_u16)(x))
#define M_HTON32(x) (M_IS_LITTLE_ENDIAN() ? M_BSWAP32(x) : (m_u32)(x))
#define M_HTON64(x) (M_IS_LITTLE_ENDIAN() ? M_BSWAP64(x) : (m_u64)(x))

#define M_NTOH16(x) M_HTON16(x)
#define M_NTOH32(x) M_HTON32(x)
#define M_NTOH64(x) M_HTON64(x)

#endif


// =====================================================
// BIT INTRINSICS
//
//  M_POPCOUNT32(x)
//  M_POPCOUNT64(x)
//
//  M_CLZ32(x)
//  M_CLZ64(x)
//
//  M_CTZ32(x)
//  M_CTZ64(x)
//
//  M_BSR32(x)
//  M_BSR64(x)
//
//  M_BSF32(x)
//  M_BSF64(x)
//
//  M_ROTL32(x, n)
//  M_ROTR32(x, n)
//  M_ROTL64(x, n)
//  M_ROTR64(x, n)
//
// WARNING:
// - CLZ/CTZ are undefined for x == 0 (use SAFE variants)
// - all inputs must be unsigned
//

#if M_COMPILER_GCC || M_COMPILER_CLANG

#   define M_POPCOUNT32(x) __builtin_popcount((unsigned int)(x))
#   define M_POPCOUNT64(x) __builtin_popcountll((unsigned long long)(x))

#   define M_CLZ32(x)      __builtin_clz((unsigned int)(x))
#   define M_CLZ64(x)      __builtin_clzll((unsigned long long)(x))

#   define M_CTZ32(x)      __builtin_ctz((unsigned int)(x))
#   define M_CTZ64(x)      __builtin_ctzll((unsigned long long)(x))

#   define M_BSR32(x)      (31  - __builtin_clz((unsigned int)(x)))
#   define M_BSR64(x)      (63  - __builtin_clzll((unsigned long long)(x)))
#   define M_BSF32(x)      __builtin_ctz((unsigned int)(x))
#   define M_BSF64(x)      __builtin_ctzll((unsigned long long)(x))

#   if (M_COMPILER_GCC && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8))) \
     || M_COMPILER_CLANG
#       define M_ROTL32(x, n) \
            (((m_u32)(x) << ((n) & 31)) | ((m_u32)(x) >> ((-((int)(n))) & 31)))
#       define M_ROTR32(x, n) \
            (((m_u32)(x) >> ((n) & 31)) | ((m_u32)(x) << ((-((int)(n))) & 31)))
#       define M_ROTL64(x, n) \
            (((m_u64)(x) << ((n) & 63)) | ((m_u64)(x) >> ((-((int)(n))) & 63)))
#       define M_ROTR64(x, n) \
            (((m_u64)(x) >> ((n) & 63)) | ((m_u64)(x) << ((-((int)(n))) & 63)))
#   else
#       define M_ROTL32(x, n) \
            (((m_u32)(x) << ((n) % 32)) | ((m_u32)(x) >> ((32 - (n)) % 32)))
#       define M_ROTR32(x, n) \
            (((m_u32)(x) >> ((n) % 32)) | ((m_u32)(x) << ((32 - (n)) % 32)))
#       define M_ROTL64(x, n) \
            (((m_u64)(x) << ((n) % 64)) | ((m_u64)(x) >> ((64 - (n)) % 64)))
#       define M_ROTR64(x, n) \
            (((m_u64)(x) >> ((n) % 64)) | ((m_u64)(x) << ((64 - (n)) % 64)))
#   endif

#elif M_COMPILER_MSVC

#   include <intrin.h>

#   define M_POPCOUNT32(x) __popcnt((unsigned int)(x))
#   define M_POPCOUNT64(x) __popcnt64((unsigned __int64)(x))



static M_INLINE m_i32 m__msvc_clz32(m_u32 x)
{
    unsigned long idx;
    _BitScanReverse(&idx, (unsigned long)x);
    return 31 - (m_i32)idx;
}

static M_INLINE m_i32 m__msvc_clz64(m_u64 x)
{
#   if defined(M_ARCH_X64) || defined(M_ARCH_ARM64)
    unsigned long idx;
    _BitScanReverse64(&idx, (unsigned __int64)x);
    return 63 - (m_i32)idx;
#   else
    unsigned long idx;
    if (x >> 32)
    {
        _BitScanReverse(&idx, (unsigned long)(x >> 32));
        return 31 - (m_i32)idx;
    }
    _BitScanReverse(&idx, (unsigned long)(x & 0xFFFFFFFFu));
    return 63 - (m_i32)idx;
#   endif
}

static M_INLINE m_i32 m__msvc_ctz32(m_u32 x)
{
    unsigned long idx;
    _BitScanForward(&idx, (unsigned long)x);
    return (m_i32)idx;
}

static M_INLINE m_i32 m__msvc_ctz64(m_u64 x)
{
#   if defined(M_ARCH_X64) || defined(M_ARCH_ARM64)
    unsigned long idx;
    _BitScanForward64(&idx, (unsigned __int64)x);
    return (m_i32)idx;
#   else
    unsigned long idx;
    if (x & 0xFFFFFFFFu) {
        _BitScanForward(&idx, (unsigned long)(x & 0xFFFFFFFFu));
        return (m_i32)idx;
    }
    _BitScanForward(&idx, (unsigned long)(x >> 32));
    return 32 + (m_i32)idx;
#   endif
}

#   define M_CLZ32(x)  m__msvc_clz32((m_u32)(x))
#   define M_CLZ64(x)  m__msvc_clz64((m_u64)(x))
#   define M_CTZ32(x)  m__msvc_ctz32((m_u32)(x))
#   define M_CTZ64(x)  m__msvc_ctz64((m_u64)(x))

#   define M_BSR32(x)  (31  - M_CLZ32(x))
#   define M_BSR64(x)  (63  - M_CLZ64(x))
#   define M_BSF32(x)  M_CTZ32(x)
#   define M_BSF64(x)  M_CTZ64(x)

#   define M_ROTL32(x, n) _rotl( (unsigned int)(x),    (int)(n))
#   define M_ROTR32(x, n) _rotr( (unsigned int)(x),    (int)(n))
#   define M_ROTL64(x, n) _rotl64((unsigned __int64)(x),(int)(n))
#   define M_ROTR64(x, n) _rotr64((unsigned __int64)(x),(int)(n))

#else

static M_INLINE m_i32 M_POPCOUNT32(m_u32 x)
{
    x = x - ((x >> 1) & 0x55555555u);
    x = (x & 0x33333333u) + ((x >> 2) & 0x33333333u);
    x = (x + (x >> 4)) & 0x0F0F0F0Fu;
    return (m_i32)((x * 0x01010101u) >> 24);
}

static M_INLINE m_i32 M_POPCOUNT64(m_u64 x)
{
    x = x - ((x >> 1) & 0x5555555555555555ull);
    x = (x & 0x3333333333333333ull) + ((x >> 2) & 0x3333333333333333ull);
    x = (x + (x >> 4)) & 0x0F0F0F0F0F0F0F0Full;
    return (m_i32)((x * 0x0101010101010101ull) >> 56);
}

static M_INLINE m_i32 M_CLZ32(m_u32 x)
{
    m_i32 n = 0;
    if (!(x & 0xFFFF0000u)) { n += 16; x <<= 16; }
    if (!(x & 0xFF000000u)) { n +=  8; x <<=  8; }
    if (!(x & 0xF0000000u)) { n +=  4; x <<=  4; }
    if (!(x & 0xC0000000u)) { n +=  2; x <<=  2; }
    if (!(x & 0x80000000u)) { n +=  1; }
    return n;
}

static M_INLINE m_i32 M_CLZ64(m_u64 x)
{
    if (x >> 32) return M_CLZ32((m_u32)(x >> 32));
    return 32 + M_CLZ32((m_u32)(x & 0xFFFFFFFFu));
}

static M_INLINE m_i32 M_CTZ32(m_u32 x)
{
    return M_POPCOUNT32((x & (m_u32)(-(m_i32)x)) - 1u);
}

static M_INLINE m_i32 M_CTZ64(m_u64 x)
{
    if (x & 0xFFFFFFFFu) return M_CTZ32((m_u32)(x & 0xFFFFFFFFu));
    return 32 + M_CTZ32((m_u32)(x >> 32));
}

#   define M_BSR32(x)  (31  - M_CLZ32((m_u32)(x)))
#   define M_BSR64(x)  (63  - M_CLZ64((m_u64)(x)))
#   define M_BSF32(x)  M_CTZ32((m_u32)(x))
#   define M_BSF64(x)  M_CTZ64((m_u64)(x))

#   define M_ROTL32(x, n) \
        (((m_u32)(x) << ((n) % 32)) | ((m_u32)(x) >> ((32 - (n)) % 32)))
#   define M_ROTR32(x, n) \
        (((m_u32)(x) >> ((n) % 32)) | ((m_u32)(x) << ((32 - (n)) % 32)))
#   define M_ROTL64(x, n) \
        (((m_u64)(x) << ((n) % 64)) | ((m_u64)(x) >> ((64 - (n)) % 64)))
#   define M_ROTR64(x, n) \
        (((m_u64)(x) >> ((n) % 64)) | ((m_u64)(x) << ((64 - (n)) % 64)))

#endif // compiler dispatch


// ================================================
//           BIT UTILITY MACROS
//
//

// safe variants: return the specified value when x == 0
// (avoids UB from CLZ/CTZ on zero)
#define M_CLZ32_SAFE(x, val_if_zero) ((x) ? M_CLZ32(x) : (val_if_zero))
#define M_CLZ64_SAFE(x, val_if_zero) ((x) ? M_CLZ64(x) : (val_if_zero))
#define M_CTZ32_SAFE(x, val_if_zero) ((x) ? M_CTZ32(x) : (val_if_zero))
#define M_CTZ64_SAFE(x, val_if_zero) ((x) ? M_CTZ64(x) : (val_if_zero))

// true iff x is an exact power of two  (x must be > 0)
#define M_IS_POW2(x)    (((x) & ((x) - 1)) == 0)

// round x up to the next power of two.
// returns 1 for x == 0.
// returns x if x is already a power of two.
// returns 0 if the result would overflow the 32-bit/64-bit range
// (i.e. x > 2^31 for 32-bit, x > 2^63 for 64-bit).
#define M_NEXT_POW2_32(x) \
    (((m_u32)(x) <= 1u) ? 1u \
    : (M_CLZ32((m_u32)(x) - 1u) == 0) ? 0u \
    : (m_u32)1u << (32 - M_CLZ32((m_u32)(x) - 1u)))
#define M_NEXT_POW2_64(x) \
    (((m_u64)(x) <= 1ull) ? 1ull \
    : (M_CLZ64((m_u64)(x) - 1ull) == 0) ? 0ull \
    : (m_u64)1ull << (64 - M_CLZ64((m_u64)(x) - 1ull)))

// round x up to the nearest multiple of align (align must be a power of two)
#define M_ALIGN_UP(x, align)   (((x) + (align) - 1) & ~((align) - 1))
#define M_ALIGN_DOWN(x, align) ((x) & ~((align) - 1))
#define M_IS_ALIGNED(x, align) (((x) & ((align) - 1)) == 0)

// extract / set / clear individual bits
#define M_BIT_GET(x, bit)      (((x) >> (bit)) & 1ull)
#define M_BIT_SET(x, bit)      ((x) |  ((m_u64)1ull << (bit)))
#define M_BIT_CLEAR(x, bit)    ((x) & ~((m_u64)1ull << (bit)))
#define M_BIT_TOGGLE(x, bit)   ((x) ^  ((m_u64)1ull << (bit)))

// mask of n low bits  (n must be < width of result type)
#define M_MASK32(n)            ((m_u32)((n) < 32 ? ((m_u32)1u  << (n)) - 1u   : 0xFFFFFFFFu))
#define M_MASK64(n)            ((m_u64)((n) < 64 ? ((m_u64)1ull << (n)) - 1ull : 0xFFFFFFFFFFFFFFFFull))


#endif //M_H