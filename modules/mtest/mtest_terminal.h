#ifndef M_MTEST_TERMINAL_H
#define M_MTEST_TERMINAL_H

#include "mtest.h"


// ================================================
//                   OPTIONS
//

struct mtest_terminal_opts
{
    int only_on_fail;
    int tree;
    int verbose;
};
typedef struct mtest_terminal_opts mtest_terminal_opts;

#define MTEST_TERMINAL_DEFAULT ((mtest_terminal_opts){ 0, 1, 0 })
#define MTEST_TERMINAL_CI      ((mtest_terminal_opts){ 1, 0, 0 })


// ================================================
//                     STATE
//

struct mtest_terminal_ctx
{
    mtest_terminal_opts    opts;
    mtest_buffered_ctx     _buffered;
};
typedef struct mtest_terminal_ctx mtest_terminal_ctx;


// ================================================
//                      PUBLIC
//

static mtest_observer mtest_terminal_new(mtest_terminal_ctx *ctx, mtest_terminal_opts opts);
static int mtest_terminal_summary(mtest* t);


// ================================================
//                      PRIVATE
//

static void mtest_terminal__indent(int depth)
{
    for (int i = 0; i < depth; i++)
        printf("  ");
}

static void mtest_terminal__path(mtest *t)
{
    for (int i = 0; i < mtest_depth(t); i++) {
        if (i > 0) printf("/");
        printf("%s", mtest_group_name(t, i));
    }
    if (mtest_case_name(t)[0] != '\0')
        printf("/%s", mtest_case_name(t));
}

static void mtest_terminal__on_group_start(mtest *t, const char *name, void *ctx)
{
    mtest_terminal_ctx *c = ctx;
    if (!c->opts.tree) return;
    mtest_terminal__indent(mtest_depth(t));
    printf("%s\n", name);
}

static void mtest_terminal__on_group_end(mtest *t, const char *name, int passed, int failed, void *ctx)
{
    mtest_terminal_ctx *c = ctx;
    if (!c->opts.verbose || !c->opts.tree) return;
    mtest_terminal__indent(mtest_depth(t));
    printf("%s: %d passed  %d failed\n", name, passed, failed);
}

static void mtest_terminal__on_case(mtest *t, const char *name, mtest_case_result result,
                                    const mtest_event *events, int event_count, void *ctx)
{
    mtest_terminal_ctx *c = ctx;
    int depth = mtest_depth(t);

    if (c->opts.only_on_fail && result == MTEST_CASE_PASS) return;

    if (c->opts.tree)
    {
        mtest_terminal__indent(depth + 1);
        printf("%s  %s\n", result == MTEST_CASE_PASS ? "PASS" : "FAIL", name);

        if (result == MTEST_CASE_FAIL)
        {
            for (int i = 0; i < event_count; i++)
            {
                const mtest_event *e = &events[i];
                mtest_terminal__indent(depth + 2);
                printf("%-5s %s\n", e->type == MTEST_EVENT_MSG ? "MSG" : "FAIL", e->msg);
                mtest_terminal__indent(depth + 2);
                printf("      %s:%d\n", e->file, e->line);
            }
        }
    }
    else
    {
        if (result == MTEST_CASE_PASS)
        {
            printf("PASS  ");
            mtest_terminal__path(t);
            printf("/%s\n", name);
        }
        else
        {
            printf("FAIL  ");
            mtest_terminal__path(t);
            printf("/%s\n", name);
            for (int i = 0; i < event_count; i++)
            {
                const mtest_event *e = &events[i];
                printf("      %-5s %s\n", e->type == MTEST_EVENT_MSG ? "MSG" : "FAIL", e->msg);
                printf("            %s:%d\n", e->file, e->line);
            }
            printf("\n");
        }
    }
}

static mtest_observer mtest_terminal_new(mtest_terminal_ctx *ctx, mtest_terminal_opts opts)
{
    memset(ctx, 0, sizeof(*ctx));
    ctx->opts = opts;

    mtest_buffered_handler handler;
    memset(&handler, 0, sizeof(handler));
    handler.on_group_start = mtest_terminal__on_group_start;
    handler.on_group_end   = mtest_terminal__on_group_end;
    handler.on_case        = mtest_terminal__on_case;
    handler.ctx            = ctx;

    return mtest_buffered_new(&ctx->_buffered, handler);
}

static int mtest_terminal_summary(mtest* t)
{
    printf("\nfailed: %d,  passed: %d\n", t->total_failed, t->total_passed);
    return t->total_failed;
}

#endif //M_MTEST_TERMINAL_H