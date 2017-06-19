#ifndef BVH_H
#define BVH_H

#include "types.h"
#include "sptr.hpp"
#include "ray.hpp"
#include "hitable.hpp"

struct BVH_node : Hitable {

    static sptr<BVH_node> create(const std::vector<sptr<Hitable>> &v);

    virtual bool hit(const sptr<ray> &r, float min, float max, hit_record &rec) const = 0;
    virtual bounds3f bounds() const = 0;
};

#endif
