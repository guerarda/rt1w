#ifndef HITABLE_H
#define HITABLE_H

#include "ray.hpp"
#include "types.h"

struct Material;

struct hit_record {
    float t;
    v2f   uv;
    v3f   p;
    v3f   normal;
    sptr<Material> mat;
};

struct Hitable : Object {
    static sptr<Hitable> create(const std::vector<sptr<Hitable>> &v);

    virtual bool      hit(const sptr<ray> &r, float min, float max, hit_record &rec) const = 0;
    virtual bounds3f  bounds() const = 0;
 };

#endif
