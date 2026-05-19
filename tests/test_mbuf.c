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

    MTEST_GROUP(&t, "mbuf utility")
    {
        MTEST_CASE(&t, "mbuf_reserve ensures minimum capacity")
        {
            mbuf buf = mbuf_new();

            mbuf_result r = mbuf_reserve(&buf, a, 32);
            MTEST_ASSERT(&t, r == MBUF_OK, "reserve OK");
            MTEST_ASSERT(&t, buf.cap >= 32, "capacity meets minimum");

            m_usize old_cap = buf.cap;
            r = mbuf_reserve(&buf, a, 16);
            MTEST_ASSERT(&t, r == MBUF_OK, "reserve lower OK");
            MTEST_ASSERT(&t, buf.cap == old_cap, "capacity remains unchanged");

            mbuf_free(&buf, a);
        }

        MTEST_CASE(&t, "mbuf_shrink reduces capacity down to max(min_cap, size)")
        {
            mbuf buf = mbuf_new();
            mbuf_setcap(&buf, a, 128);
            buf.size = 20;

            mbuf_result r = mbuf_shrink(&buf, a, 32);
            MTEST_ASSERT(&t, r == MBUF_OK, "shrink OK");
            MTEST_ASSERT(&t, buf.cap == 32, "shrunk to min_cap floor");
            MTEST_ASSERT(&t, buf.size == 20, "size intact");

            r = mbuf_shrink(&buf, a, 5);
            MTEST_ASSERT(&t, r == MBUF_OK, "shrink OK");
            MTEST_ASSERT(&t, buf.cap == 20, "shrunk and clamped to size floor");

            mbuf_free(&buf, a);
        }

        MTEST_CASE(&t, "mbuf_clear resets size but preserves capacity")
        {
            mbuf buf = mbuf_new();
            mbuf_setcap(&buf, a, 64);
            buf.size = 32;

            mbuf_clear(&buf);
            MTEST_ASSERT(&t, buf.size == 0, "size is zeroed");
            MTEST_ASSERT(&t, buf.cap == 64, "capacity is preserved");
            MTEST_ASSERT(&t, buf.data != NULL, "allocation stays active");

            mbuf_free(&buf, a);
        }

        MTEST_CASE(&t, "mbuf_insert copies mview data and triggers growth")
        {
            mbuf buf = mbuf_new();
            const m_u8 initial[] = { 'A', 'D' };
            mbuf_setcap(&buf, a, 2);
            mbuf_write(&buf, 0, initial, 2);
            buf.size = 2;

            const m_u8 payload[] = { 'B', 'C' };
            mview src = mview_make(payload, 2);

            mbuf_result r = mbuf_insert(&buf, a, 1, src, mbuf_grow_x2);
            MTEST_ASSERT(&t, r == MBUF_OK, "insert OK");
            MTEST_ASSERT(&t, buf.size == 4, "size is now 4");
            MTEST_ASSERT(&t, buf.cap >= 4, "buffer grew safely");

            const m_u8 *d = (const m_u8*)buf.data;
            MTEST_ASSERT(&t, d[0] == 'A', "A at pos 0");
            MTEST_ASSERT(&t, d[1] == 'B', "B inserted at pos 1");
            MTEST_ASSERT(&t, d[2] == 'C', "C inserted at pos 2");
            MTEST_ASSERT(&t, d[3] == 'D', "D shifted to pos 3");

            mbuf_free(&buf, a);
        }

        MTEST_CASE(&t, "mbuf_remove deletes a middle chunk")
        {
            mbuf buf = mbuf_new();
            mbuf_setcap(&buf, a, 5);
            const m_u8 initial[] = { 10, 20, 30, 40, 50 };
            mbuf_write(&buf, 0, initial, 5);
            buf.size = 5;

            mbuf_remove(&buf, 1, 2);
            MTEST_ASSERT(&t, buf.size == 3, "size dropped down by 2");

            const m_u8 *d = (const m_u8*)buf.data;
            MTEST_ASSERT(&t, d[0] == 10, "10 intact");
            MTEST_ASSERT(&t, d[1] == 40, "40 shifted left");
            MTEST_ASSERT(&t, d[2] == 50, "50 shifted left");

            mbuf_free(&buf, a);
        }

        MTEST_CASE(&t, "mbuf slices and views match internal layouts")
        {
            mbuf buf = mbuf_new();
            mbuf_setcap(&buf, a, 4);
            const m_u8 data[] = { 'W', 'X', 'Y', 'Z' };
            mbuf_write(&buf, 0, data, 4);
            buf.size = 4;

            mview view = mbuf_as_view(&buf);
            MTEST_ASSERT(&t, view.data == buf.data, "view data matches buffer");
            MTEST_ASSERT(&t, view.size == buf.size, "view size matches buffer");

            mspan span = mbuf_as_span(&buf);
            MTEST_ASSERT(&t, span.data == buf.data, "span data matches buffer");
            MTEST_ASSERT(&t, span.size == buf.size, "span size matches buffer");

            mview slice = mbuf_slice(&buf, 1, 2);
            MTEST_ASSERT(&t, slice.size == 2, "slice length correct");
            MTEST_ASSERT(&t, ((const m_u8*)slice.data)[0] == 'X', "slice start correct");
            MTEST_ASSERT(&t, ((const m_u8*)slice.data)[1] == 'Y', "slice end correct");

            mview clamped_slice = mbuf_slice(&buf, 2, 100);
            MTEST_ASSERT(&t, clamped_slice.size == 2, "slice size clamped to maximum available tail");

            mbuf_free(&buf, a);
        }
    }

    MTEST_GROUP(&t, "grow procs")
    {
        MTEST_CASE(&t, "mbuf_grow_exact returns requested size")
        {
            MTEST_ASSERT(&t, mbuf_grow_exact(0, 10) == 10, "0 to 10 is 10");
            MTEST_ASSERT(&t, mbuf_grow_exact(16, 20) == 20, "16 to 20 is 20");
            MTEST_ASSERT(&t, mbuf_grow_exact(0, 20000) == 20000, "0 to 20000 is 20000");
        }

        MTEST_CASE(&t, "mbuf_grow_x2")
        {
            MTEST_ASSERT(&t, mbuf_grow_x2(0, 4) >= 4, "bigger or equal to 4");

            MTEST_ASSERT(&t, mbuf_grow_x2(16, 20) == 32, "16 doubled is 32");

            MTEST_ASSERT(&t, mbuf_grow_x2(16, 50) == 50, "16 doubled (32) < 50, so return 50");

            MTEST_ASSERT(&t, mbuf_grow_x2(64, 100) == 128, "64 doubled is 128");
        }

        MTEST_CASE(&t, "mbuf_grow_x1_5")
        {
            MTEST_ASSERT(&t, mbuf_grow_x1_5(0, 4) == 8, "0 cap defaults to 8");

            MTEST_ASSERT(&t, mbuf_grow_x1_5(16, 17) == 24, "16 scaled by 1.5x is 24");
            MTEST_ASSERT(&t, mbuf_grow_x1_5(16, 24) == 24, "16 scaled by 1.5x is 24");

            MTEST_ASSERT(&t, mbuf_grow_x1_5(16, 40) == 40, "16 scaled (24) < 40, jumps to 40");
        }

        MTEST_GROUP(&t, "grow pow2")
        {

            MTEST_CASE(&t, "mbuf_grow_pow2 clean power of 2 targets")
            {
                MTEST_ASSERT(&t, mbuf_grow_pow2(0, 4)   == 4,   "requested 4 returns 4");
                MTEST_ASSERT(&t, mbuf_grow_pow2(4, 8)   == 8,   "requested 8 returns 8");
                MTEST_ASSERT(&t, mbuf_grow_pow2(8, 16)  == 16,  "requested 16 returns 16");
                MTEST_ASSERT(&t, mbuf_grow_pow2(16, 64) == 64,  "requested 64 returns 64");
            }

            MTEST_CASE(&t, "mbuf_grow_pow2 mid range rounding")
            {
                MTEST_ASSERT(&t, mbuf_grow_pow2(4, 5)   == 8,   "requested 5 rounds up to 8");
                MTEST_ASSERT(&t, mbuf_grow_pow2(4, 7)   == 8,   "requested 7 rounds up to 8");
                MTEST_ASSERT(&t, mbuf_grow_pow2(8, 9)   == 16,  "requested 9 rounds up to 16");
                MTEST_ASSERT(&t, mbuf_grow_pow2(16, 33) == 64,  "requested 33 rounds up to 64");
            }

            MTEST_CASE(&t, "mbuf_grow_pow2 extreme overflow protection")
            {
    #if M_64_BIT
                m_usize giant_request = ((m_usize)1 << 63) + 5;
    #else
                m_usize giant_request = ((m_usize)1 << 31) + 5;
    #endif
                m_usize result = mbuf_grow_pow2(0, giant_request);
                MTEST_ASSERT(&t, result == 0, "catches overflow and errors");
            }
        }
    }

    return mtest_terminal_summary(&t);
}