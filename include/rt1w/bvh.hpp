#pragma once

#include "rt1w/primitive.hpp"
#include "rt1w/sptr.hpp"

#include <vector>

struct BVHAccelerator : Aggregate {
    static sptr<BVHAccelerator> create(const std::vector<sptr<Primitive>> &v);
};
