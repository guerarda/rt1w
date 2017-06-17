#include "hitablelist.hpp"
#include <assert.h>
#include <vector>
#include <math.h>
#include <cfloat>

struct _Hitable_list : Hitable_list {

    _Hitable_list(size_t count, sptr<Hitable> *l);

    bool hit(const sptr<ray> &r, float min, float max, hit_record &rec) const;
    bounds3f bounds() const;

    std::vector<sptr<Hitable>> m_hitables;
};

_Hitable_list::_Hitable_list(size_t count, sptr<Hitable> *list)
{
    assert(list);
    m_hitables.assign(list, list + count);
}

bool _Hitable_list::hit(const sptr<ray> &r, float min, float max, hit_record &rec) const
{
    hit_record tmp;
    bool hit = false;
    float t = max;

    for (sptr<Hitable> h : m_hitables) {
        if (h->hit(r, min, t, tmp)) {
            hit = true;
            t = tmp.t;
            rec = tmp;
        }
    }
    return hit;
}

bounds3f _Hitable_list::bounds() const
{
    bounds3f rbox = bounds3f();
    for (size_t i = 0; i < m_hitables.size(); i++) {
        rbox = Union(rbox, m_hitables[i]->bounds());
    }
    return rbox;
}

#pragma mark - Static constructors

sptr<Hitable_list> Hitable_list::create(size_t count, sptr<Hitable> *l)
{
    return std::make_shared<_Hitable_list>(count, l);
}
