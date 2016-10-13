#include "hitablelist.hpp"
#include <assert.h>
#include <vector>
#include <math.h>

struct _hitable_list : hitable_list {

    _hitable_list(size_t count, sptr<hitable> *l);
    virtual ~_hitable_list() { }

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

const box box_zero = {
    { 0.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 0.0f }
};

static bool box_eq(const box &a, const box &b)
{
    return a.m_lo.x == b.m_lo.x
        && a.m_lo.y == b.m_lo.y
        && a.m_lo.z == b.m_lo.z
        && a.m_hi.x == b.m_hi.x
        && a.m_hi.y == b.m_hi.y
        && a.m_hi.z == b.m_hi.z;
}

static box box_merge(const box &a, const box &b)
{
    v3f lo = {
        fminf(a.m_lo.x, b.m_lo.x),
        fminf(a.m_lo.y, b.m_lo.y),
        fminf(a.m_lo.z, b.m_lo.z)
    };
    v3f hi = {
        fmaxf(a.m_hi.x, b.m_hi.x),
        fmaxf(a.m_hi.y, b.m_hi.y),
        fmaxf(a.m_hi.z, b.m_hi.z)
    };
    return { lo, hi };
}

box _hitable_list::bounding_box() const
{
    box rbox = box_zero;
    if (m_hitables.size() > 0) {
        rbox = m_hitables[0]->bounding_box();
        if (!box_eq(rbox, box_zero)) {
            for (size_t i = 1; i < m_hitables.size(); i++) {
                box b = m_hitables[i]->bounding_box();
                if (!box_eq(b, box_zero)) {
                    rbox = box_merge(rbox, b);
                } else {
                    return box_zero;
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
