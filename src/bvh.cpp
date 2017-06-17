#include "bvh.hpp"
#include <vector>
#include <algorithm>
#include <functional>
#include <cfloat>
#include <math.h>
#include <assert.h>

static bool box_hit(const bounds3f &b, const sptr<ray> &r, float tmin, float tmax)
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

struct _BVH_node : BVH_node {

    _BVH_node(size_t count, sptr<Hitable> *l);
    ~_BVH_node() { }


    bool hit(const sptr<ray> &r, float min, float max, hit_record &rec) const;
    bounds3f bounds() const { return m_box; }

    bounds3f      m_box;
    sptr<Hitable> m_left;
    sptr<Hitable> m_right;
};

static bool bvh_x_cmp(const sptr<Hitable> &a, const sptr<Hitable> &b)
{
    return a->bounds().lo.x < b->bounds().lo.x;
}

static bool bvh_y_cmp(const sptr<Hitable> &a, const sptr<Hitable> &b)
{
    return a->bounds().lo.y < b->bounds().lo.y;
}

static bool bvh_z_cmp(const sptr<Hitable> &a, const sptr<Hitable> &b)
{
    return a->bounds().lo.z < b->bounds().lo.z;
}

_BVH_node::_BVH_node(size_t n, sptr<Hitable> *ptr)
{
    assert(ptr);
    assert(n > 0);

    std::vector<sptr<Hitable>> best_v;
    std::vector<float> left_area(n), right_area(n);
    bounds3f b;
    size_t idx = 0;
    float min_sha = FLT_MAX;

    /* Find the optimal split plane for an axis and then
     * compare the cost to the split plance for the previous
     * axis
     */
    if (n > 2) {
        for (size_t axis = 0; axis < 3; axis++) {
            std::vector<sptr<Hitable>> v(n);
            std::function<bool(const sptr<Hitable> &, const sptr<Hitable> &)> cmp_fn;

            v.assign(ptr, ptr + n);
            cmp_fn = axis == 0 ? bvh_x_cmp : (axis == 1 ? bvh_y_cmp : bvh_z_cmp);

            std::sort(v.begin(), v.end(), cmp_fn);

            b = bounds3f();
            for (size_t i = 0; i < n - 1; i++) {
                b = Union(b, v[i]->bounds());
                left_area[i] = b.area();
            }
            b = bounds3f();
            for (size_t i = n; i-- > 1;) {
                b = Union(b, v[i]->bounds());
                right_area[i] = b.area();
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
            m_left = BVH_node::create(idx + 1, &best_v[0]);
        }
        if (idx == n - 2) {
            m_right = best_v[n - 1];
        } else {
            m_right = BVH_node::create(n - 1 - idx, &best_v[idx + 1]);
        }
    } else if (n == 2) {
        m_left = ptr[0];
        m_right = ptr[1];
    } else if (n == 1) {
        m_left = ptr[0];
        m_right = ptr[0];
    }
    m_box = Union(m_left->bounds(), m_right->bounds());
}

bool _BVH_node::hit(const sptr<ray> &r, float min, float max, hit_record &rec) const
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

sptr<BVH_node> BVH_node::create(size_t count, sptr<Hitable> *l)
{
    return std::make_shared<_BVH_node>(count, l);
}
