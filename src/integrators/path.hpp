#pragma once

#include "rt1w/integrator.hpp"

struct Sampler;

struct PathIntegrator : Integrator {
    static sptr<PathIntegrator> create(const sptr<Sampler> &sampler, size_t maxDepth);
};
