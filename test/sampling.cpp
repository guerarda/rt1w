#include "catch.hpp"

#include "rt1w/sampling.hpp"
#include "rt1w/utils.hpp"

#include <random>

static v2f randomSample()
{
    static std::random_device rd;
    static std::mt19937 __prng(rd());
    static std::uniform_real_distribution<float> dist(0.0, 1.0f);

    return { dist(__prng), dist(__prng) };
}

/* Verify that the point sampled are always inside the shape.
 * This doesn't test the uniformity of the ditribution.
 */

TEST_CASE("Disk Sampling")
{
    size_t n = 0;
    for (size_t i = 0; i < 10000; ++i) {
        v2f p = UniformSampleDisk(randomSample());
        float r = p.x * p.x + p.y * p.y;
        if (r > 1.0f && r != Approx(1.0f)) {
            ++n;
        }
    }
    CHECK(n == 0);
}

TEST_CASE("Sphere Sampling")
{
    size_t n = 0;
    for (size_t i = 0; i < 10000; ++i) {
        v3f p = UniformSampleSphere(randomSample());
        float r = p.x * p.x + p.y * p.y + p.z * p.z;
        if (r > 1.0f && r != Approx(1.0f)) {
            ++n;
        }
    }
    CHECK(n == 0);
}

TEST_CASE("Triangle Sampling")
{
    size_t n = 0;
    for (size_t i = 0; i < 10000; ++i) {
        v2f p = UniformSampleTriangle(randomSample());
        float e0 = p.x;
        float e1 = p.y;
        float e2 = 1.0f - e0 - e1;
        if (e0 < 0.0f || e0 > 1.0f || e1 < 0.0f || e1 > 1.0f || e2 < 0.0f || e2 > 1.0f) {
            ++n;
        }
    }
    CHECK(n == 0);
}
