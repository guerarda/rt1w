#pragma once

#include "geometry.hpp"
#include "sptr.hpp"

struct Fresnel : Object {
    virtual v3f eval(float cosThetaI) const = 0;
};

struct FresnelDielectric : Fresnel {
    static uptr<FresnelDielectric> create(float etaI, float etaT);
};

struct FresnelConductor : Fresnel {
    static uptr<FresnelConductor> create(v3f etaI, v3f etaT, v3f k);
};
