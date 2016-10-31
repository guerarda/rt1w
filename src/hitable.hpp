#ifndef HITABLE_H
#define HITABLE_H

#include "ray.hpp"

struct Material;

struct hit_record {
    float t;
    v2f   uv;
    v3f   p;
    v3f   normal;
    sptr<Material> mat;
};

struct Hitable : Object {
     virtual bool hit(const sptr<ray> &r, float min, float max, hit_record &rec) const = 0;
     virtual box_t bounding_box() const = 0;
 };

#endif
