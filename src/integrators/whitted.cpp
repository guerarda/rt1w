#include "integrators/whitted.hpp"

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
    Spectrum m_background;
};

Spectrum _WhittedIntegrator::Li(const Ray &ray,
                                const sptr<Scene> &scene,
                                const sptr<Sampler> &sampler,
                                size_t depth,
                                v3f *N,
                                Spectrum *A) const
{
    Interaction isect;
    if (scene->intersect(ray, isect)) {
        Spectrum attenuation;
        v3f wi;
        if (depth < m_maxDepth && isect.mat->scatter(ray, isect, attenuation, wi)) {
            Spectrum L;
            for (const auto &light : scene->lights()) {
                v3f lwi;
                VisibilityTester vis;
                v2f u = sampler->sample2D();
                float pdf;
                Spectrum Li = light->sample_Li(isect, u, lwi, pdf, vis);
                if (vis.visible(scene)) {
                    L += Li * isect.mat->f(isect, isect.wo, lwi);
                }
            }
            Ray scattered = SpawnRay(isect, wi);
            L += attenuation * Li(scattered, scene, sampler, depth + 1);
            if (N) {
                *N = isect.n;
            }
            if (A) {
                *A = attenuation;
            }
            return L;
        }
    }
    return m_background;
}

#pragma mark - Static constructor

sptr<WhittedIntegrator> WhittedIntegrator::create(const sptr<Sampler> &sampler,
                                                  size_t maxDepth)
{
    return std::make_shared<_WhittedIntegrator>(sampler, maxDepth);
}
