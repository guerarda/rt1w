#include "rt1w/accelerator.hpp"
#include "rt1w/error.h"
#include "rt1w/primitive.hpp"

#include "accelerators/bvh.hpp"
#include "accelerators/qbvh.cpp"

sptr<Accelerator> Accelerator::create(const std::string &name,
                                      const std::vector<sptr<Primitive>> &v)
{
    if (name == "qbvh") {
        return QBVHAccelerator::create(v);
    }
    if (name == "bvh") {
        return BVHAccelerator::create(v);
    }
    WARNING("Unknown accelerator named %s", name.c_str());

    return nullptr;
}
