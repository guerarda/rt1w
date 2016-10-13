#include "sphere.hpp"
#include <math.h>
#include "material.hpp"

struct _sphere : sphere {

    _sphere(const v3f &c, float r, const sptr<material> &m);
    virtual ~_sphere() { }

    bool  hit(const sptr<ray> &r, float min, float max, hit_record &rec) const;
    box   bounding_box() const { return m_box; }
    v3f   center() const { return m_center; }
    float radius() const { return m_radius; }

    v3f   m_center;
    float m_radius;
    box   m_box;
    sptr<material> m_material;
};

_sphere::_sphere(const v3f &c, float r, const sptr<material> &m)
{
    m_center = c;
    m_radius = r;
    m_material = m;
    m_box = { v3f_sub(c, { r, r, r }), v3f_add(c, { r, r, r }) };
}

bool _sphere::hit(const sptr<ray> &r, float min, float max, hit_record &rec) const
{
    v3f rdir = r->direction();
    v3f oc = v3f_sub(r->origin(), m_center);
    float a = v3f_dot(rdir, rdir);
    float b = 2 * v3f_dot(rdir, oc);
    float c = v3f_dot(oc, oc) - m_radius * m_radius;
    float delta = b * b - 4 * a * c;

    if (delta >= 0.0f) {
        float t = (-b - sqrtf(delta)) / (2.0f * a);
        if (!(t > min && t < max)) {
            t = (-b + sqrtf(delta)) / (2.0f * a);
        }
        if (t > min && t < max) {
            rec.t = t;
            rec.p = r->point(t);
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
