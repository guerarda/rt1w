#include "light.hpp"

#include "error.h"
#include "interaction.hpp"
#include "material.hpp"
#include "params.hpp"
#include "primitive.hpp"
#include "ray.hpp"
#include "sampler.hpp"
#include "scene.hpp"
#include "texture.hpp"
#include "value.hpp"

#pragma mark - Light Sampling

v3f EstimateDirect(const Interaction &isect,
                   const sptr<Light> &light,
                   const sptr<Scene> &scene,
                   const sptr<Sampler> &)
{
    v3f wi;
    v3f Li = light->sample_Li(isect, wi);
    if (!light->visible(isect, scene)) {
        return v3f();
    }
    v3f f = isect.mat->f(isect, isect.wo, wi);

    return f * Li;
}

v3f UniformSampleOneLight(const Interaction &isect,
                          const sptr<Scene> &scene,
                          const sptr<Sampler> &sampler)
{
    auto lights = scene->lights();
    ASSERT(!lights.empty());

    /* Randomly pick one light */
    size_t n = lights.size();
    auto ix = (size_t)floor(sampler->sample1D() * n);

    sptr<Light> light = lights[ix];

    return n * EstimateDirect(isect, light, scene, sampler);
}

#pragma mark - Point Light

struct _PointLight : PointLight {
    _PointLight(const v3f &p, const v3f &I) : m_p(p), m_I(I) {}

    v3f sample_Li(const Interaction &isect, v3f &wi) const override;
    v3f Le(const sptr<Ray> &) const override { return v3f(); }
    bool visible(const Interaction &isect, const sptr<Scene> &scene) const override;

    v3f m_p;
    v3f m_I;
};

v3f _PointLight::sample_Li(const Interaction &isect, v3f &wi) const
{
    wi = Normalize(m_p - isect.p);

    return m_I / DistanceSquared(isect.p, m_p);
}

bool _PointLight::visible(const Interaction &isect, const sptr<Scene> &scene) const
{
    sptr<Ray> ray = Ray::create(m_p, isect.p - m_p);
    Interaction i;
    return !scene->world()->intersect(ray, 0.001f, 1.0f, i);
}

#pragma mark - Static Constructors

sptr<Light> Light::create(const sptr<Params> &p)
{
    std::string type = p->string("type");
    WARNING_IF(type.empty(), "Light parameter \"type\" not specified");

    if (type == "point") {
        return PointLight::create(p);
    }
    warning("Light parameter \"type\" not recognized");

    return nullptr;
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
