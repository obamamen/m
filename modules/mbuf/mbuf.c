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