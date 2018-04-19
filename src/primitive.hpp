#pragma once

#include "ray.hpp"
#include "types.h"

#include <vector>

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
    static sptr<Primitive> load_obj(const std::string &path);

    virtual bool      hit(const sptr<ray> &r,
                          float min,
                          float max,
                          hit_record &rec) const = 0;
    virtual bounds3f  bounds() const = 0;
 };

struct Aggregate : Primitive {

    static sptr<Aggregate> create(const std::vector<sptr<Primitive>> prims);

    virtual const std::vector<sptr<Primitive>> &primitives() const = 0;
};
