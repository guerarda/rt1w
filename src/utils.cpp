#include "utils.h"
#include <limits>
#include <cmath>
#include <algorithm>

template <typename T>
static inline int32_t float_compare(T a, T b)
{
    if (std::abs(a - b) <= std::max(std::abs(a), std::abs(b)) * std::numeric_limits<T>::epsilon()) {
        return 0;
    }
    return a < b ? -1 : 1;
}

int32_t f32_compare(float a, float b)
{
    return float_compare<float>(a, b);
}

bool f32_equal(float a, float b)
{
    return f32_compare(a, b) == 0;
}

int32_t f64_compare(double a, double b)
{
    return float_compare<double>(a, b);
}

bool f64_equal(double a, double b)
{
    return f64_compare(a, b) == 0;
}
