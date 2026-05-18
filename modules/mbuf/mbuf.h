#ifndef M_MBUF_H
#define M_MBUF_H

#include <m/m.h>

enum mbuf_result
{
    MBUF_OK = M_OK,
    MBUF_OOM = M_OOM,
};
typedef enum mbuf_result mbuf_result;

struct mbuf
{
    void*   data;
    m_isize size;
    m_isize cap;
};
typedef struct mbuf mbuf;

struct mview
{
    const void*     data;
    const m_isize   size;
};
typedef struct mview mview;

struct mspan
{
    void*           data;
    const m_isize   size;
};
typedef struct mspan mspan;

#endif //M_MBUF_H