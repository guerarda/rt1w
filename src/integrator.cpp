#include "integrator.hpp"

#include "bxdf.hpp"
#include "interaction.hpp"
#include "light.hpp"
#include "material.hpp"
#include "primitive.hpp"
#include "ray.hpp"
#include "sampler.hpp"
#include "sampling.hpp"
#include "scene.hpp"

#include <limits>

#pragma mark - Light Sampling

static Spectrum EstimateDirect(const Interaction &isect,
                               const v2f &uScaterring,
                               const sptr<Light> &light,
                               const v2f &uLight,
                               const sptr<Scene> &scene)
{
    ASSERT(light);

    BxDFType bsdfFlags = BSDF_ALL;
    Spectrum L = {};

    v3f wi;
    VisibilityTester vis;
    float lPdf;
    sptr<BSDF> bsdf = ComputeBSDF(isect);
    Spectrum Li = light->sample_Li(isect, uLight, wi, lPdf, vis);

    if (!Li.isBlack() && lPdf > .0f) {
        Spectrum f = bsdf->f(isect.wo, wi, bsdfFlags) * AbsDot(wi, isect.shading.n);
        float sPdf = bsdf->pdf(isect.wo, wi, bsdfFlags);

        if (!f.isBlack()) {
            if (!vis.visible(scene)) {
                Li = {};
            }
            if (!Li.isBlack()) {
                if (IsDeltaLight(light)) {
                    L += f * Li / lPdf;
                }
                else {
                    float weight = PowerHeuristic(1, lPdf, 1, sPdf);
                    L += f * Li * weight / lPdf;
                }
            }
        }
    }
    if (!IsDeltaLight(light)) {
        float sPdf;
        BxDFType sampledType;
        Spectrum f =
            bsdf->sample_f(isect.wo, uScaterring, wi, sPdf, bsdfFlags, &sampledType)
            * AbsDot(wi, isect.shading.n);
        if (!f.isBlack() && sPdf > .0f) {
            float weight = 1.f;
            if (!(sampledType & BSDF_SPECULAR)) {
                lPdf = light->pdf_Li(isect, wi);
                if (FloatEqual(lPdf, .0f)) {
                    return L;
                }
                weight = PowerHeuristic(1, sPdf, 1, lPdf);
            }
            Interaction lIsect;
            Ray r = SpawnRay(isect, wi);
            Li = {};
            if (scene->world()->intersect(r, lIsect)) {
                if (lIsect.prim->light() == light) {
                    Li = LightEmitted(lIsect, -wi);
                }
            }
            else {
                Li = light->Le(r);
            }
            if (!Li.isBlack()) {
                L += f * Li * weight / sPdf;
            }
        }
    }
    return L;
}

static Spectrum UniformSampleOneLight(const Interaction &isect,
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

    return n
           * EstimateDirect(isect,
                            sampler->sample2D(),
                            light,
                            sampler->sample2D(),
                            scene);
}

#pragma mark - Integrator Implementation

struct _Integrator : Integrator {
    _Integrator(const sptr<Sampler> &s, size_t m) : m_sampler(s), m_maxDepth(m) {}

    sptr<const Sampler> sampler() const override { return m_sampler; }
    Spectrum Li(const Ray &ray,
                const sptr<Scene> &scene,
                const sptr<Sampler> &sampler,
                size_t depth) const override;

    IntegratorResult NALi(const Ray &,
                          const sptr<Scene> &,
                          const sptr<Sampler> &,
                          size_t) const override;

    sptr<Sampler> m_sampler;
    size_t m_maxDepth;
    Spectrum m_background;
};

Spectrum _Integrator::Li(const Ray &ray,
                         const sptr<Scene> &scene,
                         const sptr<Sampler> &sampler,
                         size_t depth) const
{
    Interaction isect;
    if (scene->world()->intersect(ray, isect)) {
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
        Spectrum attenuation;
        v3f wi;
        if (isect.mat->scatter(ray, isect, attenuation, wi)) {
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
    Spectrum Li(const Ray &ray,
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

Spectrum _PathIntegrator::Li(const Ray &r,
                             const sptr<Scene> &scene,
                             const sptr<Sampler> &sampler,
                             size_t) const
{
    Ray ray = r;
    Spectrum L;
    Spectrum beta = { 1.f };
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
        BxDFType sampled;
        float pdf;
        Spectrum f =
            bsdf->sample_f(isect.wo, sampler->sample2D(), wi, pdf, BSDF_ALL, &sampled);
        if (f.isBlack() || FloatEqual(pdf, .0f)) {
            break;
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

IntegratorResult _PathIntegrator::NALi(const Ray &r,
                                       const sptr<Scene> &scene,
                                       const sptr<Sampler> &sampler,
                                       size_t) const
{
    Ray ray = r;
    Spectrum L, A;
    v3f N;
    Spectrum beta = { 1.f };
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
        BxDFType sampled;
        float pdf;
        Spectrum f =
            bsdf->sample_f(isect.wo, sampler->sample2D(), wi, pdf, BSDF_ALL, &sampled);
        if (f.isBlack() || FloatEqual(pdf, .0f)) {
            break;
        }
        if (bounces == 0) {
            A = f;
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
