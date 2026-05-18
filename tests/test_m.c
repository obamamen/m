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
            const m_allocator* a = m_heap_allocator();
            char* m = M_ALLOC(a, 128);

            MTEST_ASSERT_FATAL(&t, m != NULL, "alloc failed");

            m[0] = 'X';

            char* newptr = M_REALLOC(a, m, 128, 256);
            if (!newptr) M_FREE(a, m, 128);

            MTEST_ASSERT_FATAL(&t, newptr != NULL,"realloc returned non-null");
            MTEST_ASSERT(&t, newptr[0] == 'X',    "data preserved after grow");

            M_FREE(a, newptr, 256);
        }

        MTEST_CASE(&t, "realloc shrink")
        {
            const m_allocator* a = m_heap_allocator();
            char *m = M_ALLOC(a, 128);

            MTEST_ASSERT_FATAL(&t, m != NULL, "alloc failed");

            m[0] = 'X';

            char* newptr = M_REALLOC(a, m, 128, 8);
            if (!newptr) M_FREE(a, m, 128);

            MTEST_ASSERT_FATAL(&t, newptr != NULL,   "realloc shrink returned non-null");
            MTEST_ASSERT(&t, newptr[0] == 'X', "data preserved after shrink");

            M_FREE(a, newptr, 8);
        }

        MTEST_CASE(&t, "free null")
        {
            const m_allocator* a = m_heap_allocator();
            M_FREE(a, NULL, 0);
            MTEST_ASSERT(&t, 1, "free null does not crash");
        }

        MTEST_CASE(&t, "write read")
        {
            const m_allocator* a = m_heap_allocator();
            int* m = M_ALLOC(a, sizeof(int) * 4);
            MTEST_ASSERT_FATAL(&t, m != NULL, "alloc failed");
            m[0] = 1; m[1] = 2; m[2] = 3; m[3] = 4;
            MTEST_ASSERT(&t, m[0] == 1, "m[0] == 1");
            MTEST_ASSERT(&t, m[1] == 2, "m[1] == 2");
            MTEST_ASSERT(&t, m[2] == 3, "m[2] == 3");
            MTEST_ASSERT(&t, m[3] == 4, "m[3] == 4");
            M_FREE(a, m, sizeof(int) * 4);
        }
    }

    MTEST_GROUP(&t, "byte_swap")
    {
        MTEST_CASE(&t, "bswap16")
        {
            MTEST_ASSERT(&t, M_BSWAP16(0x1234) == 0x3412, "bswap16 0x1234");
            MTEST_ASSERT(&t, M_BSWAP16(0x0001) == 0x0100, "bswap16 0x0001");
            MTEST_ASSERT(&t, M_BSWAP16(0xFF00) == 0x00FF, "bswap16 0xFF00");
        }

        MTEST_CASE(&t, "bswap32")
        {
            MTEST_ASSERT(&t, M_BSWAP32(0x12345678) == 0x78563412, "bswap32 0x12345678");
            MTEST_ASSERT(&t, M_BSWAP32(0xFF000000) == 0x000000FF, "bswap32 0xFF000000");
        }

        MTEST_CASE(&t, "bswap64")
        {
            MTEST_ASSERT(&t, M_BSWAP64(0x123456789ABCDEF0ULL) == 0xF0DEBC9A78563412ULL, "bswap64");
        }

        MTEST_CASE(&t, "endianness detected")
        {
            MTEST_ASSERT(&t, M_IS_LITTLE_ENDIAN || M_IS_BIG_ENDIAN,   "one must be true");
            MTEST_ASSERT(&t, !(M_IS_LITTLE_ENDIAN && M_IS_BIG_ENDIAN), "not both at once");
        }

        MTEST_CASE(&t, "hton roundtrip")
        {
            MTEST_ASSERT(&t, M_NTOH16(M_HTON16(0x1234))     == 0x1234,     "hton16 roundtrip");
            MTEST_ASSERT(&t, M_NTOH32(M_HTON32(0x12345678)) == 0x12345678, "hton32 roundtrip");
            MTEST_ASSERT(&t, M_NTOH64(M_HTON64(0x123456789ABCDEF0ULL)) == 0x123456789ABCDEF0ULL, "hton64 roundtrip");
        }
    }


    return mtest_terminal_summary(&t);
}