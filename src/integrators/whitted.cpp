#include "integrators/whitted.hpp"

#include "rt1w/bxdf.hpp"
#include "rt1w/interaction.hpp"
#include "rt1w/light.hpp"
#include "rt1w/material.hpp"
#include "rt1w/ray.hpp"
#include "rt1w/sampler.hpp"
#include "rt1w/scene.hpp"

struct _WhittedIntegrator : WhittedIntegrator {
    _WhittedIntegrator(const sptr<Sampler> &s, size_t m) : m_sampler(s), m_maxDepth(m) {}

    sptr<const Sampler> sampler() const override { return m_sampler; }
    Spectrum Li(const Ray &ray,
                const sptr<Scene> &scene,
                const sptr<Sampler> &sampler,
                size_t depth,
                v3f *N = nullptr,
                Spectrum *A = nullptr) const override;

    sptr<Sampler> m_sampler;
    size_t m_maxDepth;
};

Spectrum _WhittedIntegrator::Li(const Ray &ray,
                                const sptr<Scene> &scene,
                                const sptr<Sampler> &sampler,
                                size_t depth,
                                v3f *N,
                                Spectrum *A) const
{
    ASSERT(scene);
    ASSERT(sampler);

    Interaction isect;
    if (!scene->intersect(ray, isect)) {
        Spectrum L;
        for (const auto &light : scene->lights()) {
            L += light->Le(ray);
        }
        return L;
    }
    sptr<BSDF> bsdf = ComputeBSDF(isect);
    Spectrum L = LightEmitted(isect, isect.wo);

    for (const auto &light : scene->lights()) {
        v3f wi;
        VisibilityTester vis;
        v2f u = sampler->sample2D();
        float pdf;
        Spectrum Li = light->sample_Li(isect, u, wi, pdf, vis);
        if (Li.isBlack() || FloatEqual(pdf, .0f)) {
            continue;
        }
        Spectrum f = bsdf->f(isect.wo, wi);

        if (!f.isBlack() && vis.visible(scene)) {
            L += Li * f * AbsDot(wi, isect.shading.n) / pdf;
        }
    }
    v3f wi;
    float pdf;
    Spectrum f = bsdf->sample_f(isect.wo, sampler->sample2D(), wi, pdf);
    if (depth + 1 < m_maxDepth) {
        L += f * Li(SpawnRay(isect, wi), scene, sampler, depth + 1)
             * AbsDot(wi, isect.shading.n) / pdf;
    }

    if (N) {
        *N = isect.n;
    }
    if (A) {
        *A = f;
    }
    return L;
}

#pragma mark - Static constructor

sptr<WhittedIntegrator> WhittedIntegrator::create(const sptr<Sampler> &sampler,
                                                  size_t maxDepth)
{
    return std::make_shared<_WhittedIntegrator>(sampler, maxDepth);
}
