#include "sampling.hpp"

#include "utils.hpp"

v2f UniformSampleDisk(const v2f &u)
{
    double r = std::sqrt(u.x);
    double a = 2 * Pi * u.y;

    return { (float)(r * std::cos(a)), (float)(r * std::sin(a)) };
}

v3f UniformSampleSphere(const v2f &u)
{
    float z = 1.0f - 2.0f * u.x;
    float r = std::sqrt(std::max(0.0f, 1.0f - z * z));
    float phi = 2.0f * (float)Pi * u.y;

    return { r * std::cos(phi), r * std::sin(phi), z };
}

v2f UniformSampleTriangle(const v2f &u)
{
    float su0 = std::sqrt(u.x);
    return { 1.0f - su0, u.y * su0 };
}
