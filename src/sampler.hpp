#pragma once

#include "sptr.hpp"
#include "geometry.hpp"
#include "camera.hpp"

struct Sampler : Object {
    static sptr<Sampler> create(uint32_t x, uint32_t y, uint32_t dim, bool jitter);

    virtual uint64_t samplesPerPixel() const = 0;
    virtual sptr<Sampler> clone() const = 0;

    virtual float sample1D() = 0;
    virtual v2f   sample2D() = 0;
    virtual CameraSample cameraSample() = 0;

    virtual void startPixel(v2i p) = 0;
    virtual bool startNextSample() = 0;
};
