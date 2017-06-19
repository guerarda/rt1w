#include "primitive.hpp"
#include <assert.h>
#include <vector>

#pragma mark - Primitive

struct _Primitive : Primitive {

    _Primitive(const sptr<Shape> &s, const sptr<Material> &m) : m_shape(s),
                                                                m_material(m) { }

    bool     hit(const sptr<ray> &, float, float, hit_record &) const;
    bounds3f bounds() const;

    sptr<Shape>    m_shape;
    sptr<Material> m_material;
};

bool _Primitive::hit(const sptr<ray> &r, float min, float max, hit_record &rec) const
{
    if (m_shape->hit(r, min, max, rec)) {
        rec.mat = m_material;
        return true;
    }
    return false;
}

bounds3f _Primitive::bounds() const
{
    return m_shape->bounds();
}

#pragma mark - Primitive List

struct _Primitive_list : Primitive {

    _Primitive_list(const std::vector<sptr<Primitive>> &v) : m_primitives(v) { }

    bool hit(const sptr<ray> &r, float min, float max, hit_record &rec) const;
    bounds3f bounds() const;

    std::vector<sptr<Primitive>> m_primitives;
};

bool _Primitive_list::hit(const sptr<ray> &r, float min, float max, hit_record &rec) const
{
    hit_record tmp;
    bool hit = false;
    float t = max;

    for (sptr<Primitive> h : m_primitives) {
        if (h->hit(r, min, t, tmp)) {
            hit = true;
            t = tmp.t;
            rec = tmp;
        }
    }
    return hit;
}

bounds3f _Primitive_list::bounds() const
{
    bounds3f rbox = bounds3f();
    for (size_t i = 0; i < m_primitives.size(); i++) {
        rbox = Union(rbox, m_primitives[i]->bounds());
    }
    return rbox;
}

#pragma mark - Static constructors

sptr<Primitive> Primitive::create(const sptr<Shape> &s, const sptr<Material> &m)
{
    return std::make_shared<_Primitive>(s, m);
}

sptr<Primitive> Primitive::create(const std::vector<sptr<Primitive>> &v)
{
    return std::make_shared<_Primitive_list>(v);
}
