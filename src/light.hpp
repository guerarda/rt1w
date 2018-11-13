#pragma once

#include <vector>

#include "geometry.hpp"
#include "sptr.hpp"

struct Params;
struct Ray;
struct Sampler;
struct Scene;
struct hit_record;

struct Light : Object {
    static sptr<Light> create(const sptr<Params> &p);

    virtual v3f sample_Li(const hit_record &rec, v3f &wi) const = 0;
    virtual v3f Le(const sptr<Ray> &r) const = 0;
    virtual bool visible(const hit_record &rec, const sptr<Scene> &scene) const = 0;
};

struct PointLight : Light {
    static sptr<PointLight> create(const v3f &pos, const v3f &intensity);
    static sptr<PointLight> create(const sptr<Params> &p);
};

v3f EstimateDirect(const hit_record &rec,
                   const sptr<Light> &light,
                   const sptr<Scene> &scene,
                   const sptr<Sampler> &sampler);
v3f UniformSampleOneLight(const hit_record &rec,
                          const sptr<Scene> &scene,
                          const sptr<Sampler> &sampler);
