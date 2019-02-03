#pragma once

#include "geometry.hpp"
#include "sptr.hpp"
#include "utils.hpp"

struct Interaction;
struct Params;
struct Ray;
struct Transform;

struct Shape : Object {
    static sptr<Shape> create(const sptr<Params> &p);

    virtual bool intersect(const Ray &r,
                           Interaction &isect,
                           float max = Infinity) const = 0;
    virtual bool qIntersect(const Ray &r, float max = Infinity) const = 0;

    virtual bounds3f bounds() const = 0;
    virtual Transform worldToObj() const = 0;
    virtual Interaction sample(const v2f &u) const = 0;
};
