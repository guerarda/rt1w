#ifndef BVH_H
#define BVH_H

#include "types.h"
#include "sptr.hpp"
#include "ray.hpp"
#include "hitable.hpp"

bool box_hit(const box_t &b, const sptr<ray> &r, float tmin, float tmax);

struct BVH_node : Hitable {

    static sptr<BVH_node> create(size_t count, sptr<Hitable> *l);

    virtual bool hit(const sptr<ray> &r, float min, float max, hit_record &rec) const = 0;
    virtual box_t bounding_box() const = 0;
};

#endif
