#include "rt1w/light.hpp"

#include "rt1w/error.h"
#include "rt1w/material.hpp"
#include "rt1w/params.hpp"
#include "rt1w/primitive.hpp"
#include "rt1w/ray.hpp"
#include "rt1w/scene.hpp"
#include "rt1w/shape.hpp"
#include "rt1w/spectrum.hpp"
#include "rt1w/texture.hpp"
#include "rt1w/transform.hpp"
#include "rt1w/value.hpp"

#pragma mark - Interaction

Spectrum LightEmitted(const Interaction &isect, const v3f &wi)
{
    if (auto light = isect.prim->light()) {
        return light->L(isect, wi);
    }
    return {};
}

#pragma mark - Visibility Tester

bool VisibilityTester::visible(const sptr<Scene> &scene) const
{
    ASSERT(scene);

    Ray ray = SpawnRayTo(m_p0, m_p1);
    return !scene->qIntersect(ray);
}

#pragma mark - Point Light

struct _PointLight : PointLight {
    _PointLight(const v3f &p, const Spectrum &I) : m_p(p), m_I(I), m_type(LIGHT_DELTA_POS)
    {}

    LightType type() const override { return m_type; }
    Spectrum sample_Li(const Interaction &isect,
                       v2f u,
                       v3f &wi,
                       float &pdf,
                       VisibilityTester &vis) const override;
    Spectrum Le(const Ray &) const override { return {}; }
    float pdf_Li(const Interaction &, const v3f &) const override { return .0f; }

    v3f m_p;
    Spectrum m_I;
    LightType m_type;
};

Spectrum _PointLight::sample_Li(const Interaction &isect,
                                v2f,
                                v3f &wi,
                                float &pdf,
                                VisibilityTester &vis) const
{
    wi = Normalize(m_p - isect.p);
    pdf = 1.f;
    vis = { Interaction(m_p), isect };

    return m_I / DistanceSquared(isect.p, m_p);
}

sptr<PointLight> PointLight::create(const v3f &pos, const Spectrum &intensity)
{
    return std::make_shared<_PointLight>(pos, intensity);
}

sptr<PointLight> PointLight::create(const sptr<Params> &p)
{
    sptr<Value> pos = p->value("position");
    Spectrum I = Spectrum::fromRGB(Params::vector3f(p, "intensity", { 1.f, 1.f, 1.f }));

    if (pos) {
        return PointLight::create(pos->vector3f(), I);
    }
    warning("Point Light parameter \"position\" not specified");

    return nullptr;
}

#pragma mark - Area Light

struct _AreaLight : AreaLight {
    _AreaLight(const sptr<Shape> &s, const Spectrum &Lemit) :
        m_shape(s),
        m_Lemit(Lemit),
        m_type(LIGHT_AREA)
    {}

    LightType type() const override { return m_type; }
    Spectrum sample_Li(const Interaction &isect,
                       v2f u,
                       v3f &wi,
                       float &pdf,
                       VisibilityTester &vis) const override;
    Spectrum Le(const Ray &) const override { return {}; }
    float pdf_Li(const Interaction &isect, const v3f &wi) const override;

    Spectrum L(const Interaction &isect, const v3f &w) const override;
    sptr<Shape> shape() const override { return m_shape; }

    sptr<Shape> m_shape;
    Spectrum m_Lemit;
    LightType m_type;
};

Spectrum _AreaLight::sample_Li(const Interaction &isect,
                               v2f u,
                               v3f &wi,
                               float &pdf,
                               VisibilityTester &vis) const
{
    Interaction it = m_shape->sample(u);
    wi = Normalize(it.p - isect.p);
    pdf = m_shape->pdf();
    vis = { it, isect };
    return L(it, -wi);
}

float _AreaLight::pdf_Li(const Interaction &isect, const v3f &wi) const
{
    return m_shape->pdf(isect, wi);
}

Spectrum _AreaLight::L(const Interaction &isect, const v3f &w) const
{
    return Dot(isect.n, w) > 0.0f ? m_Lemit : Spectrum();
}

sptr<AreaLight> AreaLight::create(const sptr<Shape> &s, const Spectrum &Lemit)
{
    return std::make_shared<_AreaLight>(s, Lemit);
}

sptr<AreaLight> AreaLight::create(const sptr<Params> &p)
{
    sptr<Shape> shape = p->shape("shape");
    if (shape) {
        v3f c = Params::vector3f(p, "emit", { 1.0f, 1.0f, 1.0f });
        Spectrum s = Spectrum::fromRGB(c);
        return AreaLight::create(shape, s);
    }
    return nullptr;
}

