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
    return !scene->world()->qIntersect(ray, 0.99f);
}

#pragma mark - Point Light

struct _PointLight : PointLight {
    _PointLight(const v3f &p, const Spectrum &I) : m_p(p), m_I(I) {}

    Spectrum sample_Li(const Interaction &isect,
                       v2f u,
                       v3f &wi,
                       VisibilityTester &vis) const override;
    Spectrum Le(const Ray &) const override { return {}; }

    v3f m_p;
    Spectrum m_I;
};

Spectrum _PointLight::sample_Li(const Interaction &isect,
                                v2f,
                                v3f &wi,
                                VisibilityTester &vis) const
{
    wi = Normalize(m_p - isect.p);
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
    _AreaLight(const sptr<Shape> &s, const Spectrum &Lemit) : m_shape(s), m_Lemit(Lemit)
    {}

    Spectrum sample_Li(const Interaction &isect,
                       v2f u,
                       v3f &wi,
                       VisibilityTester &vis) const override;
    Spectrum Le(const Ray &) const override { return {}; }

    Spectrum L(const Interaction &isect, const v3f &w) const override;
    sptr<Shape> shape() const override { return m_shape; }

    sptr<Shape> m_shape;
    Spectrum m_Lemit;
};

Spectrum _AreaLight::sample_Li(const Interaction &isect,
                               v2f u,
                               v3f &wi,
                               VisibilityTester &vis) const
{
    Interaction it = m_shape->sample(u);
    wi = Normalize(it.p - isect.p);
    vis = { it, isect };
    return L(it, -wi);
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
