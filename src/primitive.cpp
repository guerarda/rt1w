#include "primitive.hpp"
#include "shape.hpp"
#include "error.h"

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

#pragma mark - Static constructors

sptr<Primitive> Primitive::create(const sptr<Shape> &s, const sptr<Material> &m)
{
    if (s && m) {
        return std::make_shared<_Primitive>(s, m);
    }
    WARNING_IF(!s, "Primitive has no shape");
    WARNING_IF(!m, "Primitive has no material");

    return nullptr;
}
