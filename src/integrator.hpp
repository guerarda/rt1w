#pragma once

#include "sptr.hpp"
#include "geometry.hpp"

struct Ray;
struct Sampler;
struct Scene;

struct Integrator : Object {
    static sptr<Integrator> create(const sptr<Sampler> &sampler, size_t maxDepth);

    virtual sptr<const Sampler> sampler() const = 0;
    virtual v3f Li(const sptr<Ray> &ray,
                   const sptr<Scene> &scene,
                   const sptr<Sampler> &sampler,
                   size_t depth) const = 0;
};

struct PathIntegrator : Integrator {
    static sptr<PathIntegrator> create(const sptr<Sampler> &sampler, size_t maxDepth);
};
