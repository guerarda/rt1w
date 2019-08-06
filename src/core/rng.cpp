#include "rng.hpp"

#include <limits>
#include <random>

#include "error.h"

/* See http://xoshiro.di.unimi.it */
static inline uint64_t splitmix64(uint64_t x)
{
    uint64_t z = (x += 0x9e3779b97f4a7c15);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
    z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
    return z ^ (z >> 31);
}

static inline uint64_t rotl(const uint64_t x, int k)
{
    return (x << k) | (x >> (64 - k));
}

struct xoroshiro256plus {
    xoroshiro256plus(uint64_t seed)
    {
        uint64_t x = seed;
        for (size_t i = 0; i < 4; i++) {
            x = splitmix64(x);
            s[i] = x;
        }
    }
    uint64_t next()
    {
        const uint64_t r = s[0] + s[3];
        const uint64_t t = s[1] << 17;

        s[2] ^= s[0];
        s[3] ^= s[1];
        s[1] ^= s[2];
        s[0] ^= s[3];

        s[2] ^= t;
        s[3] = rotl(s[3], 45);

        return r;
    }

    double uniform_double()
    {
        uint64_t x = next();
        return (x >> 11) * (1. / (UINT64_C(1) << 53));
    }

    uint64_t s[4];
};

struct _RNG : RNG {
    _RNG(uint64_t seed) : m_xs256p(xoroshiro256plus(seed)) {}

    uint32_t u32() override;
    float f32() override;

    uint32_t u32(uint32_t b) override;
    float f32(float b) override;

    xoroshiro256plus m_xs256p;
};

uint32_t _RNG::u32()
{
    uint32_t max = std::numeric_limits<uint32_t>::max();
    return (uint32_t)floor(f32() * max);
}

float _RNG::f32()
{
    float f;
    do {
        f = (float)m_xs256p.uniform_double();
    } while (f >= 1.0f);
    return f;
}

uint32_t _RNG::u32(uint32_t b)
{
    float f = f32();
    auto v = (uint32_t)floor(f * b);
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

uptr<RNG> RNG::create()
{
    return std::make_unique<_RNG>(std::random_device()());
}
