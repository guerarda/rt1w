#include "sphere.hpp"
#include <math.h>
#include "material.hpp"

struct _sphere : sphere {

    _sphere(const v3f &c, float r, const sptr<material> &m);
    virtual ~_sphere() { }

    bool  hit(const sptr<ray> &r, float min, float max, hit_record &rec) const;

    v3f   center() const { return m_center; }
    float radius() const { return m_radius; }

    v3f   m_center;
    float m_radius;
    sptr<material> m_material;
};

_sphere::_sphere(const v3f &c, float r, const sptr<material> &m)
{
    m_center = c;
    m_radius = r;
    m_material = m;
}

bool _sphere::hit(const sptr<ray> &r, float min, float max, hit_record &rec) const
{
    v3f oc = v3f_sub(r->origin(), m_center);
    float a = v3f_dot(r->direction(), r->direction());
    float b = 2 * v3f_dot(r->direction(), oc);
    float c = v3f_dot(oc, oc) - m_radius * m_radius;
    float delta = b * b - 4 * a * c;

    if (delta > 0.0f) {
        float t = (-b - sqrtf(delta)) / (2.0 * a);
        if (!(t > min && t < max)) {
            t = (-b + sqrtf(delta)) / (2.0 * a);
        }
        if (t > min && t < max) {
            rec.t = t;
            rec.p = r->point_at_param(t);
            rec.normal = v3f_normalize(v3f_sub(rec.p, m_center));
            rec.mat = m_material;
            return true;
        }
    }
    return false;
}

#pragma mark - Static constructors

sptr<sphere> sphere::create(const v3f &c, float r, const sptr<material> &m)
{
    return std::make_shared<_sphere>(c, r, m);
}
