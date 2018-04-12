#pragma once

#include "ray.hpp"
#include "types.h"

struct Material;
struct Shape;

struct hit_record {
    float t;
    v2f   uv;
    v3f   p;
    v3f   normal;
    sptr<Material> mat;
};

struct Primitive : Object {
    static sptr<Primitive> create(const sptr<Shape> &s,
                                  const sptr<Material> &m);

    virtual bool      hit(const sptr<ray> &r,
                          float min,
                          float max,
                          hit_record &rec) const = 0;
    virtual bounds3f  bounds() const = 0;
 };
