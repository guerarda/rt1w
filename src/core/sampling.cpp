#include "rt1w/sampling.hpp"

#include "rt1w/utils.hpp"

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

v3f CosineSampleHemisphere(const v2f &u)
{
    v2f d = UniformSampleDisk(u);
    float z = std::sqrt(std::max(.0f, 1.f - d.x * d.x - d.y * d.y));

    return { d.x, d.y, z };
}

float CosineHemispherePdf(float cosTheta)
{
    return (float)(cosTheta * InvPi);
}

float PowerHeuristic(int32_t nf, float fpdf, int32_t ng, float gpdf)
{
    float f = nf * fpdf;
    float g = ng * gpdf;

    return (f * f) / (f * f + g * g);
}
