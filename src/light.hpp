#pragma once

#include "geometry.hpp"
#include "interaction.hpp"
#include "sptr.hpp"

struct Params;
struct Ray;
struct Sampler;
struct Scene;
struct Shape;

v3f LightEmitted(const Interaction &isect, const v3f &wi);

struct VisibilityTester {
    VisibilityTester() = default;
    VisibilityTester(Interaction p0, Interaction p1) : m_p0(p0), m_p1(p1)
    {
        ASSERT(!HasNaN(m_p0));
        ASSERT(!HasNaN(m_p1));
    }

    bool visible(const sptr<Scene> &scene) const;

private:
    Interaction m_p0;
    Interaction m_p1;
};

struct Light : Object {
    static sptr<Light> create(const sptr<Params> &p);

    virtual v3f sample_Li(const Interaction &isect,
                          v2f u,
                          v3f &wi,
                          VisibilityTester &vis) const = 0;
    virtual v3f Le(const Ray &r) const = 0;
};

struct PointLight : Light {
    static sptr<PointLight> create(const v3f &pos, const v3f &intensity);
    static sptr<PointLight> create(const sptr<Params> &p);
};

struct AreaLight : Light {
    static sptr<AreaLight> create(const sptr<Shape> &s, const v3f &Lemit);
    static sptr<AreaLight> create(const sptr<Params> &p);

    virtual v3f L(const Interaction &isect, const v3f &w) const = 0;
    virtual sptr<Shape> shape() const = 0;
};
