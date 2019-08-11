#include "integrators/path.hpp"

#include "rt1w/bxdf.hpp"
#include "rt1w/interaction.hpp"
#include "rt1w/light.hpp"
#include "rt1w/material.hpp"
#include "rt1w/ray.hpp"
#include "rt1w/sampler.hpp"
#include "rt1w/scene.hpp"

struct _PathIntegrator : PathIntegrator {
    _PathIntegrator(const sptr<Sampler> &s, size_t m) : m_sampler(s), m_maxDepth(m) {}

    sptr<const Sampler> sampler() const override { return m_sampler; }
    Spectrum Li(const Ray &ray,
                const sptr<Scene> &scene,
                const sptr<Sampler> &sampler,
                size_t depth,
                v3f *N,
                Spectrum *A) const override;

    sptr<Sampler> m_sampler;
    size_t m_maxDepth;
};

Spectrum _PathIntegrator::Li(const Ray &r,
                             const sptr<Scene> &scene,
                             const sptr<Sampler> &sampler,
                             size_t,
                             v3f *N,
                             Spectrum *A) const
{
    Ray ray = r;
    Spectrum L;
    Spectrum beta = { 1.f };
    bool specular = false;

    for (size_t bounces = 0;; bounces++) {
        Interaction isect;
        bool intersect = scene->intersect(ray, isect);
        if (bounces == 0) {
            if (intersect) {
                if (N) {
                    *N = isect.n;
                }
                L = LightEmitted(isect, -ray.dir());
            }
            else {
                for (const auto &light : scene->lights()) {
                    L += light->Le(ray);
                }
            }
        }
        if (specular) {
            if (intersect) {
                L += beta * LightEmitted(isect, -ray.dir());
            }
            else {
                for (const auto &light : scene->lights()) {
                    L += beta * light->Le(ray);
                }
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
        BxDFType sampled;
        float pdf;
        Spectrum f =
            bsdf->sample_f(isect.wo, sampler->sample2D(), wi, pdf, BSDF_ALL, &sampled);
        if (f.isBlack() || FloatEqual(pdf, .0f)) {
            break;
        }
        if (A && bounces == 0) {
            *A = f;
        }
        beta *= f * AbsDot(wi, isect.shading.n) / pdf;
        specular = sampled & BSDF_SPECULAR;
        ray = SpawnRay(isect, wi);

        if (bounces > 3) {
            float q = std::max(.5f, 1.f - MaxComponent(beta));
            if (sampler->sample1D() < q) {
                break;
            }
            beta /= 1 - q;
        }
    }
    return L;
}

sptr<PathIntegrator> PathIntegrator::create(const sptr<Sampler> &sampler, size_t maxDepth)
{
    return std::make_shared<_PathIntegrator>(sampler, maxDepth);
}
