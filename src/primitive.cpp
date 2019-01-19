#include "primitive.hpp"

#include "error.h"
#include "interaction.hpp"
#include "shape.hpp"

#pragma mark - Primitive

struct _Primitive : Primitive, std::enable_shared_from_this<Primitive> {
    _Primitive(const sptr<Shape> &s, const sptr<Material> &m, const sptr<AreaLight> &l) :
        m_shape(s),
        m_material(m),
        m_light(l)
    {}

    bool intersect(const Ray &r, Interaction &isect, float max) const override;
    bounds3f bounds() const override;
    sptr<AreaLight> light() const override { return m_light; }

    sptr<Shape> m_shape;
    sptr<Material> m_material;
    sptr<AreaLight> m_light;
};

bool _Primitive::intersect(const Ray &r, Interaction &isect, float max) const
{
    if (m_shape->intersect(r, isect, max)) {
        ASSERT(FloatCompare(isect.uv.x, 0.0f) >= 0.0f);
        ASSERT(FloatCompare(isect.uv.x, 1.0f) <= 0.0f);
        ASSERT(FloatCompare(isect.uv.y, 0.0f) >= 0.0f);
        ASSERT(FloatCompare(isect.uv.y, 1.0f) <= 0.0f);

        isect.mat = m_material;
        /* This is ok because Primitive has only const methods */
        isect.prim = std::const_pointer_cast<Primitive>(shared_from_this());
        return true;
    }
    return false;
}

bounds3f _Primitive::bounds() const
{
    return m_shape->bounds();
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

    bool intersect(const Ray &r, Interaction &isect, float max) const override;
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
    return nullptr;
}

bool _Aggregate::intersect(const Ray &r, Interaction &isect, float max) const
{
    bool hit = false;
    isect.t = std::numeric_limits<float>::max();

    for (auto &p : m_primitives) {
        Interaction hr;
        if (p->intersect(r, hr, max)) {
            if (hr.t < isect.t) {
                isect = hr;
            }
            hit = true;
        }
    }
    return hit;
}

#pragma mark - Static Constructor

sptr<Aggregate> Aggregate::create(const std::vector<sptr<Primitive>> &primitives)
{
    return std::make_shared<_Aggregate>(primitives);
}
