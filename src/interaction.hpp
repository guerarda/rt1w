#pragma once

#include "geometry.hpp"
#include "sptr.hpp"

struct Material;
struct Primitive;

struct Interaction {
    Interaction() = default;
    Interaction(v3f p) : p(p) {}

    v3f p;
    v3f wo;
    v3f n;
    v2f uv;
    v3f dpdu;
    v3f dpdv;
    v3f error;
    float t = .0f;
    struct {
        v3f n;
        v3f dpdu;
        v3f dpdv;
    } shading;
    sptr<Material> mat;
    sptr<Primitive> prim;
};

inline bool HasNaN(const Interaction &i)
{
    return HasNaN(i.p) || HasNaN(i.uv) || HasNaN(i.wo) || HasNaN(i.n) || HasNaN(i.dpdu)
           || HasNaN(i.dpdv) || HasNaN(i.error) || IsNaN(i.t) || HasNaN(i.shading.n)
           || HasNaN(i.shading.dpdu) || HasNaN(i.shading.dpdv);
}
