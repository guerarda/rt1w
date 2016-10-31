#include "hitablelist.hpp"
#include <assert.h>
#include <vector>
#include <math.h>
#include <cfloat>

struct _Hitable_list : Hitable_list {

    _Hitable_list(size_t count, sptr<Hitable> *l);

    bool hit(const sptr<ray> &r, float min, float max, hit_record &rec) const;
    box_t bounding_box() const;

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

static int32_t f32_cmp(float x, float y)
{
    if (fabs(x - y) <= fmaxf(fabsf(x), fabsf(y)) * FLT_EPSILON) {
        return 0;
    } else {
        return x < y ? -1 : + 1;
    }
}

static bool box_eq(const box_t &a, const box_t &b)
{
    return f32_cmp(a.lo.x, b.lo.x) == 0
        && f32_cmp(a.lo.y, b.lo.y) == 0
        && f32_cmp(a.lo.z, b.lo.z) == 0
        && f32_cmp(a.hi.x, b.hi.x) == 0
        && f32_cmp(a.hi.y, b.hi.y) == 0
        && f32_cmp(a.hi.z, b.hi.z) == 0;
}

static box_t box_merge(const box_t &a, const box_t &b)
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

box_t _Hitable_list::bounding_box() const
{
    box_t rbox = __zero_box;
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

sptr<Hitable_list> Hitable_list::create(size_t count, sptr<Hitable> *l)
{
    return std::make_shared<_Hitable_list>(count, l);
}
