#include "rt1w/integrator.hpp"

#include "integrators/path.hpp"
#include "integrators/whitted.hpp"

#include "rt1w/bxdf.hpp"
#include "rt1w/interaction.hpp"
#include "rt1w/light.hpp"
#include "rt1w/material.hpp"
#include "rt1w/primitive.hpp"
#include "rt1w/ray.hpp"
#include "rt1w/sampler.hpp"
#include "rt1w/sampling.hpp"
#include "rt1w/scene.hpp"

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
            if (scene->intersect(r, lIsect)) {
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

Spectrum UniformSampleOneLight(const Interaction &isect,
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

#pragma mark - Static Constructor

sptr<Integrator> Integrator::create(const std::string &type,
                                    const sptr<Sampler> &sampler,
                                    size_t maxDepth)
{
    if (type == "path") {
        return PathIntegrator::create(sampler, maxDepth);
    }
    return WhittedIntegrator::create(sampler, maxDepth);
}
