#include "hitablelist.hpp"
#include <assert.h>
#include <vector>
#include <math.h>
#include <cfloat>

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

static int32_t f32_cmp(float x, float y)
{
    if (fabs(x - y) <= fmaxf(fabsf(x), fabsf(y)) * FLT_EPSILON) {
        return 0;
    } else {
        return x < y ? -1 : + 1;
    }
}

static bool box_eq(const box &a, const box &b)
{
    return f32_cmp(a.lo.x, b.lo.x) == 0
        && f32_cmp(a.lo.y, b.lo.y) == 0
        && f32_cmp(a.lo.z, b.lo.z) == 0
        && f32_cmp(a.hi.x, b.hi.x) == 0
        && f32_cmp(a.hi.y, b.hi.y) == 0
        && f32_cmp(a.hi.z, b.hi.z) == 0;
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
