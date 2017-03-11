#ifndef SPHERE_H
#define SPHERE_H

#include "hitable.hpp"
#include "sptr.hpp"

struct sphere;

struct sphere : hitable {

    static sptr<sphere> create(const v3f &center, float radius, const sptr<material> &mat);

    virtual bool  hit(const sptr<ray> &r, float min, float max, hit_record &rec) const = 0;
    virtual box   bounding_box() const = 0;
    virtual v3f   center() const = 0;
    virtual float radius() const = 0;
};

#endif
