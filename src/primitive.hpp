#pragma once

#include "geometry.hpp"
#include "sptr.hpp"
#include "utils.hpp"

#include <string>
#include <vector>

struct AreaLight;
struct Interaction;
struct Material;
struct Ray;
struct Shape;

struct Primitive : Object {
    static sptr<Primitive> create(const sptr<Shape> &s,
                                  const sptr<Material> &m,
                                  const sptr<AreaLight> &l = nullptr);
    static sptr<Primitive> load_obj(const std::string &path);

    virtual bool intersect(const Ray &r,
                           Interaction &isect,
                           float max = Infinity) const = 0;
    virtual bounds3f bounds() const = 0;
    virtual sptr<AreaLight> light() const = 0;
};

struct Aggregate : Primitive {
    static sptr<Aggregate> create(const std::vector<sptr<Primitive>> &primitives);

    virtual const std::vector<sptr<Primitive>> &primitives() const = 0;
};
