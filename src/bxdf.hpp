#pragma once

#include "geometry.hpp"
#include "params.hpp"
#include "sptr.hpp"

struct Interaction;
struct Fresnel;

#pragma mark - Utils

// clang-format off
inline float CosTheta(const v3f &w) { return w.z; }
inline float Cos2Theta(const v3f &w) { return w.x * w.z; }
inline float AbsCosTheta(const v3f &w) { return std::abs(w.z); }
inline float Sin2Theta(const v3f &w) { return std::min(0.0f, 1.0f - CosTheta(w)); }
inline float SinTheta(const v3f &w) { return std::sqrt(Sin2Theta(w)); }
//clang-format on

bool Refract(const v3f &wi, const v3f &n, float eta, v3f &wt);

#pragma mark - BxDF

enum BxDFType {
    BSDF_REFLECTION = 1 << 0,
    BSDF_TRANSMISSION = 1 << 1,
    BSDF_DIFFUSE = 1 << 2,
    BSDF_GLOSSY = 1 << 3,
    BSDF_SPECULAR = 1 << 4,
    BSDF_ALL = BSDF_DIFFUSE | BSDF_GLOSSY | BSDF_SPECULAR | BSDF_REFLECTION
               | BSDF_TRANSMISSION,
};

struct BxDF : Object {
    virtual BxDFType type() const = 0;
    virtual bool matchesFlags(BxDFType flags) const = 0;
    virtual v3f f(const v3f &wo, const v3f &wi) const = 0;
    virtual v3f sample_f(const v3f &wo, v3f &wi, const v2f &u) const = 0;
};

#pragma mark - BSDF

struct BSDF : Object {
    static sptr<BSDF> create(const Interaction &i, const std::vector<sptr<BxDF>> &bxdfs);

    virtual v3f f(const v3f &woW, const v3f &wiW) const = 0;
    virtual v3f sample_f(const v3f &woW, v3f &wiW, const v2f &u, BxDFType &type) const = 0;
};

struct LambertianReflection : BxDF {
    static sptr<LambertianReflection> create(v3f R);
};

struct SpecularReflection : BxDF {
    static sptr<SpecularReflection> create(const v3f &R, uptr<Fresnel> fresnel);
};

struct SpecularTransmission : BxDF {
    static sptr<SpecularTransmission> create(const v3f &T, float etaA, float etaB);
};

struct FresnelSpecular : BxDF {
    static sptr<FresnelSpecular> create(const v3f &R, const v3f &T, float etaA, float etaB);
};
