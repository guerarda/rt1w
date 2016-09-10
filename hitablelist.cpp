#include "hitablelist.hpp"
#include <assert.h>

hitable_list::hitable_list(size_t count, hitable **list)
{
    assert(list);
    m_count = count;
    m_list = list;
}

bool hitable_list::hit(const ray &r, float min, float max, hit_record &rec) const
{
    hit_record tmp;
    bool hit = false;
    float t = max;

    for (size_t i = 0; i < m_count; i++) {
        if (m_list[i]->hit(r, min, t, tmp)) {
            hit = true;
            t = tmp.t;
            rec = tmp;
        }
    }
    return hit;
}
