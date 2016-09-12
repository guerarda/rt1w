#ifndef CAMERA_H
#define CAMERA_H

#include "sptr.hpp"
#include "ray.hpp"

struct camera;

struct camera {
    static sptr<camera> create(const v3f &eye,
                               const v3f &lookat,
                               const v3f &up,
                               float vfov,
                               float aspect);

    virtual sptr<ray> make_ray(float u, float v) const = 0;
};

#endif
