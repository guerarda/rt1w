#include "sphere.hpp"

#include "error.h"
#include "params.hpp"
#include "primitive.hpp"
#include "value.hpp"

#include <cmath>

static v2f sphere_uv(const v3f &p)
{
    float x = std::min(1.0f, std::max(-1.0f, p.x));
    float y = std::min(1.0f, std::max(-1.0f, p.y));
    float z = std::min(1.0f, std::max(-1.0f, p.z));

    float phi = atan2(z, x);
    float theta = asin(y);

    double u = 1.0 - (phi + M_PI) / (2.0 * M_PI);
    double v = (theta + M_PI_2) / M_PI;

    return { (float)u, (float)v };
}

struct _Sphere : Sphere {
    _Sphere(const v3f &c, float r);

    bool hit(const sptr<Ray> &, float, float, hit_record &) const override;
    bounds3f bounds() const override { return m_box; }
    v3f center() const override { return m_center; }
    float radius() const override { return m_radius; }

    v3f m_center;
    float m_radius;
    bounds3f m_box;
};

_Sphere::_Sphere(const v3f &c, float r)
{
    m_center = c;
    m_radius = r;
    m_box = { c - v3f{ r, r, r }, c + v3f{ r, r, r } };
}

bool _Sphere::hit(const sptr<Ray> &r, float min, float max, hit_record &rec) const
{
    v3f rdir = r->direction();
    v3f oc = r->origin() - m_center;
    float a = Dot(rdir, rdir);
    float b = 2 * Dot(rdir, oc);
    float c = Dot(oc, oc) - m_radius * m_radius;
    float delta = b * b - 4 * a * c;

    if (delta >= 0.0f) {
        float t = (-b - sqrtf(delta)) / (2.0f * a);
        if (!(t > min && t < max)) {
            t = (-b + sqrtf(delta)) / (2.0f * a);
        }
        if (t > min && t < max) {
            rec.t = t;
            rec.p = r->point(t);
            rec.normal = Normalize((rec.p - m_center));
            v3f v = 1.0f / m_radius * (rec.p - m_center);
            rec.uv = sphere_uv(v);
            return true;
        }
    }
    return false;
}

#pragma mark - Static constructors

sptr<Sphere> Sphere::create(const v3f &c, float r)
{
    return std::make_shared<_Sphere>(c, r);
}

sptr<Sphere> Sphere::create(const sptr<Params> &p)
{
    sptr<Value> org = p->value("center");
    sptr<Value> rad = p->value("radius");

    if (org && rad) {
        return Sphere::create(org->vector3f(), rad->f32());
    }
    WARNING_IF(!org, "Sphere parameter \"center\" not specified");
    WARNING_IF(!rad, "Sphere parameter \"radius\" not specified");

    return nullptr;
}
