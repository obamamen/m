#include "mtest.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

mtest mtest_new(const mtest_observer observer)
{
    const mtest t = {
        .observer = observer,

        ._case_active = 0,
        ._group_depth = 0,

        .total_failed = 0,
        .total_passed = 0,
    };


    return t;
}

void mtest_group_begin(mtest *t, const char* name)
{
    assert(t->_group_depth < MTEST_MAX_GROUPS);

    snprintf(t->_groups[t->_group_depth], sizeof(t->_groups[t->_group_depth]), "%s", name);

    t->_group_passed[t->_group_depth] = 0;
    t->_group_failed[t->_group_depth] = 0;

    if (t->observer.on_group_start)
        t->observer.on_group_start(t, t->_groups[t->_group_depth], t->observer.ctx);

    t->_group_depth++;
}

void mtest_group_beginf(mtest *t, const char* fmt, ...)
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
    assert(t->_group_depth > 0);

    t->_group_depth--;

    if (t->_group_depth > 0)
    {
        t->_group_passed[t->_group_depth - 1] += t->_group_passed[t->_group_depth];
        t->_group_failed[t->_group_depth - 1] += t->_group_failed[t->_group_depth];
    }

    if (t->observer.on_group_end)
        t->observer.on_group_end(
            t,
            t->_groups[t->_group_depth],
            t->_group_passed[t->_group_depth],
            t->_group_failed[t->_group_depth],
            t->observer.ctx
        );
}

void mtest_case_begin(mtest *t, const char* name)
{
    assert(t->_case_active == 0);

    snprintf(t->_case_name, sizeof(t->_case_name), "%s", name);
    t->_case_result = MTEST_CASE_PASS;
    t->_case_active = 1;

    if (t->observer.on_case_start)
        t->observer.on_case_start(t, t->_case_name, t->observer.ctx);
}

void mtest_case_beginf(mtest *t, const char* fmt, ...)
{
    char buf[MTEST_MAX_CASE_NAME];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    mtest_case_begin(t, buf);
}

void mtest_case_end(mtest* t)
{
    assert(t->_case_active == 1);

    t->_case_active = 0;

    if (t->_case_result == MTEST_CASE_PASS)
    {
        t->total_passed++;
        if (t->_group_depth > 0)
            t->_group_passed[t->_group_depth - 1]++;
    }
    else
    {
        t->total_failed++;
        if (t->_group_depth > 0)
            t->_group_failed[t->_group_depth - 1]++;
    }

    if (t->observer.on_case_end)
        t->observer.on_case_end(t, t->_case_name, t->_case_result, t->observer.ctx);
}

void mtest_event_(mtest* t, mtest_event_type type, const char* file, int line, const char* msg)
{
    if (type == MTEST_EVENT_FAIL)
        t->_case_result = MTEST_CASE_FAIL;

    if (t->observer.on_event)
    {
        mtest_event e;
        e.type = type;
        e.file = file;
        e.line = line;
        snprintf(e.msg, sizeof(e.msg), "%s", msg);
        t->observer.on_event(t, &e, t->observer.ctx);
    }
}

void mtest_eventf_(mtest* t, mtest_event_type type, const char* file, int line, const char* fmt, ...)
{
    char buf[MTEST_MSG_MAX];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    mtest_event_(t, type, file, line, buf);
}

static void mtest_buffered__on_group_start(mtest *t, const char *name, void *ctx)
{
    mtest_buffered_ctx *c = ctx;
    if (c->handler.on_group_start)
        c->handler.on_group_start(t, name, c->handler.ctx);
}

static void mtest_buffered__on_group_end(mtest *t, const char *name, int passed, int failed, void *ctx)
{
    mtest_buffered_ctx *c = ctx;
    if (c->handler.on_group_end)
        c->handler.on_group_end(t, name, passed, failed, c->handler.ctx);
}

static void mtest_buffered__on_case_start(mtest *t, const char *name, void *ctx)
{
    (void)t; (void)name;
    mtest_buffered_ctx *c = ctx;
    c->event_count = 0;
}

static void mtest_buffered__on_case_end(mtest *t, const char *name, mtest_case_result result, void *ctx)
{
    mtest_buffered_ctx *c = ctx;
    if (c->handler.on_case)
        c->handler.on_case(t, name, result, c->events, c->event_count, c->handler.ctx);
    c->event_count = 0;
}

static void mtest_buffered__on_event(mtest *t, const mtest_event *e, void *ctx)
{
    (void)t;
    mtest_buffered_ctx *c = ctx;
    if (c->event_count < MTEST_MAX_BUFFERED_EVENTS)
        c->events[c->event_count++] = *e;
}

mtest_observer mtest_buffered_new(mtest_buffered_ctx *ctx, mtest_buffered_handler handler)
{
    memset(ctx, 0, sizeof(*ctx));
    ctx->handler = handler;

    mtest_observer o = {0};
    o.on_group_start = mtest_buffered__on_group_start;
    o.on_group_end   = mtest_buffered__on_group_end;
    o.on_case_start  = mtest_buffered__on_case_start;
    o.on_case_end    = mtest_buffered__on_case_end;
    o.on_event       = mtest_buffered__on_event;
    o.ctx            = ctx;

    return o;
}




