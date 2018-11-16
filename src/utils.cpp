#include "utils.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <type_traits>

template <typename T>
inline int32_t flt_compare(T a, T b)
{
    static_assert(std::is_floating_point<T>::value, "FloatCompare with non floating point type");
    if (std::abs(a - b) <= std::max(std::abs(a), std::abs(b)) * std::numeric_limits<T>::epsilon()) {
        return 0;
    }
    return a < b ? -1 : 1;
}

int32_t FloatCompare(float a, float b)
{
    return flt_compare(a, b);
}

int32_t FloatCompare(double a, double b)
{
    return flt_compare(a, b);
}

bool FloatEqual(float a, float b)
{
    return flt_compare(a, b) == 0;
}

bool FloatEqual(double a, double b)
{
    return flt_compare(a, b) == 0;
}

int32_t f32_compare(float a, float b)
{
    return flt_compare(a, b);
}

bool f32_equal(float a, float b)
{
    return flt_compare(a, b) == 0;
}

int32_t f64_compare(double a, double b)
{
    return flt_compare(a, b);
}

bool f64_equal(double a, double b)
{
    return flt_compare(a, b) == 0;
}
