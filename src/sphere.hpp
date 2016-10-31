#ifndef SPHERE_H
#define SPHERE_H

#include "hitable.hpp"
#include "sptr.hpp"

struct Sphere : Hitable {

    static sptr<Sphere> create(const v3f &center, float radius, const sptr<Material> &mat);

    virtual bool  hit(const sptr<ray> &r, float min, float max, hit_record &rec) const = 0;
    virtual box_t   bounding_box() const = 0;
    virtual v3f   center() const = 0;
    virtual float radius() const = 0;
};

#endif
