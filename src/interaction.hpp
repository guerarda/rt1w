#pragma once

#include "geometry.hpp"
#include "sptr.hpp"

struct Material;
struct Primitive;

struct Interaction {
    v3f p;
    float t;
    v2f uv;
    v3f wo;
    v3f n;
    v3f dpdu;
    v3f dpdv;
    struct {
        v3f n;
        v3f dpdu;
        v3f dpdv;
    } shading;
    sptr<Material> mat;
    sptr<Primitive> prim;
};
