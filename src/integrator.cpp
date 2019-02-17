#include "integrator.hpp"

#include "bxdf.hpp"
#include "interaction.hpp"
#include "light.hpp"
#include "material.hpp"
#include "primitive.hpp"
#include "ray.hpp"
#include "sampler.hpp"
#include "scene.hpp"
#include "utils.hpp"

#include <limits>

#pragma mark - Light Sampling

static v3f EstimateDirect(const Interaction &isect,
                          const sptr<Light> &light,
                          const sptr<Scene> &scene,
                          const sptr<Sampler> &sampler)
{
    ASSERT(light);

    v3f wi;
    VisibilityTester vis;
    v2f u = sampler->sample2D();
    v3f Li = light->sample_Li(isect, u, wi, vis);
    if (!vis.visible(scene)) {
        return {};
    }
    v3f f = isect.mat->f(isect, isect.wo, wi);

    return f * Li;
}

static v3f UniformSampleOneLight(const Interaction &isect,
                                 const sptr<Scene> &scene,
                                 const sptr<Sampler> &sampler)
{
    ASSERT(scene);
    ASSERT(sampler);

    auto lights = scene->lights();
    if (lights.empty()) {
        return {};
    }
    /* Randomly pick one light */
    size_t n = lights.size();
    auto ix = (size_t)floor(sampler->sample1D() * n);

    sptr<Light> light = lights[ix];

    return n * EstimateDirect(isect, light, scene, sampler);
}

#pragma mark - Integrator Implementation

struct _Integrator : Integrator {
    _Integrator(const sptr<Sampler> &s, size_t m) : m_sampler(s), m_maxDepth(m) {}

    sptr<const Sampler> sampler() const override { return m_sampler; }
    v3f Li(const Ray &ray,
           const sptr<Scene> &scene,
           const sptr<Sampler> &sampler,
           size_t depth) const override;

    IntegratorResult NALi(const Ray &,
                          const sptr<Scene> &,
                          const sptr<Sampler> &,
                          size_t) const override;

    sptr<Sampler> m_sampler;
    size_t m_maxDepth;
    v3f m_background;
};

v3f _Integrator::Li(const Ray &ray,
                    const sptr<Scene> &scene,
                    const sptr<Sampler> &sampler,
                    size_t depth) const
{
    Interaction isect;
    if (scene->world()->intersect(ray, isect)) {
        v3f attenuation;
        v3f wi;
        if (depth < m_maxDepth && isect.mat->scatter(ray, isect, attenuation, wi)) {
            v3f L;
            for (const auto &light : scene->lights()) {
                v3f lwi;
                VisibilityTester vis;
                v2f u = sampler->sample2D();
                v3f Li = light->sample_Li(isect, u, lwi, vis);
                if (vis.visible(scene)) {
                    L += Li * isect.mat->f(isect, isect.wo, lwi);
                }
            }
            Ray scattered = SpawnRay(isect, wi);
            L += attenuation * Li(scattered, scene, sampler, depth + 1);
            return L;
        }
    }
    return m_background;
}

IntegratorResult _Integrator::NALi(const Ray &ray,
                                   const sptr<Scene> &scene,
                                   const sptr<Sampler> &sampler,
                                   size_t) const
{
    Interaction isect;
    if (scene->world()->intersect(ray, isect)) {
        v3f attenuation;
        v3f wi;
        if (isect.mat->scatter(ray, isect, attenuation, wi)) {
            v3f L;
            for (const auto &light : scene->lights()) {
                v3f lwi;
                VisibilityTester vis;
                v2f u = sampler->sample2D();
                v3f Li = light->sample_Li(isect, u, lwi, vis);
                if (vis.visible(scene)) {
                    L += Li * isect.mat->f(isect, isect.wo, lwi);
                }
            }
            Ray scattered = SpawnRay(isect, wi);
            L += attenuation * Li(scattered, scene, sampler, 1);
            return { isect.n, attenuation, L };
        }
    }
    return { {}, m_background, m_background };
}

#pragma mark - Path Integrator

struct _PathIntegrator : PathIntegrator {
    _PathIntegrator(const sptr<Sampler> &s, size_t m) : m_sampler(s), m_maxDepth(m) {}

