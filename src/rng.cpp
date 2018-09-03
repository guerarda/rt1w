#include "rng.hpp"

#include <limits>
#include <random>

#include "error.h"

struct _RNG : RNG {

    void init();

    uint32_t u32() override;
    float    f32() override;

    uint32_t u32(uint32_t b) override;
    float    f32(float b) override;

    std::mt19937 m_mt;
    std::uniform_real_distribution<float> m_dist;
};


void _RNG::init()
{
    m_mt = std::mt19937(std::random_device()());
    m_dist = std::uniform_real_distribution<float>(0.0, 1.0);
}

uint32_t _RNG::u32()
{
    uint32_t max = std::numeric_limits<uint32_t>::max();
    return (uint32_t)floor(f32() * max);
}

float _RNG::f32()
{
    float f;
    do {
        f = m_dist(m_mt);
    } while (f >= 1.0f);
    return f;
}

uint32_t _RNG::u32(uint32_t b)
{
    float f = f32();
    uint32_t v = (uint32_t)floor(f * b);
    ASSERT(v < b);
    return v;
}

float _RNG::f32(float b)
{
    float v = f32() * b;
    ASSERT(v < b);
    return v;
}

#pragma mark - Static Constructor

sptr<RNG> RNG::create()
{
    auto rng = std::make_shared<_RNG>();
    rng->init();
    return rng;
}
