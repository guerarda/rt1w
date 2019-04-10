#include "light.hpp"

#include "error.h"
#include "material.hpp"
#include "params.hpp"
#include "primitive.hpp"
#include "ray.hpp"
#include "scene.hpp"
#include "shape.hpp"
#include "spectrum.hpp"
#include "value.hpp"

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
    return !scene->world()->qIntersect(ray, .99f);
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
    warning("Light parameter \"type\" not recognized");

    return nullptr;
}
