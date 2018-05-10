#pragma once

#include "sptr.hpp"
#include "geometry.hpp"

struct Params;
struct ray;

struct Camera : Object {
    static sptr<Camera> create(const v3f &eye,
                               const v3f &lookat,
                               const v3f &up,
                               v2u resolution,
                               float fov,
                               float aperture,
                               float focus_dist);

    static sptr<Camera> create(const sptr<Params> &p);

    virtual v2u resolution() const = 0;
    virtual sptr<ray> make_ray(float u, float v) const = 0;
};
