#pragma once

#include "sptr.hpp"
#include "geometry.hpp"

struct Camera;
struct Event;
struct Ray;
struct Primitive;
struct Sampler;

struct Integrator : Object {
    static sptr<Integrator> create(const sptr<Sampler> &sampler, size_t maxDepth);

    virtual sptr<const Sampler> sampler() const = 0;
    virtual v3f Li(const sptr<Ray> &ray, const sptr<Primitive> &world, size_t depth) const = 0;
};
