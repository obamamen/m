#include <m/m.h>

#include <mtest/mtest.h>
#include <mtest/mtest_terminal.h>


int main(void)
{
    mtest_terminal_ctx ctx;
    mtest t = mtest_new(mtest_terminal_new(&ctx, MTEST_TERMINAL_DEFAULT));

    MTEST_GROUP(&t, "heap_allocator")
    {
        MTEST_CASE(&t, "alloc")
        {
            const m_allocator *a = m_heap_allocator();
            char *m = M_ALLOC(a, 128);
            MTEST_ASSERT(&t, m != NULL, "alloc returned non-null");
            if (m) M_FREE(a, m, 128);
        }

        MTEST_CASE(&t, "alloc zero")
        {
            const m_allocator *a = m_heap_allocator();
            void *m = M_ALLOC(a, 0);
            if (m) M_FREE(a, m, 0);
            MTEST_ASSERT(&t, 1, "alloc zero does not crash");
        }

        MTEST_CASE(&t, "realloc grow")
        {
            const m_allocator *a = m_heap_allocator();
            char *m = M_ALLOC(a, 128);
            MTEST_ASSERT_FATAL(&t, m != NULL, "alloc failed");
            m[0] = 'X';
            char *newptr = M_REALLOC(a, m, 128, 256);
            if (!newptr) M_FREE(a, m, 128);
            MTEST_ASSERT_FATAL(&t, newptr != NULL, "realloc returned non-null");
            MTEST_ASSERT(&t, newptr[0] == 'X',     "data preserved after grow");
            M_FREE(a, newptr, 256);
        }

        MTEST_CASE(&t, "realloc shrink")
        {
            const m_allocator *a = m_heap_allocator();
            char *m = M_ALLOC(a, 128);
            MTEST_ASSERT_FATAL(&t, m != NULL, "alloc failed");
            m[0] = 'X';
            char *newptr = M_REALLOC(a, m, 128, 8);
            if (!newptr) M_FREE(a, m, 128);
            MTEST_ASSERT_FATAL(&t, newptr != NULL, "realloc shrink returned non-null");
            MTEST_ASSERT(&t, newptr[0] == 'X',     "data preserved after shrink");
            M_FREE(a, newptr, 8);
        }

        MTEST_CASE(&t, "realloc from null")
        {
            const m_allocator *a = m_heap_allocator();
            char *m = M_REALLOC(a, NULL, 0, 64);
            MTEST_ASSERT_FATAL(&t, m != NULL, "realloc from null returned non-null");
            m[0] = 'A';
            MTEST_ASSERT(&t, m[0] == 'A', "writable after realloc-from-null");
            M_FREE(a, m, 64);
        }

        MTEST_CASE(&t, "free null")
        {
            const m_allocator *a = m_heap_allocator();
            M_FREE(a, NULL, 0);
            MTEST_ASSERT(&t, 1, "free null does not crash");
        }

        MTEST_CASE(&t, "write read")
        {
            const m_allocator *a = m_heap_allocator();
            int *m = M_ALLOC(a, sizeof(int) * 4);
            MTEST_ASSERT_FATAL(&t, m != NULL, "alloc failed");
            m[0] = 1; m[1] = 2; m[2] = 3; m[3] = 4;
            MTEST_ASSERT(&t, m[0] == 1, "m[0] == 1");
            MTEST_ASSERT(&t, m[1] == 2, "m[1] == 2");
            MTEST_ASSERT(&t, m[2] == 3, "m[2] == 3");
            MTEST_ASSERT(&t, m[3] == 4, "m[3] == 4");
            M_FREE(a, m, sizeof(int) * 4);
        }

        MTEST_CASE(&t, "multiple independent allocs")
        {
            const m_allocator *a = m_heap_allocator();
            int *p = M_ALLOC(a, sizeof(int));
            int *q = M_ALLOC(a, sizeof(int));
            MTEST_ASSERT_FATAL(&t, p != NULL, "p alloc failed");
            MTEST_ASSERT_FATAL(&t, q != NULL, "q alloc failed");
            *p = 111; *q = 222;
            MTEST_ASSERT(&t, *p == 111, "p not clobbered by q");
            MTEST_ASSERT(&t, *q == 222, "q not clobbered by p");
            MTEST_ASSERT(&t, p != q,    "distinct pointers");
            M_FREE(a, p, sizeof(int));
            M_FREE(a, q, sizeof(int));
        }
    }

    MTEST_GROUP(&t, "byte_swap")
    {
        MTEST_CASE(&t, "bswap16")
        {
            MTEST_ASSERT(&t, M_BSWAP16(0x1234) == 0x3412, "bswap16 0x1234");
            MTEST_ASSERT(&t, M_BSWAP16(0x0001) == 0x0100, "bswap16 0x0001");
            MTEST_ASSERT(&t, M_BSWAP16(0xFF00) == 0x00FF, "bswap16 0xFF00");
            MTEST_ASSERT(&t, M_BSWAP16(0xFFFF) == 0xFFFF, "bswap16 all-ones is same");
            MTEST_ASSERT(&t, M_BSWAP16(0x0000) == 0x0000, "bswap16 zero is same");
        }

        MTEST_CASE(&t, "bswap32")
        {
            MTEST_ASSERT(&t, M_BSWAP32(0x12345678u) == 0x78563412u, "bswap32 0x12345678");
            MTEST_ASSERT(&t, M_BSWAP32(0xFF000000u) == 0x000000FFu, "bswap32 0xFF000000");
            MTEST_ASSERT(&t, M_BSWAP32(0x000000FFu) == 0xFF000000u, "bswap32 0x000000FF");
            MTEST_ASSERT(&t, M_BSWAP32(0xFFFFFFFFu) == 0xFFFFFFFFu, "bswap32 all-ones is same");
            MTEST_ASSERT(&t, M_BSWAP32(0x00000000u) == 0x00000000u, "bswap32 zero is same");
            MTEST_ASSERT(&t, M_BSWAP32(M_BSWAP32(0xDEADBEEFu)) == 0xDEADBEEFu, "bswap32 double is same");
        }

        MTEST_CASE(&t, "bswap64")
        {
            MTEST_ASSERT(&t, M_BSWAP64(0x123456789ABCDEF0ULL) == 0xF0DEBC9A78563412ULL, "bswap64 0x123456789ABCDEF0");
            MTEST_ASSERT(&t, M_BSWAP64(0xFF00000000000000ULL) == 0x00000000000000FFULL, "bswap64 high byte to low");
            MTEST_ASSERT(&t, M_BSWAP64(0x00000000000000FFULL) == 0xFF00000000000000ULL, "bswap64 low byte to high");
            MTEST_ASSERT(&t, M_BSWAP64(0xFFFFFFFFFFFFFFFFULL) == 0xFFFFFFFFFFFFFFFFULL, "bswap64 all-ones is same");
            MTEST_ASSERT(&t, M_BSWAP64(M_BSWAP64(0xDEADBEEFCAFEBABEULL)) == 0xDEADBEEFCAFEBABEULL, "bswap64 double is same");
        }

        MTEST_CASE(&t, "endianness detected")
        {
            MTEST_ASSERT(&t,  (M_IS_LITTLE_ENDIAN() || M_IS_BIG_ENDIAN()),  "one must be true");
            MTEST_ASSERT(&t, !(M_IS_LITTLE_ENDIAN() && M_IS_BIG_ENDIAN()),  "not both at once");
        }

        MTEST_CASE(&t, "hton roundtrip")
        {
            MTEST_ASSERT(&t, M_NTOH16(M_HTON16(0x1234))               == 0x1234,               "hton16 roundtrip");
            MTEST_ASSERT(&t, M_NTOH32(M_HTON32(0x12345678u))          == 0x12345678u,          "hton32 roundtrip");
            MTEST_ASSERT(&t, M_NTOH64(M_HTON64(0x123456789ABCDEF0ULL))== 0x123456789ABCDEF0ULL,"hton64 roundtrip");
        }

        MTEST_CASE(&t, "hton known big-endian value")
        {
            m_u32 net = M_HTON32(1u);
            if (M_IS_LITTLE_ENDIAN())
            {
                MTEST_ASSERT(&t, net == 0x01000000u, "LE: HTON32(1) == 0x01000000");
            }
            else
            {
                MTEST_ASSERT(&t, net == 0x00000001u, "BE: HTON32(1) == 0x00000001");
            }
        }
    }

    MTEST_GROUP(&t, "popcount")
    {
        MTEST_CASE(&t, "popcount32 basic")
        {
            MTEST_ASSERT(&t, M_POPCOUNT32(0x00000000u) == 0,  "popcount32(0)");
            MTEST_ASSERT(&t, M_POPCOUNT32(0x00000001u) == 1,  "popcount32(1)");
            MTEST_ASSERT(&t, M_POPCOUNT32(0x80000000u) == 1,  "popcount32 high bit");
            MTEST_ASSERT(&t, M_POPCOUNT32(0xFFFFFFFFu) == 32, "popcount32 all-ones");
            MTEST_ASSERT(&t, M_POPCOUNT32(0xAAAAAAAAu) == 16, "popcount32 alternating");
            MTEST_ASSERT(&t, M_POPCOUNT32(0x12345678u) == 13, "popcount32 0x12345678");
        }

        MTEST_CASE(&t, "popcount64 basic")
        {
            MTEST_ASSERT(&t, M_POPCOUNT64(0ULL)                  == 0,  "popcount64(0)");
            MTEST_ASSERT(&t, M_POPCOUNT64(1ULL)                  == 1,  "popcount64(1)");
            MTEST_ASSERT(&t, M_POPCOUNT64(0x8000000000000000ULL) == 1,  "popcount64 high bit");
            MTEST_ASSERT(&t, M_POPCOUNT64(0xFFFFFFFFFFFFFFFFULL) == 64, "popcount64 all-ones");
            MTEST_ASSERT(&t, M_POPCOUNT64(0xAAAAAAAAAAAAAAAAULL) == 32, "popcount64 alternating");
        }

        MTEST_CASE(&t, "popcount32 == popcount64 for 32-bit inputs")
        {
            // the 64-bit path must agree with the 32-bit path on values that fit
            MTEST_ASSERT(&t, M_POPCOUNT64(0xDEADBEEFu) == M_POPCOUNT32(0xDEADBEEFu), "agree on 32-bit value");
            MTEST_ASSERT(&t, M_POPCOUNT64(0xFFFFFFFFu) == M_POPCOUNT32(0xFFFFFFFFu), "agree on all-ones-32");
        }
    }

    MTEST_GROUP(&t, "clz_ctz")
    {
        MTEST_CASE(&t, "clz32")
        {
            MTEST_ASSERT(&t, M_CLZ32(0x80000000u) == 0,  "clz32 bit31 set");
            MTEST_ASSERT(&t, M_CLZ32(0x40000000u) == 1,  "clz32 bit30 set");
            MTEST_ASSERT(&t, M_CLZ32(0x00000001u) == 31, "clz32 bit0 set");
            MTEST_ASSERT(&t, M_CLZ32(0xFFFFFFFFu) == 0,  "clz32 all-ones");
            MTEST_ASSERT(&t, M_CLZ32(0x00008000u) == 16, "clz32 bit15 set");
            MTEST_ASSERT(&t, M_CLZ32(0x00010000u) == 15, "clz32 bit16 set");
        }

        MTEST_CASE(&t, "clz64")
        {
            MTEST_ASSERT(&t, M_CLZ64(0x8000000000000000ULL) == 0,  "clz64 bit63 set");
            MTEST_ASSERT(&t, M_CLZ64(0x4000000000000000ULL) == 1,  "clz64 bit62 set");
            MTEST_ASSERT(&t, M_CLZ64(0x0000000000000001ULL) == 63, "clz64 bit0 set");
            MTEST_ASSERT(&t, M_CLZ64(0xFFFFFFFFFFFFFFFFULL) == 0,  "clz64 all-ones");

            MTEST_ASSERT(&t, M_CLZ64(0x0000000100000000ULL) == 31, "clz64 bit32 set");
            MTEST_ASSERT(&t, M_CLZ64(0x00000000FFFFFFFFULL) == 32, "clz64 only low 32 bits set");
        }

        MTEST_CASE(&t, "ctz32")
        {
            MTEST_ASSERT(&t, M_CTZ32(0x00000001u) == 0,  "ctz32 bit0 set");
            MTEST_ASSERT(&t, M_CTZ32(0x00000002u) == 1,  "ctz32 bit1 set");
            MTEST_ASSERT(&t, M_CTZ32(0x80000000u) == 31, "ctz32 bit31 set");
            MTEST_ASSERT(&t, M_CTZ32(0xFFFFFFFFu) == 0,  "ctz32 all-ones");
            MTEST_ASSERT(&t, M_CTZ32(0x00010000u) == 16, "ctz32 bit16 set");
        }

        MTEST_CASE(&t, "ctz64")
        {
            MTEST_ASSERT(&t, M_CTZ64(0x0000000000000001ULL) == 0,  "ctz64 bit0 set");
            MTEST_ASSERT(&t, M_CTZ64(0x0000000000000002ULL) == 1,  "ctz64 bit1 set");
            MTEST_ASSERT(&t, M_CTZ64(0x8000000000000000ULL) == 63, "ctz64 bit63 set");
            MTEST_ASSERT(&t, M_CTZ64(0xFFFFFFFFFFFFFFFFULL) == 0,  "ctz64 all-ones");

            MTEST_ASSERT(&t, M_CTZ64(0x0000000100000000ULL) == 32, "ctz64 bit32 set");
            MTEST_ASSERT(&t, M_CTZ64(0xFFFFFFFF00000000ULL) == 32, "ctz64 only high 32 bits set");
        }

        MTEST_CASE(&t, "clz32 safe zero")
        {
            MTEST_ASSERT(&t, M_CLZ32_SAFE(0u, 32) == 32, "clz32 safe zero -> 32");
            MTEST_ASSERT(&t, M_CLZ32_SAFE(1u, 32) == 31, "clz32 safe non-zero normal");
        }

        MTEST_CASE(&t, "ctz32 safe zero")
        {
            MTEST_ASSERT(&t, M_CTZ32_SAFE(0u, 32) == 32, "ctz32 safe zero -> 32");
            MTEST_ASSERT(&t, M_CTZ32_SAFE(1u, 32) == 0,  "ctz32 safe non-zero normal");
        }

        MTEST_CASE(&t, "clz64 safe zero")
        {
            MTEST_ASSERT(&t, M_CLZ64_SAFE(0ULL, 64) == 64, "clz64 safe zero -> 64");
            MTEST_ASSERT(&t, M_CLZ64_SAFE(1ULL, 64) == 63, "clz64 safe non-zero normal");
        }

        MTEST_CASE(&t, "ctz64 safe zero")
        {
            MTEST_ASSERT(&t, M_CTZ64_SAFE(0ULL, 64) == 64, "ctz64 safe zero -> 64");
            MTEST_ASSERT(&t, M_CTZ64_SAFE(1ULL, 64) == 0,  "ctz64 safe non-zero normal");
        }

        MTEST_CASE(&t, "clz + ctz consistency")
        {

            MTEST_ASSERT(&t, M_CLZ32(0x00010000u) + M_CTZ32(0x00010000u) == 31, "clz32 + ctz32 single bit");
            MTEST_ASSERT(&t, M_CLZ64(0x0001000000000000ULL) + M_CTZ64(0x0001000000000000ULL) == 63, "clz64 + ctz64 single bit");
        }
    }

    MTEST_GROUP(&t, "bsr_bsf")
    {
        MTEST_CASE(&t, "bsr32")
        {
            MTEST_ASSERT(&t, M_BSR32(0x00000001u) == 0,  "bsr32(1) == 0");
            MTEST_ASSERT(&t, M_BSR32(0x00000002u) == 1,  "bsr32(2) == 1");
            MTEST_ASSERT(&t, M_BSR32(0x80000000u) == 31, "bsr32 bit31");
            MTEST_ASSERT(&t, M_BSR32(0xFFFFFFFFu) == 31, "bsr32 all-ones == 31");
            MTEST_ASSERT(&t, M_BSR32(0x00010000u) == 16, "bsr32 bit16");
        }

        MTEST_CASE(&t, "bsr64")
        {
            MTEST_ASSERT(&t, M_BSR64(0x0000000000000001ULL) == 0,  "bsr64(1) == 0");
            MTEST_ASSERT(&t, M_BSR64(0x8000000000000000ULL) == 63, "bsr64 bit63");
            MTEST_ASSERT(&t, M_BSR64(0xFFFFFFFFFFFFFFFFULL) == 63, "bsr64 all-ones == 63");
            MTEST_ASSERT(&t, M_BSR64(0x0000000100000000ULL) == 32, "bsr64 bit32");
        }

        MTEST_CASE(&t, "bsf32")
        {
            MTEST_ASSERT(&t, M_BSF32(0x00000001u) == 0,  "bsf32(1) == 0");
            MTEST_ASSERT(&t, M_BSF32(0x00000002u) == 1,  "bsf32(2) == 1");
            MTEST_ASSERT(&t, M_BSF32(0x80000000u) == 31, "bsf32 bit31 only");
            MTEST_ASSERT(&t, M_BSF32(0xFFFFFFFFu) == 0,  "bsf32 all-ones == 0");
            MTEST_ASSERT(&t, M_BSF32(0x00010000u) == 16, "bsf32 bit16");
        }

        MTEST_CASE(&t, "bsf64")
        {
            MTEST_ASSERT(&t, M_BSF64(0x0000000000000001ULL) == 0,  "bsf64(1) == 0");
            MTEST_ASSERT(&t, M_BSF64(0x8000000000000000ULL) == 63, "bsf64 bit63 only");
            MTEST_ASSERT(&t, M_BSF64(0xFFFFFFFFFFFFFFFFULL) == 0,  "bsf64 all-ones == 0");
            MTEST_ASSERT(&t, M_BSF64(0x0000000100000000ULL) == 32, "bsf64 bit32");
        }

        MTEST_CASE(&t, "bsr == 31 - clz (32)")
        {
            MTEST_ASSERT(&t, M_BSR32(0x12345678u) == 31 - M_CLZ32(0x12345678u), "bsr32 == 31-clz32");
        }

        MTEST_CASE(&t, "bsf == ctz (32)")
        {
            MTEST_ASSERT(&t, M_BSF32(0x12345678u) == M_CTZ32(0x12345678u), "bsf32 == ctz32");
        }
    }

    MTEST_GROUP(&t, "rotate")
    {
        MTEST_CASE(&t, "rotl32 basic")
        {
            MTEST_ASSERT(&t, M_ROTL32(0x80000000u, 1) == 0x00000001u, "rotl32 high bit wraps");
            MTEST_ASSERT(&t, M_ROTL32(0x00000001u, 1) == 0x00000002u, "rotl32 shift left");
            MTEST_ASSERT(&t, M_ROTL32(0x12345678u, 4) == 0x23456781u, "rotl32 nibble");
            MTEST_ASSERT(&t, M_ROTL32(0xFFFFFFFFu, 7) == 0xFFFFFFFFu, "rotl32 all-ones unchanged");
        }

        MTEST_CASE(&t, "rotr32 basic")
        {
            MTEST_ASSERT(&t, M_ROTR32(0x00000001u, 1) == 0x80000000u, "rotr32 low bit wraps");
            MTEST_ASSERT(&t, M_ROTR32(0x80000000u, 1) == 0x40000000u, "rotr32 shift right");
            MTEST_ASSERT(&t, M_ROTR32(0x12345678u, 4) == 0x81234567u, "rotr32 nibble");
            MTEST_ASSERT(&t, M_ROTR32(0xFFFFFFFFu, 7) == 0xFFFFFFFFu, "rotr32 all-ones unchanged");
        }

        MTEST_CASE(&t, "rotl32 / rotr32 are inverses")
        {
            MTEST_ASSERT(&t, M_ROTR32(M_ROTL32(0xDEADBEEFu, 13), 13) == 0xDEADBEEFu, "rotl32/rotr32 roundtrip");
            MTEST_ASSERT(&t, M_ROTL32(M_ROTR32(0xDEADBEEFu, 17), 17) == 0xDEADBEEFu, "rotr32/rotl32 roundtrip");
        }

        MTEST_CASE(&t, "rotl32 by 0 / 32")
        {
            MTEST_ASSERT(&t, M_ROTL32(0xDEADBEEFu,  0) == 0xDEADBEEFu, "rotl32 by 0 is same");
            MTEST_ASSERT(&t, M_ROTL32(0xDEADBEEFu, 32) == 0xDEADBEEFu, "rotl32 by 32 is same");
        }

        MTEST_CASE(&t, "rotl64 basic")
        {
            MTEST_ASSERT(&t, M_ROTL64(0x8000000000000000ULL, 1) == 0x0000000000000001ULL, "rotl64 high bit wraps");
            MTEST_ASSERT(&t, M_ROTL64(0x0000000000000001ULL, 1) == 0x0000000000000002ULL, "rotl64 shift left");
        }

        MTEST_CASE(&t, "rotr64 basic")
        {
            MTEST_ASSERT(&t, M_ROTR64(0x0000000000000001ULL, 1) == 0x8000000000000000ULL, "rotr64 low bit wraps");
            MTEST_ASSERT(&t, M_ROTR64(0x8000000000000000ULL, 1) == 0x4000000000000000ULL, "rotr64 shift right");
        }

        MTEST_CASE(&t, "rotl64 / rotr64 are inverses")
        {
            MTEST_ASSERT(&t, M_ROTR64(M_ROTL64(0xDEADBEEFCAFEBABEULL, 37), 37) == 0xDEADBEEFCAFEBABEULL, "rotl64/rotr64 roundtrip");
        }
    }

    MTEST_GROUP(&t, "pow2_align")
    {
        MTEST_CASE(&t, "is_pow2")
        {
            MTEST_ASSERT(&t,  M_IS_POW2(1),    "is_pow2(1)");
            MTEST_ASSERT(&t,  M_IS_POW2(2),    "is_pow2(2)");
            MTEST_ASSERT(&t,  M_IS_POW2(4),    "is_pow2(4)");
            MTEST_ASSERT(&t,  M_IS_POW2(1024), "is_pow2(1024)");
            MTEST_ASSERT(&t, !M_IS_POW2(3),    "!is_pow2(3)");
            MTEST_ASSERT(&t, !M_IS_POW2(5),    "!is_pow2(5)");
            MTEST_ASSERT(&t, !M_IS_POW2(6),    "!is_pow2(6)");
            MTEST_ASSERT(&t, !M_IS_POW2(1000), "!is_pow2(1000)");
        }

        MTEST_CASE(&t, "next_pow2_32")
        {
            MTEST_ASSERT(&t, M_NEXT_POW2_32(0)    == 1,    "next_pow2_32(0) == 1");
            MTEST_ASSERT(&t, M_NEXT_POW2_32(1)    == 1,    "next_pow2_32(1) == 1");
            MTEST_ASSERT(&t, M_NEXT_POW2_32(2)    == 2,    "next_pow2_32(2) == 2");
            MTEST_ASSERT(&t, M_NEXT_POW2_32(3)    == 4,    "next_pow2_32(3) == 4");
            MTEST_ASSERT(&t, M_NEXT_POW2_32(4)    == 4,    "next_pow2_32(4) == 4");
            MTEST_ASSERT(&t, M_NEXT_POW2_32(5)    == 8,    "next_pow2_32(5) == 8");
            MTEST_ASSERT(&t, M_NEXT_POW2_32(1000) == 1024, "next_pow2_32(1000) == 1024");
            MTEST_ASSERT(&t, M_NEXT_POW2_32(1024) == 1024, "next_pow2_32(1024) == 1024");

            MTEST_ASSERT(&t, M_IS_POW2(M_NEXT_POW2_32(777)), "next_pow2_32 result is pow2");
        }

        MTEST_CASE(&t, "next_pow2_64")
        {
            MTEST_ASSERT(&t, M_NEXT_POW2_64(0ULL)          == 1ULL,           "next_pow2_64(0) == 1");
            MTEST_ASSERT(&t, M_NEXT_POW2_64(1ULL)          == 1ULL,           "next_pow2_64(1) == 1");
            MTEST_ASSERT(&t, M_NEXT_POW2_64(3ULL)          == 4ULL,           "next_pow2_64(3) == 4");
            MTEST_ASSERT(&t, M_NEXT_POW2_64(0x100000000ULL)== 0x100000000ULL, "next_pow2_64 large exact");
            MTEST_ASSERT(&t, M_NEXT_POW2_64(0x100000001ULL)== 0x200000000ULL, "next_pow2_64 large +1");
        }

        MTEST_CASE(&t, "align_up")
        {
            MTEST_ASSERT(&t, M_ALIGN_UP(0,  16) == 0,  "align_up(0,16)");
            MTEST_ASSERT(&t, M_ALIGN_UP(1,  16) == 16, "align_up(1,16)");
            MTEST_ASSERT(&t, M_ALIGN_UP(15, 16) == 16, "align_up(15,16)");
            MTEST_ASSERT(&t, M_ALIGN_UP(16, 16) == 16, "align_up(16,16) already aligned");
            MTEST_ASSERT(&t, M_ALIGN_UP(17, 16) == 32, "align_up(17,16)");
            MTEST_ASSERT(&t, M_ALIGN_UP(7,   8) == 8,  "align_up(7,8)");
            MTEST_ASSERT(&t, M_ALIGN_UP(8,   8) == 8,  "align_up(8,8)");
        }

        MTEST_CASE(&t, "align_down")
        {
            MTEST_ASSERT(&t, M_ALIGN_DOWN(0,  16) == 0,  "align_down(0,16)");
            MTEST_ASSERT(&t, M_ALIGN_DOWN(1,  16) == 0,  "align_down(1,16)");
            MTEST_ASSERT(&t, M_ALIGN_DOWN(15, 16) == 0,  "align_down(15,16)");
            MTEST_ASSERT(&t, M_ALIGN_DOWN(16, 16) == 16, "align_down(16,16) already aligned");
            MTEST_ASSERT(&t, M_ALIGN_DOWN(31, 16) == 16, "align_down(31,16)");
            MTEST_ASSERT(&t, M_ALIGN_DOWN(32, 16) == 32, "align_down(32,16)");
        }

        MTEST_CASE(&t, "is_aligned")
        {
            MTEST_ASSERT(&t,  M_IS_ALIGNED(0,  16), "is_aligned(0,16)");
            MTEST_ASSERT(&t,  M_IS_ALIGNED(16, 16), "is_aligned(16,16)");
            MTEST_ASSERT(&t,  M_IS_ALIGNED(32, 16), "is_aligned(32,16)");
            MTEST_ASSERT(&t, !M_IS_ALIGNED(1,  16), "!is_aligned(1,16)");
            MTEST_ASSERT(&t, !M_IS_ALIGNED(15, 16), "!is_aligned(15,16)");
        }

        MTEST_CASE(&t, "align_up result is always aligned")
        {
            int align = 64;
            int i;
            for (i = 0; i < 200; i++)
            {
                MTEST_ASSERT(&t, M_IS_ALIGNED(M_ALIGN_UP(i, align), align), "align_up result is aligned");
            }
        }
    }

    MTEST_GROUP(&t, "bit_manip")
    {
        MTEST_CASE(&t, "bit_get")
        {
            MTEST_ASSERT(&t, M_BIT_GET(0xAAAAAAAAu,  0) == 0, "bit_get bit0 of 0xAAAAAAAA");
            MTEST_ASSERT(&t, M_BIT_GET(0xAAAAAAAAu,  1) == 1, "bit_get bit1 of 0xAAAAAAAA");
            MTEST_ASSERT(&t, M_BIT_GET(0xAAAAAAAAu, 31) == 1, "bit_get bit31 of 0xAAAAAAAA");
            MTEST_ASSERT(&t, M_BIT_GET(0x00000000u,  7) == 0, "bit_get zero");
            MTEST_ASSERT(&t, M_BIT_GET(0xFFFFFFFFu, 15) == 1, "bit_get all-ones");
        }

        MTEST_CASE(&t, "bit_set")
        {
            MTEST_ASSERT(&t, M_BIT_SET(0u,  0) == 0x00000001u, "bit_set bit0");
            MTEST_ASSERT(&t, M_BIT_SET(0u, 31) == 0x80000000u, "bit_set bit31");
            MTEST_ASSERT(&t, M_BIT_SET(0xFFFFFFFFu, 5) == 0xFFFFFFFFu, "bit_set already set");
        }

        MTEST_CASE(&t, "bit_clear")
        {
            MTEST_ASSERT(&t, M_BIT_CLEAR(0xFFFFFFFFu,  0) == 0xFFFFFFFEu, "bit_clear bit0");
            MTEST_ASSERT(&t, M_BIT_CLEAR(0xFFFFFFFFu, 31) == 0x7FFFFFFFu, "bit_clear bit31");
            MTEST_ASSERT(&t, M_BIT_CLEAR(0x00000000u,  5) == 0x00000000u, "bit_clear already clear");
        }

        MTEST_CASE(&t, "bit_toggle")
        {
            MTEST_ASSERT(&t, M_BIT_TOGGLE(0u,           4) == 0x00000010u, "bit_toggle sets bit");
            MTEST_ASSERT(&t, M_BIT_TOGGLE(0xFFFFFFFFu,  4) == 0xFFFFFFEFu, "bit_toggle clears bit");

            MTEST_ASSERT(&t, M_BIT_TOGGLE(M_BIT_TOGGLE(0xDEADBEEFu, 13), 13) == 0xDEADBEEFu, "bit_toggle twice is same");
        }

        MTEST_CASE(&t, "bit_set then bit_clear roundtrip")
        {
            m_u32 v = 0x12345678u;
            MTEST_ASSERT(&t, M_BIT_CLEAR(M_BIT_SET(v, 7), 7) == v, "set then clear is equal");
        }

        MTEST_CASE(&t, "mask32")
        {
            MTEST_ASSERT(&t, M_MASK32(0)  == 0x00000000u, "mask32(0)");
            MTEST_ASSERT(&t, M_MASK32(1)  == 0x00000001u, "mask32(1)");
            MTEST_ASSERT(&t, M_MASK32(4)  == 0x0000000Fu, "mask32(4)");
            MTEST_ASSERT(&t, M_MASK32(8)  == 0x000000FFu, "mask32(8)");
            MTEST_ASSERT(&t, M_MASK32(16) == 0x0000FFFFu, "mask32(16)");
            MTEST_ASSERT(&t, M_MASK32(31) == 0x7FFFFFFFu, "mask32(31)");
            MTEST_ASSERT(&t, M_MASK32(32) == 0xFFFFFFFFu, "mask32(32)");
        }

        MTEST_CASE(&t, "mask64")
        {
            MTEST_ASSERT(&t, M_MASK64(0)  == 0x0000000000000000ULL, "mask64(0)");
            MTEST_ASSERT(&t, M_MASK64(1)  == 0x0000000000000001ULL, "mask64(1)");
            MTEST_ASSERT(&t, M_MASK64(32) == 0x00000000FFFFFFFFull,  "mask64(32)");
            MTEST_ASSERT(&t, M_MASK64(63) == 0x7FFFFFFFFFFFFFFFull,  "mask64(63)");
            MTEST_ASSERT(&t, M_MASK64(64) == 0xFFFFFFFFFFFFFFFFULL, "mask64(64)");
        }
    }

    return mtest_terminal_summary(&t);
}