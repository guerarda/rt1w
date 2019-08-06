#pragma once

#include "rt1w/geometry.hpp"
#include "rt1w/spectrum.hpp"
#include "rt1w/sptr.hpp"

struct Ray;
struct Sampler;
struct Scene;

struct Integrator : Object {
    static sptr<Integrator> create(const sptr<Sampler> &sampler, size_t maxDepth);

    virtual sptr<const Sampler> sampler() const = 0;
    virtual Spectrum Li(const Ray &ray,
                        const sptr<Scene> &scene,
                        const sptr<Sampler> &sampler,
                        size_t depth,
                        v3f *N = nullptr,
                        Spectrum *A = nullptr) const = 0;
};

struct PathIntegrator : Integrator {
    static sptr<PathIntegrator> create(const sptr<Sampler> &sampler, size_t maxDepth);
};
