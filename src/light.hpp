#pragma once

#include "geometry.hpp"
#include "sptr.hpp"

struct Params;
struct Ray;
struct Sampler;
struct Scene;
struct Interaction;

struct VisibilityTester {
    VisibilityTester() = default;
    VisibilityTester(v3f p0, v3f p1) : m_p0(std::move(p0)), m_p1(std::move(p1)) {}
    VisibilityTester(v3f &&p0, v3f &&p1) : m_p0(p0), m_p1(p1) {}

    bool visible(const sptr<Scene> &scene) const;

private:
    v3f m_p0;
    v3f m_p1;
};

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
