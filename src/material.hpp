#ifndef MATERIAL_H
#define MATERIAL_H

#include "primitive.hpp"
#include "texture.hpp"

struct Params;

struct Material : Object {
    static sptr<Material> create(const sptr<Params> &p);

    virtual bool scatter(const sptr<ray> &r_in,
                         const hit_record &rec,
                         v3f &attenuation,
                         sptr<ray> &scattered) const = 0;
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

#endif