    sptr<const Sampler> sampler() const override { return m_sampler; }
    v3f Li(const Ray &ray,
           const sptr<Scene> &scene,
           const sptr<Sampler> &sampler,
           size_t depth) const override;
    IntegratorResult NALi(const Ray &ray,
                          const sptr<Scene> &scene,
                          const sptr<Sampler> &sampler,
                          size_t depth) const override;

    sptr<Sampler> m_sampler;
    size_t m_maxDepth;
};

v3f _PathIntegrator::Li(const Ray &r,
                        const sptr<Scene> &scene,
                        const sptr<Sampler> &sampler,
                        size_t) const
{
    Ray ray = r;
    v3f L;
    v3f beta = { 1.0f, 1.0f, 1.0f };
    bool specular = false;

    for (size_t bounces = 0;; bounces++) {
        Interaction isect;
        bool intersect = scene->world()->intersect(ray, isect);
        if (bounces == 0 || specular) {
            if (intersect) {
                L += beta * LightEmitted(isect, -ray.dir());
            }
        }
        if (!intersect || bounces > m_maxDepth) {
            break;
        }
        L += beta * UniformSampleOneLight(isect, scene, sampler);

        v3f wi;

        sptr<BSDF> bsdf = ComputeBSDF(isect);
        if (!bsdf) {
            break;
        }
        BxDFType type;
        v3f f = bsdf->sample_f(isect.wo, wi, sampler->sample2D(), type);
        if (FloatEqual(f.x, 0.0f) && FloatEqual(f.y, 0.0f) && FloatEqual(f.z, 0.0f)) {
            break;
        }
        specular = type & BSDF_SPECULAR;
        beta *= f;
        ray = SpawnRay(isect, wi);

        if (bounces > 3) {
            float q = std::max(0.5f, 1 - beta.length());
            if (sampler->sample1D() < q) {
                break;
            }
            beta /= 1 - q;
        }
    }
    return L;
}

IntegratorResult _PathIntegrator::NALi(const Ray &r,
                                       const sptr<Scene> &scene,
                                       const sptr<Sampler> &sampler,
                                       size_t) const
{
    Ray ray = r;
    v3f N, A, L;
    v3f beta = { 1.0f, 1.0f, 1.0f };
    bool specular = false;

    for (size_t bounces = 0;; bounces++) {
        Interaction isect;
        bool intersect = scene->world()->intersect(ray, isect);
        if (bounces == 0) {
            if (intersect) {
                N = isect.n;
                L = LightEmitted(isect, -ray.dir());
            }
        }
        if (specular) {
            if (intersect) {
                L += beta * LightEmitted(isect, -ray.dir());
            }
        }
        if (!intersect || bounces > m_maxDepth) {
            break;
        }
        L += beta * UniformSampleOneLight(isect, scene, sampler);

        v3f wi;

        sptr<BSDF> bsdf = ComputeBSDF(isect);
        if (!bsdf) {
            break;
        }
        BxDFType type;
        v3f f = bsdf->sample_f(isect.wo, wi, sampler->sample2D(), type);
        if (FloatEqual(f.x, 0.0f) && FloatEqual(f.y, 0.0f) && FloatEqual(f.z, 0.0f)) {
            break;
        }
        if (bounces == 0) {
            A = f;
        }
        specular = type & BSDF_SPECULAR;
        beta *= f;
        ray = SpawnRay(isect, wi);

        if (bounces > 3) {
            float q = std::max(0.5f, 1 - beta.length());
            if (sampler->sample1D() < q) {
                break;
            }
            beta /= 1 - q;
        }
    }
    return { N, A, L };
}

#pragma mark - Static constructor

sptr<Integrator> Integrator::create(const sptr<Sampler> &sampler, size_t maxDepth)
{
    return std::make_shared<_Integrator>(sampler, maxDepth);
}

sptr<PathIntegrator> PathIntegrator::create(const sptr<Sampler> &sampler, size_t maxDepth)
{
    return std::make_shared<_PathIntegrator>(sampler, maxDepth);
}
