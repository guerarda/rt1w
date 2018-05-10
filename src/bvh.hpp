#pragma once

#include "types.h"
#include "sptr.hpp"
#include "ray.hpp"
#include "primitive.hpp"

struct BVHNode : Primitive {

    static sptr<BVHNode> create(const std::vector<sptr<Primitive>> &v);

    virtual bool     hit(const sptr<ray> &r, float min, float max, hit_record &rec) const = 0;
    virtual bounds3f bounds() const = 0;
};

struct BVHAccelerator : Aggregate {

    static sptr<BVHAccelerator> create(const std::vector<sptr<Primitive>> &v);
};
