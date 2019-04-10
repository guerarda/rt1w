#pragma once

#include "geometry.hpp"

v2f UniformSampleDisk(const v2f &u);
v3f UniformSampleSphere(const v2f &u);
v2f UniformSampleTriangle(const v2f &u);

float PowerHeuristic(int32_t nf, float fpdf, int32_t ng, float gpdf);
