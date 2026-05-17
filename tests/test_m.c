#include <stdio.h>

#include "m/m.h"
#include "mtest/mtest.h"

void on_group_start(mtest *t, const char *name, void *ctx)
{
    for (int i = 0; i < t->group_depth; i++)
        printf("  ");

    printf("[%s]\n", name);
}

void on_group_end(mtest *t, const char *name, int passed, int failed, void *ctx)
{
    for (int i = 0; i < t->group_depth; i++)
        printf("  ");

    printf("%s: %d passed  %d failed\n", name, passed, failed);
}

void on_case_end(mtest *t, const char *name, mtest_case_result result, void *ctx)
{
    for (int i = 0; i < t->group_depth; i++)
        printf("  ");

    printf("  %s  %s\n", result == MTEST_CASE_PASS ? "PASS" : "FAIL", name);
}

void on_fail(mtest *t, const char *file, int line, const char *msg, void *ctx)
{
    for (int i = 0; i < t->group_depth; i++)
        printf("  ");

    printf("    %s\n", msg);

    for (int i = 0; i < t->group_depth; i++)
        printf("  ");

    printf("    %s:%d\n", file, line);
}

void on_msg(mtest *t, const char *msg, void *ctx)
{
    for (int i = 0; i < t->group_depth; i++)
        printf("  ");

    printf(") %s\n", msg);
}

int main(void)
{
    mtest_observer observer = {
        .on_group_start = on_group_start,
        .on_group_end   = on_group_end,
        .on_case_end    = on_case_end,
        .on_fail        = on_fail,
        .on_msg         = on_msg,
        .ctx            = NULL,
    };

    mtest t = mtest_new(observer);

    MTEST_GROUP(&t, "m")
    {
        MTEST_GROUP(&t, "types")
        {
            MTEST_CASE(&t, "sizes")
            {
                MTEST_ASSERT(&t, sizeof(m_u8)  == 1, "m_u8 is 1 byte");
                MTEST_ASSERT(&t, sizeof(m_u16) == 2, "m_u16 is 2 bytes");
                MTEST_ASSERT(&t, sizeof(m_u32) == 4, "m_u32 is 4 bytes");
                MTEST_ASSERT(&t, sizeof(m_u64) == 8, "m_u64 is 8 bytes");
                mtest_msgf(&t, "hello world");
            }
        }

        MTEST_GROUP(&t, "allocator")
        {
            MTEST_CASE(&t, "heap alloc")
            {
                const m_allocator *a = m_heap_allocator();
                void *ptr = M_ALLOC(a, 64);
                MTEST_ASSERT(&t, ptr != NULL, "alloc returns non-null");
                M_FREE(a, ptr, 64);
            }

            MTEST_CASE(&t, "heap realloc")
            {
                const m_allocator *a = m_heap_allocator();
                void *ptr = M_ALLOC(a, 64);
                ptr = M_REALLOC(a, ptr, 64, 128);
                MTEST_ASSERT(&t, ptr != NULL, "realloc returns non-null");
                M_FREE(a, ptr, 128);
            }

            MTEST_CASE(&t, "should fail")
            {
                MTEST_ASSERT(&t, sizeof(int) == 1, "int is 1 byte");
            }
        }
    }

    printf("\npassed: %d  failed: %d\n", t.total_passed, t.total_failed);

    return t.total_failed;
}