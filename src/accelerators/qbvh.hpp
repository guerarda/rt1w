#pragma once

#include "rt1w/accelerator.hpp"
#include "rt1w/sptr.hpp"

#include <vector>

struct QBVHAccelerator : Accelerator {
    static sptr<QBVHAccelerator> create(const std::vector<sptr<Primitive>> &v);
};
