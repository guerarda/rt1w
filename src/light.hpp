#pragma once

#include <vector>

#include "geometry.hpp"
#include "sptr.hpp"

struct Params;
struct Ray;
struct Sampler;
struct Scene;
struct Interaction;

struct Light : Object {
    static sptr<Light> create(const sptr<Params> &p);

    virtual v3f sample_Li(const Interaction &isect, v3f &wi) const = 0;
    virtual v3f Le(const sptr<Ray> &r) const = 0;
    virtual bool visible(const Interaction &isect, const sptr<Scene> &scene) const = 0;
};

struct PointLight : Light {
    static sptr<PointLight> create(const v3f &pos, const v3f &intensity);
    static sptr<PointLight> create(const sptr<Params> &p);
};

v3f EstimateDirect(const Interaction &isect,
                   const sptr<Light> &light,
                   const sptr<Scene> &scene,
                   const sptr<Sampler> &sampler);
v3f UniformSampleOneLight(const Interaction &isect,
                          const sptr<Scene> &scene,
                          const sptr<Sampler> &sampler);
