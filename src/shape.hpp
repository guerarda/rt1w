#pragma once

#include "geometry.hpp"
#include "interaction.hpp"
#include "sptr.hpp"
#include "utils.hpp"

struct Params;
struct Ray;

struct Shape : Object {
    static sptr<Shape> create(const sptr<Params> &p);

    virtual bool intersect(const Ray &r,
                           Interaction &isect,
                           float max = Infinity) const = 0;
    virtual bounds3f bounds() const = 0;
    virtual Interaction sample(const v2f &u) const = 0;
};
