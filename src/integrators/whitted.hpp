#pragma once

#include "rt1w/integrator.hpp"
#include "rt1w/sptr.hpp"

struct Sampler;

struct WhittedIntegrator : Integrator {
    static sptr<WhittedIntegrator> create(const sptr<Sampler> &sampler, size_t maxDepth);
};
