#include "light.hpp"

#include "error.h"
#include "interaction.hpp"
#include "material.hpp"
#include "params.hpp"
#include "primitive.hpp"
#include "ray.hpp"
#include "scene.hpp"
#include "shape.hpp"
#include "value.hpp"

#pragma mark - Interaction

v3f LightEmitted(const Interaction &isect, const v3f &wi)
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
    Interaction i;
    return !scene->world()->intersect(ray, i, 0.99f);
}

#pragma mark - Point Light

struct _PointLight : PointLight {
    _PointLight(const v3f &p, const v3f &I) : m_p(p), m_I(I) {}

    v3f sample_Li(const Interaction &isect,
                  v2f u,
                  v3f &wi,
                  VisibilityTester &vis) const override;
    v3f Le(const Ray &) const override { return {}; }

    v3f m_p;
    v3f m_I;
};

v3f _PointLight::sample_Li(const Interaction &isect,
                           v2f,
                           v3f &wi,
                           VisibilityTester &vis) const
{
    wi = Normalize(m_p - isect.p);
    vis = { Interaction(m_p), isect };

    return m_I / DistanceSquared(isect.p, m_p);
}

sptr<PointLight> PointLight::create(const v3f &p, const v3f &I)
{
    return std::make_shared<_PointLight>(p, I);
}

sptr<PointLight> PointLight::create(const sptr<Params> &p)
{
    sptr<Value> pos = p->value("position");
    v3f I = Params::vector3f(p, "intensity", { 1.0f, 1.0f, 1.0f });

    if (pos) {
        return PointLight::create(pos->vector3f(), I);
    }
    warning("Point Light parameter \"position\" not specified");

    return nullptr;
}

#pragma mark - Area Light

struct _AreaLight : AreaLight {
    _AreaLight(const sptr<Shape> &s, const v3f &Lemit) : m_shape(s), m_Lemit(Lemit) {}

    v3f sample_Li(const Interaction &isect,
                  v2f u,
                  v3f &wi,
                  VisibilityTester &vis) const override;
    v3f Le(const Ray &) const override { return {}; }

    v3f L(const Interaction &isect, const v3f &w) const override;
    sptr<Shape> shape() const override { return m_shape; }

    sptr<Shape> m_shape;
    v3f m_Lemit;
};

v3f _AreaLight::sample_Li(const Interaction &isect,
                          v2f u,
                          v3f &wi,
                          VisibilityTester &vis) const
{
    Interaction it = m_shape->sample(u);
    wi = Normalize(it.p - isect.p);
    vis = { it, isect };
    return L(it, -wi);
}

v3f _AreaLight::L(const Interaction &isect, const v3f &w) const
{
    return Dot(isect.n, w) > 0.0f ? m_Lemit : v3f{};
}

sptr<AreaLight> AreaLight::create(const sptr<Shape> &s, const v3f &Lemit)
{
    return std::make_shared<_AreaLight>(s, Lemit);
}

sptr<AreaLight> AreaLight::create(const sptr<Params> &p)
{
    sptr<Shape> shape = p->shape("shape");
    if (shape) {
        v3f emit = Params::vector3f(p, "emit", { 1.0f, 1.0f, 1.0f });
        return AreaLight::create(shape, emit);
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
