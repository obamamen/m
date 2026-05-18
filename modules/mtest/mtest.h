#ifndef MTEST_H
#define MTEST_H

// usage:
//
// #include <mtest/mtest_terminal.h>  // for default terminal output
//
//   mtest_terminal_ctx ctx;
//   mtest t = mtest_new(mtest_terminal_new(&ctx, MTEST_TERMINAL_DEFAULT));
//
//   MTEST_GROUP(&t, "my module") // can have groups inside groups
//   {
//       MTEST_CASE(&t, "my case")
//       {
//           MTEST_ASSERT(&t, 1 == 1, "math works");
//           MTEST_MSG(&t, "some info: %d", 42);
//       }
//   }
//
//   return mtest_terminal_summary(&t); // does need mtest_terminal.h

#include <m/m.h>

// ----------------------------------------------------------------
// (mtest*, const char*)
//
// starts a new group.
//
// note:
//  do not put multiple on same line.
// ----------------------------------------------------------------
#define MTEST_GROUP(t, name) \
    M_BLOCK(M_CONCAT(mtest_g_, __LINE__), mtest_group_begin(t, name), mtest_group_end(t))

// ----------------------------------------------------------------
// (mtest*, const char*)
//
// starts a new case.
//
// note:
//  do not put multiple on same line.
// ----------------------------------------------------------------
#define MTEST_CASE(t, name) \
    M_BLOCK(M_CONCAT(mtest_c_, __LINE__), mtest_case_begin(t, name), mtest_case_end(t))

// ----------------------------------------------------------------
// (mtest*, {}, const char*)
// ----------------------------------------------------------------
#define MTEST_ASSERT(t, cond, msg) \
    if (!(cond)) { mtest_event_(t, MTEST_EVENT_FAIL, __FILE__, __LINE__, msg); }

// ----------------------------------------------------------------
// (mtest*, {}, const char*)
//
// will also break out of current test case.
// ----------------------------------------------------------------
#define MTEST_ASSERT_FATAL(t, cond, msg)                                        \
    if (!(cond)) {  mtest_event_(t, MTEST_EVENT_FAIL, __FILE__, __LINE__, msg);  \
                    mtest_case_end(t); break; }

#define MTEST_FAIL(t, ...)  mtest_eventf_(t, MTEST_EVENT_FAIL, __FILE__, __LINE__, __VA_ARGS__)
#define MTEST_MSG(t, ...)   mtest_eventf_(t, MTEST_EVENT_MSG,  __FILE__, __LINE__, __VA_ARGS__)



typedef struct mtest mtest;


// ================================================
//                    EVENTS
//

#define MTEST_MSG_MAX 128

enum mtest_event_type
{
    MTEST_EVENT_FAIL,
    MTEST_EVENT_MSG,
};
typedef enum mtest_event_type mtest_event_type;

struct mtest_event
{
    mtest_event_type type;

    char msg[MTEST_MSG_MAX];

    const char* file;
    int line;
};
typedef struct mtest_event mtest_event;


// ================================================
//                 CASE RESULT
//

enum mtest_case_result
{
    MTEST_CASE_PASS,
    MTEST_CASE_FAIL,
};
typedef enum mtest_case_result mtest_case_result;


// ================================================
//                   OBSERVER
//

struct mtest_observer
{
    void (*on_group_start)(mtest* t, const char *name, void* ctx);
    void (*on_group_end)  (mtest* t, const char *name, int passed, int failed, void* ctx);
    void (*on_case_start) (mtest* t, const char *name, void* ctx);
    void (*on_case_end)   (mtest* t, const char *name, mtest_case_result result, void* ctx);
    void (*on_event)      (mtest* t, const mtest_event* e, void* ctx);
    void *ctx;
};
typedef struct mtest_observer mtest_observer;


// ================================================
//                     STATE
//

#define MTEST_MAX_CASE_NAME  128
#define MTEST_MAX_GROUP_NAME 128

#define MTEST_MAX_GROUPS     16

struct mtest
{
    mtest_observer observer;

    char _groups      [MTEST_MAX_GROUPS][MTEST_MAX_GROUP_NAME];
    int  _group_passed[MTEST_MAX_GROUPS];
    int  _group_failed[MTEST_MAX_GROUPS];
    int  _group_depth;

    char              _case_name[MTEST_MAX_CASE_NAME];
    mtest_case_result _case_result;
    int               _case_active;

    int total_passed;
    int total_failed;
};


// ================================================
//                      PUBLIC
//

// ----------------------------------------------------------------
// create a new mtest, for running tests.
//
// observer: implements mtest_observer,
//  use mtest_terminal.h if you want a default terminal implementation.
// ----------------------------------------------------------------
mtest mtest_new(mtest_observer observer);

void mtest_group_begin (mtest *t, const char *name);
void mtest_group_beginf(mtest *t, const char *fmt, ...);
void mtest_group_end   (mtest *t);

void mtest_case_begin (mtest *t, const char *name);
void mtest_case_beginf(mtest *t, const char *fmt, ...);
void mtest_case_end   (mtest *t);

static inline int         mtest_depth     (const mtest *t) { return t->_group_depth;  }
static inline const char* mtest_group_name(const mtest *t, int i) { return t->_groups[i]; }
static inline const char* mtest_case_name (const mtest *t) { return t->_case_name;    }


// internal
void mtest_event_ (mtest* t, mtest_event_type type, const char* file, int line, const char* msg);
void mtest_eventf_(mtest* t, mtest_event_type type, const char* file, int line, const char* fmt, ...);


// ================================================
//                    BUFFERED LAYER
//

struct mtest_buffered_handler
{
    void (*on_group_start)(mtest *t, const char *name, void *ctx);
    void (*on_group_end)  (mtest *t, const char *name, int passed, int failed, void *ctx);
    void (*on_case)       (mtest *t, const char *name, mtest_case_result result, const mtest_event *events, int event_count, void *ctx);
    void *ctx;
};
typedef struct mtest_buffered_handler mtest_buffered_handler ;

#define MTEST_MAX_BUFFERED_EVENTS 64

struct mtest_buffered_ctx
{
    mtest_buffered_handler handler;

    mtest_event events[MTEST_MAX_BUFFERED_EVENTS];
    int         event_count;
};
typedef struct mtest_buffered_ctx mtest_buffered_ctx;

mtest_observer mtest_buffered_new(mtest_buffered_ctx *ctx, mtest_buffered_handler handler);

#endif//MTEST_H
