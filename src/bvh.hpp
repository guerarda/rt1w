#pragma once

#include "geometry.hpp"
#include "primitive.hpp"
#include "sptr.hpp"

struct Ray;

struct BVHNode : Primitive {
    static sptr<BVHNode> create(const std::vector<sptr<Primitive>> &v);

    bool intersect(const sptr<Ray> &r,
                   float min,
                   float max,
                   Interaction &isect) const override = 0;
    bounds3f bounds() const override = 0;
};

struct BVHAccelerator : Aggregate {
    static sptr<BVHAccelerator> create(const std::vector<sptr<Primitive>> &v);
};
