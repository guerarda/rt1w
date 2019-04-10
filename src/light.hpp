#pragma once

#include "geometry.hpp"
#include "interaction.hpp"
#include "sptr.hpp"

struct Params;
struct Ray;
struct Sampler;
struct Scene;
struct Shape;
struct Spectrum;

Spectrum LightEmitted(const Interaction &isect, const v3f &wi);

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

enum LightType {
    LIGHT_DELTA_POS = 1 << 0,
    LIGHT_DELTA_DIR = 1 << 1,
    LIGHT_AREA = 1 << 2,
    LIGHT_INIFINITE = 1 << 3,
};

struct Light : Object {
    static sptr<Light> create(const sptr<Params> &p);

    virtual LightType type() const = 0;
    virtual Spectrum sample_Li(const Interaction &isect,
                               v2f u,
                               v3f &wi,
                               float &pdf,
                               VisibilityTester &vis) const = 0;
    virtual Spectrum Le(const Ray &r) const = 0;
    virtual float pdf_Li(const Interaction &isect, const v3f &wi) const = 0;
};

struct PointLight : Light {
    static sptr<PointLight> create(const v3f &pos, const Spectrum &intensity);
    static sptr<PointLight> create(const sptr<Params> &p);
};

struct AreaLight : Light {
    static sptr<AreaLight> create(const sptr<Shape> &s, const Spectrum &Lemit);
    static sptr<AreaLight> create(const sptr<Params> &p);

    virtual Spectrum L(const Interaction &isect, const v3f &w) const = 0;
    virtual sptr<Shape> shape() const = 0;
};

bool IsDeltaLight(const sptr<Light> &light);
