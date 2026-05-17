#include "mtest.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

mtest mtest_new(const mtest_observer observer)
{
    const mtest test = {
        .observer = observer,
        .group_depth = 0,
        .total_failed = 0,
        .total_passed = 0,
        .case_active = 0,
    };

    return test;
}

void mtest_group_begin(mtest *t, const char *name)
{
    assert(t->group_depth < MTEST_MAX_GROUPS);

    strncpy(t->groups[t->group_depth], name, sizeof(t->groups[t->group_depth]) - 1);
    t->groups[t->group_depth][sizeof(t->groups[t->group_depth]) - 1] = '\0';

    t->group_passed[t->group_depth] = 0;
    t->group_failed[t->group_depth] = 0;

    if (t->observer.on_group_start)
        t->observer.on_group_start(t, t->groups[t->group_depth], t->observer.ctx);

    t->group_depth++;
}

void mtest_group_beginf(mtest *t, const char *fmt, ...)
{
    char buf[MTEST_MAX_GROUP_NAME];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    mtest_group_begin(t, buf);
}

void mtest_group_end(mtest *t)
{
    assert(t->group_depth > 0);

    t->group_depth--;

    if (t->group_depth > 0)
    {
        t->group_passed[t->group_depth - 1] += t->group_passed[t->group_depth];
        t->group_failed[t->group_depth - 1] += t->group_failed[t->group_depth];
    }

    if (t->observer.on_group_end)
        t->observer.on_group_end(
            t,
            t->groups[t->group_depth],
            t->group_passed[t->group_depth],
            t->group_failed[t->group_depth],
            t->observer.ctx
        );
}

void mtest_case_begin(mtest *t, const char *name)
{
    t->case_active = 1;
    t->case_result = MTEST_CASE_PASS;

    strncpy(t->case_name, name, sizeof(t->case_name) - 1);
    t->case_name[sizeof(t->case_name) - 1] = '\0';

    if (t->observer.on_case_start)
        t->observer.on_case_start(t, t->case_name, t->observer.ctx);
}

void mtest_case_beginf(mtest *t, const char *fmt, ...)
{
    char buf[MTEST_MAX_CASE_NAME];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    mtest_case_begin(t, buf);
}

void mtest_case_end(mtest *t)
{
    assert(t->case_active == 1);
    t->case_active = 0;

    switch (t->case_result)
    {
        case MTEST_CASE_PASS:
            t->total_passed++;
            if (t->group_depth > 0) t->group_passed[t->group_depth - 1]++;
            break;

        case MTEST_CASE_FAIL:
            t->total_failed++;
            if (t->group_depth > 0) t->group_failed[t->group_depth - 1]++;
            break;

        default: break;
    }

    if (t->observer.on_case_end)
        t->observer.on_case_end(t, t->case_name, t->case_result, t->observer.ctx);
}

void mtest_fail(mtest *t, const char *file, int line, const char *msg)
{
    t->case_result = MTEST_CASE_FAIL;

    if (t->observer.on_fail)
        t->observer.on_fail(t, file, line, msg, t->observer.ctx);
}

void mtest_failf(mtest *t, const char *file, int line, const char *fmt, ...)
{
    char buf[MTEST_MAX_MSG];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    mtest_fail(t, file, line, buf);
}

void mtest_msgf(mtest *t, const char *fmt, ...)
{
    if (!t->observer.on_msg) return;

    char buf[MTEST_MAX_MSG];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    t->observer.on_msg(t, buf, t->observer.ctx);
}