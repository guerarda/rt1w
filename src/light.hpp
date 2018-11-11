#pragma once

#include "material.hpp"
#include "sptr.hpp"

struct Params;
struct Texture;

struct DiffuseLight : Material {

    static sptr<DiffuseLight> create(const sptr<Texture> &tex);
    static sptr<DiffuseLight> create(const sptr<Params> &p);

    virtual bool scatter(const sptr<Ray> &r_in,
                         const hit_record &rec,
                         v3f &attenuation,
                         v3f &) const override = 0;
    virtual v3f emitted(float u, float v, v3f p) const override = 0;
};
