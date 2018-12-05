#pragma once

#include "shape.hpp"
#include "sptr.hpp"

struct Params;

struct Sphere : Shape {
    static sptr<Sphere> create(const v3f &center, float radius);
    static sptr<Sphere> create(const sptr<Params> &params);

    virtual v3f center() const = 0;
    virtual float radius() const = 0;
};
