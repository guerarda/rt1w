#pragma once

#include "geometry.hpp"
#include "sptr.hpp"

struct Spectrum;

struct Fresnel : Object {
    virtual Spectrum eval(float cosThetaI) const = 0;
};

struct FresnelDielectric : Fresnel {
    static uptr<FresnelDielectric> create(float etaI, float etaT);
};

struct FresnelConductor : Fresnel {
    static uptr<FresnelConductor> create(const Spectrum &etaI,
                                         const Spectrum &etaT,
                                         const Spectrum &k);
};
