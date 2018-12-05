#pragma once

#include "geometry.hpp"
#include "sptr.hpp"

#include <string>
#include <vector>

struct Interaction;
struct Material;
struct Ray;
struct Shape;

struct Primitive : Object {
    static sptr<Primitive> create(const sptr<Shape> &s, const sptr<Material> &m);
    static sptr<Primitive> load_obj(const std::string &path);

    virtual bool intersect(const sptr<Ray> &r,
                           float min,
                           float max,
                           Interaction &isect) const = 0;
    virtual bounds3f bounds() const = 0;
};

struct Aggregate : Primitive {
    static sptr<Aggregate> create(const std::vector<sptr<Primitive>> &primitives);

    virtual const std::vector<sptr<Primitive>> &primitives() const = 0;
};