#pragma mark - Environment Light

static inline float SphericalPhi(const v3f &v)
{
    float phi = std::atan2(v.y, v.x);
    return phi < .0f ? (float)(phi + 2 * Pi) : phi;
}

static inline float SphericalTheta(const v3f &v)
{
    return std::acos(Clamp(v.z, -1.f, 1.f));
}

static inline v3f SphericalDirection(float theta, float phi)
{
    float sinTheta = std::sin(theta);
    return { sinTheta * std::cos(phi), sinTheta * std::sin(phi), std::cos(theta) };
}

struct _EnvironmentLight : EnvironmentLight {
    _EnvironmentLight(const v3f &center,
                      float radius,
                      const Spectrum &L,
                      const sptr<Texture> &map) :
        m_center(center),
        m_radius(radius),
        m_Lemit(L),
        m_map(map),
        m_lightToWorld(Transform::Translate(center) * Transform::Scale(radius)),
        m_type(LightType(LIGHT_AREA | LIGHT_INIFINITE))
    {}

    LightType type() const override { return m_type; }
    Spectrum sample_Li(const Interaction &isect,
                       v2f u,
                       v3f &wi,
                       float &pdf,
                       VisibilityTester &vis) const override;
    Spectrum Le(const Ray &r) const override;
    float pdf_Li(const Interaction &isect, const v3f &wi) const override;

    v3f m_center;
    float m_radius;
    Spectrum m_Lemit;
    sptr<Texture> m_map;
    Transform m_lightToWorld;
    LightType m_type;
};

Spectrum _EnvironmentLight::sample_Li(const Interaction &isect,
                                      v2f u,
                                      v3f &wi,
                                      float &pdf,
                                      VisibilityTester &vis) const
{
    /* Compute wi */
    float phi = (float)(u.x * 2. * Pi);
    float theta = (float)(u.y * Pi);
    wi = Mulv(m_lightToWorld, SphericalDirection(theta, phi));

    /* Compute radiance & pdf */
    Spectrum L = m_Lemit;
    if (m_map) {
        L *= m_map->value(u.x, u.y, {});
    }
    pdf = (float)(1. / (2. * Pi * Pi));

    /* Visibility Tester */
    vis = { { m_center + wi * m_radius }, isect };

    return L;
}

Spectrum _EnvironmentLight::Le(const Ray &r) const
{
    Spectrum L = m_Lemit;
    if (m_map) {
        v3f wi = Normalize(Mulv(Inverse(m_lightToWorld), r.dir()));
        float u = (float)(SphericalPhi(wi) * Inv2Pi);
        float v = (float)(SphericalTheta(wi) * InvPi);

        ASSERT(u >= .0 && u <= 1.);
        ASSERT(v >= .0 && v <= 1.);

        L *= m_map->value(u, v, {});
    }
    return L;
}

float _EnvironmentLight::pdf_Li(const Interaction &, const v3f &) const
{
    return (float)(1. / (2. * Pi * Pi));
}

sptr<EnvironmentLight> EnvironmentLight::create(const v3f &center,
                                                float radius,
                                                const Spectrum &L,
                                                const sptr<Texture> &map)
{
    return std::make_shared<_EnvironmentLight>(center, radius, L, map);
}

sptr<EnvironmentLight> EnvironmentLight::create(const sptr<Params> &p)
{
    auto c = p->value("center");
    auto r = p->value("radius");
    if (c && r) {
        auto L = Spectrum::fromRGB(Params::vector3f(p, "scale", { 1.f, 1.f, 1.f }));
        auto map = p->texture("radiance");

        return EnvironmentLight::create(c->vector3f(), r->f32(), L, map);
    }
    WARNING_IF(!c, "Environment Light parameter \"center\" not specified");
    WARNING_IF(!r, "Environment Light parameter \"radius\" not specified");

    return nullptr;
}

#pragma mark -

bool IsDeltaLight(const sptr<Light> &light)
{
    LightType type = light->type();
    return type & LIGHT_DELTA_POS || type & LIGHT_DELTA_DIR;
}

#pragma mark - Static Constructors

sptr<Light> Light::create(const sptr<Params> &p)
{
    std::string type = p->string("type");
    WARNING_IF(type.empty(), "Light parameter \"type\" not specified");

    if (type == "point") {
        return PointLight::create(p);
    }
    if (type == "area") {
        return AreaLight::create(p);
    }
    if (type == "environment") {
        return EnvironmentLight::create(p);
    }
    warning("Light parameter \"type\" not recognized");

    return nullptr;
}
