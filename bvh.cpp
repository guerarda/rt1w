#include "bvh.hpp"
#include <vector>
#include <random>
#include <algorithm>
#include <assert.h>

bool box_hit(const box &b, const sptr<ray> &r, float tmin, float tmax)
{
    v3f dir = r->direction();
    v3f org = r->origin();
    float i_dir, t0, t1;

    for (size_t i = 0; i < 3; i++) {
        i_dir = 1.0f / ((float *)&dir.x)[i];
        t0 = (((float *)&b.m_lo.x)[i] - ((float *)&org.x)[i]) * i_dir;
        t1 = (((float *)&b.m_hi.x)[i] - ((float *)&org.x)[i]) * i_dir;
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

struct {
    bool operator()(const sptr<hitable> &a, const sptr<hitable> &b) {
        return a->bounding_box().m_lo.x < b->bounding_box().m_lo.x;
    }
} bvh_xcmp;

struct {
    bool operator()(const sptr<hitable> &a, const sptr<hitable> &b) {
        return a->bounding_box().m_lo.y < b->bounding_box().m_lo.y;
    }
} bvh_ycmp;

struct {
    bool operator()(const sptr<hitable> &a, const sptr<hitable> &b) {
        return a->bounding_box().m_lo.z < b->bounding_box().m_lo.z;
    }
} bvh_zcmp;

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

_bvh_node::_bvh_node(size_t n, sptr<hitable> *ptr)
{
    assert(ptr);
    assert(n > 0);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 2);

    std::vector<sptr<hitable>> v;
    v.assign(ptr, ptr + n);

    int axis = dis(gen);
    if (axis == 0) {
        std::sort(v.begin(), v.end(), bvh_xcmp);
    } else if (axis == 1) {
        std::sort(v.begin(), v.end(), bvh_ycmp);
    } else {
        std::sort(v.begin(), v.end(), bvh_zcmp);
    }
    if (n == 1) {
        m_left = m_right = ptr[0];
    } else if (n == 2) {
        m_left = ptr[0];
        m_right = ptr[1];
    } else {
        m_left = bvh_node::create(n / 2, ptr);
        m_right = bvh_node::create(n - n / 2, ptr + n / 2);
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
