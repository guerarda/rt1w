#include "hitablelist.hpp"
#include <assert.h>
#include <vector>
#include <math.h>

struct _hitable_list : hitable_list {

    _hitable_list(size_t count, sptr<hitable> *l);

    bool hit(const sptr<ray> &r, float min, float max, hit_record &rec) const;
    box bounding_box() const;

    std::vector<sptr<hitable>> m_hitables;
};

_hitable_list::_hitable_list(size_t count, sptr<hitable> *list)
{
    assert(list);
    m_hitables.assign(list, list + count);
}

bool _hitable_list::hit(const sptr<ray> &r, float min, float max, hit_record &rec) const
{
    hit_record tmp;
    bool hit = false;
    float t = max;

    for (sptr<hitable> h : m_hitables) {
        if (h->hit(r, min, t, tmp)) {
            hit = true;
            t = tmp.t;
            rec = tmp;
        }
    }
    return hit;
}

static bool box_eq(const box &a, const box &b)
{
    return a.lo.x == b.lo.x
        && a.lo.y == b.lo.y
        && a.lo.z == b.lo.z
        && a.hi.x == b.hi.x
        && a.hi.y == b.hi.y
        && a.hi.z == b.hi.z;
}

static box box_merge(const box &a, const box &b)
{
    v3f lo = {
        fminf(a.lo.x, b.lo.x),
        fminf(a.lo.y, b.lo.y),
        fminf(a.lo.z, b.lo.z)
    };
    v3f hi = {
        fmaxf(a.hi.x, b.hi.x),
        fmaxf(a.hi.y, b.hi.y),
        fmaxf(a.hi.z, b.hi.z)
    };
    return { lo, hi };
}

box _hitable_list::bounding_box() const
{
    box rbox = __zero_box;
    if (m_hitables.size() > 0) {
        rbox = m_hitables[0]->bounding_box();
        if (!box_eq(rbox, __zero_box)) {
            for (size_t i = 1; i < m_hitables.size(); i++) {
                box b = m_hitables[i]->bounding_box();
                if (!box_eq(b, __zero_box)) {
                    rbox = box_merge(rbox, b);
                } else {
                    return __zero_box;
                }
            }
        }
    }
    return rbox;
}

#pragma mark - Static constructors

sptr<hitable_list> hitable_list::create(size_t count, sptr<hitable> *l)
{
    return std::make_shared<_hitable_list>(count, l);
}
