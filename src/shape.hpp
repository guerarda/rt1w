#pragma once

#include "sptr.hpp"
#include "geometry.hpp"

struct ray;
struct hit_record;
struct Params;

struct Shape : Object {
    static sptr<Shape> create(const sptr<Params> &p);

    virtual bool      hit(const sptr<ray> &r,
                          float min,
                          float max,
                          hit_record &rec) const = 0;
    virtual bounds3f  bounds() const = 0;
 };
