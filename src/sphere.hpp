#ifndef SPHERE_H
#define SPHERE_H

#include "shape.hpp"
#include "sptr.hpp"

struct Params;

struct Sphere : Shape {

    static sptr<Sphere> create(const v3f &center, float radius);
    static sptr<Sphere> create(const sptr<Params> &params);

    virtual bool      hit(const sptr<Ray> &r, float min, float max, hit_record &rec) const = 0;
    virtual bounds3f  bounds() const = 0;
    virtual v3f       center() const = 0;
    virtual float     radius() const = 0;
};

#endif
