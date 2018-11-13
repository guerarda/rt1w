#include "sptr.hpp"

constexpr float  OneMinusEpsilon_f32 = 0.99999994f;
constexpr double OneMinusEpsilon_f64 = 0.99999999999999989;

struct RNG : Object {
    static sptr<RNG> create();

    virtual uint32_t u32() = 0;
    virtual float    f32() = 0;

    virtual uint32_t u32(uint32_t bound) = 0;
    virtual float    f32(float bound) = 0;
};
