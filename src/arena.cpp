#include "arena.hpp"

#include "error.h"

constexpr size_t cache_line_size = 64;
constexpr size_t extra_alloc_size = 256 * 1024;

struct hdr {
    struct hdr *prev;
    uint8_t    *next;
    uint8_t    *limit;
};

struct _Arena : Arena {

     _Arena();
    ~_Arena();

    void *alloc(size_t n);

    struct hdr *m_hdr;
    struct {
        size_t count = 0;
        size_t allocated = 0;
        size_t used = 0;
    } m_info;
};

_Arena::_Arena()
{
    m_hdr = (struct hdr *)malloc(sizeof(*m_hdr));
    m_hdr->prev = nullptr;
    m_hdr->next = nullptr;
    m_hdr->limit = nullptr;
}

_Arena::~_Arena()
{
    struct hdr *h = m_hdr;
    while (h) {
        struct hdr *tmp = h->prev;
        free(h);
        h = tmp;
    }
}

void *_Arena::alloc(size_t n)
{
    /* Round up n so it's 16-byte aligned */
    n = ((n + 15) & (size_t)(~15));

    if (!(m_hdr->next && m_hdr->limit) || size_t(m_hdr->limit - m_hdr->next) < n) {
        size_t s = n + extra_alloc_size;
        struct hdr *p = (struct hdr *)malloc(sizeof(*p) + s);

        /* Keep track of allocations */
        m_info.count += 1;
        m_info.allocated += s;

        /* Align usable memory after the header */
        void *aligned = (void *)(p + 1);
        std::align(cache_line_size, s, aligned, s);

        *p = *m_hdr;
        m_hdr->prev = p;
        m_hdr->next = (uint8_t *)aligned;
        m_hdr->limit = m_hdr->next + s;
    }
    void *p = m_hdr->next;
    m_hdr->next += n;
    m_info.used += n;

    return p;
}

uptr<Arena> Arena::create()
{
    return std::make_unique<_Arena>();
}
