#ifndef MATERIAL_H
#define MATERIAL_H

#include "hitable.hpp"
#include "texture.hpp"

struct Material : Object {
    virtual bool scatter(const sptr<ray> &r_in,
                         const hit_record &rec,
                         v3f &attenuation,
                         sptr<ray> &scattered) const = 0;
};

struct Lambertian : Material {
    static sptr<Lambertian> create(const sptr<Texture> &tex);
};

struct Metal : Material {
    static sptr<Metal> create(const sptr<Texture> &tex, float fuzz);
};

struct Dielectric : Material {
    static sptr<Dielectric> create(float ref_index);
};

#endif
