#pragma once

#include "rt1w/geometry.hpp"
#include "rt1w/sptr.hpp"
#include "rt1w/utils.hpp"

#include <vector>

struct Interaction;
struct Params;
struct Ray;
struct Transform;

struct Shape : Object {
    static sptr<Shape> create(const sptr<Params> &p);

    virtual bool intersect(const Ray &r, Interaction &isect) const = 0;
    virtual bool qIntersect(const Ray &r) const = 0;

    virtual float area() const = 0;
    virtual bounds3f bounds() const = 0;
    virtual Transform worldToObj() const = 0;

    virtual Interaction sample(const v2f &u) const = 0;
    virtual float pdf() const = 0;

    virtual Interaction sample(const Interaction &ref, const v2f &u) const = 0;
    virtual float pdf(const Interaction &ref, const v3f &wi) const = 0;
};

struct Group : Shape {
    virtual std::vector<sptr<Shape>> faces() const = 0;
};
