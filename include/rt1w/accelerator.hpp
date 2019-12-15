#pragma once

#include "rt1w/primitive.hpp"
#include "rt1w/sptr.hpp"

#include <vector>

struct Accelerator : Aggregate {
    static sptr<Accelerator> create(const std::string &name,
                                    const std::vector<sptr<Primitive>> &v);
};

struct AcceleratorAsync : Aggregate {
    using Aggregate::intersect;
    using Aggregate::qIntersect;

    virtual sptr<Batch<Interaction>> intersect(const std::vector<Ray> &rays) const = 0;
    virtual sptr<Batch<Interaction>> qIntersect(const std::vector<Ray> &rays) const = 0;
};
