#include <mbuf/mbuf.h>

#include <mtest/mtest.h>
#include <mtest/mtest_terminal.h>

int main(void)
{
    mtest_terminal_ctx ctx;
    mtest t = mtest_new(mtest_terminal_new(&ctx, MTEST_TERMINAL_DEFAULT));

    const m_allocator* a = m_heap_allocator();

    MTEST_GROUP(&t, "core")
    {
        MTEST_CASE(&t, "mbuf_new returns zero state")
        {
            mbuf buf = mbuf_new();
            MTEST_ASSERT(&t, buf.data == NULL, "data is NULL");
            MTEST_ASSERT(&t, buf.size == 0,    "size is 0");
            MTEST_ASSERT(&t, buf.cap  == 0,    "cap is 0");
        }

        MTEST_CASE(&t, "mbuf_setcap grows")
        {
            mbuf buf = mbuf_new();
            mbuf_result r = mbuf_setcap(&buf, a, 64);
            MTEST_ASSERT(&t, r == MBUF_OK,      "returns OK");
            MTEST_ASSERT(&t, buf.data != NULL,  "data is allocated");
            MTEST_ASSERT(&t, buf.cap  == 64,    "cap is 64");
            MTEST_ASSERT(&t, buf.size == 0,     "size unchanged");
            mbuf_free(&buf, a);
        }

        MTEST_CASE(&t, "mbuf_setcap shrinks")
        {
            mbuf buf = mbuf_new();
            mbuf_setcap(&buf, a, 64);
            buf.size = 16;
            mbuf_result r = mbuf_setcap(&buf, a, 32);
            MTEST_ASSERT(&t, r == MBUF_OK,    "returns OK");
            MTEST_ASSERT(&t, buf.cap  == 32,   "cap is 32");
            MTEST_ASSERT(&t, buf.size == 16,   "size unchanged when still fits");
            mbuf_free(&buf, a);
        }

        MTEST_CASE(&t, "mbuf_setcap shrinks clamps size")
        {
            mbuf buf = mbuf_new();
            mbuf_setcap(&buf, a, 64);
            buf.size = 64;
            mbuf_setcap(&buf, a, 32);
            MTEST_ASSERT(&t, buf.size == 32, "size clamped to new cap");
            mbuf_free(&buf, a);
        }

        MTEST_CASE(&t, "mbuf_setcap to 0 frees")
        {
            mbuf buf = mbuf_new();
            mbuf_setcap(&buf, a, 64);
            mbuf_result r = mbuf_setcap(&buf, a, 0);
            MTEST_ASSERT(&t, r == MBUF_OK,      "returns OK");
            MTEST_ASSERT(&t, buf.data == NULL,  "data is NULL");
            MTEST_ASSERT(&t, buf.cap  == 0,     "cap is 0");
            MTEST_ASSERT(&t, buf.size == 0,     "size is 0");
        }

        MTEST_CASE(&t, "mbuf_write writes bytes")
        {
            mbuf buf = mbuf_new();
            mbuf_setcap(&buf, a, 4);
            buf.size = 4;
            const m_u8 src[] = { 1, 2, 3, 4 };
            mbuf_write(&buf, 0, src, 4);
            MTEST_ASSERT(&t, memcmp(buf.data, src, 4) == 0, "data matches");
            mbuf_free(&buf, a);
        }

        MTEST_CASE(&t, "mbuf_write at offset")
        {
            mbuf buf = mbuf_new();
            mbuf_setcap(&buf, a,4);
            buf.size = 4;
            const m_u8 src[] = { 0, 0, 0, 0 };
            mbuf_write(&buf, 0, src, 4);
            const m_u8 patch[] = { 9, 9 };
            mbuf_write(&buf, 1, patch, 2);
            const m_u8 expected[] = { 0, 9, 9, 0 };
            MTEST_ASSERT(&t, memcmp(buf.data, expected, 4) == 0, "patched correctly");
            mbuf_free(&buf, a);
        }

        MTEST_CASE(&t, "mbuf_open shifts tail right")
        {
            mbuf buf = mbuf_new();
            mbuf_setcap(&buf, a, 6);
            buf.size = 4;
            const m_u8 src[] = { 'A', 'B', 'C', 'D' };
            mbuf_write(&buf, 0, src, 4);
            mbuf_open(&buf, 2, 2);
            MTEST_ASSERT(&t, buf.size == 6, "size grew by 2");
            const m_u8 *d = buf.data;
            MTEST_ASSERT(&t, d[0] == 'A', "A in place");
            MTEST_ASSERT(&t, d[1] == 'B', "B in place");
            MTEST_ASSERT(&t, d[4] == 'C', "C shifted to 4");
            MTEST_ASSERT(&t, d[5] == 'D', "D shifted to 5");
            mbuf_free(&buf, a);
        }

        MTEST_CASE(&t, "mbuf_open at start")
        {
            mbuf buf = mbuf_new();
            mbuf_setcap(&buf, a, 5);
            buf.size = 3;
            const m_u8 src[] = { 1, 2, 3 };
            mbuf_write(&buf, 0, src, 3);
            mbuf_open(&buf, 0, 2);
            MTEST_ASSERT(&t, buf.size == 5, "size grew");
            const m_u8 *d = buf.data;
            MTEST_ASSERT(&t, d[2] == 1, "1 shifted to 2");
            MTEST_ASSERT(&t, d[3] == 2, "2 shifted to 3");
            MTEST_ASSERT(&t, d[4] == 3, "3 shifted to 4");
            mbuf_free(&buf, a);
        }

        MTEST_CASE(&t, "mbuf_open at end")
        {
            mbuf buf = mbuf_new();
            mbuf_setcap(&buf, a, 5);
            buf.size = 3;
            const m_u8 src[] = { 1, 2, 3 };
            mbuf_write(&buf, 0, src, 3);
            mbuf_open(&buf, 3, 2);
            MTEST_ASSERT(&t, buf.size == 5, "size grew");
            const m_u8 *d = buf.data;
            MTEST_ASSERT(&t, d[0] == 1, "1 unchanged");
            MTEST_ASSERT(&t, d[1] == 2, "2 unchanged");
            MTEST_ASSERT(&t, d[2] == 3, "3 unchanged");
            mbuf_free(&buf, a);
        }

        MTEST_CASE(&t, "mbuf_close shifts tail left")
        {
            mbuf buf = mbuf_new();
            mbuf_setcap(&buf, a, 4);
            buf.size = 4;
            const m_u8 src[] = { 'A', 'B', 'C', 'D' };
            mbuf_write(&buf, 0, src, 4);
            mbuf_close(&buf, 1, 2);
            MTEST_ASSERT(&t, buf.size == 2, "size shrunk by 2");
            const m_u8 *d = buf.data;
            MTEST_ASSERT(&t, d[0] == 'A', "A in place");
            MTEST_ASSERT(&t, d[1] == 'D', "D moved to 1");
            mbuf_free(&buf, a);
        }

        MTEST_CASE(&t, "mbuf_close at start")
        {
            mbuf buf = mbuf_new();
            mbuf_setcap(&buf, a, 3);
            buf.size = 3;
            const m_u8 src[] = { 1, 2, 3 };
            mbuf_write(&buf, 0, src, 3);
            mbuf_close(&buf, 0, 2);
            MTEST_ASSERT(&t, buf.size == 1, "size shrunk");
            MTEST_ASSERT(&t, ((m_u8*)buf.data)[0] == 3, "3 moved to front");
            mbuf_free(&buf, a);
        }

        MTEST_CASE(&t, "mbuf_close at end")
        {
            mbuf buf = mbuf_new();
            mbuf_setcap(&buf, a, 3);
            buf.size = 3;
            const m_u8 src[] = { 1, 2, 3 };
            mbuf_write(&buf, 0, src, 3);
            mbuf_close(&buf, 1, 2);
            MTEST_ASSERT(&t, buf.size == 1, "size shrunk");
            MTEST_ASSERT(&t, ((m_u8*)buf.data)[0] == 1, "1 unchanged");
            mbuf_free(&buf, a);
        }

        MTEST_CASE(&t, "mbuf_close entire buffer")
        {
            mbuf buf = mbuf_new();
            mbuf_setcap(&buf, a, 3);
            buf.size = 3;
            const m_u8 src[] = { 1, 2, 3 };
            mbuf_write(&buf, 0, src, 3);
            mbuf_close(&buf, 0, 3);
            MTEST_ASSERT(&t, buf.size == 0, "size is 0");
            mbuf_free(&buf, a);
        }
    }

    return mtest_terminal_summary(&t);
}