#include "primitive.hpp"

#include "error.h"
#include "shape.hpp"

#pragma mark - Primitive

struct _Primitive : Primitive {

    _Primitive(const sptr<Shape> &s, const sptr<Material> &m) : m_shape(s),
                                                                m_material(m) { }

    bool     hit(const sptr<Ray> &, float, float, hit_record &) const override;
    bounds3f bounds() const override;

    sptr<Shape>    m_shape;
    sptr<Material> m_material;
};

bool _Primitive::hit(const sptr<Ray> &r, float min, float max, hit_record &rec) const
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

#pragma mark - Static constructor;

sptr<Primitive> Primitive::create(const sptr<Shape> &s, const sptr<Material> &m)
{
    if (s && m) {
        return std::make_shared<_Primitive>(s, m);
    }
    WARNING_IF(!s, "Primitive has no shape");
    WARNING_IF(!m, "Primitive has no material");

    return nullptr;
}

#pragma mark - Aggregate

struct _Aggregate : Aggregate {

    _Aggregate(const std::vector<sptr<Primitive>> &prims);

    bool     hit(const sptr<Ray> &, float, float, hit_record &) const override;
    bounds3f bounds() const override { return m_bounds; }

    const std::vector<sptr<Primitive>> &primitives() const override { return m_primitives; }

    std::vector<sptr<Primitive>> m_primitives;
    bounds3f m_bounds;
};

_Aggregate::_Aggregate(const std::vector<sptr<Primitive>> &prims)
{
    m_primitives = prims;
    for (auto &p : m_primitives) {
        m_bounds = Union(m_bounds, p->bounds());
    }
}

bool _Aggregate::hit(const sptr<Ray> &r, float min, float max, hit_record &rec) const
{
    bool hit = false;
    rec.t = std::numeric_limits<float>::max();

    for (auto &p : m_primitives) {
        hit_record hr;
        if (p->hit(r, min, max, hr)) {
            if (hr.t < rec.t) {
                rec = hr;
            }
            hit = true;
        }
    }
    return hit;
}

#pragma mark - Static Constructor

sptr<Aggregate> Aggregate::create(const std::vector<sptr<Primitive>> primitives)
{
    return std::make_shared<_Aggregate>(primitives);
}
