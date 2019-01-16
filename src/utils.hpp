#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <type_traits>

#pragma mark - Mathematical constants

constexpr double Pi = 3.14159265358979323846264338327950288;
constexpr double PiOver2 = 1.57079632679489661923132169163975144;
constexpr double PiOver4 = 0.785398163397448309615660845819875721;
constexpr double InvPi = 0.318309886183790671537767526745028724;
constexpr double Inv2Pi = 0.636619772367581343075535053490057448;
constexpr double Sqrt2 = 1.41421356237309504880168872420969808;

#pragma mark - Floating Point comparisons

template <typename T>
inline int32_t FloatCompare(T a, T b)
{
    static_assert(std::is_floating_point<T>::value,
                  "FloatCompare with non floating-point type");
    if (std::abs(a - b)
        <= std::max(std::abs(a), std::abs(b)) * std::numeric_limits<T>::epsilon()) {
        return 0;
    }
    return a < b ? -1 : 1;
}

template <typename T>
inline bool FloatEqual(T a, T b)
{
    return FloatCompare(a, b) == 0;
}

#pragma mark - Floating Point Misc
inline uint32_t FloatToBits(float f)
{
    uint32_t ui;
    memcpy(&ui, &f, sizeof(f));
    return ui;
}

inline float BitsToFloat(uint32_t ui)
{
    float f;
    memcpy(&f, &ui, sizeof(ui));
    return f;
}

inline uint64_t FloatToBits(double f)
{
    uint64_t ui;
    memcpy(&ui, &f, sizeof(f));
    return ui;
}

inline double BitsToFloat(uint64_t ui)
{
    double f;
    memcpy(&f, &ui, sizeof(ui));
    return f;
}

template <typename T>
inline T NextFloatUp(T f)
{
    static_assert(std::is_floating_point<T>::value,
                  "Floating Point function with non-floating point argument");
    return std::nexttoward(f, std::numeric_limits<T>::infinity());
}

template <typename T>
inline T NextFloatDown(T f)
{
    static_assert(std::is_floating_point<T>::value,
                  "Floating Point function with non-floating point argument");
    return std::nexttoward(f, -std::numeric_limits<T>::infinity());
}

#pragma mark - Trigonometric functions

inline float Radians(float deg)
{
    return (float)(Pi / 180.0 * deg);
}

inline float Degrees(float rad)
{
    return (float)(180.0 / Pi * rad);
}

#pragma mark - Misc

template <typename T>
inline T Lerp(T t, T va, T vb)
{
    return (1 - t) * va + t * vb;
}

template <typename T, typename U, typename V>
inline T Clamp(T v, U lo, V hi)
{
    return (T)(v < lo ? lo : (v > hi ? hi : v));
}

inline bool Quadratic(double a, double b, double c, double &t0, double &t1)
{
    double delta = b * b - 4 * a * c;
    if (delta >= 0.0) {
        double inv2a = 0.5 / a;
        double sqrtDelta = std::sqrt(delta);
        t0 = (-b - sqrtDelta) * inv2a;
        t1 = (-b + sqrtDelta) * inv2a;

        return true;
    }
    return false;
}
