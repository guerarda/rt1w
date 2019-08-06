#pragma once

#include "shape.hpp"
#include "sptr.hpp"

struct Params;
struct Transform;

struct Sphere : Shape {
    static sptr<Sphere> create(const Transform &worldToObj, float radius);
    static sptr<Sphere> create(const sptr<Params> &params);

    virtual float radius() const = 0;
};
