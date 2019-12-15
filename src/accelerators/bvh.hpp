#pragma once

#include "rt1w/accelerator.hpp"
#include "rt1w/sptr.hpp"

#include <vector>

struct BVHAccelerator : Accelerator {
    static sptr<BVHAccelerator> create(const std::vector<sptr<Primitive>> &v);
};
