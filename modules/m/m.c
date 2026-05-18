#include "m.h"
#include <stdlib.h>


// ================================================
//               TYPE SIZE CHECKS
//

M_STATIC_ASSERT(sizeof(m_u8)  == 1, m_u8_size);
M_STATIC_ASSERT(sizeof(m_u16) == 2, m_u16_size);
M_STATIC_ASSERT(sizeof(m_u32) == 4, m_u32_size);
M_STATIC_ASSERT(sizeof(m_u64) == 8, m_u64_size);


// ================================================
//                HEAP ALLOCATOR
//

static void *heap_alloc(m_usize size, void *ctx)
{
    (void)ctx;
    return malloc(size);
}

static void heap_free(void *ptr, m_usize size, void* ctx)
{
    (void)ctx;
    (void)size;
    free(ptr);
}

static void *heap_realloc(void *ptr, m_usize old_size, m_usize new_size, void* ctx)
{
    (void)ctx;
    (void)old_size;
    return realloc(ptr, new_size);
}

static const m_allocator m__heap = {
    .alloc   = heap_alloc,
    .free    = heap_free,
    .realloc = heap_realloc,
    .ctx     = NULL,
};

const m_allocator *m_heap_allocator(void)
{
    return &m__heap;
}