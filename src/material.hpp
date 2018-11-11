#pragma once

#include "primitive.hpp"


struct Params;
struct Texture;

struct Material : Object {
    static sptr<Material> create(const sptr<Params> &p);

    virtual v3f  f(const hit_record &, const v3f &wo, const v3f &wi) const = 0;
    virtual bool scatter(const sptr<Ray> &r_in,
                         const hit_record &rec,
                         v3f &attenuation,
                         v3f &wi) const = 0;
    virtual v3f emitted(float u, float v, v3f p) const = 0;
};

struct Lambertian : Material {
    static sptr<Lambertian> create(const sptr<Texture> &tex);
    static sptr<Lambertian> create(const sptr<Params> &params);
};

struct Metal : Material {
    static sptr<Metal> create(const sptr<Texture> &tex, float fuzz);
    static sptr<Metal> create(const sptr<Params> &params);
};

struct Dielectric : Material {
    static sptr<Dielectric> create(float ref_index);
    static sptr<Dielectric> create(const sptr<Params> &params);
};
