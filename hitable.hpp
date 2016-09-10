#ifndef HITABLE_H
#define HITABLE_H

#include "ray.hpp"

struct hit_record {
    float t;
    v3f   p;
    v3f   normal;
};

 struct hitable {
     virtual bool hit(const sptr<ray> &r, float min, float max, hit_record &rec) const = 0;
 };

#endif
