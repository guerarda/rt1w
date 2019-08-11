#pragma once

#include "rt1w/geometry.hpp"
#include "rt1w/spectrum.hpp"
#include "rt1w/sptr.hpp"

#include <string>

struct Interaction;
struct Ray;
struct Sampler;
struct Scene;

Spectrum UniformSampleOneLight(const Interaction &isect,
                               const sptr<Scene> &scene,
                               const sptr<Sampler> &sampler);

struct Integrator : Object {
    static sptr<Integrator> create(const std::string &type,
                                   const sptr<Sampler> &sampler,
                                   size_t maxDepth);

    virtual sptr<const Sampler> sampler() const = 0;
    virtual Spectrum Li(const Ray &ray,
                        const sptr<Scene> &scene,
                        const sptr<Sampler> &sampler,
                        size_t depth,
                        v3f *N = nullptr,
                        Spectrum *A = nullptr) const = 0;
};
