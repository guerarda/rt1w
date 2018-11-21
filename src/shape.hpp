#pragma once

#include "geometry.hpp"
#include "sptr.hpp"

struct hit_record;
struct Params;
struct Ray;

struct Shape : Object {
    static sptr<Shape> create(const sptr<Params> &p);

    virtual bool hit(const sptr<Ray> &r, float min, float max, hit_record &rec) const = 0;
    virtual bounds3f bounds() const = 0;
};
