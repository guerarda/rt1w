#ifndef MATERIAL_H
#define MATERIAL_H

#include "hitable.hpp"
#include "texture.hpp"

struct material : Object {
    virtual bool scatter(const sptr<ray> &r_in,
                         const hit_record &rec,
                         v3f &attenuation,
                         sptr<ray> &scattered) const = 0;
};

struct lambertian : material {
    static sptr<lambertian> create(const sptr<Texture> &tex);
};

struct metal : material {
    static sptr<metal> create(const sptr<Texture> &tex, float fuzz);
};

struct dielectric : material {
    static sptr<dielectric> create(float ref_index);
};

#endif
