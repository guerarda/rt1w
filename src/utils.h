#pragma once

#ifdef __cplusplus
#include <cstdint>

int32_t FloatCompare(float, float);
bool FloatEqual(float, float);

int32_t FloatCompare(double, double);
bool FloatEqual(double, double);

#else
#include <stdint.h>
#endif

__BEGIN_DECLS

int32_t f32_compare(float, float);
bool f32_equal(float, float);

int32_t f64_compare(double, double);
bool f64_equal(double, double);

__END_DECLS
