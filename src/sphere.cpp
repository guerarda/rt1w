#include "sphere.hpp"

#include "error.h"
#include "params.hpp"
#include "primitive.hpp"
#include "ray.hpp"
#include "sampling.hpp"
#include "utils.hpp"
#include "value.hpp"

struct _Sphere : Sphere {
    _Sphere(const v3f &c, float r) :
        m_center(c),
        m_radius(r),
        m_box({ c - v3f{ r, r, r }, c + v3f{ r, r, r } })
    {}

    bool intersect(const sptr<Ray> &r,
                   float min,
                   float max,
                   Interaction &isect) const override;
    bounds3f bounds() const override { return m_box; }
    Interaction sample(const v2f &u) const override;
    v3f center() const override { return m_center; }
    float radius() const override { return m_radius; }

    v3f m_center;
    float m_radius;
    bounds3f m_box;
};

bool _Sphere::intersect(const sptr<Ray> &r,
                        float min,
                        float max,
                        Interaction &isect) const
{
    v3f rdir = r->direction();
    v3f oc = r->origin() - m_center;
    double a = Dot(rdir, rdir);
    double b = 2 * Dot(rdir, oc);
    double c = Dot(oc, oc) - m_radius * m_radius;

    double t0, t1;
    if (Quadratic(a, b, c, t0, t1)) {
        double t = t0 > min && t0 < max ? t0 : t1;
        if (t > min && t < max) {
            isect.t = (float)t;
            isect.p = r->point(isect.t);
            isect.n = Normalize((isect.p - m_center));
            isect.wo = -rdir;

            /* Compute sphere uv */
            v3f p = isect.n;
            double phi = std::atan2(p.x, p.z);
            double theta = std::acos(Clamp(p.y, -1.0, 1.0));

            isect.uv.x = (float)(0.5 + phi / (2.0 * Pi));
            isect.uv.y = (float)(theta / Pi);

            /* Compute dp/du & dp/dv */
            double d = std::sqrt(p.x * p.x + p.z * p.z);
            double sinPhi = p.x / d;
            double cosPhi = p.z / d;
            isect.dpdu = { (float)(2.0 * Pi * p.z), 0.0, (float)(-2.0 * Pi * p.x) };
            isect.dpdv = v3f{ (float)(Pi * p.y * sinPhi),
                              (float)(Pi * -d),
                              (float)(Pi * p.y * cosPhi) };

            /* Shading Geometry */
            isect.shading.n = isect.n;
            isect.shading.dpdu = isect.dpdu;
            isect.shading.dpdv = isect.dpdv;
            return true;
        }
    }
    return false;
}

Interaction _Sphere::sample(const v2f &u) const
{
    Interaction it;
    it.p = m_center + m_radius * UniformSampleSphere(u);
    it.n = Normalize(it.p - m_center);

    return it;
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
