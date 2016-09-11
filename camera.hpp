#ifndef CAMERA_H
#define CAMERA_H

#include "sptr.hpp"
#include "ray.hpp"

struct camera;

struct camera {
    static sptr<camera> create(const v3f &bottom_left,
                               const v3f &h,
                               const v3f &v,
                               const v3f &org);

    virtual sptr<ray> make_ray(float u, float v) const = 0;
};

#endif
