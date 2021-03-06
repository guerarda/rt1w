#pragma once

#include "rt1w/geometry.hpp"
#include "rt1w/sptr.hpp"
#include "rt1w/task.hpp"
#include "rt1w/transform.hpp"

#include <string>
#include <vector>

struct AreaLight;
struct Interaction;
struct Material;
struct Params;
struct Ray;
struct Shape;
struct Transform;

struct Primitive : Object {
    static sptr<Primitive> create(const sptr<Params> &p);
    static sptr<Primitive> create(const sptr<Shape> &s,
                                  const sptr<Material> &m,
                                  const sptr<AreaLight> &l = nullptr);
    static sptr<Primitive> load_obj(const std::string &path, const Transform &xform = {});

    virtual bounds3f bounds() const = 0;
    virtual sptr<AreaLight> light() const = 0;

    virtual bool intersect(const Ray &r, Interaction &isect) const = 0;
    virtual bool qIntersect(const Ray &r) const = 0;
};

struct Aggregate : Primitive {
    static sptr<Aggregate> create(const std::vector<sptr<Primitive>> &primitives);

    virtual const std::vector<sptr<Primitive>> &primitives() const = 0;
};
