#include "sphere.hpp"
#include "primitive.hpp"
#include <assert.h>
#include <math.h>


static v2f sphere_uv(const v3f &p)
{
    float x = fminf(1.0f, fmaxf(-1.0f, p.x));
    float y = fminf(1.0f, fmaxf(-1.0f, p.y));
    float z = fminf(1.0f, fmaxf(-1.0f, p.z));

    double phi = atan2(z, x);
    double theta = asin(y);

    double u = 1.0 - (phi + M_PI) / (2 * M_PI);
    double v = (theta + M_PI_2) / M_PI;

    return { (float)u, (float)v };
}

struct _Sphere : Sphere {

    _Sphere(const v3f &c, float r);

    bool     hit(const sptr<ray> &r, float min, float max, hit_record &rec) const;
    bounds3f bounds() const { return m_box; }
    v3f      center() const { return m_center; }
    float    radius() const { return m_radius; }

    v3f      m_center;
    float    m_radius;
    bounds3f m_box;
};

_Sphere::_Sphere(const v3f &c, float r)
{
    m_center = c;
    m_radius = r;
    m_box = { c - v3f{ r, r, r }, c + v3f{ r, r, r } };
}

bool _Sphere::hit(const sptr<ray> &r, float min, float max, hit_record &rec) const
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
            rec.normal = (rec.p - m_center).normalized();
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
