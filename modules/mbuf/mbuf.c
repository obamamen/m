#include "mbuf.h"

#include <assert.h>
#include <string.h>


// ================================================
//                  CORE
//

mbuf mbuf_new(void)
{
    const mbuf buf = {
        .data = NULL,
        .size = 0,
        .cap = 0
    };

    return buf;
}

mbuf_result mbuf_setcap(mbuf* buf, const m_allocator* a, m_usize new_cap)
{
    if (new_cap == buf->cap)
        return MBUF_OK;

    if (new_cap == 0)
    {
        M_FREE(a, buf->data, buf->cap);
        buf->data = NULL;
        buf->cap  = 0;

        if (buf->size > 0)
            buf->size = 0;

        return MBUF_OK;
    }

    void* ptr = M_REALLOC(a, buf->data, buf->cap, new_cap);
    if (M_UNLIKELY(ptr == NULL))
        return MBUF_OOM;

    buf->data = ptr;
    buf->cap  = new_cap;

    if (buf->size > new_cap)
        buf->size = new_cap;

    return MBUF_OK;
}

void mbuf_write(mbuf* buf, m_usize offset, const void* data, m_usize len)
{
    memcpy((m_u8*)buf->data + offset, data, len);
}

void mbuf_open(mbuf* buf, m_usize offset, m_usize len)
{
    assert(offset <= buf->size);
    assert(buf->size + len <= buf->cap);

    m_usize tail = buf->size - offset;

    if (tail > 0)
        memmove((m_u8 *)buf->data + offset + len,
                (m_u8 *)buf->data + offset,
                tail);

    buf->size += len;
}

void mbuf_close(mbuf *buf, m_usize offset, m_usize len)
{
    assert(offset + len <= buf->size);

    m_usize tail = buf->size - offset - len;

    if (tail > 0)
        memmove((m_u8 *)buf->data + offset,
                (m_u8 *)buf->data + offset + len,
                tail);
    buf->size -= len;
}

void mbuf_free(mbuf *buf, const m_allocator* a)
{
    mbuf_setcap(buf, a, 0);
}


// ================================================
//                  UTILITY
//

mbuf_result mbuf_reserve(mbuf* buf, const m_allocator* a, m_usize min_cap)
{
    if (min_cap <= buf->cap)
        return MBUF_OK;

    return mbuf_setcap(buf, a, min_cap);
}

mbuf_result mbuf_shrink(mbuf *buf, const m_allocator* a, m_usize min_cap)
{
    if (buf->size == buf->cap)
        return MBUF_OK;

    m_usize new_cap = buf->size;

    if (new_cap < min_cap)
        new_cap = min_cap;

    return mbuf_setcap(buf, a, new_cap);
}

mbuf_result mbuf_insert(mbuf* buf, const m_allocator* a, m_usize offset, mview src, mbuf_grow_proc grow)
{
    assert(offset <= buf->size);

    m_usize needed = buf->size + src.size;

    if (needed > buf->cap)
    {
        m_usize new_cap = grow ? grow(buf->cap, needed) : needed;

        if (M_UNLIKELY(new_cap < needed || new_cap == 0))
            return MBUF_ERR_GROW;

        mbuf_result res = mbuf_setcap(buf, a, new_cap);
        if (M_UNLIKELY(res != MBUF_OK))
            return res;
    }

    mbuf_open(buf, offset, src.size);
    mbuf_write(buf, offset, src.data, src.size);
    return MBUF_OK;
}

void mbuf_remove(mbuf* buf, m_usize offset, m_usize len)
{
    mbuf_close(buf, offset, len);
}

void mbuf_clear(mbuf* buf)
{
    buf->size = 0;
}


m_usize mbuf_grow_x2(m_usize cap, m_usize requested)
{
    assert(cap <= requested);

    m_usize new_cap = (cap == 0) ? 8 : cap * 2;

    if (M_UNLIKELY(new_cap < requested))
        new_cap = requested;


    return new_cap;
}

m_usize mbuf_grow_x1_5(m_usize cap, m_usize requested)
{
    assert(cap <= requested);

    m_usize new_cap = (cap == 0) ? 8 : cap + (cap / 2);

    if (M_UNLIKELY(new_cap < requested))
        new_cap = requested;


    return new_cap;
}

m_usize mbuf_grow_pow2(m_usize cap, m_usize requested)
{
    assert(cap <= requested);

    M_UNUSED(cap);

    if (requested < 4)
        return 4;

    m_usize new_cap = requested;

#if M_64_BIT
    new_cap = M_NEXT_POW2_64(new_cap);
#elif M_32_BIT
    new_cap = M_NEXT_POW2_32(new_cap);
#else
#   error "Can't oblige grow contract"
#endif

    if (M_UNLIKELY(new_cap < requested))
        return 0;

    return new_cap;
}

m_usize mbuf_grow_exact(m_usize cap, m_usize requested)
{
    assert(cap <= requested);

    M_UNUSED(cap);
    return requested;
}