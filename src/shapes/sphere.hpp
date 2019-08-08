#pragma once

#include "rt1w/shape.hpp"
#include "rt1w/sptr.hpp"

struct Params;
struct Transform;

struct Sphere : Shape {
    static sptr<Sphere> create(const Transform &worldToObj, float radius);
    static sptr<Sphere> create(const sptr<Params> &params);

    virtual float radius() const = 0;
};
