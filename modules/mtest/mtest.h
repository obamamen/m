#ifndef MTEST_H
#define MTEST_H

#include <m/m.h>

#define MTEST_MAX_CASE_NAME 128
#define MTEST_MAX_GROUP_NAME 128
#define MTEST_MAX_MSG 256

#define MTEST_MAX_GROUPS 16

typedef struct mtest mtest;

enum mtest_case_result
{
    MTEST_CASE_PASS,
    MTEST_CASE_FAIL,
};
typedef enum mtest_case_result mtest_case_result;

struct mtest_observer
{
    void (*on_group_start)  (mtest *t, const char *name, void *ctx);
    void (*on_group_end)    (mtest *t, const char *name, int passed, int failed, void *ctx);

    void (*on_case_start)   (mtest *t, const char *name, void *ctx);
    void (*on_case_end)     (mtest *t, const char *name, mtest_case_result result, void *ctx);

    void (*on_fail)         (mtest *t, const char *file, int line, const char *msg, void *ctx);
    void (*on_msg)          (mtest *t, const char *msg, void *ctx);

    void                    *ctx;
};
typedef struct mtest_observer mtest_observer;

struct mtest
{
    mtest_observer observer;

    char groups[MTEST_MAX_GROUPS][MTEST_MAX_GROUP_NAME];
    int group_passed[MTEST_MAX_GROUPS];
    int group_failed[MTEST_MAX_GROUPS];
    int group_depth;

    char case_name[MTEST_MAX_CASE_NAME];
    mtest_case_result case_result;
    int case_active;

    int total_passed;
    int total_failed;
};


#define MTEST_GROUP(t, name) \
    M_BLOCK(M_CONCAT(mtest_g_, __LINE__), mtest_group_begin(t, name), mtest_group_end(t))

#define MTEST_CASE(t, name) \
    M_BLOCK(M_CONCAT(mtest_c_, __LINE__), mtest_case_begin(t, name), mtest_case_end(t))

#define MTEST_ASSERT(t, cond, msg) \
    do {                           \
        if (!(cond)) mtest_fail(t, __FILE__, __LINE__, msg); \
    } while(0)


mtest mtest_new(mtest_observer observer);

void mtest_group_begin  (mtest *t, const char *name);
void mtest_group_beginf (mtest *t, const char *fmt, ...);

void mtest_group_end(mtest *t);

void mtest_case_begin   (mtest *t, const char *name);
void mtest_case_beginf  (mtest *t, const char *fmt, ...);

void mtest_case_end(mtest *t);

void mtest_fail (mtest *t, const char *file, int line, const char *msg);
void mtest_failf(mtest *t, const char *file, int line, const char *fmt, ...);
void mtest_msgf (mtest *t, const char *fmt, ...);


#endif//MTEST_H
