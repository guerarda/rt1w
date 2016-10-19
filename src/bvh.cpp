#include "bvh.hpp"
#include <vector>
#include <algorithm>
#include <functional>
#include <cfloat>
#include <math.h>
#include <assert.h>

bool box_hit(const box &b, const sptr<ray> &r, float tmin, float tmax)
{
    v3f dir = r->direction();
    v3f org = r->origin();
    float i_dir, t0, t1;

    for (size_t i = 0; i < 3; i++) {
        i_dir = 1.0f / (&dir.x)[i];
        t0 = ((&b.lo.x)[i] - (&org.x)[i]) * i_dir;
        t1 = ((&b.hi.x)[i] - (&org.x)[i]) * i_dir;
        if (i_dir < 0.0f) {
            std::swap(t0, t1);
        }
        tmin = t0 > tmin ? t0 : tmin;
        tmax = t1 < tmax ? t1 : tmax;
        if (tmax <= tmin) {
            return false;
        }
    }
    return true;
}

struct _bvh_node : bvh_node {

    _bvh_node(size_t count, sptr<hitable> *l);
    ~_bvh_node() { }


    bool hit(const sptr<ray> &r, float min, float max, hit_record &rec) const;
    box bounding_box() const { return m_box; }

    box           m_box;
    sptr<hitable> m_left;
    sptr<hitable> m_right;
};

static bool bvh_x_cmp(const sptr<hitable> &a, const sptr<hitable> &b)
{
    return a->bounding_box().lo.x < b->bounding_box().lo.x;
}

static bool bvh_y_cmp(const sptr<hitable> &a, const sptr<hitable> &b)
{
    return a->bounding_box().lo.y < b->bounding_box().lo.y;
}

static bool bvh_z_cmp(const sptr<hitable> &a, const sptr<hitable> &b)
{
    return a->bounding_box().lo.z < b->bounding_box().lo.z;
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

static float box_area(const box &box)
{
    v3f d = v3f_sub(box.hi, box.lo);
    return 2 * (d.x * d.y + d.y * d.z + d.x * d.z);
}

static box box_merge(const box &a, const box &b)
{
    if (box_eq(a, b)) {
        return a;
    } else if (box_eq(b, __zero_box)) {
        return a;
    } else if (box_eq(a, __zero_box)) {
        return b;
    } else {
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
}

_bvh_node::_bvh_node(size_t n, sptr<hitable> *ptr)
{
    assert(ptr);
    assert(n > 0);

    std::vector<sptr<hitable>> best_v;
    std::vector<float> left_area(n), right_area(n);
    box b;
    size_t idx = 0;
    float min_sha = FLT_MAX;

    /* Find the optimal split plane for an axis and then
     * compare the cost to the split plance for the previous
     * axis
     */
    for (size_t axis = 0; axis < 3; axis++) {
        std::vector<sptr<hitable>> v(n);
        std::function<bool(const sptr<hitable> &, const sptr<hitable> &)> cmp_fn;

        v.assign(ptr, ptr + n);
        cmp_fn = axis == 0 ? bvh_x_cmp : (axis == 1 ? bvh_y_cmp : bvh_z_cmp);

        std::sort(v.begin(), v.end(), cmp_fn);

        b = __zero_box;
        for (size_t i = 0; i < n - 1; i++) {
            b = box_merge(b, v[i]->bounding_box());
            left_area[i] = box_area(b);
        }
        b = __zero_box;
        for (size_t i = n; i-- > 1;) {
            b = box_merge(b, v[i]->bounding_box());
            right_area[i] = box_area(b);
        }

        bool best = false;
        for (size_t i = 0; i < n - 1; i++) {
            float sha = i * left_area[i] + (n - i - 1) * right_area[i + 1];
            if (sha < min_sha) {
                min_sha = sha;
                idx = i;
                best = true;
            }
        }
        if (best) {
            best_v = std::move(v);
        }
    }
    if (idx == 0) {
        m_left = best_v[0];
    } else {
        m_left = bvh_node::create(idx + 1, &best_v[0]);
    }
    if (idx == n - 2) {
        m_right = best_v[n - 1];
    } else {
        m_right = bvh_node::create(n - 1 - idx, &best_v[idx + 1]);
    }
    m_box = box_merge(m_left->bounding_box(), m_right->bounding_box());
}

bool _bvh_node::hit(const sptr<ray> &r, float min, float max, hit_record &rec) const
{
    if (box_hit(m_box, r, min, max)) {
        hit_record lrec, rrec;
        bool hit_left = m_left->hit(r, min, max, lrec);
        bool hit_right = m_right->hit(r, min, max, rrec);

        if (hit_left || hit_right) {
            if (hit_left) {
                if (hit_right) {
                    rec = lrec.t < rrec.t ? lrec : rrec;
                } else {
                    rec = lrec;
                }
            } else {
                rec = rrec;
            }
            return true;
        }
    }
    return false;
}

#pragma mark - Static constructor

sptr<bvh_node> bvh_node::create(size_t count, sptr<hitable> *l)
{
    return std::make_shared<_bvh_node>(count, l);
}
