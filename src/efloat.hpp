#pragma once

#include "error.h"
#include "utils.hpp"

#include <cmath>

struct EFloat {
    EFloat() = default;
    EFloat(float v, float e = .0f) : v(v), e(e) {}

    explicit operator float() const { return v; }
    explicit operator double() const { return v; }

    float lo() const { return v - e; }
    float hi() const { return v + e; }

    float min() const { return std::abs(v > .0f ? lo() : hi()); }
    float max() const { return std::abs(v > .0f ? hi() : lo()); }

    float error() const { return e; }

    EFloat operator+(EFloat ef) const
    {
        float a = Epsilon * std::fmax(std::abs(lo() + ef.lo()), std::abs(hi() + ef.hi()));
        float b = (1 + Epsilon) * (e + ef.e);
        return { v + ef.v, MinReal + a + b };
    }
    EFloat operator-(EFloat ef) const
    {
        float a = Epsilon * std::fmax(std::abs(lo() - ef.hi()), std::abs(hi() - ef.lo()));
        float b = (1 + Epsilon) * (e + ef.e);
        return { v - ef.v, MinReal + a + b };
    }
    EFloat operator*(EFloat ef) const
    {
        float a = Epsilon * max() * ef.max();
        float b = (1 + Epsilon) * (max() * ef.e + ef.max() * e + e * ef.e);
        return { v * ef.v, MinReal + a + b };
    }
    EFloat operator/(EFloat ef) const
    {
        ASSERT(ef.e < .5f * ef.min());
        float a = 1 / (ef.min() - ef.e);
        float b = ef.e / ef.min();
        float c = Epsilon + b + 2 * (b * b);
        return { v / ef.v, MinReal + a * (e + (max() + e) * c) };
    }
    EFloat operator-() const { return { -v, e }; }

    friend inline EFloat sqrt(EFloat ef);
    friend inline EFloat abs(EFloat ef);

private:
    float v;    // Interval mid
    float e;    // Error bound
};

inline EFloat operator+(float f, EFloat ef)
{
    return EFloat(f) + ef;
}

inline EFloat operator-(float f, EFloat ef)
{
    return EFloat(f) - ef;
}
inline EFloat operator*(float f, EFloat ef)
{
    return EFloat(f) * ef;
}

inline EFloat operator/(float f, EFloat ef)
{
    return EFloat(f) / ef;
}
inline bool Quadratic(EFloat a, EFloat b, EFloat c, EFloat &t0, EFloat &t1);

EFloat sqrt(EFloat ef)
{
    ASSERT(ef.lo() > ef.error() && ef.hi() > ef.error());
    float a = ef.e * (1.0f + Epsilon);
    float hlo = Epsilon * std::sqrt(ef.lo()) + a / (2 * std::sqrt(ef.lo() - ef.e));
    float hhi = Epsilon * std::sqrt(ef.hi()) + a / (2 * std::sqrt(ef.hi() - ef.e));

    return { std::sqrt(ef.v), std::fmaxf(hlo, hhi) };
}

EFloat abs(EFloat ef)
{
    if (ef.lo() >= 0) {
        return ef;
    }
    if (ef.hi() <= 0) {
        return { -ef.v, ef.e };
    }
    return { std::abs(ef.v), ef.max() };
}

bool Quadratic(EFloat a, EFloat b, EFloat c, EFloat &t0, EFloat &t1)
{
    double delta = (double)b * (double)b - 4 * (double)a * (double)c;
    if (delta >= 0.0) {
        EFloat sqrtDelta = sqrt(EFloat((float)delta));
        EFloat q = (float)b < .0f ? -.5f * (b - sqrtDelta) : -.5 * (b + sqrtDelta);
        t0 = q / a;
        t1 = c / q;
        if ((float)t0 > (float)t1) {
            std::swap(t0, t1);
        }
        return true;
    }
    return false;
}
