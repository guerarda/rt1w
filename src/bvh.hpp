#pragma once

#include "primitive.hpp"
#include "sptr.hpp"

#include <vector>

struct BVHAccelerator : Aggregate {
    static sptr<BVHAccelerator> create(const std::vector<sptr<Primitive>> &v);
};
