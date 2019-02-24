#pragma once

#include "geometry.hpp"
#include "spectrum.hpp"
#include "sptr.hpp"

struct Ray;
struct Sampler;
struct Scene;

struct IntegratorResult {
    v3f N;
    Spectrum A;
    Spectrum Li;
};

struct Integrator : Object {
    static sptr<Integrator> create(const sptr<Sampler> &sampler, size_t maxDepth);

    virtual sptr<const Sampler> sampler() const = 0;
    virtual Spectrum Li(const Ray &ray,
                        const sptr<Scene> &scene,
                        const sptr<Sampler> &sampler,
                        size_t depth) const = 0;
    virtual IntegratorResult NALi(const Ray &ray,
                                  const sptr<Scene> &scene,
                                  const sptr<Sampler> &sampler,
                                  size_t depth) const = 0;
};

struct PathIntegrator : Integrator {
    static sptr<PathIntegrator> create(const sptr<Sampler> &sampler, size_t maxDepth);
};
