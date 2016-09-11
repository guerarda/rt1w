#ifndef MATERIAL_H
#define MATERIAL_H

#include "hitable.hpp"

struct material;

struct material {
    virtual bool scatter(const sptr<ray> &r_in,
                         const hit_record &rec,
                         v3f &attenuation,
                         sptr<ray> &scattered) const = 0;
};

struct lambertian : material {

    static sptr<lambertian> create(const v3f &albedo);

    virtual bool scatter(const sptr<ray> &r_in,
                         const hit_record &rec,
                         v3f &attenuation,
                         sptr<ray> &scattered) const = 0;
};

struct metal : material {

    static sptr<metal> create(const v3f &albedo, float fuzz);

    virtual bool scatter(const sptr<ray> &r_in,
                         const hit_record &rec,
                         v3f &attenuation,
                         sptr<ray> &scattered) const = 0;
};

#endif
