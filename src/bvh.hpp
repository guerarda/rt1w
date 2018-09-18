#pragma once

#include "geometry.hpp"
#include "primitive.hpp"
#include "sptr.hpp"

struct Ray;

struct BVHNode : Primitive {

    static sptr<BVHNode> create(const std::vector<sptr<Primitive>> &v);

    virtual bool     hit(const sptr<Ray> &r, float min, float max, hit_record &rec) const = 0;
    virtual bounds3f bounds() const = 0;
};

struct BVHAccelerator : Aggregate {

    static sptr<BVHAccelerator> create(const std::vector<sptr<Primitive>> &v);
};
