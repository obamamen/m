#ifndef M_MBUF_H
#define M_MBUF_H

#include <m/m.h>


// ================================================
//                  RESULT
//

enum mbuf_result
{
    MBUF_OK = M_OK,
    MBUF_OOM = M_OOM,
    MBUF_ERR_GROW = 2 // cant fulfil grow_proc's contract.
};
typedef enum mbuf_result mbuf_result;


// ================================================
//                  CORE TYPES
//

struct mbuf
{
    void*   data;
    m_usize cap;
    m_usize size;
};
typedef struct mbuf mbuf;

struct mview
{
    const void* data;
    m_usize     size;
};
typedef struct mview mview;

struct mspan
{
    void*           data;
    const m_usize   size;
};
typedef struct mspan mspan;

// ----------------------------------------------------------------
// will return the size the caller should grow to.
//
// cap:         current allocated bytes.
// requested:   MINIMAL bytes needed. guaranteed to be > cap.
//
// pre:
//  cap < requested
//
// post:
//  must return >= requested.
//  EXCEPTION:
//      when the growth proc cant fulfill its own contract, it can return 0,
//      to indicate an error.
//      (e.g. if the next pow2 is bigger than m_usize allows)
// ----------------------------------------------------------------
typedef m_usize (*mbuf_grow_proc)(m_usize cap, m_usize requested);


// ================================================
//                  CORE
//

mbuf mbuf_new(void);

mbuf_result mbuf_setcap(mbuf *buf, const m_allocator* a, m_usize new_cap);

void mbuf_write(mbuf *buf, m_usize offset, const void *data, m_usize len);
void mbuf_open(mbuf *buf, m_usize offset, m_usize len);
void mbuf_close(mbuf *buf, m_usize offset, m_usize len);

void mbuf_free(mbuf *buf, const m_allocator* a);


// ================================================
//                  UTILITY
//

mbuf_result mbuf_reserve(mbuf* buf, const m_allocator* a, m_usize min_cap);
mbuf_result mbuf_shrink(mbuf* buf, const m_allocator* a, m_usize min_cap);
mbuf_result mbuf_insert(mbuf *buf, const m_allocator* a, m_usize offset, mview src, mbuf_grow_proc grow);
void        mbuf_remove(mbuf *buf, m_usize offset, m_usize len);
void        mbuf_clear(mbuf *buf);

static M_INLINE mview mbuf_as_view(const mbuf* buf) { return (mview){.data = buf->data, .size = buf->size}; }

static M_INLINE mspan mbuf_as_span(mbuf* buf) { return (mspan){.data = buf->data, .size = buf->size}; }

static M_INLINE mview mbuf_slice(const mbuf* buf, m_usize offset, m_usize size)
{
    if (offset > buf->size) offset = buf->size;

    m_usize max_len = buf->size - offset;
    if (size > max_len) size = max_len;

    return (mview){
        .data = (const char*)buf->data + offset,
        .size = size
    };
}

static M_INLINE mview mview_make(const void* data, m_usize size) { return (mview){ .data = data, .size = size }; }

static M_INLINE mspan mspan_make(void* data, m_usize size) { return (mspan){ .data = data, .size = size }; }


// ================================================
//                  GROW PROCS
//

m_usize mbuf_grow_x2(m_usize cap, m_usize requested);

m_usize mbuf_grow_x1_5(m_usize cap, m_usize requested);

// ----------------------------------------------------------------
// calculates the next power of two capacity for the buffer.
//
// returns:
//  the next power of two size that is >= requested.
//
// follows the mbuf_grow_proc contract; look there for more info.
// ----------------------------------------------------------------
m_usize mbuf_grow_pow2(m_usize cap, m_usize requested);

m_usize mbuf_grow_exact(m_usize cap, m_usize requested);

#endif //M_MBUF_H