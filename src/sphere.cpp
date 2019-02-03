#include "sphere.hpp"

#include "efloat.hpp"
#include "error.h"
#include "params.hpp"
#include "ray.hpp"
#include "sampling.hpp"
#include "transform.hpp"
#include "utils.hpp"
#include "value.hpp"

struct _Sphere : Sphere {
    _Sphere(const Transform &t, float r) :
        m_worldToObj(t),
        m_box(Inverse(t)(bounds3f{ -v3f{ r, r, r }, v3f{ r, r, r } })),
        m_radius(r)
    {}

    bool intersect(const Ray &r, Interaction &isect, float max) const override;
    bool qIntersect(const Ray &r, float max) const override;

    bounds3f bounds() const override { return m_box; }
    Transform worldToObj() const override { return m_worldToObj; }
    Interaction sample(const v2f &u) const override;

    float radius() const override { return m_radius; }

    Transform m_worldToObj;
    bounds3f m_box;
    float m_radius;
};

static inline bool SphereQuadratic(const Ray &r,
                                   const v3f &oError,
                                   const v3f &dError,
                                   float radius,
                                   EFloat &t0,
                                   EFloat &t1)
{
    EFloat ox = { r.org().x, oError.x };
    EFloat oy = { r.org().y, oError.y };
    EFloat oz = { r.org().z, oError.z };

    EFloat dx = { r.dir().x, dError.x };
    EFloat dy = { r.dir().y, dError.y };
    EFloat dz = { r.dir().z, dError.z };

    EFloat a = dx * dx + dy * dy + dz * dz;
    EFloat b = 2.f * (dx * ox + dy * oy + dz * oz);
    EFloat c = ox * ox + oy * oy + oz * oz - radius * radius;

    return Quadratic(a, b, c, t0, t1);
}

bool _Sphere::intersect(const Ray &ray, Interaction &isect, float max) const
{
    v3f oError, dError;
    Ray r = m_worldToObj(ray, oError, dError);

    EFloat t0, t1;
    if (!SphereQuadratic(r, oError, dError, m_radius, t0, t1)) {
        return false;
    }
    EFloat t = t0.lo() > .0f && t0.hi() < max ? t0 : t1;
    if (t.lo() <= .0f || t.hi() >= max) {
        return false;
    }
    isect.t = (float)t;
    isect.p = r(isect.t);
    isect.n = Normalize(isect.p);
    isect.wo = -r.dir();

    /* Reproject p onto the sphere */
    isect.p *= m_radius / isect.p.length();
    isect.error = gamma(5) * Abs(isect.p);

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

    isect = Inverse(m_worldToObj)(isect);

    return true;
}

bool _Sphere::qIntersect(const Ray &ray, float max) const
{
    v3f oError, dError;
    Ray r = m_worldToObj(ray, oError, dError);

    EFloat t0, t1;
    if (SphereQuadratic(r, oError, dError, m_radius, t0, t1)) {
        EFloat t = t0.lo() > .0f && t0.hi() < max ? t0 : t1;
        if (t.lo() > .0f && t.hi() < max) {
            return true;
        }
    }
    return false;
}

Interaction _Sphere::sample(const v2f &u) const
{
    Interaction it;
    it.p = m_radius * UniformSampleSphere(u);
    it.n = Normalize(it.p);

    return Inverse(m_worldToObj)(it);
}

#pragma mark - Static constructors

sptr<Sphere> Sphere::create(const Transform &worldToObj, float radius)
{
    return std::make_shared<_Sphere>(worldToObj, radius);
}

sptr<Sphere> Sphere::create(const sptr<Params> &p)
{
    float radius = Params::f32(p, "radius", 1.f);
    m44f mat = Params::matrix44f(p, "transform", m44f_identity());
    Transform t = Transform(mat);

    return Sphere::create(t, radius);
}
