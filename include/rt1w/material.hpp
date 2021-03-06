#pragma once

#include "rt1w/geometry.hpp"
#include "rt1w/sptr.hpp"

struct BSDF;
struct Ray;
struct Interaction;
struct Params;
struct Spectrum;
struct Texture;

#pragma mark - Interaction

sptr<BSDF> ComputeBSDF(const Interaction &isect);

#pragma mark - Material Interface

struct Material : Object {
    static sptr<Material> create(const sptr<Params> &p);

    virtual Spectrum f(const Interaction &isect, const v3f &wo, const v3f &wi) const = 0;
    virtual sptr<BSDF> computeBsdf(const Interaction &isect) const = 0;
};

struct Lambertian : Material {
    static sptr<Lambertian> create(const sptr<Texture> &Kd);
    static sptr<Lambertian> create(const sptr<Params> &params);
};

struct Metal : Material {
    static sptr<Metal> create(const sptr<Texture> &tex, float fuzz);
    static sptr<Metal> create(const sptr<Params> &params);
};

struct Dielectric : Material {
    static sptr<Dielectric> create(float ri);
    static sptr<Dielectric> create(const sptr<Params> &params);
};
