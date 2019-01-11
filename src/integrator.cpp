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

#pragma mark - Integrator Implementation

struct _Integrator : Integrator {
    _Integrator(const sptr<Sampler> &s, size_t m) : m_sampler(s), m_maxDepth(m) {}

    sptr<const Sampler> sampler() const override { return m_sampler; }
    v3f Li(const sptr<Ray> &ray,
           const sptr<Scene> &scene,
           const sptr<Sampler> &sampler,
           size_t depth) const override;

    sptr<Sampler> m_sampler;
    size_t m_maxDepth;
    v3f m_background;
};

v3f _Integrator::Li(const sptr<Ray> &ray,
                    const sptr<Scene> &scene,
                    const sptr<Sampler> &sampler,
                    size_t depth) const
{
    Interaction isect;
    if (scene->world()->intersect(ray,
                                  0.001f,
                                  std::numeric_limits<float>::max(),
                                  isect)) {
        v3f attenuation;
        v3f wi;
        if (depth < m_maxDepth && isect.mat->scatter(ray, isect, attenuation, wi)) {
            v3f L;
            for (const auto &light : scene->lights()) {
                v3f lwi;
                VisibilityTester vis;
                v3f Li = light->sample_Li(isect, lwi, vis);
                if (vis.visible(scene)) {
                    L += Li * isect.mat->f(isect, isect.wo, lwi);
                }
            }
            sptr<Ray> scattered = Ray::create(isect.p, wi);
            L += attenuation * Li(scattered, scene, sampler, depth + 1);
            return L;
        }
    }
    return m_background;
}

#pragma mark - Path Integrator

struct _PathIntegrator : PathIntegrator {
    _PathIntegrator(const sptr<Sampler> &s, size_t m) : m_sampler(s), m_maxDepth(m) {}

    sptr<const Sampler> sampler() const override { return m_sampler; }
    v3f Li(const sptr<Ray> &ray,
           const sptr<Scene> &scene,
           const sptr<Sampler> &sampler,
           size_t depth) const override;

    sptr<Sampler> m_sampler;
    size_t m_maxDepth;
};

v3f _PathIntegrator::Li(const sptr<Ray> &r,
                        const sptr<Scene> &scene,
                        const sptr<Sampler> &sampler,
                        size_t) const
{
    sptr<Ray> ray = r;
    v3f L;
    v3f beta = { 1.0f, 1.0f, 1.0f };
    bool specular = false;

    for (size_t bounces = 0;; bounces++) {
        Interaction isect;
        bool intersect = scene->world()->intersect(ray,
                                                   0.001f,
                                                   std::numeric_limits<float>::max(),
                                                   isect);
        if (bounces == 0 || specular) {
            if (intersect) {
                L += beta * LightEmitted(isect, -ray->direction());
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
        v3f f = bsdf->sample_f(isect.wo, wi, type);
        if (FloatEqual(f.x, 0.0f) && FloatEqual(f.y, 0.0f) && FloatEqual(f.z, 0.0f)) {
            break;
        }
        specular = type & BSDF_SPECULAR;
        beta *= f;
        ray = Ray::create(isect.p, wi);

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

#pragma mark - Static constructor

sptr<Integrator> Integrator::create(const sptr<Sampler> &sampler, size_t maxDepth)
{
    return std::make_shared<_Integrator>(sampler, maxDepth);
}

sptr<PathIntegrator> PathIntegrator::create(const sptr<Sampler> &sampler, size_t maxDepth)
{
    return std::make_shared<_PathIntegrator>(sampler, maxDepth);
}
