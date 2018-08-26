#pragma once

#include "sptr.hpp"
#include "geometry.hpp"

struct Sampler : Object {

    static sptr<Sampler> create(uint32_t x, uint32_t y, uint32_t dim, bool jitter);

    virtual float sample1D() = 0;
    virtual v2f   sample2D() = 0;

    virtual void startPixel(v2i p) = 0;
    virtual bool startNextSample() = 0;
};
