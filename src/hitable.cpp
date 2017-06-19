#include "hitable.hpp"
#include <assert.h>
#include <vector>

struct _Hitable_list : Hitable {

    _Hitable_list(const std::vector<sptr<Hitable>> &v) : m_hitables(v) { }

    bool hit(const sptr<ray> &r, float min, float max, hit_record &rec) const;
    bounds3f bounds() const;

    std::vector<sptr<Hitable>> m_hitables;
};

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

sptr<Hitable> Hitable::create(const std::vector<sptr<Hitable>> &v)
{
    return std::make_shared<_Hitable_list>(v);
}
