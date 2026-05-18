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
    const void*     data;
    const m_usize   size;
};
typedef struct mview mview;

struct mspan
{
    void*           data;
    const m_usize   size;
};
typedef struct mspan mspan;

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
mbuf_result mbuf_insert(mbuf *buf, const m_allocator* a, m_usize offset, const void* data, m_usize len, mbuf_grow_proc grow);
void        mbuf_remove(mbuf *buf, m_usize offset, m_usize len);
void        mbuf_clear(mbuf *buf);

mview mbuf_as_view(const mbuf *buf);
mspan mbuf_as_span(mbuf *buf);
mview mbuf_slice(const mbuf *buf, m_usize offset, m_usize len);


// ================================================
//                  GROW PROCS
//

m_usize mbuf_grow_x2(m_usize cap, m_usize requested);
m_usize mbuf_grow_x1_5(m_usize cap, m_usize requested);
m_usize mbuf_grow_pow2(m_usize cap, m_usize requested);
m_usize mbuf_grow_exact(m_usize cap, m_usize requested);

#endif //M_MBUF_H