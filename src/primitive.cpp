#include "primitive.hpp"

#include "error.h"
#include "interaction.hpp"
#include "ray.hpp"
#include "shape.hpp"

#pragma mark - Primitive

struct _Primitive : Primitive, std::enable_shared_from_this<Primitive> {
    _Primitive(const sptr<Shape> &s, const sptr<Material> &m, const sptr<AreaLight> &l) :
        m_shape(s),
        m_material(m),
        m_light(l)
    {}

    bounds3f bounds() const override;
    sptr<AreaLight> light() const override { return m_light; }

    bool intersect(const Ray &r, Interaction &isect) const override;
    bool qIntersect(const Ray &r) const override;

    sptr<Shape> m_shape;
    sptr<Material> m_material;
    sptr<AreaLight> m_light;
};

bounds3f _Primitive::bounds() const
{
    return m_shape->bounds();
}

bool _Primitive::intersect(const Ray &r, Interaction &isect) const
{
    if (m_shape->intersect(r, isect)) {
        isect.mat = m_material;
        /* This is ok because Primitive only has const methods */
        isect.prim = std::const_pointer_cast<Primitive>(shared_from_this());
        return true;
    }
    return false;
}

bool _Primitive::qIntersect(const Ray &r) const
{
    return m_shape->qIntersect(r);
}

#pragma mark - Static constructor;

sptr<Primitive> Primitive::create(const sptr<Shape> &s,
                                  const sptr<Material> &m,
                                  const sptr<AreaLight> &l)
{
    if (s && (m || l)) {
        return std::make_shared<_Primitive>(s, m, l);
    }
    WARNING_IF(!s, "Primitive has no shape");
    WARNING_IF(!m, "Primitive has no material");

    return nullptr;
}

#pragma mark - Aggregate

struct _Aggregate : Aggregate {
    _Aggregate(const std::vector<sptr<Primitive>> &prims);

    bool intersect(const Ray &r, Interaction &isect) const override;
    bool qIntersect(const Ray &r) const override;
    bounds3f bounds() const override { return m_bounds; }
    sptr<AreaLight> light() const override;

    const std::vector<sptr<Primitive>> &primitives() const override
    {
        return m_primitives;
    }

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

sptr<AreaLight> _Aggregate::light() const
{
    trap("Aggregate::light() should never be called");
}

bool _Aggregate::intersect(const Ray &r, Interaction &isect) const
{
    bool hit = false;
    float t = r.max();

    for (auto &p : m_primitives) {
        if (p->intersect({ r, t }, isect)) {
            t = isect.t;
            hit = true;
        }
    }
    return hit;
}

bool _Aggregate::qIntersect(const Ray &r) const
{
    for (auto &p : m_primitives) {
        if (p->qIntersect(r)) {
            return true;
        }
    }
    return false;
}

#pragma mark - Static Constructor

sptr<Aggregate> Aggregate::create(const std::vector<sptr<Primitive>> &primitives)
{
    return std::make_shared<_Aggregate>(primitives);
}
